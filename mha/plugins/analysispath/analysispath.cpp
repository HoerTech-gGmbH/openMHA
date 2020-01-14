// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2007 2008 2009 2010 2013 2014 2015 2017 2018 HörTech gGmbH
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

#include "mha_signal.hh"
#include "mha_fifo.h"
#include "mha_plugin.hh"
#include "mha_toolbox.h"
#include "mha_events.h"
#include "mha_defs.h"
#include "mha_os.h"
#include "mhapluginloader.h"
#include "mha_algo_comm.hh"

class analysepath_t
{
public:
    analysepath_t(unsigned int nchannels_in,
                  unsigned int outer_fragsize,
                  unsigned int inner_fragsize,
                  int priority,
                  MHAProc_wave2wave_t inner_proc_wave2wave,
                  MHAProc_wave2spec_t inner_proc_wave2spec,
                  void* ilibdata,
                  algo_comm_t outer_ac,
                  const MHA_AC::acspace2matrix_t& acspace_template,
                  mha_domain_t inner_out_domain,
                  unsigned int fifo_len_blocks);
    virtual ~analysepath_t();
    void rt_process(mha_wave_t*);
    virtual int svc();
private:
    MHAProc_wave2wave_t inner_process_wave2wave;
    MHAProc_wave2spec_t inner_process_wave2spec;
    MHASignal::waveform_t inner_input;
    void* libdata;
    mha_fifo_t<mha_real_t> wave_fifo;
    mha_fifo_t<MHA_AC::acspace2matrix_t> ac_fifo;
    MHA_AC::acspace2matrix_t inner_ac_copy;
    MHA_AC::acspace2matrix_t outer_ac_copy;
    algo_comm_t outer_ac;
    mha_domain_t inner_out_domain;
    MHA_Error inner_error;
    bool has_inner_error;
    bool flag_terminate_inner_thread;
    int input_to_process;

    pthread_mutex_t ProcessMutex;

    pthread_attr_t attr;
    struct sched_param priority;
    int scheduler;
    pthread_t thread;
    pthread_cond_t cond_to_process;
};


static void * thread_start(void * instance) {
    ((analysepath_t*)instance)->svc();
    return NULL;
}

void analysepath_t::rt_process(mha_wave_t* outer_input)
{
    int mutex_val;

    if( outer_input->num_channels != inner_input.num_channels )
        throw MHA_Error(__FILE__,__LINE__,
                        "got %u input channels, expected %u.",
                        outer_input->num_channels, inner_input.num_channels);
    if( wave_fifo.get_available_space() >= size(outer_input) )
        wave_fifo.write(outer_input->buf,size(outer_input));
    if( ac_fifo.get_fill_count() > 0 ){
        ac_fifo.read(&outer_ac_copy,1);
        outer_ac_copy.insert(outer_ac);
    }
    if( has_inner_error )
        throw inner_error;

    if ((mutex_val = pthread_mutex_lock(&ProcessMutex)) == 0) {
        input_to_process++;
        pthread_cond_signal(&cond_to_process);

        pthread_mutex_unlock(&ProcessMutex);
    }
}

int analysepath_t::svc()
{
    int mutex_val;

    mha_wave_t * inner_output_wave = NULL;
    mha_spec_t * inner_output_spec = NULL;
    try {
        while(!flag_terminate_inner_thread){
            if( (wave_fifo.get_fill_count() >= size(inner_input)) && (ac_fifo.get_available_space() > 0) ){
                wave_fifo.read(inner_input.buf,size(inner_input));
                if( inner_out_domain == MHA_WAVEFORM )
                    inner_process_wave2wave(libdata,&inner_input,&inner_output_wave);
                else
                    inner_process_wave2spec(libdata,&inner_input,&inner_output_spec);
                inner_ac_copy.update();
                ac_fifo.write(&inner_ac_copy,1);
            }else{
                // Wait for reading to finish
                if ((mutex_val = pthread_mutex_lock(&ProcessMutex)) == 0) {

                    while(!input_to_process)
                        pthread_cond_wait(&cond_to_process, &ProcessMutex);

                    input_to_process--;

                    pthread_mutex_unlock(&ProcessMutex);
                }

            }
        }
    }
    catch (MHA_Error & e) {
        std::cerr << Getmsg(e) << std::endl;
        inner_error = e;
        has_inner_error = true;
        return -1;
    }
    return 0;
}

analysepath_t::analysepath_t(unsigned int nchannels_in,
                             unsigned int outer_fragsize,
                             unsigned int inner_fragsize,
                             int thread_priority,
                             MHAProc_wave2wave_t inner_proc_wave2wave,
                             MHAProc_wave2spec_t inner_proc_wave2spec,
                             void* ilibdata,
                             algo_comm_t iouter_ac,
                             const MHA_AC::acspace2matrix_t& acspace_template,
                             mha_domain_t iinner_out_domain,
                             unsigned int fifo_len_blocks)
    : inner_process_wave2wave(inner_proc_wave2wave),
      inner_process_wave2spec(inner_proc_wave2spec),
      inner_input(inner_fragsize,nchannels_in),
      libdata(ilibdata),
      wave_fifo(nchannels_in*fifo_len_blocks*inner_fragsize),
      ac_fifo(2,acspace_template),
      inner_ac_copy(acspace_template),
      outer_ac_copy(acspace_template),
      outer_ac(iouter_ac),
      inner_out_domain(iinner_out_domain),
      inner_error("", 0, "(uninitialized MHA_Error object used for temporary storage of exceptions in plugin analysispath)"),
      has_inner_error(false),
      flag_terminate_inner_thread(false),
      input_to_process(0)
{
    if( inner_out_domain == MHA_WAVEFORM ){
        if( !inner_process_wave2wave )
            throw MHA_Error(__FILE__,__LINE__,"No waveform to waveform process callback provided.");
    }else{
        if( !inner_process_wave2spec )
            throw MHA_Error(__FILE__,__LINE__,"No waveform to spectrum process callback provided.");
    }

    if(fifo_len_blocks < 1)
        throw MHA_Error(__FILE__,__LINE__,"FIFO length to short (%u)",fifo_len_blocks);

    // Initialize the mutex
    pthread_mutex_init(&ProcessMutex, NULL);

    // Initialize the condition variable
    pthread_cond_init(&cond_to_process, NULL);

    bool setting_priority = thread_priority > 0;
    if (setting_priority) {
        pthread_attr_init(&attr);
        pthread_attr_setschedpolicy(&attr, scheduler = SCHED_FIFO);
        priority.sched_priority = thread_priority;
        pthread_attr_setschedparam(&attr, &priority);
    }
    pthread_create(&thread,
                   (setting_priority ? &attr : NULL),
                   &thread_start, 
                   this);
    if (setting_priority) {
        pthread_setschedparam(thread, scheduler, &priority);
    }
}

analysepath_t::~analysepath_t()
{
    flag_terminate_inner_thread = true;

    input_to_process = 1;

    // Release the condition variables
    pthread_cond_broadcast(&cond_to_process);

    // Wait for the thread to join
    pthread_join(thread, NULL);

    //Destroy the mutex
    pthread_mutex_destroy(&ProcessMutex);

    // Cancel the thread
    pthread_cancel(thread);
}

class plug_t : private MHAKernel::algo_comm_class_t, public PluginLoader::mhapluginloader_t  {
public:
    plug_t(const std::string& libname,const std::string& chain,const std::string& algo);
    ~plug_t() throw () {}
    MHAProc_wave2wave_t get_process_wave();
    MHAProc_wave2spec_t get_process_spec();
    void* get_handle();
    algo_comm_t get_ac() { return get_c_handle();};
};

class analysispath_if_t : public MHAPlugin::plugin_t< analysepath_t > {
public:
    analysispath_if_t(algo_comm_t,std::string,std::string);
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release();
    ~analysispath_if_t();
private:
    void loadlib();
    MHAEvents::patchbay_t< analysispath_if_t > patchbay;
    MHAParser::string_t libname;
    MHAParser::int_t fragsize;
    MHAParser::int_t fifolen;
    MHAParser::int_t priority;
    MHAParser::vstring_t vars;
    plug_t* plug;
    std::string chain;
    std::string algo;
    MHA_AC::acspace2matrix_t* acspace_template;
};

analysispath_if_t::analysispath_if_t(algo_comm_t iac,std::string th,std::string al)
    : MHAPlugin::plugin_t<analysepath_t>(
        "Split-up of signal analysis and filtering, with asychronous processing of filter path and thread-safe exchange of filter parameters as AC variables.",iac),
      libname("inner plugin name, receives adapted fragment size",""),
      fragsize("fragment size of inner plugin","200","[1,]"), 
      fifolen("length of double buffer in inner fragment size","10","[1,]"),
      priority("SCHED_FIFO priority (<0 for no real-time scheduling)","-1"),
      vars("Names of AC variables to be copied back to processing thread (empty: all)","[]"),
      plug(NULL),
      chain(th),
      algo(al),
      acspace_template(NULL)
{
    insert_item("plugname",&libname);
    insert_item("fragsize",&fragsize);
    insert_item("fifolen",&fifolen);
    insert_item("priority",&priority);
    insert_item("acvars",&vars);
    patchbay.connect(&libname.writeaccess,this,&analysispath_if_t::loadlib);
}

void analysispath_if_t::loadlib()
{
    if( plug ){
        force_remove_item("plug");
        delete plug;
        plug = NULL;
    }
    plug = new plug_t(libname.data,chain,algo);
    try{
        if( !plug->has_process(MHA_WAVEFORM,MHA_WAVEFORM) )
            throw MHA_Error(__FILE__,__LINE__,
                            "The plugin %s does not provide waveform processing.",
                            libname.data.c_str());
        if( plug->has_parser() )
            insert_item("plug",plug);
    }
    catch(MHA_Error& e){
        force_remove_item("plug");
        delete plug;
        throw e;
    }
}

analysispath_if_t::~analysispath_if_t()
{
    if( plug )
        delete plug;
    if( acspace_template )
        delete acspace_template;
    acspace_template = NULL;
}

void analysispath_if_t::prepare(mhaconfig_t& conf)
{
    if( fragsize.data < (int)conf.fragsize )
        throw MHA_Error(__FILE__,__LINE__,
                        "Inner fragment size must be at least the same as outer fragment size (inner: %d, outer: %u)",
                        fragsize.data, conf.fragsize);
    if( !plug )
        throw MHA_ErrorMsg("No plugin was loaded.");
    if( conf.domain != MHA_WAVEFORM )
        throw MHA_ErrorMsg("Only waveform data can be processed.");
    if( acspace_template )
        throw MHA_Error(__FILE__,__LINE__,"Internal bug.");
    mhaconfig_t inner_conf = conf;
    inner_conf.fragsize = fragsize.data;
    // sugest configuration to inner plugin, query requirements:
    plug->prepare(inner_conf);
    acspace_template = new MHA_AC::acspace2matrix_t(plug->get_ac(),vars.data);
    acspace_template->insert(ac);
    // update the configuration, create an instance of the double buffer:
    push_config(new analysepath_t(conf.channels,
                                  conf.fragsize,
                                  fragsize.data,
                                  priority.data,
                                  plug->get_process_wave(),
                                  plug->get_process_spec(),
                                  plug->get_handle(),
                                  ac,
                                  *acspace_template,
                                  inner_conf.domain,
                                  fifolen.data));
}

void analysispath_if_t::release()
{
    if( plug )
        plug->release();
    if( acspace_template )
        delete acspace_template;
    acspace_template = NULL;
}

mha_wave_t* analysispath_if_t::process(mha_wave_t* s)
{
    poll_config()->rt_process(s);
    return s;
}

MHAProc_wave2wave_t plug_t::get_process_wave()
{
    return MHAProc_wave2wave_cb;
}

MHAProc_wave2spec_t plug_t::get_process_spec()
{
    return MHAProc_wave2spec_cb;
}

void* plug_t::get_handle()
{
    return lib_data;
}

plug_t::plug_t(const std::string& libname,const std::string& chain,const std::string& algo)
    : PluginLoader::mhapluginloader_t(get_c_handle(),libname)
{
}

MHAPLUGIN_CALLBACKS(analysispath,analysispath_if_t,wave,wave)
MHAPLUGIN_DOCUMENTATION\
(analysispath,
 "plugin-arrangement algorithm-communication data-flow",
 "In many signal processing scenarios, the signal analysis\n"
 "requires larger block sizes and more processing time than\n"
 "the filtering itself. If the filters do not change rapidly,\n"
 "the filter coefficients can be processed independently from\n"
 "the filter process. This is realized in this plugin: A copy\n"
 "of the input signal is stored in a double buffer, which is\n"
 "then processed asynchronously in a thread with lower priority.\n"
 "At the same time, a snapshot of the AC space (or a subset of it)\n"
 "can be transferred from the analysis thread to the main\n"
 "processing thread.\n\n"
 "\\MHAfigure{Schematic signal flow in the analysis path scenario.}"
 "{analysispath_sigflow}\n\n"
 "Please note that the AC variables which should be copied to the\n"
 "processing thread must exist after the prepare() callback and should\n"
 "not change their size during run-time."
 )

/*
 * Local variables:
 * c-basic-offset: 4
 * compile-command: "make"
 * indent-tabs-mode: nil
 * coding: utf-8-unix
 * End:
 */
