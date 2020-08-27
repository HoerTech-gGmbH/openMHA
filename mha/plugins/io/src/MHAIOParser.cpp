// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2008 2009 2013 2014 2015 2016 2017 2020 HörTech gGmbH
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
#include <math.h>


/** 
    \brief Main class for Parser IO

*/
class io_parser_t : public MHAParser::parser_t
{
public:
    io_parser_t(unsigned int fragsize, 
                IOProcessEvent_t proc_event,
                void* proc_handle,
                IOStartedEvent_t start_event,
                void* start_handle,
                IOStoppedEvent_t stop_event,
                void* stop_handle);
    ~io_parser_t();
    void prepare(int,int);
    void start();
    void stop();
    void release();
private:
    void stopped(int,int);
    void started();
    void process_frame();
    unsigned int fragsize;
    unsigned int nchannels_in;
    unsigned int nchannels_out;
    IOProcessEvent_t proc_event;
    void* proc_handle;
    IOStartedEvent_t start_event;
    void* start_handle;
    IOStoppedEvent_t stop_event;
    void* stop_handle;
    MHAParser::mfloat_t input;
    MHAParser::mfloat_mon_t output;
    MHASignal::waveform_t* s_in;
    mha_wave_t* s_out;
    bool b_fw_started;
    bool b_stopped;
    bool b_prepared;
    bool b_starting;
    MHAEvents::patchbay_t<io_parser_t> patchbay;
};

/** 
    \brief Remove JACK client and deallocate internal ports and buffers
  
*/
void io_parser_t::release()
{
    delete s_in;
    b_prepared = false;
}

/** 
    \brief Allocate buffers, activate JACK client and install internal ports
  
*/
void io_parser_t::prepare(int nch_in,int nch_out)
{
    if( nch_in < 1 )
        throw MHA_ErrorMsg("MHAIOParser: At least one input channel is required.");
    nchannels_in = nch_in;
    nchannels_out = nch_out;
    b_fw_started = false;
    b_stopped = false;
    b_prepared = true;
    s_in = new MHASignal::waveform_t(fragsize,nchannels_in);
}

io_parser_t::io_parser_t(unsigned int ifragsize, 
                         IOProcessEvent_t iproc_event,
                         void* iproc_handle,
                         IOStartedEvent_t istart_event,
                         void* istart_handle,
                         IOStoppedEvent_t istop_event,
                         void* istop_handle)
    : MHAParser::parser_t("process data from parser input"),
      fragsize(ifragsize),
      proc_event(iproc_event),
      proc_handle(iproc_handle),
      start_event(istart_event),
      start_handle(istart_handle),
      stop_event(istop_event),
      stop_handle(istop_handle),
      input("input signal buffer (size: nchannels x fragsize)","[[]]"),
      output("output signal buffer"),
      s_in(NULL),
      s_out(NULL),
      b_fw_started(false),
      b_stopped(false),
      b_prepared(false),
      b_starting(false)
{
    insert_item("input",&input);
    insert_item("output",&output);
    patchbay.connect(&input.writeaccess,this,&io_parser_t::process_frame);
}

void io_parser_t::start()
{
    if( !b_prepared )
        throw MHA_Error(__FILE__,__LINE__,
                        "MHAIOParser: Not prepared for start.");
    b_fw_started = true;
    b_stopped = false;
    b_starting = true;
}

void io_parser_t::stop()
{
    b_fw_started = false;
    // The below code does not cause a race condition as all process callbacks
    // are also called from the configuration thread. This is a special case
    // and can not be generalized to other io libraries.
    stopped(0,0);
}

void io_parser_t::stopped(int proc_err,int io_err)
{
    if( stop_event )
        stop_event(stop_handle,proc_err,io_err);
}

void io_parser_t::started()
{
    if( start_event )
        start_event(start_handle);
}

void io_parser_t::process_frame()
{
    if( !b_prepared )
        return;
    if( input.data.size() != nchannels_in )
        throw MHA_Error(__FILE__,__LINE__,
                        "MHAIOParser: The input variable has %zu channels, expected %u.",
                        input.data.size(), nchannels_in );
    if( input.data[0].size() != fragsize )
        throw MHA_Error(__FILE__,__LINE__,
                        "MHAIOParser: The input fragsize is %zu, expected %u.",
                        input.data[0].size(), fragsize );
    started();
    unsigned int ch, fr;
    for(ch=0;ch<nchannels_in;ch++){
        for(fr=0;fr<fragsize;fr++){
            s_in->value(fr,ch) = input.data[ch][fr];
        }
    }
    int err = proc_event(proc_handle,s_in,&s_out);
    // on error switch to stopped state:
    if( err != 0 ){
        b_fw_started = false;
        stopped(err,0);
        return;
    }
    output.data.resize(s_out->num_channels);
    for(ch=0;ch<s_out->num_channels;ch++){
        output.data[ch].resize(s_out->num_frames);
        for(fr=0;fr<s_out->num_frames;fr++){
            output.data[ch][fr] = value(s_out,fr,ch);
        }
    }
    stopped(0,0);
}

io_parser_t::~io_parser_t()
{
}

#define ERR_SUCCESS 0
#define ERR_IHANDLE -1
#define ERR_USER -1000

#define MAX_USER_ERR 0x500
static char user_err_msg[MAX_USER_ERR];

extern "C" {
  
#ifdef MHA_STATIC_PLUGINS
#define IOInit               MHA_STATIC_MHAIOParser_IOInit
#define IOPrepare            MHA_STATIC_MHAIOParser_IOPrepare
#define IOStart              MHA_STATIC_MHAIOParser_IOStart
#define IOStop               MHA_STATIC_MHAIOParser_IOStop
#define IORelease            MHA_STATIC_MHAIOParser_IORelease
#define IOSetVar             MHA_STATIC_MHAIOParser_IOSetVar
#define IOStrError           MHA_STATIC_MHAIOParser_IOStrError
#define IODestroy            MHA_STATIC_MHAIOParser_IODestroy
#define dummy_interface_test MHA_STATIC_MHAIOParser_dummy_interface_test
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
               float /*samplerate*/,
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
            io_parser_t* cl = new io_parser_t(fragsize,proc_event,proc_handle,start_event,start_handle,stop_event,stop_handle);
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
            ((io_parser_t*)handle)->prepare(nch_in,nch_out);
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
            ((io_parser_t*)handle)->start();
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
            ((io_parser_t*)handle)->stop();
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
            ((io_parser_t*)handle)->release();
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
            ((io_parser_t*)handle)->parse(command,retval,maxretlen);
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
        delete (io_parser_t*)handle;
    }

    void dummy_interface_test(void){
#ifdef MHA_STATIC_PLUGINS
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOParser_,IOInit);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOParser_,IOPrepare);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOParser_,IOStart);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOParser_,IOStop);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOParser_,IORelease);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOParser_,IOSetVar);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOParser_,IOStrError);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOParser_,IODestroy);
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
 * compile-command: "make -C .. "
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * coding: utf-8-unix
 * End:
 */
