// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2011 2012 2013 2014 2015 2018 2019 2020 HörTech gGmbH
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

#include <lo/lo.h>
#include <pthread.h>
#include <sched.h>

#include "mha_algo_comm.h"
#include "mha_fifo.h"
#include "mha_plugin.hh"
#include "mha_os.h"
#include "mha_events.h"
#include "mha_defs.h"
/** Plugin class of the ac2osc plugin. */
class ac2osc_t : public MHAPlugin::plugin_t<int>
{
public:
    /** C'tor of plugin class. */
    ac2osc_t(algo_comm_t iac,const char* chain, const char* algo);
    void prepare(mhaconfig_t&);
    /** Processing fct for waveforms. Calls process(void). */
    mha_wave_t* process(mha_wave_t* s) {process();return s;};
    /** Processing fct for spectra. Calls process(void). */
    mha_spec_t* process(mha_spec_t* s) {process();return s;};
    /** Process function. Checks once if the plugin is run in a
     * real-time thread and throws if rt_strict is true,
     * sends osc messages according to config. */
    void process();
    /** Release frees osc related memory, does cleanup*/
    void release();
private:
    void send_osc_float();
    /** Start/Stop sending of messages */
    void update_mode();
    /** OSC server host name */
    MHAParser::string_t host;
    /** OSC server port */
    MHAParser::string_t port;
    /** Time-to-live of UDP packages */
    MHAParser::int_t ttl;
    /** List of AC variables to be saved, empty for all */
    MHAParser::vstring_t vars;
    /** Record mode */
    MHAParser::kw_t mode;
    /** number of frames to skip after sending */
    MHAParser::int_t skip;
    /** abort if used in real-time thread? */
    MHAParser::bool_t rt_strict;
    MHA_AC::acspace2matrix_t* acspace;
    MHAEvents::patchbay_t<ac2osc_t> patchbay;
    bool b_record;
    uint8_t* rtmem;
    float framerate;
    int skipcnt;
    lo_address lo_addr;
    bool is_first_run;
};

ac2osc_t::ac2osc_t(algo_comm_t iac,const char* chain, const char* algo)
    : MHAPlugin::plugin_t<int>("Send AC variables as OSC messages over udp.",iac),
    host("OSC server host name","localhost"),
    port("OSC server port","7777"),
    ttl("Time-to-live of UDP packages (1 = subnet)","1"),
    vars("List of AC variables to be saved, empty for all. A colon may be used to specify target address.","[]"),
    mode("record mode","pause","[rec pause]"),
    skip("number of frames to skip after sending","0","[0,]"),
    rt_strict("abort if used in real-time thread?","yes"),
    acspace(nullptr),
    b_record(false),
    rtmem(new uint8_t[0x100000]),
    skipcnt(0),
    is_first_run(true)
{
    insert_member(host);
    insert_member(port);
    insert_member(ttl);
    insert_member(vars);
    insert_member(mode);
    insert_member(skip);
    insert_member(rt_strict);

    patchbay.connect(&(mode.writeaccess),this,&ac2osc_t::update_mode);
}

void ac2osc_t::prepare(mhaconfig_t& cf)
{
    try {
    acspace = new MHA_AC::acspace2matrix_t(ac,vars.data);
    framerate = cf.srate / cf.fragsize;
    lo_addr = lo_address_new( host.data.c_str(), port.data.c_str() );
    lo_address_set_ttl(lo_addr, ttl.data );
    lo_send( lo_addr, "/mhastate","s","prepared");
    host.setlock(true);
    port.setlock(true);
    ttl.setlock(true);
    vars.setlock(true);
    mode.setlock(true);
    rt_strict.setlock(true);
    }
    catch(MHA_Error& e){
        host.setlock(false);
        port.setlock(false);
        ttl.setlock(false);
        vars.setlock(false);
        mode.setlock(false);
        rt_strict.setlock(false);
        lo_send( lo_addr, "/mhastate","s","unprepared");
        lo_address_free( lo_addr );
        delete acspace;
        throw e;
    }
}

void ac2osc_t::release()
{
    is_first_run=true;
    lo_send( lo_addr, "/mhastate","s","released");
    lo_address_free( lo_addr );
    delete acspace;
    host.setlock(false);
    port.setlock(false);
    ttl.setlock(false);
    vars.setlock(false);
    mode.setlock(false);
    rt_strict.setlock(false);
}

void ac2osc_t::process()
{
    if(is_first_run){
        if(rt_strict.data){
            is_first_run=false;
            pthread_t this_thread=pthread_self();
            int policy=0;
            struct sched_param params;
            auto ret=pthread_getschedparam(this_thread,&policy,&params);
            if(ret != 0)
                throw MHA_Error(__FILE__,__LINE__,"could not retrieve"
                                " thread scheduling parameters!");
            if(policy == SCHED_FIFO or policy==SCHED_RR)
                throw MHA_Error(__FILE__,__LINE__,"ac2osc used in"
                                " real-time thread with"
                                " rt-strict=true!");
        }
    }
    acspace->update();
    if( b_record ){
        if( !skipcnt ){
            send_osc_float();
            skipcnt = skip.data;
        }else{
            skipcnt--;
        }
    }
}

void ac2osc_t::send_osc_float()
{
    for(unsigned int mat=0;mat < acspace->size(); mat++){
        lo_message msg(lo_message_new());
        unsigned int N((*acspace)[mat].get_nelements());
        const float* data((*acspace)[mat].get_rdata());
        for(unsigned int k=0;k<N;k++)
            lo_message_add_float(msg,data[k]);
        lo_send_message(lo_addr,(*acspace)[mat].getusername().c_str(),msg);
        lo_message_free(msg);
    }
}

void ac2osc_t::update_mode()
{
    switch( mode.data.get_index() ){
    case 0 :
        b_record = true;
        break;
    case 1 :
        b_record = false;
        break;
    default:
        throw MHA_Error(__FILE__,__LINE__,"Unhandled mode %zu.",mode.data.get_index());
    };
}

MHAPLUGIN_CALLBACKS(ac2osc,ac2osc_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(ac2osc,ac2osc_t,spec,spec)
MHAPLUGIN_DOCUMENTATION\
(ac2osc,
 "data-export network-communication open-sound-control",
 "Send AC variables as OSC"
 "variables using the UDP transport layer. The variable"
 " \"vars\" can be used to select variables from the AC"
 " space for sending. "
 " The sending of variables can be verified using the"
 " open source tool \"dump\\_osc\". When selecting an AC"
 " variable, a target path can be specified using the"
 " colon delimiter, e.g.:"
 "\\begin{verbatim}vars = [level:/mhalevels]"
 " \\end{verbatim}")

/*
 * Local variables:
 * c-basic-offset: 4
 * compile-command: "make"
 * End:
 */
