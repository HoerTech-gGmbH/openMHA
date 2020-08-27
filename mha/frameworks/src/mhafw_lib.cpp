// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2009 2011 2013 2014 2016 2017 2018 HörTech gGmbH
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

#include "mhafw_lib.h"
#include "mha_parser.hh"
#include "mha_error.hh"
#include "mha.hh"
#include "mha_algo_comm.hh"
#include "mha_plugin.hh"
#include "mha_signal.hh"
#include "mha_defs.h"
#include "mha_os.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include <stdio.h>
#include <string>
#include <math.h>
#include <unistd.h>
#include "mha_os.h"

void fw_t::fw_sleep_cmd()
{
    mha_msleep(fw_sleep.data * 1000);
}

void fw_t::fw_until_cmd()
{
    if( fw_until.data.size() == 0 )
        return;
    unsigned int p = fw_until.data.find(":");
    if( p >= fw_until.data.size() )
        throw MHA_Error(__FILE__,__LINE__,"Invalid until-syntax: \"%s\"",
                        fw_until.data.c_str());
    std::string varname = fw_until.data.substr(0,p);
    std::string value = fw_until.data.substr(p+1);
    std::string vartype = parse(varname+"?perm");
    std::string newval;
    if( parse(varname+"?perm") != "monitor" )
        throw MHA_Error(__FILE__,__LINE__,"The variable \"%s\" is not a monitor variable.",varname.c_str());
    bool ok = false;
    while( !ok ){
        mha_msleep( 1000 );
        newval = parse(varname+"?val");
        ok = (newval == value);
    }
}

void fw_t::get_input_signal_dimension()
{
    memset(&cfin,0,sizeof(cfin));
    memset(&cfout,0,sizeof(cfout));
    cfin.channels = prepare_vars.pinchannels.data;
    cfin.domain = MHA_WAVEFORM;
    cfin.fragsize = prepare_vars.pfragmentsize.data;
    cfin.srate = prepare_vars.psrate.data;
}

fw_vars_t::fw_vars_t(MHAParser::parser_t& p)
    : pinchannels("input channel number","1","[1,["),
      pfragmentsize("fragment size in samples","200","[1,["),
      psrate("sampling rate in Hz","44100","]0,[")
{
    p.insert_item("nchannels_in",&pinchannels);
    p.insert_item("fragsize",&pfragmentsize);
    p.insert_item("srate",&psrate);
}

fw_t::fw_t()
    : MHAParser::parser_t("MHA Framework"),
      prepare_vars(static_cast<MHAParser::parser_t&>(*this)),
      nchannels_out("Number of output channels (valid after prepare)."),
      proc_name("MHA library name",""),
      io_name("IO plugin library name",""),
      exit_on_stop("exit on stop event?","no"),
      fw_sleep("sleep command (user interface)","0","[0,["),
      fw_until("until command (user interface)\nUsage:\nuntil = monitor name:expected content\nWait, until the content of the given variable matches the expected content.",""),
      fw_cmd("MHA state control command.\nBy setting this variable, the processing state of the MHA, its\nIO plugins and the processing plugins can be changed.","nop","[nop prepare start stop release quit]"),
      parserstate("internal state of MHA"),
      errorlog("Name of the error log file",""),
      fatallog("Name of the fatal error log file", ""),
      plugins("List of plugins in the current configuration", "[]"),
      plugin_paths("Paths of the plugins in the current configuration", "[]"),
      dump_mha("dump the current configuration of the mha into a text file", "no"),
      inst_name("Name of this mha instance","mha"),
      proc_lib(NULL),
      io_lib(NULL),
      state(fw_unprepared),
      b_exit_request(false),
      proc_error(0),
      io_error(0),
      proc_error_string("last error in asynchronous callback")
{
    insert_item("nchannels_out",&nchannels_out);
    insert_item("mhalib",&proc_name);
    insert_item("iolib",&io_name);
    insert_item("sleep",&fw_sleep);
    //insert_item("until",&fw_until);
    insert_item("cmd",&fw_cmd);
    insert_item("asyncerror",&proc_error_string);
    insert_item("state",&parserstate);
    insert_item("errorlog", &errorlog);
    insert_item("fatallog", &fatallog);
    insert_item("plugins", &plugins);
    insert_item("plugin_paths", &plugin_paths);
    insert_item("dump_mha", &dump_mha);
    insert_item("instance",&inst_name); 
    patchbay.connect(&proc_name.writeaccess,this,&fw_t::load_proc_lib);
    patchbay.connect(&io_name.writeaccess,this,&fw_t::load_io_lib);
    patchbay.connect(&fw_cmd.writeaccess,this,&fw_t::exec_fw_command);
    patchbay.connect(&fw_sleep.writeaccess,this,&fw_t::fw_sleep_cmd);
    patchbay.connect(&fw_until.writeaccess,this,&fw_t::fw_until_cmd);
    patchbay.connect(&proc_error_string.readaccess,this,&fw_t::async_read);
    patchbay.connect(&proc_error_string.prereadaccess,this,&fw_t::async_poll_msg);
    patchbay.connect(&parserstate.prereadaccess,this,&fw_t::get_parserstate);
}

void fw_t::exec_fw_command()
{
    switch( fw_cmd.data.get_index() ){
        case 1 : // prepare
            prepare();
            break;
        case 2 : // start
            start();
            break;
        case 3 : // stop
            stop();
            break;
        case 4 : // release
            release();
            break;
        case 5 : // quit
            quit();
            break;
        default :
            break;
    }
    fw_cmd.data.set_index(0);
}

void fw_t::load_proc_lib()
{
    if( proc_name.data.size() ){
        proc_lib = new PluginLoader::mhapluginloader_t(ac.get_c_handle(),proc_name.data);
        if( proc_lib->has_parser() )
            insert_item("mha",proc_lib);
        proc_name.setlock(true);
    }
}

void fw_t::load_io_lib()
{
    if( io_name.data.size() ){
        prepare_vars.lock_srate_fragsize();
        io_lib = new io_lib_t(prepare_vars.pfragmentsize.data,
                              prepare_vars.psrate.data,
                              process,(void*)this,
                              started,(void*)this,
                              stopped,(void*)this,
                              io_name.data);
        insert_item("io",io_lib);
        io_name.setlock(true);
    }
}

fw_t::~fw_t()
{
    try{
        quit();
        if( proc_lib )
            delete proc_lib;
        if( io_lib )
            delete io_lib;
    }
    catch(MHA_Error& e){
        std::cerr << Getmsg(e) << std::endl;
    }
}

void fw_t::stopped(int proc_err, int io_err)
{
    switch( state ){
        case fw_unprepared :
        case fw_exiting :
            break;
        case fw_stopped :
        case fw_starting :
        case fw_running :
        case fw_stopping :
            if( exit_on_stop.data )
                b_exit_request = true;
            proc_error = proc_err;
            io_error = io_err;
            state = fw_stopped;
            break;
    }
}

void fw_t::started()
{
    switch( state ){
        case fw_unprepared :
        case fw_stopping :
        case fw_exiting :
            break;
        case fw_stopped :
        case fw_starting :
        case fw_running :
            state = fw_running;
            break;
    }
}

int fw_t::process(mha_wave_t* s_in,mha_wave_t** s_out)
{
    try{
        // The iolib may continue to send data after we got a stop request
        // but before it processed it. As the state fw_stopping is active only
        // during this time, we may suppress the error in that specific circumstance.
        if( state != fw_running and state != fw_stopping )
            throw MHA_ErrorMsg("The framework is not in a running state.");
        if( !s_out )
            throw MHA_Error(__FILE__,__LINE__,"Output signal pointer is undefined.");
        proc_lib->process(s_in,s_out);
        return 0;
    }
    catch( MHA_Error& e ){
        proc_error_string.data = Getmsg(e);

        fprintf(stderr, "MHA Error: %s\n", proc_error_string.data.c_str());
        fflush(stderr);

        std::size_t found = proc_error_string.data.find("Fatal error");
        std::ofstream logfile;
        if (found == std::string::npos) {
            // No Fatal Error found. So, in the error log
            if (!errorlog.data.empty())
                logfile.open(errorlog.data, std::ios::out | std::ios::app | std::ios::ate);
        } else {
            // Fatal error found. In the fatal error log
            if (!fatallog.data.empty())
                logfile.open(fatallog.data, std::ios::out | std::ios::ate);
        }

        if (logfile.is_open()) {
            printf("log file written\n");
            std::time_t err_time = std::time(NULL);
            char str_time[25];
            std::strftime(str_time, 25, "%c", std::localtime(&err_time));

            std::stringstream ss_error_msg;
            ss_error_msg << str_time << ": " << proc_error_string.data << ".";
            std::string error_msg = ss_error_msg.str();

            logfile << error_msg << "\n";

            logfile.close();
        }

        return 1;
    }
}

void fw_t::prepare()
{
    switch( state ){
    case fw_unprepared :
        if( !proc_lib )
            throw MHA_ErrorMsg("No processing library loaded.");
        if( !io_lib )
            throw MHA_ErrorMsg("No IO library loaded.");

        prepare_vars.lock_channels();
        get_input_signal_dimension();
        cfout = cfin;
        proc_lib->prepare(cfout);
        try{
            if( cfout.domain != MHA_WAVEFORM )
                throw MHA_ErrorMsg("The processing library does not return waveform data.");
            if( cfout.fragsize != cfin.fragsize )
                throw MHA_ErrorMsg("The processing library returned invalid fragment size.");
            if( cfout.srate != cfin.srate )
                throw MHA_ErrorMsg("The processing library returned invalid sampling rate.");
            io_lib->prepare(cfin.channels,cfout.channels);
            nchannels_out.data = cfout.channels;
        }
        catch(...){
            proc_lib->release();
            throw;
        }
        state = fw_stopped;
        break;
    case fw_stopped :
        break;
    case fw_running :
        throw MHA_ErrorMsg("Program is running.");
        // break not needed, would be unreachable
    case fw_starting :
        throw MHA_ErrorMsg("Program is starting.");
        // break not needed, would be unreachable
    case fw_stopping :
        throw MHA_ErrorMsg("Program is stopping.");
        // break not needed, would be unreachable
    case fw_exiting :
        throw MHA_ErrorMsg("Program is exiting.");
        // break not needed, would be unreachable
    }
}

void fw_t::start()
{
    switch( state ){
        case fw_unprepared :
            prepare();
            start();
            break;
        case fw_stopped :
            if( !io_lib )
                throw MHA_ErrorMsg("No IO library loaded.");
            state = fw_starting;
            io_lib->start();
            break;
        case fw_starting :
        case fw_running :
        case fw_stopping :
            break;
        case fw_exiting :
            throw MHA_ErrorMsg("Program is exiting.");
            // break not needed, would be unreachable
    }
}

void fw_t::stop()
{
    int timeout = 2000; // ms
    switch( state ){
        case fw_unprepared :
            throw MHA_ErrorMsg("Program is not running.");
            // break not needed, would be unreachable
        case fw_stopped :
        case fw_stopping :
            break;
        case fw_starting :
        case fw_running :
            if( !io_lib )
                throw MHA_ErrorMsg("No IO library loaded.");
            state = fw_stopping;
            io_lib->stop();
            while( state != fw_stopped ){
                mha_msleep( 1 );
                timeout --;
                if( !timeout )
                    throw MHA_Error(__FILE__,__LINE__,"Stop request timed out.");
            }
            break;
        case fw_exiting :
            throw MHA_ErrorMsg("Program is exiting.");
            // break not needed, would be unreachable
    }
}

void fw_t::release()
{
    switch( state ){
        case fw_unprepared :
            break;
        case fw_stopped :
        case fw_stopping :
            if( !io_lib )
                throw MHA_ErrorMsg("No IO library loaded.");
            io_lib->release();
            proc_lib->release();
            state = fw_unprepared;
            nchannels_out.data = 0;
            prepare_vars.unlock_channels();
            break;
        case fw_starting :
        case fw_running :
            stop();
            release();
            break;
        case fw_exiting :
            throw MHA_ErrorMsg("Program is exiting.");
            // break not needed, would be unreachable
    }
}

void fw_t::quit()
{
    switch( state ){
        case fw_unprepared :
            break;
        case fw_stopped :
        case fw_starting :
        case fw_stopping :
        case fw_running :
            release();
            break;
        case fw_exiting :
            break;
    }
    state = fw_exiting;
    b_exit_request = true;
}


io_lib_t::io_lib_t(int fragsize, 
                   float samplerate,
                   IOProcessEvent_t proc_event,
                   void* proc_handle,
                   IOStartedEvent_t start_event,
                   void* start_handle,
                   IOStoppedEvent_t stop_event,
                   void* stop_handle,
                   std::string libname)
    : MHAParser::c_ifc_parser_t(libname),
      lib_handle(libname.c_str()),
      lib_data(NULL),
      IOInit_cb(NULL),
      IOPrepare_cb(NULL),
      IOStart_cb(NULL),
      IOStop_cb(NULL),
      IORelease_cb(NULL),
      IOSetVar_cb(NULL),
      IOStrError_cb(NULL),
      IODestroy_cb(NULL)
{
    MHA_RESOLVE_CHECKED((&lib_handle),IOInit);
    MHA_RESOLVE_CHECKED((&lib_handle),IOPrepare);
    MHA_RESOLVE_CHECKED((&lib_handle),IOStart);
    MHA_RESOLVE_CHECKED((&lib_handle),IOStop);
    MHA_RESOLVE_CHECKED((&lib_handle),IORelease);
    MHA_RESOLVE_CHECKED((&lib_handle),IOSetVar);
    MHA_RESOLVE_CHECKED((&lib_handle),IOStrError);
    MHA_RESOLVE_CHECKED((&lib_handle),IODestroy);
    lib_err = IOInit_cb(fragsize,samplerate,proc_event,proc_handle,start_event,start_handle,stop_event,stop_handle,&lib_data);
    test_error();
    set_parse_cb(IOSetVar_cb,IOStrError_cb,lib_data);
}

std::string io_lib_t::lib_str_error(int err)
{
    if( err )
        return IOStrError_cb(lib_data,err);
    else
        return "";
}

io_lib_t::~io_lib_t()
{
    try{
        IODestroy_cb(lib_data);
    }
    catch(MHA_Error& e){
        std::cerr << Getmsg(e) << std::endl;
    }
}

void io_lib_t::prepare(unsigned int inch,unsigned int outch)
{
    lib_err = IOPrepare_cb(lib_data,inch,outch);
    test_error();
}

void io_lib_t::start()
{
    lib_err = IOStart_cb(lib_data);
    test_error();
}

void io_lib_t::stop()
{
    lib_err = IOStop_cb(lib_data);
    test_error();
}

void io_lib_t::release()
{
    lib_err = IORelease_cb(lib_data);
    test_error();
}

void io_lib_t::test_error()
{
    if( lib_err != 0 ){
        throw MHA_Error(__FILE__,__LINE__,
                        "IO error: %s", IOStrError_cb(lib_data,lib_err));
    }
}

void fw_vars_t::lock_srate_fragsize()
{
    pfragmentsize.setlock(true);
    psrate.setlock(true);
}

void fw_vars_t::lock_channels()
{
    pinchannels.setlock(true);
}

void fw_vars_t::unlock_srate_fragsize()
{
    pfragmentsize.setlock(false);
    psrate.setlock(false);
}
void fw_vars_t::unlock_channels()
{
    pinchannels.setlock(false);
}

void fw_t::async_poll_msg()
{
    if( io_error && io_lib ){
        if( proc_error_string.data.size() )
            proc_error_string.data += "\n";
        proc_error_string.data += "IO Error: ";
        proc_error_string.data += io_lib->lib_str_error(io_error);
        io_error = 0;
    }
}

void fw_t::get_parserstate()
{
    switch( state ){
    case fw_unprepared :
        parserstate.data = "unprepared";
        break;
    case fw_stopped: 
        parserstate.data = "stopped";
        break;
    case fw_starting:
        parserstate.data = "starting";
        break;
    case fw_running:
        parserstate.data = "running";
        break;
    case fw_stopping:
        parserstate.data = "stopping";
        break;
    case fw_exiting:
        parserstate.data = "exiting";
        break;
    }    
}

// Local Variables:
// compile-command: "make -C .."
// coding: utf-8-unix
// indent-tabs-mode: nil
// c-basic-offset: 4
// End:
