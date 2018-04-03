// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2008 2011 2012 2013 2016 2017 HörTech gGmbH
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

#ifndef MHAFW_LIB_H
#define MHAFW_LIB_H

#include "mha_toolbox.h"
#include "mha_plugin.hh"
#include "mha_io_ifc.h"
#include "mha_filter.hh"
#include "mha_algo_comm.hh"
#include "mha_os.h"
#include "mhapluginloader.h"

/// Class for loading MHA sound IO module.
class io_lib_t : public MHAParser::c_ifc_parser_t {
public:
    /// load and initialize MHA sound io module.
    io_lib_t(int fragsize, 
             float samplerate,
             IOProcessEvent_t proc_event,
             void* proc_handle,
             IOStartedEvent_t start_event,
             void* start_handle,
             IOStoppedEvent_t stop_event,
             void* stop_handle,
             std::string libname);
    /// Deinitialize and unload this MHA sound io module
    ~io_lib_t();

    /// Prepare the sound io module. After preparation, the sound io module
    /// may start the sound processing at any time (external trigger).
    /// When the sound processing is started, the sound io module will call
    /// the start_event callback.
    /// @param inch number of input channels
    /// @param outch number of output channels
    void prepare(unsigned int inch,unsigned int outch);

    /// Tell the sound io module to start sound processing. Some io modules
    /// need this, for others that wait for external events this method might
    /// do nothing.
    void start();
    void stop();
    void release();
    std::string lib_str_error(int err);
protected:
    void test_error();
    int lib_err;
    dynamiclib_t lib_handle;
    void* lib_data;
    // callback handles:
    IOInit_t IOInit_cb;
    IOPrepare_t IOPrepare_cb;
    IOStart_t IOStart_cb;
    IOStop_t IOStop_cb;
    IORelease_t IORelease_cb;
    IOSetVar_t IOSetVar_cb;
    IOStrError_t IOStrError_cb;
    IODestroy_t IODestroy_cb;
};

class fw_vars_t {
public:
    fw_vars_t(MHAParser::parser_t&);
    void lock_srate_fragsize();
    void lock_channels();
    void unlock_srate_fragsize();
    void unlock_channels();

    MHAParser::int_t pinchannels;
    MHAParser::int_t pfragmentsize;
    MHAParser::float_t psrate;
};

class fw_t : public MHAParser::parser_t {
public:
    fw_t();
    ~fw_t();
    bool exit_request() const {return b_exit_request;};
private:
    void prepare(); ///< preparation for processing
    void start();   ///< start of processing
    void stop();    ///< stop/pause of processing
    void release(); ///< release of IO device
    void quit();    ///< controlled quit
    static void stopped(void* h,int proc_err,int io_err){((fw_t*)h)->stopped(proc_err,io_err);};
    static void started(void* h){((fw_t*)h)->started();};
    static int process(void* h,mha_wave_t* sIn,mha_wave_t** sOut){return ((fw_t*)h)->process(sIn,sOut);};
    void stopped(int,int);
    void started();
    int process(mha_wave_t*,mha_wave_t**);
    void exec_fw_command();
    void load_proc_lib();
    void load_io_lib();
    void fw_sleep_cmd();
    void fw_until_cmd();
    void get_input_signal_dimension();
    fw_vars_t prepare_vars;
    MHAParser::int_mon_t nchannels_out;
    MHAParser::string_t proc_name;
    MHAParser::string_t io_name;
    MHAParser::bool_t exit_on_stop;
    MHAParser::int_t fw_sleep;
    MHAParser::string_t fw_until;
    MHAParser::kw_t fw_cmd;
    MHAParser::string_mon_t parserstate;
    MHAParser::string_t errorlog;
    MHAParser::string_t fatallog;
    MHAParser::vstring_t plugins;
    MHAParser::vstring_t plugin_paths;
    MHAParser::bool_t dump_mha;
    /// A variable for naming MHA instances
    MHAParser::string_t inst_name;
    MHAKernel::algo_comm_class_t ac;
    PluginLoader::mhapluginloader_t* proc_lib;
    io_lib_t* io_lib;
    mhaconfig_t cfin, cfout;
    enum state_t {
        fw_unprepared, fw_stopped, fw_starting, fw_running, fw_stopping, fw_exiting
    };
    state_t state;
    bool b_exit_request;
protected:
    int proc_error;
    int io_error;
private:
    void async_read() {proc_error_string.data = "";};
    void async_poll_msg();
    void get_parserstate();
    MHAParser::string_mon_t proc_error_string;
    MHAEvents::patchbay_t<fw_t> patchbay;
};

#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * coding: utf-8-unix
 * End:
 */
