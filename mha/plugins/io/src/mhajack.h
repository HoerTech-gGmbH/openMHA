// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2008 2010 2012 2013 2016 2017 HörTech gGmbH
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

#ifndef MHAJACK_H
#define MHAJACK_H

#include <stdint.h>
#include <jack/jack.h>
#include "mha_io_ifc.h"
#include "mha_signal.hh"

#ifdef __cplusplus

#define MHAJACK_FW_STARTED 1
#define MHAJACK_STOPPED 2
#define MHAJACK_STARTING 8


#define IO_ERROR_JACK 11
#define IO_ERROR_MHAJACKLIB 12


#define MAX_USER_ERR 0x500

extern char last_jack_err_msg[MAX_USER_ERR];

/** 
        \brief Classes and functions for \mha and JACK interaction
*/
namespace MHAJack {

    /** 
        \brief Class for one channel/port
        
        This class represents one JACK port. Double buffering for
        asynchronous process callbacks is managed by this class.
        
    */
    class port_t {
    public:
        enum dir_t {
            input,
            output
        };
        /** 
            \param jc JACK client.
            \param dir Direction (input/output).
            \param id Number in port name (starting with 1).
         */
        port_t(jack_client_t* jc,dir_t dir,int id);
        /** 
            \brief Constructor to create port with specific name
            \param jc JACK client.
            \param dir Direction (input/output).
            \param id Port name.
         */
        port_t(jack_client_t* jc,dir_t dir,const std::string& id);
        ~port_t();
        /** 
            \param s Signal structure to store the audio data.
            \param ch Channel number in audio data structure to be used.
         */
        void read(mha_wave_t* s,unsigned int ch);
        /** 
            \param s Signal structure from which the audio data is read.
            \param ch Channel number in audio data structure to be used.
         */
        void write(mha_wave_t* s,unsigned int ch);
        /**  
            \param n Number of samples to be muted (must be
            the same as reported by Jack processing callback).
         */
        void mute(unsigned int n);
        /**  
            \param pn Port name to connect to
        */
        void connect_to(const char* pn);
        /**
           \brief Return the port name.
         */
        const char* get_short_name();
    private:
        dir_t dir_type;
        jack_port_t* port;
        jack_default_audio_sample_t* iob;
        jack_client_t* jc;
    };

/** 
    \brief Generic asynchronous JACK client
  
*/
    class client_t {
    public:
        client_t(IOProcessEvent_t proc_event,
                 void* proc_handle = NULL,
                 IOStartedEvent_t start_event = NULL,
                 void* start_handle = NULL,
                 IOStoppedEvent_t stop_event = NULL,
                 void* stop_handle = NULL,
                 bool use_jack_transport = false);
        void prepare(const std::string& client_name,
                     const unsigned int& nchannels_in,
                     const unsigned int& nchannels_out);
        void prepare(const std::string& server_name,
                     const std::string& client_name,
                     const unsigned int& nchannels_in,
                     const unsigned int& nchannels_out);
        void release();
        void start(bool fail_on_async_jack_error = true);
        void stop();
        void connect_input(const std::vector<std::string>&);
        void connect_output(const std::vector<std::string>&);
        unsigned int get_fragsize() const {return fragsize;};
        float get_srate() const {return samplerate;};
        unsigned long get_xruns(){return num_xruns;};
        unsigned long get_xruns_reset();
        std::string str_error(int err);
        void get_ports(std::vector<std::string>&,unsigned long jack_flags);
        std::vector<std::string> get_my_input_ports();
        std::vector<std::string> get_my_output_ports();
        void set_input_portnames(const std::vector<std::string>&);
        void set_output_portnames(const std::vector<std::string>&);
        float get_cpu_load();
        void set_use_jack_transport(bool ut){ use_jack_transport = ut;};
    private:
        void prepare_impl(const char* server_name,
                          const char* client_name,
                          const unsigned int& nchannels_in,
                          const unsigned int& nchannels_out);
        void internal_start();
        void internal_stop();
        void stopped(int,int);
        static int jack_proc_cb(jack_nframes_t,void*);
        int jack_proc_cb(jack_nframes_t);
        static int jack_xrun_cb(void*);
        int jack_xrun_cb(){num_xruns++;return 0;};
        unsigned long num_xruns;
        unsigned int fragsize;
        float samplerate;
        unsigned int nchannels_in;
        unsigned int nchannels_out;
        IOProcessEvent_t proc_event;
        void* proc_handle;
        IOStartedEvent_t start_event;
        void* start_handle;
        IOStoppedEvent_t stop_event;
        void* stop_handle;
        MHASignal::waveform_t* s_in;
        mha_wave_t* s_out;
        MHAJack::port_t** inch;
        MHAJack::port_t** outch;
    protected:
        jack_client_t* jc;
    private:
        unsigned int flags;
        bool b_prepared;
        bool use_jack_transport;
        jack_transport_state_t jstate_prev;
        std::vector<std::string> input_portnames;
        std::vector<std::string> output_portnames;
        bool fail_on_async_jackerror;
    };

/** 
    \brief Generic client for synchronous playback and recording of waveform fragments.
  
*/
    class client_noncont_t : public MHAJack::client_t {
    public:
        client_noncont_t(const std::string& name,bool use_jack_transport=false);
        void io(mha_wave_t* s_out,mha_wave_t* s_in,
                const std::vector<std::string>& p_out,
                const std::vector<std::string>& p_in,
                float* srate = NULL, unsigned int* fragsize = NULL);
    private:
        static int proc(void* handle,mha_wave_t* sIn,mha_wave_t** sOut);
        static void IOStoppedEvent(void* handle, int proc_err, int io_err);
        void proc(mha_wave_t* sIn,mha_wave_t** sOut);
        void IOStoppedEvent();
        bool b_stopped;
        unsigned int pos;
        mha_wave_t* sn_in;
        mha_wave_t* sn_out;
        std::string name;
        MHASignal::waveform_t* frag_out;
    };

/** 
    \brief Generic JACK client for averaging a system response across time
  
*/
    class client_avg_t : public MHAJack::client_t {
    public:
        client_avg_t(const std::string& name,const unsigned int& nrep_);
        void io(mha_wave_t* s_out,mha_wave_t* s_in,
                const std::vector<std::string>& p_out,
                const std::vector<std::string>& p_in,
                float* srate = NULL, unsigned int* fragsize = NULL);
    private:
        static int proc(void* handle,mha_wave_t* sIn,mha_wave_t** sOut);
        static void IOStoppedEvent(void* handle, int proc_err, int io_err);
        void proc(mha_wave_t* sIn,mha_wave_t** sOut);
        void IOStoppedEvent();
        bool b_stopped;
        unsigned int pos;
        mha_wave_t* sn_in;
        mha_wave_t* sn_out;
        std::string name;
        MHASignal::waveform_t* frag_out;
        const unsigned int nrep;
        unsigned int n;
        bool b_ready;
    };

/** 
    \brief Functional form of generic client for synchronous playback and recording of waveform fragments.
  
*/
    void io(mha_wave_t* s_out,mha_wave_t* s_in,
            const std::string& name,
            const std::vector<std::string>& p_out,
            const std::vector<std::string>& p_in,
            float* srate=NULL,
            unsigned int* fragsize=NULL,
            bool use_jack_transport=false);
        
    std::vector<unsigned int> get_port_capture_latency(const std::vector<std::string>& ports);
    std::vector<int> get_port_capture_latency_int(const std::vector<std::string>& ports);

    std::vector<unsigned int> get_port_playback_latency(const std::vector<std::string>& ports);
    std::vector<int> get_port_playback_latency_int(const std::vector<std::string>& ports);

}

#endif /* __cplusplus */
#endif /* MHAJACK_H */

/*
 * Local Variables:
 * mode: c++
 * compile-command: "make -C .."
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * coding: utf-8-unix
 * End:
 */
