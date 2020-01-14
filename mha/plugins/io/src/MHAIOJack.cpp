// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2007 2008 2010 2012 2013 2014 2015 2016 2017 HörTech gGmbH
// Copyright © 2018 2020 HörTech gGmbH
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

#include "mha_io_ifc.h"
#include "mha_toolbox.h"
#include "mha_signal.hh"
#include "mha_events.h"
#include <jack/jack.h>
#include <math.h>
#include "mhajack.h"
#include <unistd.h>

/** 
    \brief JACK IO
*/
namespace MHAIOJack {

    /** 
        \brief Main class for JACK IO
        
        This class registers a JACK client. JACK and framework states are
        managed by this class.
        
    */
    class io_jack_t : public MHAParser::parser_t, public MHAJack::client_t
    {
    public:
        io_jack_t(unsigned int fragsize, 
                  float samplerate,
                  IOProcessEvent_t proc_event,
                  void* proc_handle,
                  IOStartedEvent_t start_event,
                  void* start_handle,
                  IOStoppedEvent_t stop_event,
                  void* stop_handle);
        void prepare(int,int);
        void release();
    private:
        void reconnect_inports();
        void reconnect_outports();
        void get_physical_input_ports();
        void get_physical_output_ports();
        void get_all_input_ports();
        void get_all_output_ports();
        void get_delays_in();
        void get_delays_out();
        void read_get_cpu_load();
        void read_get_xruns();
        void read_get_scheduler();
        unsigned int fw_fragsize;
        float fw_samplerate;
        MHAParser::string_t servername;
        MHAParser::string_t clientname;
        MHAParser::vstring_t connections_in;
        MHAParser::vint_mon_t delays_in;
        MHAParser::vstring_t connections_out;
        MHAParser::vint_mon_t delays_out;
        MHAParser::vstring_t portnames_in;
        MHAParser::vstring_t portnames_out;
        MHAParser::vstring_mon_t ports_in_physical;
        MHAParser::vstring_mon_t ports_out_physical;
        MHAParser::vstring_mon_t ports_in_all;
        MHAParser::vstring_mon_t ports_out_all;
        MHAParser::parser_t ports_parser;
        MHAParser::float_mon_t state_cpuload;
        MHAParser::int_mon_t state_xruns;
        MHAParser::int_mon_t state_priority;
        MHAParser::string_mon_t state_scheduler;
        MHAParser::parser_t state_parser;
        MHAEvents::patchbay_t<io_jack_t> patchbay;
    };

}

using namespace MHAIOJack;

void io_jack_t::get_delays_in()
{
    delays_in.data = MHAJack::get_port_capture_latency_int(connections_in.data);
}

void io_jack_t::get_delays_out()
{
    delays_out.data = MHAJack::get_port_playback_latency_int(connections_out.data);
}

void io_jack_t::read_get_cpu_load()
{
    state_cpuload.data = get_cpu_load();
}

void io_jack_t::read_get_xruns()
{
    state_xruns.data = get_xruns();
}

void io_jack_t::read_get_scheduler()
{
#ifdef _WIN32
   state_scheduler.data = "";
   state_priority.data = -1;
#else
    if( jc ){
        int policy;
        struct sched_param priority;
        pthread_getschedparam(jack_client_thread_id(jc), &policy, &priority);
        if (policy == SCHED_RR)
            state_scheduler.data = "SCHED_RR";
        else if (policy == SCHED_FIFO)
            state_scheduler.data = "SCHED_FIFO";
        else state_scheduler.data = "SCHED_OTHER";
        state_priority.data = priority.sched_priority;
    }else{
        state_scheduler.data = "";
        state_priority.data = -1;
    }
#endif //_WIN32
}

void io_jack_t::get_physical_input_ports()
{
    get_ports(ports_in_physical.data,JackPortIsInput|JackPortIsPhysical);
}

void io_jack_t::get_physical_output_ports()
{
    get_ports(ports_out_physical.data,JackPortIsOutput|JackPortIsPhysical);
}

void io_jack_t::get_all_input_ports()
{
    get_ports(ports_in_all.data,JackPortIsInput);
}

void io_jack_t::get_all_output_ports()
{
    get_ports(ports_out_all.data,JackPortIsOutput);
}

/** 
    \brief Connect the input ports when connection variable is accessed
  
*/
void io_jack_t::reconnect_inports()
{
    connect_input(connections_in.data);
}

/** 
    \brief Connect the output ports when connection variable is accessed
  
*/
void io_jack_t::reconnect_outports()
{
    connect_output(connections_out.data);
}

/** 
    \brief Allocate buffers, activate JACK client and install internal ports
  
*/
void io_jack_t::prepare(int nch_in,int nch_out)
{
    set_input_portnames(portnames_in.data);
    set_output_portnames(portnames_out.data);
    MHAJack::client_t::prepare(servername.data,clientname.data,nch_in,nch_out);
    try{
        portnames_in.setlock(true);
        portnames_out.setlock(true);
        portnames_in.data = get_my_input_ports();
        portnames_out.data = get_my_output_ports();
        clientname.setlock(true);
        servername.setlock(true);
        if( fw_samplerate != get_srate() )
            throw MHA_Error(__FILE__,__LINE__,
                            "Mismatching sample rate:\n"
                            "JACK is running with %g kHz, the framework requires %g kHz.",
                            get_srate()/1000.0, fw_samplerate/1000.0);
        if( fw_fragsize != get_fragsize() )
            throw MHA_Error(__FILE__,__LINE__,
                            "Mismatching buffer size:\n"
                            "JACK has %u, MHA needs %u.",
                            get_fragsize(), fw_fragsize );
        reconnect_inports();
        reconnect_outports();
    }
    catch(...){
        release();
        throw;
    }
}

void io_jack_t::release()
{
    portnames_in.setlock(false);
    portnames_out.setlock(false);
    clientname.setlock(false);
    MHAJack::client_t::release();
}

io_jack_t::io_jack_t(unsigned int ifragsize, 
                     float isamplerate,
                     IOProcessEvent_t iproc_event,
                     void* iproc_handle,
                     IOStartedEvent_t istart_event,
                     void* istart_handle,
                     IOStoppedEvent_t istop_event,
                     void* istop_handle)
    : MHAParser::parser_t("JACK client"),
      MHAJack::client_t(iproc_event,
                        iproc_handle,
                        istart_event,
                        istart_handle,
                        istop_event,
                        istop_handle),
      fw_fragsize(ifragsize),
      fw_samplerate(isamplerate),
      servername("Name of JACK server",
                 (mha_getenv("JACK_DEFAULT_SERVER").length()
                  ? mha_getenv("JACK_DEFAULT_SERVER")
                  : std::string("default"))),
      clientname("Name of JACK client","MHA"),
      connections_in("Connections for input ports","[]"),
      delays_in("Input delay in samples as reported by JACK"),
      connections_out("Connections for output ports","[]"),
      delays_out("Output delay in samples as reported by JACK"),
      portnames_in("Names of input ports (empty for automatic names)","[]"),
      portnames_out("Names of output ports (empty for automatic names)","[]"),
      ports_in_physical("Physical (hardware) input ports"),
      ports_out_physical("Physical (hardware) output ports"),
      ports_in_all("All input ports (software/hardware)"),
      ports_out_all("All output ports (software/hardware)"),
      ports_parser("Jack ports"),
      state_cpuload("Current CPU load in Jack."),
      state_xruns("Number of xruns since first connection of MHA to jack."),
      state_priority("Jack thread priority."),
      state_scheduler("Jack thread scheduler model."),
      state_parser("Jack state.")
{
    set_node_id("MHAIOJack");
    insert_member(servername);
    insert_item("name",&clientname);
    insert_item("con_in",&connections_in);
    insert_member(delays_in);
    insert_item("con_out",&connections_out);
    insert_member(delays_out);
    insert_item("names_in",&portnames_in);
    insert_item("names_out",&portnames_out);
    insert_item("ports",&ports_parser);
    ports_parser.insert_item("physical_inputs",&ports_in_physical);
    ports_parser.insert_item("physical_outputs",&ports_out_physical);
    ports_parser.insert_item("all_inputs",&ports_in_all);
    ports_parser.insert_item("all_outputs",&ports_out_all);
    insert_item("state",&state_parser);
    state_parser.insert_item("xruns",&state_xruns);
    state_parser.insert_item("cpuload",&state_cpuload);
    state_parser.insert_item("priority",&state_priority);
    state_parser.insert_item("scheduler",&state_scheduler);
    patchbay.connect(&connections_in.writeaccess,this,&io_jack_t::reconnect_inports);
    patchbay.connect(&connections_out.writeaccess,this,&io_jack_t::reconnect_outports);
    // connect jack get_port events:
    patchbay.connect(&ports_in_physical.prereadaccess,this,&io_jack_t::get_physical_input_ports);
    patchbay.connect(&ports_out_physical.prereadaccess,this,&io_jack_t::get_physical_output_ports);
    patchbay.connect(&ports_in_all.prereadaccess,this,&io_jack_t::get_all_input_ports);
    patchbay.connect(&ports_out_all.prereadaccess,this,&io_jack_t::get_all_output_ports);
    patchbay.connect(&state_xruns.prereadaccess,this,&io_jack_t::read_get_xruns);
    patchbay.connect(&state_cpuload.prereadaccess,this,&io_jack_t::read_get_cpu_load);
    patchbay.connect(&state_priority.prereadaccess,this,&io_jack_t::read_get_scheduler);
    patchbay.connect(&state_scheduler.prereadaccess,this,&io_jack_t::read_get_scheduler);
    patchbay.connect(&delays_in.prereadaccess,this,&io_jack_t::get_delays_in);
    patchbay.connect(&delays_out.prereadaccess,this,&io_jack_t::get_delays_out);
}

#define ERR_SUCCESS 0
#define ERR_IHANDLE -1
#define ERR_USER -1000

#define MAX_USER_ERR 0x500
static char user_err_msg[MAX_USER_ERR] = "";

extern "C" {
#ifdef MHA_STATIC_PLUGINS
#define IOInit               MHA_STATIC_MHAIOJack_IOInit
#define IOPrepare            MHA_STATIC_MHAIOJack_IOPrepare
#define IOStart              MHA_STATIC_MHAIOJack_IOStart
#define IOStop               MHA_STATIC_MHAIOJack_IOStop
#define IORelease            MHA_STATIC_MHAIOJack_IORelease
#define IOSetVar             MHA_STATIC_MHAIOJack_IOSetVar
#define IOStrError           MHA_STATIC_MHAIOJack_IOStrError
#define IODestroy            MHA_STATIC_MHAIOJack_IODestroy
#define dummy_interface_test MHA_STATIC_MHAIOJack_dummy_interface_test
#else
#define IOInit               MHA_DYNAMIC_IOInit
#define IOPrepare            MHA_DYNAMIC_IOPrepare
#define IOStart              MHA_DYNAMIC_IOStart
#define IOStop               MHA_DYNAMIC_IOStop
#define IORelease            MHA_DYNAMIC_IORelease
#define IOSetVar             MHA_DYNAMIC_IOSetVar
#define IOStrError           MHA_DYNAMIC_IOStrError
#define IODestroy            MHA_DYNAMIC_IODestroy
#define dummy_interface_test MHA_DYNAMIC_dummy_interface_test
#endif  
    int IOInit(int fragsize,
               float samplerate,
               IOProcessEvent_t proc_event,
               void* proc_handle,
               IOStartedEvent_t start_event,
               void* start_handle,
               IOStoppedEvent_t stop_event,
               void* stop_handle,
               void** handle)
    {
        if( !handle )
            return ERR_IHANDLE;
        try{
            io_jack_t* cl = new io_jack_t(fragsize,samplerate,proc_event,proc_handle,start_event,start_handle,stop_event,stop_handle);
            *handle = (void*)cl;
            return ERR_SUCCESS;
        }
        catch( MHA_Error& e ){
            strncpy( user_err_msg, Getmsg(e), MAX_USER_ERR-1 );
            user_err_msg[MAX_USER_ERR-1] = 0;
            return ERR_USER;
        }
    }

    int IOPrepare(void* handle,int nch_in,int nch_out){
        if( !handle )
            return ERR_IHANDLE;
        try{
            ((io_jack_t*)handle)->prepare(nch_in,nch_out);
            return ERR_SUCCESS;
        }
        catch( MHA_Error& e ){
            strncpy( user_err_msg, Getmsg(e), MAX_USER_ERR-1 );
            user_err_msg[MAX_USER_ERR-1] = 0;
            return ERR_USER;
        }
    }

    int IOStart(void* handle){
        if( !handle )
            return ERR_IHANDLE;
        try{
            ((io_jack_t*)handle)->start();
            return ERR_SUCCESS;
        }
        catch( MHA_Error& e ){
            strncpy( user_err_msg, Getmsg(e), MAX_USER_ERR-1 );
            user_err_msg[MAX_USER_ERR-1] = 0;
            return ERR_USER;
        }
    }

    int IOStop(void* handle){
        if( !handle )
            return ERR_IHANDLE;
        try{
            ((io_jack_t*)handle)->stop();
            return ERR_SUCCESS;
        }
        catch( MHA_Error& e ){
            strncpy( user_err_msg, Getmsg(e), MAX_USER_ERR-1 );
            user_err_msg[MAX_USER_ERR-1] = 0;
            return ERR_USER;
        }
    }

    int IORelease(void* handle){
        if( !handle )
            return ERR_IHANDLE;
        try{
            ((io_jack_t*)handle)->release();
            return ERR_SUCCESS;
        }
        catch( MHA_Error& e ){
            strncpy( user_err_msg, Getmsg(e), MAX_USER_ERR-1 );
            user_err_msg[MAX_USER_ERR-1] = 0;
            return ERR_USER;
        }
    }

    int IOSetVar(void* handle,const char *command,char *retval,unsigned int maxretlen)
    {
        if( !handle )
            return ERR_IHANDLE;
        try{
            ((io_jack_t*)handle)->parse(command,retval,maxretlen);
            return ERR_SUCCESS;
        }
        catch(MHA_Error& e){
            strncpy( user_err_msg, Getmsg(e), MAX_USER_ERR-1 );
            user_err_msg[MAX_USER_ERR-1] = 0;
            return ERR_USER;
        }
    }

    const char* IOStrError(void* handle,int err)
    {
        switch( err ){
        case ERR_SUCCESS :
            return "Success";
        case ERR_IHANDLE :
            return "Invalid handle.";
        case IO_ERROR_JACK:
        case IO_ERROR_MHAJACKLIB:
            return last_jack_err_msg;
        case ERR_USER :
            return user_err_msg;
        default :
            return "Unknown error.";
        }
    }

    void IODestroy(void* handle)
    {
        if( !handle )
            return;
        delete (io_jack_t*)handle;
    }

    void dummy_interface_test(void){
#ifdef MHA_STATIC_PLUGINS
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOJack_,IOInit);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOJack_,IOPrepare);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOJack_,IOStart);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOJack_,IOStop);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOJack_,IORelease);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOJack_,IOSetVar);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOJack_,IOStrError);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOJack_,IODestroy);
#else
        MHA_CALLBACK_TEST(IOInit);
        MHA_CALLBACK_TEST(IOPrepare);
        MHA_CALLBACK_TEST(IOStart);
        MHA_CALLBACK_TEST(IOStop);
        MHA_CALLBACK_TEST(IORelease);
        MHA_CALLBACK_TEST(IOSetVar);
        MHA_CALLBACK_TEST(IOStrError);
        MHA_CALLBACK_TEST(IODestroy);
#endif
    }
}
/*
 * Local Variables:
 * compile-command: "make -C .."
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * coding: utf-8-unix
 * End:
 */
