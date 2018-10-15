// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2013 2014 2015 2016 2018 HörTech gGmbH
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

#include <lsl_cpp.h>
#include <complex>
#include <memory>
#include <mutex>
#include <pthread.h>
#include <sched.h>
#include "mha_algo_comm.h"
#include "mha_fifo.h"
#include "mha_plugin.hh"
#include "mha_os.h"
#include "mha_events.h"
#include "mha_defs.h"

/** All types for the ac2lsl plugins live in this namespace. */
namespace ac2lsl{
    /** Interface for ac to lsl bridge variable*/
    class save_var_base_t{
    public:
        virtual void send_frame()=0;
        virtual ~save_var_base_t()=default;
    };

    /** Implementation for all ac to lsl bridges except complex types. */
    template<typename T>
    class save_var_t : public save_var_base_t {
    public:
        /** C'tor of generic ac to lsl bridge.
         * @param name Name of variable as seen in lsl
         * @param type Type of variable as seen in lsl
         * @param data Pointer to data buffer of the ac variable
         * @param num_entries Number of entries if the ac variable is a vector.
                              Should be set to one if not a vector.
         */
        save_var_t(const std::string& name, const std::string& type, void* data,
                   size_t num_entries):
            stream(lsl::stream_info(name,type,num_entries)),
            buf(static_cast<T*>(data))
        {};
        virtual ~save_var_t()=default;
        /** Send a frame to lsl. */
        virtual void send_frame(){stream.push_sample(buf);};
    private:
        /** LSL stream outlet. Interface to lsl */
        lsl::stream_outlet stream;
        /** Pointer to data buffer of the ac variable. */
        T* buf;
    };


    /** Template specialization of the ac2lsl bridge to take care of complex
     * numbers. This specialization is needed because lsl does not support
     * complex numbers.
     * Order is [re(0), im(0), re(1), im(1), ....]
     */
    template<>
    class save_var_t<mha_complex_t> : public save_var_base_t {
    public:
        /** C'tor of specialization for complex types.
         * See generic c'tor for details. */
        save_var_t(const std::string& name,
                   const std::string& type,
                   void* data ,
                   size_t _num_entries):
            stream(lsl::stream_info(name,type,2*_num_entries)),
            buf(static_cast<mha_complex_t*>(data)),
            num_entries(_num_entries)
        {vec.resize(2*num_entries);};
        virtual ~save_var_t()=default;
        /** Send a frame of complex types.
         * Reorders real and imaginary parts into one vector. */
        virtual void send_frame(){
            for(size_t ii=0;ii<num_entries;ii+=2){
                vec[ii]=buf[ii].re;
                vec[ii+1]=buf[ii].im;
            }
            stream.push_sample(vec);};
    private:
        /** LSL stream outlet. Interface to lsl */
        lsl::stream_outlet stream;
        /** Vector that gets sent over to the lsl instead of the actual
            complex type*/
        std::vector<mha_real_t> vec;
        /** Pointer to data buffer of the ac variable. */
        mha_complex_t* buf;
        /** Number of entries in the vector */
        size_t num_entries;
    };

    /** Runtime configuration class of the ac2lsl plugin */
    class cfg_t {
        /** Pointer to vector of unique ptr's of ac to lsl bridges.
         * Raw pointer because we do not take ownership */
        std::vector<std::unique_ptr<save_var_base_t>>* varlist;
        /** Counter of frames to skip */
        unsigned skipcnt;
        /** Number of frames to skip after each send */
        unsigned skip;
        /** Activate/Deactivate sending to lsl */
        bool activate;
    public:

        /** C'tor of ac2lsl run time configuration
         * @param activate_ Activate/Deactivate sending
         * @param skipcnt_ Number of frames to skip after each send
         * @param varlist_ Pointer to vector of unique ptr's of ac to
         * lsl bridges.
         */
        cfg_t(bool activate_, unsigned skipcnt_,
              std::vector<std::unique_ptr<save_var_base_t>>* varlist_);
        void process();

    };

    /** Plugin class of ac2lsl */
    class ac2lsl_t : public MHAPlugin::plugin_t<cfg_t>
    {
    public:
        ac2lsl_t(algo_comm_t iac,const char* chain, const char* algo);
        /** Prepare constructs the vector of bridge variables and locks
         * the configuration, then calls update(). */
        void prepare(mhaconfig_t&);
        /** Processing fct for waveforms. Calls process(void). */
        mha_wave_t* process(mha_wave_t* s) {process();return s;};
        /** Processing fct for spectra. Calls process(void). */
        mha_spec_t* process(mha_spec_t* s) {process();return s;};
        /** Process function. Checks once if the plugin is run in a
         * real-time thread and throws if rt_strict is true,
         * then forwards to cfg_t::process(). */
        void process();
        /** Release fct. Unlocks variable name list */
        void release();
    private:
        /** Construct new runtime configuration */
        void update();
        MHAParser::vstring_t vars;
        MHAParser::bool_t rt_strict;
        MHAParser::bool_t activate;
        MHAParser::int_t skip;
        MHAEvents::patchbay_t<ac2lsl_t> patchbay;
        std::vector<std::unique_ptr<save_var_base_t>> varlist;
        std::once_flag rt_strict_flag;
    };
}

ac2lsl::ac2lsl_t::ac2lsl_t(algo_comm_t iac,const char* chain, const char* algo)
    : MHAPlugin::plugin_t<ac2lsl::cfg_t>("Send AC variables as"
                                         " LSL messages.",iac),
    vars("List of AC variables to be saved, empty for all.","[]"),
    rt_strict("abort if used in real-time thread?","yes"),
    activate("send frames to network?","yes"),
    skip("Number of frames to skip after sending","0","[0,]")
    {
        insert_member(vars);
        insert_member(rt_strict);
        insert_member(activate);
        insert_member(skip);
        patchbay.connect(&activate.writeaccess,this,&ac2lsl_t::update);
        patchbay.connect(&rt_strict.writeaccess,this,&ac2lsl_t::update);
        patchbay.connect(&skip.writeaccess,this,&ac2lsl_t::update);
        patchbay.connect(&vars.writeaccess,this,&ac2lsl_t::update);
    }

void ac2lsl::ac2lsl_t::prepare(mhaconfig_t& cf)
    {
        auto varnames=vars.data;

        //No variable names were given in the configuration,
        //meaning we have to scan the whole ac space
        if( !varnames.size() ){
            int get_entries_error_code;
            unsigned int cstr_len = 512;
            std::string entr;
            //error code -3 means buffer too short to save
            //list of ac variables, so we double the buffer size
            //try again until we succeed, aborting when we reach
            //1MiB. After that, we add brackets and tokenize the
            //space separated list, getting a vector of strings
            do {
                cstr_len <<= 1;
                if (cstr_len > 0x100000)
                    throw MHA_ErrorMsg("list of all ac variables is longer than"
                                       " 1MiB. You should select a subset"
                                       " using vars.");
                char* temp_cstr;
                temp_cstr = new char[cstr_len];
                temp_cstr[0] = 0;
                get_entries_error_code =
                    ac.get_entries(ac.handle, temp_cstr, cstr_len);
                entr = temp_cstr;
                delete [] temp_cstr; temp_cstr = 0;
            } while (get_entries_error_code == -3);
            if (get_entries_error_code == -1)
                throw MHA_ErrorMsg("Bug: ac handle used is invalid");
            entr = std::string("[") + entr + std::string("]");
            std::vector<std::string> entrl;
            MHAParser::StrCnv::str2val(entr,entrl);
            varnames = entrl;
        }
        varlist.clear();
        for(const auto& name : varnames){
            comm_var_t v;
            std::string data_type;
            if( ac.get_var(ac.handle,name.c_str(),&v) )
                throw MHA_Error(__FILE__,__LINE__,
                                "No such variable: \"%s\"",name.c_str());
            switch( v.data_type ){
            case MHA_AC_INT :
                varlist.push_back(std::make_unique<save_var_t<int>>
                                  (name,"MHA_AC_INT",v.data,
                                   v.num_entries));
                break;
            case MHA_AC_FLOAT :
                varlist.push_back(std::make_unique<save_var_t<float>>
                                  (name,"MHA_AC_FLOAT",v.data,
                                   v.num_entries));
                break;
            case MHA_AC_DOUBLE :
                varlist.push_back(std::make_unique<save_var_t<double>>
                                  (name,"MHA_AC_DOUBLE",v.data,
                                   v.num_entries));
                break;
            case MHA_AC_MHAREAL :
                varlist.push_back(std::make_unique<save_var_t<mha_real_t>>
                                  (name,"MHA_AC_MHAREAL",v.data,
                                   v.num_entries));
                break;
            case MHA_AC_MHACOMPLEX :
                varlist.push_back(std::make_unique<save_var_t<mha_complex_t>>
                                  (name,"MHA_AC_MHACOMPLEX",v.data,
                                   v.num_entries));
                break;
            default:
                throw MHA_Error(__FILE__,__LINE__,
                                "Unknown data type: \"%i\"",v.data_type);
            }
        }
        vars.setlock(true);
        update();
    }

void ac2lsl::ac2lsl_t::release()
    {
        vars.setlock(false);
        varlist.clear();
    }

void ac2lsl::ac2lsl_t::process()
{
    std::call_once(rt_strict_flag,[&](){if(rt_strict.data){
                pthread_t this_thread=pthread_self();
                int policy=0;
                struct sched_param params;
                auto ret=pthread_getschedparam(this_thread,&policy,&params);
                if(ret != 0)
                    throw MHA_Error(__FILE__,__LINE__,"could not retrieve"
                                    " thread scheduling parameters!");
                if(policy == SCHED_FIFO or policy==SCHED_RR)
                    throw MHA_Error(__FILE__,__LINE__,"ac2lsl used in"
                                    " real-time thread with"
                                    " rt-strict=true!");
            }});
    poll_config();
    cfg->process();
}

void ac2lsl::ac2lsl_t::update(){
    if(is_prepared()){
        auto c=new cfg_t(activate.data,skip.data, &varlist);
        push_config(c);
    }
}

ac2lsl::cfg_t::cfg_t(bool activate_, unsigned skip_,
                     std::vector<std::unique_ptr<save_var_base_t>>* varlist_):
    varlist(varlist_),
    skipcnt(skip_),
    skip(skip_),
    activate(activate_){}

void ac2lsl::cfg_t::process(){
    if(activate){
        if(!skipcnt){
            for(auto& var : *varlist)
                var->send_frame();
            skipcnt=skip;
        }
        else{
            skipcnt--;
        }
    }
}
MHAPLUGIN_CALLBACKS(ac2lsl,ac2lsl::ac2lsl_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(ac2lsl,ac2lsl::ac2lsl_t,spec,spec)
MHAPLUGIN_DOCUMENTATION(ac2lsl,"AC-variables","This plugin provides a mechanism"
                        " to send ac variables over the network using the lab"
                        " streaming layer (lsl). Currently no user-defined"
                        " types are supported.")

/*
 * Local variables:
 * c-basic-offset: 4
 * compile-command: "make"
 * End:
 */
