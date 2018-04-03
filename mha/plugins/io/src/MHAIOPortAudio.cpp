// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2014 2015 2017 HörTech gGmbH
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
#include <portaudio.h>
#include <math.h>
#include <unistd.h>
#include <stdio.h>

#define ERR_SUCCESS 0
#define ERR_IHANDLE -1
#define ERR_USER -1000

#define MAX_USER_ERR 0x500
static char user_err_msg[MAX_USER_ERR] = "";


extern "C" PaStreamCallback portaudio_callback;


namespace MHAIOPortAudio {

  static std::string parserFriendlyName(const std::string & in)
  {
    // replace spaces with underscores
    unsigned n;
    std::string out = in;
    for (n = 0; n < in.length(); ++n)
      {
        if (in[n] <= ' ' || in[n] >= 0x7F)
          {
            out[n] = '_';
          }
      }
    return out;
  }

  using MHAParser::parser_t;

  class device_info_t : public parser_t {
  public:
    MHAParser::int_mon_t numDevices;
    MHAParser::vint_mon_t structVersion;
    MHAParser::vstring_mon_t name;
    MHAParser::vint_mon_t hostApi;
    MHAParser::vint_mon_t maxInputChannels;
    MHAParser::vint_mon_t maxOutputChannels;
    MHAParser::vfloat_mon_t defaultLowInputLatency;
    MHAParser::vfloat_mon_t defaultLowOutputLatency;
    MHAParser::vfloat_mon_t defaultHighInputLatency;
    MHAParser::vfloat_mon_t defaultHighOutputLatency;
    MHAParser::vfloat_mon_t defaultSampleRate;

    device_info_t() : 
      parser_t("PortAudio's information about the sound devices"
               " present on this system"),
      numDevices("Number of sound devices on system as recognized by PortAudio v19"),
      structVersion("PortAudio has no documentation for this field"),
      name("PortAudio has no documentation for this field"),
      hostApi("The type used to enumerate to host APIs at runtime. Values of this type range from 0 to (Pa_GetHostApiCount()-1)."),
      maxInputChannels("PortAudio has no documentation for this field"),
      maxOutputChannels("PortAudio has no documentation for this field"),
      defaultLowInputLatency("Default latency values for interactive performance. Time values in seconds."),
      defaultLowOutputLatency("PortAudio has no documentation for this field"),
      defaultHighInputLatency("Default latency values for robust non-interactive applications.\nTime values in seconds. "),
      defaultHighOutputLatency("PortAudio has no documentation for this field"),
      defaultSampleRate("PortAudio has no documentation for this field")
    {
      insert_member(numDevices);
      insert_member(structVersion);
      insert_member(name);
      insert_member(hostApi);
      insert_member(maxInputChannels);
      insert_member(maxOutputChannels);
      insert_member(defaultLowInputLatency);
      insert_member(defaultLowOutputLatency);
      insert_member(defaultHighInputLatency);
      insert_member(defaultHighOutputLatency);
      insert_member(defaultSampleRate);
    }
    void fill_info() {
      int old_numDevices = numDevices.data;
      numDevices.data = Pa_GetDeviceCount();
      if (numDevices.data < 0) {
        int err = numDevices.data;
        numDevices.data = old_numDevices;
        throw MHA_Error(__FILE__,__LINE__,
                        "Could not get number of portaudio sound devices: %s",
                        Pa_GetErrorText(err));
      }
      if (numDevices.data == 0)
        throw MHA_Error(__FILE__,__LINE__,
                        "Could not find any portaudio sound devices");

      structVersion.data.clear();
      name.data.clear();
      hostApi.data.clear();
      maxInputChannels.data.clear();
      maxOutputChannels.data.clear();
      defaultLowInputLatency.data.clear();
      defaultLowOutputLatency.data.clear();
      defaultHighInputLatency.data.clear();
      defaultHighOutputLatency.data.clear();
      defaultSampleRate.data.clear();

      structVersion.data.resize(numDevices.data,-1);
      name.data.resize(numDevices.data);
      hostApi.data.resize(numDevices.data,-1);
      maxInputChannels.data.resize(numDevices.data,-1);
      maxOutputChannels.data.resize(numDevices.data,-1);
      defaultLowInputLatency.data.resize(numDevices.data,INFINITY);
      defaultLowOutputLatency.data.resize(numDevices.data,INFINITY);
      defaultHighInputLatency.data.resize(numDevices.data,INFINITY);
      defaultHighOutputLatency.data.resize(numDevices.data,INFINITY);
      defaultSampleRate.data.resize(numDevices.data,-1);

      for (int i = 0; i < numDevices.data; ++i) {
        const PaDeviceInfo *device_info = Pa_GetDeviceInfo(i);
        if (device_info) {
          structVersion.data[i] = device_info->structVersion;
          name.data[i] = parserFriendlyName(device_info->name);
          hostApi.data[i] = device_info->hostApi;
          maxInputChannels.data[i] = device_info->maxInputChannels;
          maxOutputChannels.data[i] = device_info->maxOutputChannels;
          defaultLowInputLatency.data[i] = device_info->defaultLowInputLatency;
          defaultLowOutputLatency.data[i]=device_info->defaultLowOutputLatency;
          defaultHighInputLatency.data[i]=device_info->defaultHighInputLatency;
          defaultHighOutputLatency.data[i] =
            device_info->defaultHighOutputLatency;
          defaultSampleRate.data[i] = device_info->defaultSampleRate;
        }
      }
    }
  };


  /** \internal
      \brief Main class for Portaudio sound IO
  */
  class io_portaudio_t : public MHAParser::parser_t
  {
  public:
    io_portaudio_t(unsigned int fragsize, 
                   float samplerate,
                   IOProcessEvent_t proc_event,
                   void* proc_handle,
                   IOStartedEvent_t start_event,
                   void* start_handle,
                   IOStoppedEvent_t stop_event,
                   void* stop_handle)
      : MHAParser::parser_t("MHA IO library for portaudio V19 backend"),
        device_name("Variable to load device by name.  This name has to"
                    " match the portaudio device\nname, exactly or as a"
                    " substring.  Exact matches take precedence over\nsubstring"
                    " matches.  Thereafter, matches at lower device indices are"
                    " preferred.\ndevice_index will be updated when a match is"
                    " found.",
                    "default"),
        device_index("Variable to load device by index.  Upon setting"
                     " device_index,\ndevice_name will be"
                     " updated to the full portaudio name of this device.",
                     "0", "[0,32767]")
    {
      if (proc_event == NULL || start_event == NULL || stop_event == NULL)
        throw MHA_Error(__FILE__,__LINE__,
                        "BUG: discovered by io_portaudio_t: proc_event,"
                        " start_event, stop_event all need to be valid"
                        " function pointers, but at least one of them is"
                        " NULL");
      this->fragsize = fragsize;
      nchannels_in = nchannels_out = 0U;
      this->samplerate = samplerate;
      this->proc_event = proc_event;
      this->proc_handle = proc_handle;
      this->start_event = start_event;
      this->start_handle = start_handle;
      this->stop_event = stop_event;
      this->stop_handle = stop_handle;
      s_out = s_in = NULL;
      portaudio_stream = NULL;
      
      PaError err = Pa_Initialize();
      if (err != paNoError)
        throw MHA_Error(__FILE__,__LINE__,
                        "Could not initialize portaudio: %s",
                        Pa_GetErrorText(err));
      device_info.fill_info();
      insert_member(device_info);
      insert_member(device_name);
      insert_member(device_index);
      device_index.set_range("[0," + MHAParser::StrCnv::
                             val2str(device_info.numDevices.data) + "[");
      try {
        device_name_updated();
      } catch (MHA_Error & e) {
        device_index_updated();
      }
      patchbay.connect(&device_name.writeaccess, this,
                       &io_portaudio_t::device_name_updated);
      patchbay.connect(&device_index.writeaccess, this,
                       &io_portaudio_t::device_index_updated);
    }
    
    void device_name_updated() {
      for (size_t i = 0; i < device_info.name.data.size(); ++i)
        if (device_info.name.data[i] == device_name.data) {
          device_index.data = i;
          return;
        }
      for (size_t i = 0; i < device_info.name.data.size(); ++i)
        if (device_info.name.data[i].find(device_name.data) !=
            std::string::npos) {
          device_index.data = i;
          return;
        }
      throw MHA_Error(__FILE__,__LINE__, "No portaudio device name contains"
                      " \"%s\". Valid portaudio device names are: %s",
                      device_name.data.c_str(),
                      MHAParser::StrCnv::
                      val2str(device_info.name.data).c_str());
    }

    void device_index_updated() {
      if ((device_index.data < 0) ||
           (device_index.data > device_info.numDevices.data))
        throw MHA_Error(__FILE__,__LINE__, "Device index %d is out of range",
                        device_index.data);
      device_name.data = device_info.name.data[device_index.data];
    }

    ~io_portaudio_t()
    {
      PaError err = Pa_Terminate();
      if (err != paNoError)
        fprintf(stderr, "Could not terminate portaudio: %s",
                Pa_GetErrorText(err));
    }

    void cmd_prepare(int,int);
    void cmd_start();
    void cmd_stop();
    void cmd_release();
    int portaudio_callback(const void * input, void * output,
                           unsigned long frame_count,
                           const PaStreamCallbackTimeInfo * time_info,
                           PaStreamCallbackFlags status_flags);
  private:
    device_info_t device_info;
    // The input signal provided by portaudio needs to be copied into this
    // as portaudio callback signature declares the values const
    MHASignal::waveform_t * s_in;
    // The output signal is provided by MHA processing
    mha_wave_t *s_out;
    float samplerate;
    unsigned int nchannels_out, nchannels_in, fragsize;
    IOProcessEvent_t proc_event;
    void* proc_handle;
    IOStartedEvent_t start_event;
    void* start_handle;
    IOStoppedEvent_t stop_event;
    void* stop_handle;
    
    // PortAudio stream handle
    PaStream * portaudio_stream;

    MHAParser::string_t device_name;
    MHAParser::int_t device_index;
    MHAEvents::patchbay_t<io_portaudio_t> patchbay;
  };

}
extern "C"  int portaudio_callback(const void *input, void *output,
                                   unsigned long frameCount,
                                   const PaStreamCallbackTimeInfo* timeInfo,
                                   PaStreamCallbackFlags statusFlags,
                                   void *userData )
{
  return ((MHAIOPortAudio::io_portaudio_t*)userData)->
    portaudio_callback(input, output, frameCount, timeInfo, statusFlags);
}

int MHAIOPortAudio::io_portaudio_t::portaudio_callback
(const void * input, void * output,
 unsigned long frame_count,
 const PaStreamCallbackTimeInfo * time_info,
 PaStreamCallbackFlags status_flags)
{
  start_event(start_handle);
  memcpy(s_in->buf, input, sizeof(mha_real_t) * fragsize * nchannels_in);
  int err = proc_event(proc_handle, s_in, &s_out);
  if (err) {
    memset(output,0,sizeof(mha_real_t)*fragsize*nchannels_out);
    stop_event(stop_handle,err,0);
    return paAbort;
  }
  if (fragsize != s_out->num_frames || nchannels_out != s_out->num_channels)  {
    MHA_Error e(__FILE__,__LINE__,
                "BUG: mha process callback returned unexpected"
                " fragsize (%u) or channel count (%u). Expected %u,%u.",
                s_out->num_frames, s_out->num_channels,
                fragsize, nchannels_out);
    strncpy(user_err_msg, Getmsg(e), MAX_USER_ERR-1);
    user_err_msg[MAX_USER_ERR-1] = 0;
    stop_event(stop_handle,0,ERR_USER);
    memset(output,0,sizeof(mha_real_t)*fragsize*nchannels_out);
    return paAbort;
  }
  memcpy(output,s_out->buf,sizeof(mha_real_t)*fragsize*nchannels_out);
  return paContinue;
}

void MHAIOPortAudio::io_portaudio_t::cmd_prepare(int nchannels_in,
                                                 int nchannels_out)
{
  if (nchannels_in < 0)
    throw MHA_Error(__FILE__,__LINE__,
                    "prepare: nonsense input channel count %d received",
                    nchannels_in);
  if (nchannels_out < 0)
    throw MHA_Error(__FILE__,__LINE__,
                    "prepare: nonsense output channel count %d received",
                    nchannels_out);
  if (nchannels_in == 0 && nchannels_out == 0)
    throw MHA_Error(__FILE__,__LINE__,
                    "prepare called with 0 input channels and, at the same"
                    " time, 0 output channels.  This is note permitted in"
                    " portaudio");
  this->nchannels_in = nchannels_in;
  this->nchannels_out = nchannels_out;

  PaStreamParameters outpar, inpar;
  memset(&outpar, 0, sizeof(outpar));  memset(&inpar, 0, sizeof(inpar));

  outpar.channelCount = nchannels_out;
  inpar.channelCount = nchannels_in;
  outpar.device = inpar.device = device_index.data;
  outpar.sampleFormat = inpar.sampleFormat = paFloat32;

  PaError err = Pa_OpenStream(&portaudio_stream,
                              &inpar, &outpar,
                              samplerate,
                              fragsize,
                              paNoFlag,
                              ::portaudio_callback,
                              this);
  if (err != paNoError)
    throw MHA_Error(__FILE__,__LINE__,
                    "Could not open portaudio stream: %s",
                    Pa_GetErrorText(err));
  s_in = new MHASignal::waveform_t(fragsize, nchannels_in);

  device_name.setlock(true);
  device_index.setlock(true);
}

void MHAIOPortAudio::io_portaudio_t::cmd_start()
{
  PaError err = Pa_StartStream(portaudio_stream);
  if (err != paNoError)
    throw MHA_Error(__FILE__,__LINE__,
                    "Could not start the portaudio stream: %s",
                    Pa_GetErrorText(err));
}

void MHAIOPortAudio::io_portaudio_t::cmd_stop()
{
  PaError err = Pa_AbortStream(portaudio_stream);
  if (err != paNoError)
    fprintf(stderr, "Could not stop the portaudio stream: %s",
            Pa_GetErrorText(err));
  stop_event(stop_handle,0,0);
}

void MHAIOPortAudio::io_portaudio_t::cmd_release()
{
  PaError err = Pa_CloseStream(portaudio_stream);
  if (err != paNoError)
    fprintf(stderr, "Could not stop the portaudio stream: %s",
            Pa_GetErrorText(err));
  delete s_in;
  s_out = s_in = 0;
  device_name.setlock(false);
  device_index.setlock(false);
}


extern "C" {
#ifdef MHA_STATIC_PLUGINS
#define IOInit               MHA_STATIC_MHAIOPortAudio_IOInit
#define IOPrepare            MHA_STATIC_MHAIOPortAudio_IOPrepare
#define IOStart              MHA_STATIC_MHAIOPortAudio_IOStart
#define IOStop               MHA_STATIC_MHAIOPortAudio_IOStop
#define IORelease            MHA_STATIC_MHAIOPortAudio_IORelease
#define IOSetVar             MHA_STATIC_MHAIOPortAudio_IOSetVar
#define IOStrError           MHA_STATIC_MHAIOPortAudio_IOStrError
#define IODestroy            MHA_STATIC_MHAIOPortAudio_IODestroy
#define dummy_interface_test MHA_STATIC_MHAIOPortAudio_dummy_interface_test
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
      MHAIOPortAudio::io_portaudio_t* cl =
        new MHAIOPortAudio::io_portaudio_t(fragsize,samplerate,
                                           proc_event,proc_handle,
                                           start_event,start_handle,
                                           stop_event,stop_handle);
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
      ((MHAIOPortAudio::io_portaudio_t*)handle)->cmd_prepare(nch_in,nch_out);
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
      ((MHAIOPortAudio::io_portaudio_t*)handle)->cmd_start();
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
      ((MHAIOPortAudio::io_portaudio_t*)handle)->cmd_stop();
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
      ((MHAIOPortAudio::io_portaudio_t*)handle)->cmd_release();
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
      ((MHAIOPortAudio::io_portaudio_t*)handle)->parse(command,
                                                       retval,
                                                       maxretlen);
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
    delete ((MHAIOPortAudio::io_portaudio_t*)handle);
  }

  void dummy_interface_test(void){
#ifdef MHA_STATIC_PLUGINS
    MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOPortAudio_,IOInit);
    MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOPortAudio_,IOPrepare);
    MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOPortAudio_,IOStart);
    MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOPortAudio_,IOStop);
    MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOPortAudio_,IORelease);
    MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOPortAudio_,IOSetVar);
    MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOPortAudio_,IOStrError);
    MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOPortAudio_,IODestroy);
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
 * c-basic-offset: 2
 * End:
 */
