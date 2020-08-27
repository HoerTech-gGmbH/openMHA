// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2009 2010 2012 2013 2014 2015 2018 HörTech gGmbH
// Copyright © 2019 2020 HörTech gGmbH
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

#ifdef _WIN32
#include <winsock2.h>
#include <windows.h>
#endif

#include "mha_signal.hh"
#include "mha_fifo.h"
#include "mha_plugin.hh"
#include "mha_toolbox.h"
#include "mha_events.h"
#include "mha_defs.h"
#include "mha_os.h"
#include "mhapluginloader.h"
#include "mha_algo_comm.hh"

namespace dbasync_native {

enum {INVALID_THREAD_PRIORITY = 999999999};

class delay_check_t {
protected:
    delay_check_t(int delay, unsigned inner_fragsize, unsigned outer_fragsize);
}; 


class dbasync_t :
    public delay_check_t,
    public mha_dblbuf_t<mha_fifo_lw_t<mha_real_t> >
{
public:
    dbasync_t(unsigned int nchannels_in,
              unsigned int nchannels_out,
              unsigned int outer_fragsize,
              unsigned int inner_fragsize,
              int delay,
              const std::string & thread_scheduler,
              int thread_priority,
              MHAParser::mhapluginloader_t& plug);
    ~dbasync_t();
    mha_wave_t* outer_process(mha_wave_t*);
    // thread executes this method
    int svc();
private:
    MHAParser::mhapluginloader_t& plugloader;
    MHASignal::waveform_t inner_input;
    MHASignal::waveform_t outer_output;
#ifdef _WIN32
    HANDLE thread;
#else
    pthread_attr_t attr;
    struct sched_param priority;
    int scheduler;
    pthread_t thread;
#endif
};

#if _WIN32
static DWORD WINAPI thread_start(void * instance) {
    (static_cast<dbasync_t*>(instance))->svc();
    return 0U;
}
#else
static void * thread_start(void * instance) {
    (static_cast<dbasync_t*>(instance))->svc();
    return nullptr;
}
#endif

mha_wave_t* dbasync_t::outer_process(mha_wave_t* outer_input)
{
    if( outer_output.num_frames < outer_input->num_frames )
        throw MHA_Error(__FILE__,__LINE__,
                        "got fragment size of %u, expected max.%u.",
                        outer_input->num_frames, outer_output.num_frames);
    if( outer_input->num_channels != inner_input.num_channels )
        throw MHA_Error(__FILE__,__LINE__,
                        "got %u input channels, expected %u.",
                        outer_input->num_channels, inner_input.num_channels);
    process(outer_input->buf, outer_output.buf, outer_input->num_frames);
    return &outer_output;
}

int dbasync_t::svc()
{
    mha_wave_t * inner_output = 0;
    try {
        for(;;) {
            input(inner_input.buf);
            plugloader.process( &inner_input, &inner_output );
            output(inner_output->buf);
        }
    }
    catch (MHA_Error & e) {
        provoke_outer_error(e);
        return -1;
    }
}

static inline unsigned gcd(unsigned a, unsigned b)
{
    return b ? gcd(b, a%b) : a;
}


delay_check_t::delay_check_t(int delay,
                             unsigned inner_fragsize,
                             unsigned outer_fragsize)
{
    unsigned min_delay = inner_fragsize - gcd(inner_fragsize, outer_fragsize);
    if (delay < 0 || unsigned(delay) < min_delay)
        throw MHA_Error(__FILE__, __LINE__,
                        "Delay %d too small: need at least %u delay for"
                        " outer fragsize %u and inner fragsize %u",
                        delay, min_delay, outer_fragsize, inner_fragsize);
}

dbasync_t::dbasync_t(unsigned int nchannels_in,
                     unsigned int nchannels_out,
                     unsigned int outer_fragsize,
                     unsigned int inner_fragsize,
                     int delay,
                     const std::string & thread_scheduler,
                     int thread_priority,
                     MHAParser::mhapluginloader_t& plug)
    : delay_check_t(delay, inner_fragsize, outer_fragsize),
      mha_dblbuf_t<mha_fifo_lw_t<mha_real_t> >(outer_fragsize,
                                               inner_fragsize,
                                               delay,
                                               nchannels_in,
                                               nchannels_out,
                                               0),
      plugloader(plug),
      inner_input(inner_fragsize,nchannels_in),
      outer_output(outer_fragsize,nchannels_out)
{
    if (delay < 0 ||
        unsigned(delay) <
        (inner_fragsize - gcd(inner_fragsize, outer_fragsize)))
        throw MHA_Error(__FILE__,__LINE__,
                        "configured delay (%d) too small: must be"
                        " > %u", delay,
                        inner_fragsize - gcd(inner_fragsize, outer_fragsize));
    bool setting_priority = thread_priority != INVALID_THREAD_PRIORITY;
#ifdef _WIN32
    (void) thread_scheduler;
    thread = CreateThread(0,0,
                          thread_start, this,
                          0,0);
    if (thread == 0)
        throw MHA_ErrorMsg("Cannot create win32 thread");
    if (setting_priority) {
        if (SetThreadPriority(thread, thread_priority)
            == ((BOOL)0)) {
            throw MHA_ErrorMsg("Cannot set priority of win32 thread");
        }
    }
#else
    if (setting_priority) {
        pthread_attr_init(&attr);
        if (thread_scheduler == "SCHED_OTHER")
            pthread_attr_setschedpolicy(&attr, scheduler = SCHED_OTHER);
        else if (thread_scheduler == "SCHED_RR")
            pthread_attr_setschedpolicy(&attr, scheduler = SCHED_RR);
        else if (thread_scheduler == "SCHED_FIFO")
            pthread_attr_setschedpolicy(&attr, scheduler = SCHED_FIFO);
        else
            throw MHA_Error(__FILE__,__LINE__,
                            "Unknown thread scheduler \"%s\" specified",
                            thread_scheduler.c_str());
        priority.sched_priority = thread_priority;
        pthread_attr_setschedparam(&attr, &priority);
    }
    pthread_create(&thread,
                   (setting_priority ? &attr : nullptr),
                   &thread_start, 
                   this);
    if (setting_priority) {
        pthread_setschedparam(thread, scheduler, &priority);
    }
#endif
}

dbasync_t::~dbasync_t()
{
    provoke_inner_error(MHA_ErrorMsg("processing terminates"));
#ifdef _WIN32
    if (thread) {
        WaitForSingleObject(thread,100);
        CloseHandle(thread);
    }
#else
    pthread_join(thread, 0);
#endif
}

class db_if_t : public MHAPlugin::plugin_t< dbasync_t > {
public:
    db_if_t(algo_comm_t,const std::string& ,const std::string&);
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release();
    ~db_if_t()=default;
private:
    MHAKernel::algo_comm_class_t sub_ac;
    MHAParser::mhapluginloader_t plugloader;
    MHAParser::int_t fragsize;
    MHAParser::int_t delay;
    /// Scheduler used for worker thread
    MHAParser::kw_t worker_thread_scheduler;
    /// Priority of worker thread
    MHAParser::int_t worker_thread_priority;
    /// Scheduler of the signal processing thread
    MHAParser::string_mon_t framework_thread_scheduler;
    /// Priority of signal processing thread
    MHAParser::int_mon_t framework_thread_priority;
    std::string chain;
    std::string algo;
};

db_if_t::db_if_t(algo_comm_t iac,const std::string& th, const std::string& al)
    : MHAPlugin::plugin_t<dbasync_t>("Bidirectional fragment size adaptor"
                                     " (double buffer) with asynchronous"
                                     " processing",iac),
      plugloader(*this,sub_ac.get_c_handle()),
      fragsize("fragment size of inner plugin","200","[1,]"), 
      delay("algorithmic delay present after bidirectional fragment size adaptation"
            " (minimum is inner_fragment_size - gcd(inner_fragment_size, outer_fragment_size)","0","[0,]"),
      worker_thread_scheduler("Scheduler used for worker thread."
                              " Only used for posix threads.\n"
                              "Suggested setting is: The same as present"
                              " in framework_thread_scheduler\n"
                              "after prepare.",
                              "SCHED_OTHER",
                              "[SCHED_OTHER SCHED_RR SCHED_FIFO]"),
      worker_thread_priority("Priority assigned to worker threads.  Suggested"
                             " setting is:\n"
                             "Something minimally less important than the"
                             " priority of the\n"
                             "framework processing thread"
                             " (see framework_thread_priority after\n"
                             "preparing the MHA).  The default thread"
                             " priority given here is\n"
                             "invalid.  No attempt will be made to set the"
                             " priority of the\n"
                             "worker thread if this value remains unchanged.\n",
                             MHAParser::StrCnv::
                             val2str(INVALID_THREAD_PRIORITY),
                             "],["),
      framework_thread_scheduler("Scheduler used by the framework's"
                                 " processing thread.\n"
                                 "Only valid after first signal processing"
                                 " callback."),
      framework_thread_priority("Priority of the frameworks processing"
                                " thread.\n"
                                "Only valid after first signal processing"
                                " callback."),
      chain(th),
      algo(al)
{
    insert_member(fragsize);
    insert_member(delay);
    insert_member(worker_thread_priority);
    insert_member(worker_thread_scheduler);
    insert_member(framework_thread_priority);
    insert_member(framework_thread_scheduler);
}

void db_if_t::prepare(mhaconfig_t& conf)
{
    if( conf.domain != MHA_WAVEFORM )
        throw MHA_ErrorMsg("Doublebuffer: Only waveform data can be processed.");
    // remember the outer fragsize:
    unsigned int outer_fragsize = conf.fragsize;
    unsigned int inner_fragsize = fragsize.data;
    unsigned int input_channels = conf.channels;
    conf.fragsize = inner_fragsize;
    // sugest configuration to inner plugin, query requirements:
    plugloader.prepare(conf);
    if( conf.domain != MHA_WAVEFORM )
        throw MHA_ErrorMsg("Doublebuffer: Only waveform data can be processed.");
    // only fixed input/output fragsizes are allowed:
    if( inner_fragsize != conf.fragsize )
        throw MHA_ErrorMsg("Doublebuffer: Plugin modified the fragment size.");
    // update the configuration, create an instance of the double buffer:
    push_config(new dbasync_t(input_channels,
                              conf.channels,
                              outer_fragsize,
                              inner_fragsize,
                              delay.data,
                              worker_thread_scheduler.data.get_value(),
                              worker_thread_priority.data,
                              plugloader));
    conf.fragsize = outer_fragsize;
}

void db_if_t::release()
{
    plugloader.release();
}

mha_wave_t* db_if_t::process(mha_wave_t* s)
{
    poll_config();
    return cfg->outer_process(s);
}

}

MHAPLUGIN_CALLBACKS(dbasync,dbasync_native::db_if_t,wave,wave)
MHAPLUGIN_DOCUMENTATION(dbasync,"data-flow signal-transformation","Bidirectional fragment size adaptor"
                        " (double buffer) with asynchronous"
                        " processing")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
