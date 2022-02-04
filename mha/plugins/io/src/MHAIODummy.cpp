// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2022 Hörzentrum Oldenburg gGmbH
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
#include <unistd.h>
#include <thread>
#include <chrono>
#include <atomic>

#define ERR_SUCCESS 0
#define ERR_IHANDLE -1
#define ERR_USER -1000

#define MAX_USER_ERR 0x500
static char user_err_msg[MAX_USER_ERR] = "";


/** Dummy sound io library. Simulates real time sound. Input is always zero, output is always ignored. */
class io_dummy_t : public MHAParser::parser_t
{
public:
    io_dummy_t(unsigned int fragsize,
               float samplerate,
               IOProcessEvent_t proc_event,
               void* proc_handle,
               IOStartedEvent_t start_event,
               void* start_handle,
               IOStoppedEvent_t stop_event,
               void* stop_handle);

    /** Prepare. Initialized the input buffer */
    void prepare(int nch_in);

    /** Release. Frees the input buffer */
    void release();

    /** Initialize main_loop and start the loop immediately, stop on error */
    void start();

    /** Send stop request to main loop and join thread */
    void stop();

private:
    /** The framework's sampling rate */
    float samplerate;

    /** The framework's frag size */
    unsigned int fragsize;

    /** Pointer to signal processing callback function. */
    IOProcessEvent_t proc_event;

    /** Pointer to start notification callback function.  Called when
     * the user issues the start command. */
    IOStartedEvent_t start_event;

    /** Pointer to stop notification callback function. Called when the user issues a stop request */
    IOStoppedEvent_t stop_event;

    /** Handles belonging to framework. */
    void *proc_handle, *start_handle, *stop_handle;

    /** Input sound for framework, always zero */
    std::unique_ptr<MHASignal::waveform_t> in;

    /** Output from framework, always ignored */
    mha_wave_t* out;

    /** Main event loop. Calls the framework's process chain periodically according to sampling rate and fragsize */
    std::thread main_loop;

    /** Stop request flag for main_loop */
    std::atomic<bool> stop_request;
};

void io_dummy_t::prepare(int nch_in){
    in=std::make_unique<MHASignal::waveform_t>(fragsize,nch_in);
    in->assign(0);
}

void io_dummy_t::release(){
    in.reset(nullptr);
}

void io_dummy_t::start(){
    main_loop=std::thread([this](){
        while(!stop_request.load()){
            start_event(start_handle);
            auto tic=std::chrono::system_clock::now();
            auto intervall=std::chrono::microseconds(int(fragsize/samplerate*1e6));
            int err = proc_event( proc_handle, in.get(), &out );
            // on error switch to stopped state:
            if( err != 0 ){
                if(stop_event)
                    stop_event(stop_handle,err,0);
                return;
            }
            std::this_thread::sleep_until(tic+intervall);
        }
    });
}

void io_dummy_t::stop(){
    stop_request=true;
    main_loop.join();
    stop_event(stop_handle,0,0);
}

io_dummy_t::io_dummy_t(unsigned int ifragsize,
                       float isamplerate,
                       IOProcessEvent_t iproc_event,
                       void* iproc_handle,
                       IOStartedEvent_t istart_event,
                       void* istart_handle,
                       IOStoppedEvent_t istop_event,
                       void* istop_handle)
    : MHAParser::parser_t("Dummy client"),
      samplerate(isamplerate),
      fragsize(ifragsize),
      proc_event(iproc_event),
      start_event(istart_event),
      stop_event(istop_event),
      proc_handle(iproc_handle),
      start_handle(istart_handle),
      stop_handle(istop_handle),
      stop_request(false)
{}

extern "C" {
#ifdef MHA_STATIC_PLUGINS
#define IOInit               MHA_STATIC_MHAIODummy_IOInit
#define IOPrepare            MHA_STATIC_MHAIODummy_IOPrepare
#define IOStart              MHA_STATIC_MHAIODummy_IOStart
#define IOStop               MHA_STATIC_MHAIODummy_IOStop
#define IORelease            MHA_STATIC_MHAIODummy_IORelease
#define IOSetVar             MHA_STATIC_MHAIODummy_IOSetVar
#define IOStrError           MHA_STATIC_MHAIODummy_IOStrError
#define IODestroy            MHA_STATIC_MHAIODummy_IODestroy
#define dummy_interface_test MHA_STATIC_MHAIODummy_dummy_interface_test
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
            io_dummy_t* cl = new io_dummy_t(fragsize,samplerate,proc_event,proc_handle,start_event,start_handle,stop_event,stop_handle);
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
        (void)nch_out;
        if( !handle )
            return ERR_IHANDLE;
        try{
            ((io_dummy_t*)handle)->prepare(nch_in);
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
            ((io_dummy_t*)handle)->start();
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
            ((io_dummy_t*)handle)->stop();
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
            ((io_dummy_t*)handle)->release();
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
            ((io_dummy_t*)handle)->parse(command,retval,maxretlen);
            return ERR_SUCCESS;
        }
        catch(MHA_Error& e){
            strncpy( user_err_msg, Getmsg(e), MAX_USER_ERR-1 );
            user_err_msg[MAX_USER_ERR-1] = 0;
            return ERR_USER;
        }
    }

    const char* IOStrError(void*,int err)
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
        delete (io_dummy_t*)handle;
    }

    void dummy_interface_test(void){
#ifdef MHA_STATIC_PLUGINS
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIODummy_,IOInit);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIODummy_,IOPrepare);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIODummy_,IOStart);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIODummy_,IOStop);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIODummy_,IORelease);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIODummy_,IOSetVar);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIODummy_,IOStrError);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIODummy_,IODestroy);
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
