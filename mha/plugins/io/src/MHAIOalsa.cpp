// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2008 2009 2010 2011 2013 2014 2015 2018 2019 HörTech gGmbH
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

#include "mha_io_ifc.h"
#include "mha_toolbox.h"
#include "mha_signal.hh"
#include "mha_events.h"
#include <math.h>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <limits>

#define DBG(x) fprintf(stderr,"%s:%d\n",__FILE__,__LINE__)

//MHAIOalsa can at the moment only handle little endian. NOTE: If we cross compile for big endian system, this check will also fail
#ifdef __BYTE_ORDER__
static_assert(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__,"MHAIOalsa only works on little endian systems");
#else
static_assert(false, "Can not check for byte order")
#endif


class alsa_base_t {
public:
    alsa_base_t():pcm(nullptr){};
    virtual ~alsa_base_t()=default;
    /** start puts alsa device in usable state */
    virtual void start()=0;
    /** stop informs alsa device that we do not need any more samples
     * / will not provide any more samples */
    virtual void stop()=0;
    /** read audio samples from the device into an internal mha_wave_t
     * buffer, then update the pointer given as parameter to point to
     * the internal structure.  Converts sound samples from the integer
     * data type provided by the sound card to floating-point values
     * needed by the MHA in the range [-1.0,1.0] */
    virtual bool read(mha_wave_t**)=0;
    /** write audio samples from the given waveform buffer to the sound device.
     * converts the floating point values coming from the MHA to the integer
     * samples required by the sound card.*/
    virtual bool write(mha_wave_t*)=0;
    /** The underlying alsa handle to this sound card. */
    snd_pcm_t* pcm;

};

/** Parser variables corresponding to one alsa device.  ALSA separates
 * audio capture and audio playback into two different devices that have
 * to be opened separately. This class encapsulates the parser
 * variables that pertain to one such direction.
 */
class alsa_dev_par_parser_t : public MHAParser::parser_t
{
public:
    /** Constructor inserts the parser variables into this sub-parser.
     * @param stream_dir capture or playback
     */
    alsa_dev_par_parser_t(snd_pcm_stream_t stream_dir);
    /** Name of the device in the alsa world, like "hw:0.0", "default", etc */
    MHAParser::string_t device;

    /** Number of buffers of fragsize to hold in the alsa buffer.
     * Usually 2, the minimum possible. */
    MHAParser::int_t nperiods;
    /** Remember the direction (capture/playback) of this device */
    snd_pcm_stream_t stream_dir;
};

/** Our representation of one alsa device.  We can start and stop the device,
 * and depending on the direction, read or write samples. */
template <typename T>
class alsa_t : public alsa_base_t
{
public:
    /** Constructor receives the parameters for this device.  It opens
     * the sound device using the alsa library and selects the given
     * parameters, but does not yet start the sound device to perform
     * real I/O.
     * @param par      our parser variable aggregator (containing direction,
     *                 device name, and number of periods to place in alsa
     *                 buffer)
     * @param rate     sampling rate in Hz
     * @param fragsize samples per block per channel
     * @param channels number of audio channels to open
     */
    alsa_t(const alsa_dev_par_parser_t& par,
           unsigned int rate,
           unsigned int fragsize,
           unsigned int channels);
    /** Destructor closes the sound device. */
    ~alsa_t();
    /** start puts alsa device in usable state */
    void start() override;
    /** stop informs alsa device that we do not need any more samples
     * / will not provide any more samples */
    void stop() override;
    /** read audio samples from the device into an internal mha_wave_t
     * buffer, then update the pointer given as parameter to point to
     * the internal structure.  Converts sound samples from the integer
     * data type provided by the sound card to floating-point values
     * needed by the MHA in the range [-1.0,1.0] */
    bool read(mha_wave_t**) override;
    /** write audio samples from the given waveform buffer to the sound device.
     * converts the floating point values coming from the MHA to the integer
     * samples required by the sound card.*/
    bool write(mha_wave_t*) override;

private:
    unsigned int channels;
    unsigned int fragsize;
    T * buffer;
    std::vector<mha_real_t> frame_data;
    /** internal buffer to store sound samples coming from the sound card. */
    MHASignal::waveform_t wave;
    const mha_real_t gain;
    const mha_real_t invgain;
    const mha_real_t val_min, val_max;
    snd_pcm_format_t pcm_format;
};

template <typename T>
bool alsa_t<T>::read(mha_wave_t** s)
{
    unsigned k, ch;
    snd_pcm_sframes_t cnt;
    *s = &wave;
    clear(wave);
    cnt = snd_pcm_readi(pcm,buffer,fragsize);
    if (cnt == -EPIPE or (cnt > 0 and cnt < static_cast<int>(fragsize))){
        return false;
    }
    if( cnt < 0 )
        throw MHA_Error(__FILE__,__LINE__,
                        "Read failed: %s",snd_strerror(cnt));
    for(k=0;k<fragsize;k++)
        for(ch=0;ch<channels;ch++)
            wave(k,ch) = gain * buffer[channels*k+ch];
    return true;
}

template <typename T>
bool alsa_t<T>::write(mha_wave_t* s)
{
    unsigned int k, ch;
    snd_pcm_sframes_t cnt;
    mha_real_t val;
    if (s)
        for(k=0;k<fragsize;k++)
            for(ch=0;ch<channels;ch++){
                val = invgain * value(s,k,ch);
                if (isnanf(val))
                    val = 0.0f;
                else if( val < val_min )
                    val = val_min;
                else if( val > val_max )
                    val = val_max;
                buffer[channels*k+ch] = static_cast<T>(val);
            }
    else
        for(k=0;k<fragsize;k++)
            for(ch=0;ch<channels;ch++)
                buffer[channels*k+ch] = 0;
    cnt = snd_pcm_writei(pcm,buffer,fragsize);
    if (cnt == -EPIPE or (cnt > 0 and cnt < (int)fragsize) ) {
        return false;
    }
    if (cnt < 0)
        throw MHA_Error(__FILE__,__LINE__,
                        "Write failed: %s",snd_strerror(cnt));
    return true;
}
template <typename T>
void alsa_t<T>::start()
{
    int err;
    if( (err = snd_pcm_start(pcm) ) < 0 )
        throw MHA_Error(__FILE__,__LINE__,"Unable to start device: %s",
                        snd_strerror(err));
}
template <typename T>
void alsa_t<T>::stop()
{
    int err;
    if( (err = snd_pcm_drop(pcm) ) < 0 )
        throw MHA_Error(__FILE__,__LINE__,"Unable to stop device: %s",
                        snd_strerror(err));
}
template <typename T>
alsa_t<T>::alsa_t(const alsa_dev_par_parser_t& par,
               unsigned int rate,
               unsigned int fragsize_,
               unsigned int channels_)
    : alsa_base_t(),
      channels(channels_),
      fragsize(fragsize_),
      buffer(nullptr),
      frame_data(channels,0),
      wave(fragsize,channels),
      gain(-1.0f/std::numeric_limits<T>::min()),
      invgain(1.0f/gain),
      val_min(std::numeric_limits<T>::min()),
      val_max(std::numeric_limits<T>::max())
{
    if( std::is_same<T,int32_t>::value) {
        pcm_format=SND_PCM_FORMAT_S32_LE;
    }
    else if( std::is_same<T,int16_t>::value) {
        pcm_format=SND_PCM_FORMAT_S16_LE;
    }
    else {
        throw MHA_Error(__FILE__,__LINE__,"Unknown PCM sample format");
    }
    int err;
    if( (err = snd_pcm_open(&pcm,par.device.data.c_str(),par.stream_dir,0)) < 0 )
        throw MHA_Error(__FILE__,__LINE__,
                        "Unable to open device \"%s\" for %s: %s",
                        par.device.data.c_str(),
                        (par.stream_dir==SND_PCM_STREAM_CAPTURE)?"capture":"playback",
                        snd_strerror(err));
    try{
        snd_pcm_hw_params_t* params;
        snd_pcm_hw_params_malloc(&params);
        try{
            if( (err = snd_pcm_hw_params_any(pcm,params)) < 0 )
                throw MHA_Error(__FILE__,__LINE__,
                                "Unable to fill hw params for %s: %s\n",
                                par.device.data.c_str(), snd_strerror(err));
            if( (err = snd_pcm_hw_params_set_channels(
                                                      pcm,params,
                                                      channels)) < 0 )
                throw MHA_Error(__FILE__,__LINE__,
                                "Unable to set channel number for %s: %s",
                                par.device.data.c_str(), snd_strerror(err));
            if( (err = snd_pcm_hw_params_set_rate(pcm,params,rate,0)) < 0 )
                throw MHA_Error(__FILE__,__LINE__,
                                "The device %s does not support %u Hz sampling rate: %s",
                                par.device.data.c_str(),rate,snd_strerror(err));
            if( (err = snd_pcm_hw_params_set_period_size(pcm,params,fragsize,0)) < 0 )
                throw MHA_Error(__FILE__,__LINE__,
                                "Unable to set %s to period size to %u: %s",
                                par.device.data.c_str(),fragsize,snd_strerror(err));
            if( (err = snd_pcm_hw_params_set_format(pcm,params,pcm_format)) < 0 )
                throw MHA_Error(__FILE__,__LINE__,
                                "Unable to set sample format on %s to %i bits signed int: %s",
                                par.device.data.c_str(),
                                pcm_format==SND_PCM_FORMAT_S16_LE ? 16 : 32,
                                snd_strerror(err));
            if( (err = snd_pcm_hw_params_set_access(pcm,params,SND_PCM_ACCESS_RW_INTERLEAVED))<0)
                throw MHA_Error(__FILE__,__LINE__,
                                "Unable to set access mode (%s): %s",
                                par.device.data.c_str(),snd_strerror(err));
            {
                unsigned int actual_periods = par.nperiods.data;
                int direction = 0;
                if( (err = snd_pcm_hw_params_set_periods_min(pcm,params,&actual_periods,&direction))<0)
                    throw MHA_Error(__FILE__,__LINE__,
                                    "Unable to set nperiods to %d (%s): %s",
                                    par.nperiods.data, par.device.data.c_str(),snd_strerror(err));
                if( (err = snd_pcm_hw_params_set_periods_first(pcm,params, &actual_periods,&direction))<0)
                    throw MHA_Error(__FILE__,__LINE__,
                                    "Unable to set nperiods to %d (actual %u|%d) (%s): %s",
                                    par.nperiods.data, actual_periods, direction, par.device.data.c_str(),snd_strerror(err));
                if (int(actual_periods) != par.nperiods.data)
                    printf("Info: Configuring alsa device with %u periods but using only %d. Configuring with %d is not supported\n", actual_periods,  par.nperiods.data,  par.nperiods.data);
            }

            if( (err = snd_pcm_hw_params(pcm, params)) < 0 )
                throw MHA_Error(__FILE__,__LINE__,
                                "Unable to set hw params for %s: %s\n",
                                par.device.data.c_str(), snd_strerror(err));
            snd_pcm_hw_params_free( params );
        }
        catch(...){
            snd_pcm_hw_params_free( params );
            throw;
        }
        buffer = new T[channels*fragsize];
    }
    catch( ... ){
        snd_pcm_close(pcm);
        throw;
    }
}
template <typename T>
alsa_t<T>::~alsa_t()
{
    snd_pcm_close( pcm );
    delete [] buffer;
    buffer = 0;
}

/**
    \brief MHA IO interface class for ALSA IO
*/
class io_alsa_t : public MHAParser::parser_t
{
public:
    /** Constructor, receives the callback handles to interact with
     * the MHA framework. */
    io_alsa_t(unsigned int fragsize,
              float samplerate,
              IOProcessEvent_t proc_event,
              void* proc_handle,
              IOStartedEvent_t start_event,
              void* start_handle,
              IOStoppedEvent_t stop_event,
              void* stop_handle);
    /** Called after the framework has perpared the processing plugins
     * and the number of input and output channels are fixed. */
    template<typename T=void>
    void prepare(int,int);

    /** MHA framework leaves prepared state. */
    void release();

    /** MHA framework calls this function when signal processing should start. */
    void start();

    /** MHA framework calls this function when signal processing should stop. */
    void stop();

    /** MHAIOAlsa uses a separate thread that calls the alsa
     * read and write functions to read and write audio samples, these
     * functions are blocking until samples can be read or written.
     * This is the start function of that thread. */
    static void* thread_start(void* h);
private:
    void process();
    bool b_process;
    unsigned int fw_fragsize;
    unsigned int fw_samplerate;
    IOProcessEvent_t proc_event;
    void* proc_handle;
    IOStartedEvent_t start_event;
    void* start_handle;
    IOStoppedEvent_t stop_event;
    void* stop_handle;
    alsa_base_t* dev_in;
    alsa_base_t* dev_out;
    pthread_t proc_thread;
    alsa_dev_par_parser_t p_in;
    alsa_dev_par_parser_t p_out;
    MHAParser::bool_t pcmlink;
    MHAParser::int_t priority;
    MHAParser::kw_t format;
    MHAParser::int_mon_t alsa_start_counter;
    MHAEvents::patchbay_t<io_alsa_t> patchbay;
};

void* io_alsa_t::thread_start(void* h)
{
    ((io_alsa_t*)h)->process();
    return NULL;
}

void io_alsa_t::process()
{
    mha_wave_t* s = 0;
    bool devices_running = false;
    try{
        if( start_event )
            start_event(start_handle);
        while( b_process ){
            if (devices_running == false) {
                snd_pcm_drop(dev_in->pcm);
                snd_pcm_drop(dev_out->pcm);
                snd_pcm_prepare(dev_in->pcm);
                snd_pcm_prepare(dev_out->pcm);
                for (int i = 0; i < p_out.nperiods.data; ++i)
                    dev_out->write(0);
                alsa_start_counter.data++;
                devices_running = true;
            }
            if (!dev_in->read(&s)) {
                devices_running = false;
                continue;
            }
            if( b_process ){
                proc_event(proc_handle,s,&s);
                if (!dev_out->write(s)){
                    devices_running = false;
                }
            }
        }
        if( stop_event ){
            stop_event(stop_handle,0,0);
        }
    }
    catch( MHA_Error& e){
        b_process = false;
        if( stop_event )
            stop_event(stop_handle,0,1);
        std::cerr << Getmsg(e) << std::endl;
    }
    dev_in->stop();
    dev_out->stop();
}

void io_alsa_t::start()
{
    b_process = true;
    pthread_create(&proc_thread,NULL,&thread_start,this);
    if( priority.data >= 0 ){
        struct sched_param sp;
        sp.sched_priority = priority.data;
        pthread_setschedparam( proc_thread, SCHED_FIFO, &sp);
    }
}

void io_alsa_t::stop()
{
    b_process = false;
    pthread_join(proc_thread,NULL);
}

/** \brief open pcm streams */
template<typename T>
void io_alsa_t::prepare(int nch_in,int nch_out)
{
    int err;
        dev_in = new alsa_t<T>(p_in,fw_samplerate,fw_fragsize,nch_in);
        try{
            dev_out = new alsa_t<T>(p_out,fw_samplerate,fw_fragsize,nch_out);
            try{
                if( pcmlink.data ){
                    if( (err = snd_pcm_link( dev_in->pcm, dev_out->pcm ) ) < 0 )
                        throw MHA_Error(__FILE__,__LINE__,
                                        "Unable to link PCMs: %s",
                                        snd_strerror(err));
                }
            }
            catch(...){
                delete dev_out;
                throw;
            }
        }
        catch(...){
            delete dev_in;
            throw;
        }
}

template<>
void io_alsa_t::prepare<void>(int nch_in,int nch_out)
{
    switch(format.data.get_index()){
    case 0:
        prepare<int32_t>(nch_in,nch_out);
        break;
    case 1:
        prepare<int16_t>(nch_in,nch_out);
        break;
    default:
        throw MHA_Error(__FILE__,__LINE__,"Unknown pcm format: %zu",format.data.get_index());
        break;
    }
}

void io_alsa_t::release()
{
    delete dev_in;
    delete dev_out;
}

io_alsa_t::io_alsa_t(unsigned int ifragsize,
                     float isamplerate,
                     IOProcessEvent_t iproc_event,
                     void* iproc_handle,
                     IOStartedEvent_t istart_event,
                     void* istart_handle,
                     IOStoppedEvent_t istop_event,
                     void* istop_handle)
    : MHAParser::parser_t("ALSA client"),
    b_process(false),
    fw_fragsize(ifragsize),
    fw_samplerate((unsigned int)isamplerate),
    proc_event(iproc_event),
    proc_handle(iproc_handle),
    start_event(istart_event),
    start_handle(istart_handle),
    stop_event(istop_event),
    stop_handle(istop_handle),
    p_in(SND_PCM_STREAM_CAPTURE),
    p_out(SND_PCM_STREAM_PLAYBACK),
    pcmlink("link PCM devices","yes"),
    priority("Set SCHED_FIFO priority of processing thread\n"
             "or -1 for no realtime scheduling","-1","[-1,99]"),
    format("PCM sample format","S32_LE","[S32_LE S16_LE]"),
    alsa_start_counter("alsa is started on startup and for every dropout.")
{
    insert_item("in",&p_in);
    insert_item("out",&p_out);
    insert_item("priority",&priority);
    insert_item("format",&format);
    insert_item("link",&pcmlink);
    insert_member(alsa_start_counter);
    alsa_start_counter.data = 0;
}

alsa_dev_par_parser_t::alsa_dev_par_parser_t(snd_pcm_stream_t _stream_dir)
    : device(std::string("device name of the alsa PCM device for audio ") +
             (_stream_dir == SND_PCM_STREAM_PLAYBACK ? "playback" : "capture"),
             ""),
      nperiods("number of periods in alsa buffer","2","[2,]"),
      stream_dir(_stream_dir)
{
    insert_member(device);
    insert_member(nperiods);
}


#define ERR_SUCCESS 0
#define ERR_IHANDLE -1
#define ERR_USER -1000

#define MAX_USER_ERR 0x500
static char user_err_msg[MAX_USER_ERR];

extern "C" {

#ifdef MHA_STATIC_PLUGINS
#define IOInit               MHA_STATIC_MHAIOalsa_IOInit
#define IOPrepare            MHA_STATIC_MHAIOalsa_IOPrepare
#define IOStart              MHA_STATIC_MHAIOalsa_IOStart
#define IOStop               MHA_STATIC_MHAIOalsa_IOStop
#define IORelease            MHA_STATIC_MHAIOalsa_IORelease
#define IOSetVar             MHA_STATIC_MHAIOalsa_IOSetVar
#define IOStrError           MHA_STATIC_MHAIOalsa_IOStrError
#define IODestroy            MHA_STATIC_MHAIOalsa_IODestroy
#define dummy_interface_test MHA_STATIC_MHAIOalsa_dummy_interface_test
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

    // The following functions are part of the API that every MHA IO
    // library exposes.  These functions are called by the MHA framework.

    /** IO library initialization function, called by framework after
     * loading this IO library into the MHA process.  Gives plugin
     * callback functions and callback handles to interact with the
     * MHA framework.
     * @param handle output parameter. IO library returns pointer to
     *               void to the caller via this parameter.  All other
     *               function calls from the MHA framework will use
     *               this handle. */
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
            io_alsa_t* cl = new io_alsa_t(fragsize,samplerate,proc_event,proc_handle,start_event,start_handle,stop_event,stop_handle);
            *handle = (void*)cl;
            return ERR_SUCCESS;
        }
        catch( MHA_Error& e ){
            strncpy( user_err_msg, Getmsg(e), MAX_USER_ERR-1 );
            user_err_msg[MAX_USER_ERR-1] = 0;
            return ERR_USER;
        }
    }

    /** IO library prepare function, called after the MHA prepared the
     * processing plugins. */
    int IOPrepare(void* handle,int nch_in,int nch_out){
        if( !handle )
            return ERR_IHANDLE;
        try{
            ((io_alsa_t*)handle)->prepare(nch_in,nch_out);
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
            ((io_alsa_t*)handle)->start();
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
            ((io_alsa_t*)handle)->stop();
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
            ((io_alsa_t*)handle)->release();
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
            ((io_alsa_t*)handle)->parse(command,retval,maxretlen);
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
        delete (io_alsa_t*)handle;
    }

    void dummy_interface_test(void){
#ifdef MHA_STATIC_PLUGINS
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOalsa_,IOInit);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOalsa_,IOPrepare);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOalsa_,IOStart);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOalsa_,IOStop);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOalsa_,IORelease);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOalsa_,IOSetVar);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOalsa_,IOStrError);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOalsa_,IODestroy);
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
 * End:
 */
