// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2020 HörTech gGmbH
//
// openMHA is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// openMHA is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License, version 3 for more details.
//
// You should have received a copy of the GNU Affero General Public License, 
// version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

#include "mha_algo_comm.h"
#include "mha_fifo.h"
#include "mha_plugin.hh"
#include "mha_os.h"
#include "mha_events.h"
#include "mha_defs.h"

#include "lsl_cpp.h"

#include <memory>
#include <map>
#include <string>
#include <exception>
#include <vector>

/** All types for the lsl2ac plugins live in this namespace. */
namespace lsl2ac{
enum class underrun_behavior { Zero=0, Copy, Abort};

/** LSL to AC bridge variable */
class save_var_t {
public:
    /** C'tor of lsl to ac bridge.
     * @param name_ Name of LSL stream to be received
     * @param info_ LSL stream info object containing metadata
     * @param ac_ Handle to ac space
     * @param ub_ Underrun behavior. 0=Zero out, 1=Copy, 2=Abort
     */
    // This is a function try block. Really ugly syntax but the only way to handle exceptions
    // in the ctor initializer list. See http://www.gotw.ca/gotw/066.htm
    save_var_t(const std::string& name_, const lsl::stream_info& info_, const algo_comm_t& ac_, underrun_behavior ub_) try :
        stream(info_),
            buf(new mha_real_t[info_.channel_count()]),
            ac(ac_),
            ub(ub_),
            name(name_)
            {
                stream.open_stream();
                //channel_count() = LSL speak for number of samples received per pull_sample
                cv.num_entries=info_.channel_count();
                cv.stride=1;
                cv.data_type=MHA_AC_FLOAT;
                cv.data=buf.get();
                ac.insert_var(ac.handle,info_.name().c_str(),cv);
            }
    catch (MHA_Error& e) {
        //The framework can handle MHA_Errors. Just re-throw
        throw;
    }
    catch (std::exception & e) {
        // LSL does not document its exceptions very well, but all exceptions I encountered inherit from std::exception
        // Rethrow as MHA_Error
        throw MHA_Error(__FILE__,__LINE__,"Could not subscribe to stream %s: %s",info_.name().c_str(),e.what());
    }
    catch (...) {
        //No matter what happened before, throw an MHA_Error so that the framework can handle that
        throw MHA_Error(__FILE__,__LINE__,"Could not subscribe to stream %s: Unknown error",info_.name().c_str());
    } ;
    /** Get stream info object from stream inlet */
    lsl::stream_info info() {
        return stream.info();
    };
    /** Get number of entries in the stream object*/
    unsigned num_entries() {
        return stream.info().channel_count();
    };

    ~save_var_t()=default;
    /** Receive a sample from lsl and copy to AC space. Handling of underrun is configuration-dependent */
    void receive_frame()  {
        if(skip){
            return;
        }
        else{
            if(ub==underrun_behavior::Zero)
                impl_receive_frame_zero();
            else if(ub==underrun_behavior::Copy)
                impl_receive_frame_copy();
            else if(ub==underrun_behavior::Abort)
                impl_receive_frame_abort();
            else
                throw MHA_Error(__FILE__,__LINE__,"Bug: Unknown underrun behavior: %i!",static_cast<int>(ub));
        }
    };
private:
    /** LSL stream outlet. Interface to lsl */
    lsl::stream_inlet stream;
    /** Pointer to data buffer of the ac variable. */
    std::unique_ptr<mha_real_t[]> buf;
    /** Handle to AC space */
    const algo_comm_t& ac;
    /** AC variable */
    comm_var_t cv;
    /** Should the variable be skipped in future process calls? Only true when error occured. */
    bool skip=false;
    /** Behavior on stream underrun */
    underrun_behavior ub;
    /** Name of stream. Must be saved separately because the stream info might be unrecoverable in error cases */
    const std::string name;
    void impl_receive_frame_zero(){
        try {
            auto ts=stream.pull_sample(buf.get(),num_entries(),0.0f);
            if(ts==0.0){
                for(size_t i=0;i<num_entries();i++)
                    buf[i]=0.0f;
            }
            ac.insert_var(ac.handle,stream.info().name().c_str(),cv);
        }
        //If the stream is recoverable, lsl does not throw, instead tries to recover.
        // Handle any other exception as a permanent underrun
        catch(...){
            for(size_t i=0;i<num_entries();i++)
                buf[i]=0.0f;
            stream.close_stream();
            skip=true;
        }
    }

    void impl_receive_frame_copy(){
        try {
            //If no new samples are there, buf does not get touched
            stream.pull_sample(buf.get(),num_entries(),0.0f);
            ac.insert_var(ac.handle,name.c_str(),cv);
        }
        //If the stream is recoverable, lsl does not throw, instead tries to recover.
        // Handle any other exception as a permanent underrun
        catch(...){
            stream.close_stream();
            skip=true;
        }
    }
    void impl_receive_frame_abort(){
        class UnderrunError{};
        try {
            auto ts=stream.pull_sample(buf.get(),num_entries(),0.0f);
            if(ts==0.0){
                throw UnderrunError();
            }
        }
        catch(UnderrunError &e){
            throw MHA_Error(__FILE__,__LINE__,"Stream: %s has no new samples!",name.c_str());
        }
        catch(std::exception &e){
            throw MHA_Error(__FILE__,__LINE__,"Error in stream %s: %s",name.c_str(),e.what());
        }
        catch(...){
            throw MHA_Error(__FILE__,__LINE__,"Unknown error in stream %s!",name.c_str());
        }
    }
};

/** Runtime configuration class of the lsl2ac plugin */
class cfg_t {
    /** Maps variable name to unique ptr's of lsl to ac bridges. */
    std::map<std::string, std::unique_ptr<save_var_t>> varlist;
public:

    /** C'tor of lsl2ac run time configuration
     * @param ac_         AC space, data from LSL will be inserted as AC variables
     * @param streamnames_   Names of LSL streams to be subscribed to
     */
    cfg_t(const algo_comm_t& ac_, underrun_behavior underrun_, const std::vector<std::string>& streamnames_);
    void process();

};

/** Plugin class of lsl2ac */
class lsl2ac_t : public MHAPlugin::plugin_t<cfg_t>
{
public:
    lsl2ac_t(algo_comm_t iac,const char* chain, const char* algo);
    /** Prepare constructs the vector of bridge variables and locks
     * the configuration, then calls update(). */
    void prepare(mhaconfig_t&);
    /** Processing fct for waveforms. Calls process(void). */
    mha_wave_t* process(mha_wave_t* s) {process();return s;};
    /** Processing fct for spectra. Calls process(void). */
    mha_spec_t* process(mha_spec_t* s) {process();return s;};
    /** Process function. */
    void process();
    /** Release fct. Unlocks variable name list */
    void release();
private:
    /** Retrieves all stream names from LSL and fills them into
        available_streams. 
     */
    void get_all_stream_names();
    /** Construct new runtime configuration */
    void update();
    MHAParser::vstring_t streams;
    MHAParser::bool_t activate;
    MHAParser::kw_t underrun_behavior;
    MHAEvents::patchbay_t<lsl2ac_t> patchbay;
    /** Monitor variable containing all available streams. */
    MHAParser::vstring_mon_t available_streams;
};
}

lsl2ac::lsl2ac_t::lsl2ac_t(algo_comm_t iac,const char* chain, const char* algo)
    : MHAPlugin::plugin_t<lsl2ac::cfg_t>("Receive LSL streams and copy"
                                         " them to AC variables.",iac),
      streams("List of LSL streams to be saved, empty for all.","[]"),
      activate("Receive from network?","yes"),
      underrun_behavior("How to handle underrun","zero","[zero copy abort]"),
      available_streams("List of all available LSL streams")
{
    //Nota bene: The configuration variables are not connected to the patch bay because
    // we either lock them anyway during prepare or they are not used within the context
    // of the cfg_t class.
    insert_member(streams);
    insert_member(activate);
    insert_member(underrun_behavior);
    insert_item("available_streams",&available_streams);
    patchbay.connect(&available_streams.prereadaccess,this,&lsl2ac_t::get_all_stream_names);
}

void lsl2ac::lsl2ac_t::prepare(mhaconfig_t& cf)
{
    try {
        underrun_behavior.setlock(true);
        streams.setlock(true);
        update();
    }
    catch(MHA_Error& e){
        underrun_behavior.setlock(false);
        streams.setlock(false);
        throw;
    }
}

void lsl2ac::lsl2ac_t::release()
{
    underrun_behavior.setlock(false);
    streams.setlock(false);
}

void lsl2ac::lsl2ac_t::process()
{
    poll_config();
    if(activate.data)
        cfg->process();
}

void lsl2ac::lsl2ac_t::update(){
    if(is_prepared()){
        auto c=new cfg_t(ac, static_cast<lsl2ac::underrun_behavior>(underrun_behavior.data.get_index()), streams.data);
        push_config(c);
    }
}

void lsl2ac::lsl2ac_t::get_all_stream_names()
{
    available_streams.data.clear();
    std::vector<lsl::stream_info> streams = lsl::resolve_streams();
    available_streams.data.resize(streams.size());
    for(const auto& istream : streams)
        available_streams.data.push_back(istream.name());
}

lsl2ac::cfg_t::cfg_t(const algo_comm_t& ac_, underrun_behavior ub_, const std::vector<std::string>& streamnames_)
{
    for(auto& name : streamnames_) {
        //Find all streams with matching name and take the first one, throw if none found
        auto matching_streams=lsl::resolve_stream("name",name,/*minimum=*/1,/*timeout=*/1.0);
        if(!matching_streams.size())
            throw MHA_Error(__FILE__,__LINE__,"No stream with name %s found!",name.c_str());
        varlist.insert({name,std::make_unique<save_var_t>(name,matching_streams[0],ac_,ub_)});
    }
}

void lsl2ac::cfg_t::process(){
    for(auto& var : varlist){
        var.second->receive_frame();
    }
}

MHAPLUGIN_CALLBACKS(lsl2ac,lsl2ac::lsl2ac_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(lsl2ac,lsl2ac::lsl2ac_t,spec,spec)
MHAPLUGIN_DOCUMENTATION\
(lsl2ac,
 "data-import network-communication",
 " This plugin provides a mechanism to receive lsl streams and"
 " convert them to ac variables."
 " It currently only supports MHA\\_AC\\_FLOAT-type variables. Type conversions from other"
 " types of stream should be handled in the background."
 "\n"
 "This is a very early version of the plugin and lacks many features."
 " It is probably not real-time safe. Only the latest"
 " chunk of data will be pulled from lsl's internal buffers and stored"
 " as ac variable."
 " If there is no new data, the ac variables get, depending on the configuration variable underrun\\_behavior,"
 " either zero'ed, copied from the last pull, or the plugin will abort."
 " This implementation should only be used for evaluation purposes and"
 " is not intended for production use.")
/*
 * Local variables:
 * c-basic-offset: 4
 * compile-command: "make"
 * End:
 */
