// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2008 2009 2010 2012 2013 2014 2016 HörTech gGmbH
// Copyright © 2017 2020 HörTech gGmbH
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

#include <stdio.h>
#include "mhajack.h"
#include "mha_error.hh"
#include "mha_signal.hh"
#include <math.h>
#include "mha_defs.h"
#include <unistd.h>

char last_jack_err_msg[MAX_USER_ERR] = "";
int last_jack_err = 0;

static void jack_error_handler(const char* msg)
{
    strncpy(last_jack_err_msg,msg,MAX_USER_ERR-1);
    last_jack_err = IO_ERROR_JACK;
}

static int dummy_jack_proc_cb(jack_nframes_t,void*)
{
    return 0;
}

/** 
    \brief Return the JACK port latency of ports
  
    \param ports        Ports to be tested
    \return             Latency vector (one entry for each port)
*/
std::vector<unsigned int> MHAJack::get_port_capture_latency(const std::vector<std::string>& ports)
{
    std::vector<unsigned int> retv;
    jack_client_t* jc;
    jc = jack_client_open("MHAJack__get_port_latency",JackNullOption,NULL);
    if( !jc)
        throw MHA_Error(__FILE__,__LINE__,"Unable to connect to JACK.");
    try{
        for(unsigned int k=0;k<ports.size();k++){
            jack_port_t* jp = NULL;
            jp = jack_port_by_name(jc,ports[k].c_str());
            if( !jp )
                throw MHA_Error(__FILE__,__LINE__,
                                "Invalid port name: \"%s\"",
                                ports[k].c_str());
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 3)
            jack_latency_range_t lat_rg;
            jack_port_get_latency_range( jp, JackCaptureLatency, &lat_rg);
            retv.push_back(lat_rg.min);
#else
            retv.push_back(0);
#endif
        }
        jack_client_close(jc);
    }
    catch(...){
        jack_client_close(jc);
        throw;
    }
    return retv;
}

/** 
    \brief Return the JACK port latency of ports
  
    \param ports        Ports to be tested
    \return             Latency vector (one entry for each port)
*/
std::vector<unsigned int> MHAJack::get_port_playback_latency(const std::vector<std::string>& ports)
{
    std::vector<unsigned int> retv;
    jack_client_t* jc;
    jc = jack_client_open("MHAJack__get_port_latency",JackNullOption,NULL);
    if( !jc)
        throw MHA_Error(__FILE__,__LINE__,"Unable to connect to JACK.");
    try{
        for(unsigned int k=0;k<ports.size();k++){
            jack_port_t* jp = NULL;
            jp = jack_port_by_name(jc,ports[k].c_str());
            if( !jp )
                throw MHA_Error(__FILE__,__LINE__,
                                "Invalid port name: \"%s\"",
                                ports[k].c_str());
#if __GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ > 3)
            jack_latency_range_t lat_rg;
            jack_port_get_latency_range( jp, JackPlaybackLatency, &lat_rg);
            retv.push_back(lat_rg.min);
#else
            retv.push_back(0);
#endif
        }
        jack_client_close(jc);
    }
    catch(...){
        jack_client_close(jc);
        throw;
    }
    return retv;
}

/** 
    \brief Return the JACK port latency of ports
  
    \param ports        Ports to be tested
    \return             Latency vector (one entry for each port)
*/
std::vector<int> MHAJack::get_port_capture_latency_int(const std::vector<std::string>& ports)
{
    std::vector<int> retv;
    std::vector<unsigned int> rv = MHAJack::get_port_capture_latency(ports);
    for(unsigned int k=0;k<rv.size();k++)
        retv.push_back(rv[k]);
    return retv;
}

std::vector<int> MHAJack::get_port_playback_latency_int(const std::vector<std::string>& ports)
{
    
    std::vector<int> retv;
    std::vector<unsigned int> rv = MHAJack::get_port_playback_latency(ports);
    for(unsigned int k=0;k<rv.size();k++)
        retv.push_back(rv[k]);
    return retv;
}

MHAJack::port_t::port_t(jack_client_t* ijc,dir_t dir,int id)
    : dir_type(dir),
      port(NULL),
      iob(NULL),
      jc(ijc)
{
    char cs_pname[64];
    switch( dir ){
    case input :
        sprintf(cs_pname,"in_%d",id);
        port = jack_port_register(jc,cs_pname,
                                  JACK_DEFAULT_AUDIO_TYPE,
                                  JackPortIsInput,0);
        break;
    case output :
        sprintf(cs_pname,"out_%d",id);
        port = jack_port_register(jc,cs_pname,
                                  JACK_DEFAULT_AUDIO_TYPE,
                                  JackPortIsOutput,0);
        break;
    }
    if( !port )
        throw MHA_Error(__FILE__,__LINE__,
                        "Unable to register JACK port %s.",cs_pname);
}

MHAJack::port_t::port_t(jack_client_t* ijc,dir_t dir,const std::string& id)
    : dir_type(dir),
      port(NULL),
      iob(NULL),
      jc(ijc)
{
    switch( dir ){
    case input :
        port = jack_port_register(jc,id.c_str(),
                                  JACK_DEFAULT_AUDIO_TYPE,
                                  JackPortIsInput,0);
        break;
    case output :
        port = jack_port_register(jc,id.c_str(),
                                  JACK_DEFAULT_AUDIO_TYPE,
                                  JackPortIsOutput,0);
        break;
    }
    if( !port )
        throw MHA_Error(__FILE__,__LINE__,
                        "Unable to register JACK port %s.",id.c_str());
}

MHAJack::port_t::~port_t()
{
    jack_port_unregister(jc,port);
    port = NULL;
    jc = NULL;
    iob = NULL;
}

const char* MHAJack::port_t::get_short_name()
{
    return jack_port_short_name(port);
}

inline void make_friendly_number(jack_default_audio_sample_t& x)
{
    if( (0 < x) && (x < std::numeric_limits<jack_default_audio_sample_t>::min()) )
        x = 0;
    if( (0 > x) && (x > -std::numeric_limits<jack_default_audio_sample_t>::min()) )
        x = 0;
    if( x == std::numeric_limits<jack_default_audio_sample_t>::infinity() )
        x = std::numeric_limits<jack_default_audio_sample_t>::max();
    if( x == -std::numeric_limits<jack_default_audio_sample_t>::infinity() )
        x = -std::numeric_limits<jack_default_audio_sample_t>::max();
}


void MHAJack::port_t::read(mha_wave_t* s,unsigned int ch)
{
    if( dir_type == output )
        throw MHA_ErrorMsg("Unable to read from output port.");
    iob = (jack_default_audio_sample_t*)jack_port_get_buffer(port,s->num_frames);
    for(unsigned int kfr=0;kfr<s->num_frames;kfr++){
        make_friendly_number(iob[kfr]);
        value(s,kfr,ch) = iob[kfr];
    }
}

void MHAJack::port_t::write(mha_wave_t* s,unsigned int ch)
{
    if( dir_type == input )
        throw MHA_ErrorMsg("Unable to write to input port.");
    iob = (jack_default_audio_sample_t*)jack_port_get_buffer(port,s->num_frames);
    for(unsigned int kfr=0;kfr<s->num_frames;kfr++)
        iob[kfr] = value(s,kfr,ch);
}

void MHAJack::port_t::mute(unsigned int n)
{
    if( dir_type == input )
        throw MHA_ErrorMsg("Unable to mute input port.");
    iob = (jack_default_audio_sample_t*)jack_port_get_buffer(port,n);
    memset(iob,0,sizeof(jack_default_audio_sample_t)*n);
}

void MHAJack::port_t::connect_to(const char* targ)
{
    int jc_err = 0;
    jack_port_disconnect(jc,port);
    if( targ && strlen(targ) ){
        switch( dir_type ){
        case MHAJack::port_t::input :
            jc_err = jack_connect(jc,targ,jack_port_name(port));
            break;
        case MHAJack::port_t::output :
            jc_err = jack_connect(jc,jack_port_name(port),targ);
            break;
        }
        if( jc_err )
            throw MHA_Error(__FILE__,__LINE__,"connecting port %s to %s failed.",jack_port_name(port),targ);
    }
}

std::string MHAJack::client_t::str_error(int err)
{
    std::string msg("");
    switch( err ){
    case IO_ERROR_JACK : 
        msg += "JACK error: ";
        msg += last_jack_err_msg;
        return msg;
    case IO_ERROR_MHAJACKLIB :
        msg += "mhajack library error: ";
        msg += last_jack_err_msg;
        return msg;
    default:
        return "unknown error";
    }
}


/** 
    \brief Remove JACK client and deallocate internal ports and buffers
  
*/
void MHAJack::client_t::release()
{
    b_prepared = false;
    jack_deactivate( jc );
    jack_set_process_callback(jc,dummy_jack_proc_cb,NULL);
    unsigned int ch;
    delete s_in;
    for(ch=0;ch<nchannels_in;ch++)
        delete inch[ch];
    delete [] inch;
    inch = NULL;
    for(ch=0;ch<nchannels_out;ch++)
        delete outch[ch];
    delete [] outch;
    outch = NULL;
    jack_client_close( jc );
    jc = NULL;
}

/** 
    \brief Allocate buffers, activate JACK client and install internal ports

    Registers the jack client with the default jack server and activates it.
    @param client_name Name of this jack client
    @param nch_in      Input ports to register
    @param nch_out     Output ports to register
*/
void MHAJack::client_t::prepare(const std::string& client_name,
                                const unsigned int& nch_in,
                                const unsigned int& nch_out)
{
    const char * default_server = NULL;
    prepare_impl(default_server, client_name.c_str(), nch_in, nch_out);
}

/** 
    \brief Allocate buffers, ports, and activates JACK client

    Registers the jack client with specified jack server and activates it.
    @param server_name Name of the jack server to register with
    @param client_name Name of this jack client
    @param nch_in      Input ports to register
    @param nch_out     Output ports to register
*/
void MHAJack::client_t::prepare(const std::string& server_name,
                                const std::string& client_name,
                                const unsigned int& nch_in,
                                const unsigned int& nch_out)
{
    prepare_impl(server_name.c_str(), client_name.c_str(), nch_in, nch_out);
}

/** 
    Allocate buffers, activate JACK client and allocates jack ports
    Registers the jack client with the given server and activates it.
    @param server_name Name of the jack server to register with
    @param client_name Name of this jack client
    @param nch_in      Input ports to register
    @param nch_out     Output ports to register
*/
void MHAJack::client_t::prepare_impl(const char* server_name,
                                     const char* client_name,
                                     const unsigned int& nch_in,
                                     const unsigned int& nch_out)
{
    unsigned int ch;
    num_xruns = 0;
    jack_set_error_function(jack_error_handler);
    nchannels_in = nch_in;
    nchannels_out = nch_out;
    flags = MHAJACK_STOPPED;
    jc = jack_client_open(client_name,
                          JackOptions(JackNoStartServer
                                      | JackUseExactName
                                      | (server_name ? JackServerName : 0)),
                          NULL,
                          server_name);
    if( !jc )
        throw MHA_Error(__FILE__,__LINE__,
                        "Unable to connect to JACK server '%s' as client '%s'.",
                        server_name ? server_name : "''", client_name);
    try{
        samplerate = jack_get_sample_rate( jc );
        fragsize = jack_get_buffer_size( jc );
        if( jack_set_process_callback(jc,jack_proc_cb,this) )
            throw MHA_ErrorMsg("Not able to set process callback.");
        if( jack_set_xrun_callback(jc,jack_xrun_cb,this) )
            throw MHA_ErrorMsg("Not able to set xrun handler.");
        s_in = new MHASignal::waveform_t(fragsize,nchannels_in);
        inch = new MHAJack::port_t*[nchannels_in];
        for(ch=0;ch<nchannels_in;ch++){
            if( ch < input_portnames.size() )
                inch[ch] = new MHAJack::port_t(jc,MHAJack::port_t::input,input_portnames[ch]);
            else
                inch[ch] = new MHAJack::port_t(jc,MHAJack::port_t::input,ch+1);
        }
        outch = new MHAJack::port_t*[nchannels_out];
        for(ch=0;ch<nchannels_out;ch++){
            if( ch < output_portnames.size() )
                outch[ch] = new MHAJack::port_t(jc,MHAJack::port_t::output,output_portnames[ch]);
            else
                outch[ch] = new MHAJack::port_t(jc,MHAJack::port_t::output,ch+1);
        }
        if( jack_activate( jc ) ){
            if( last_jack_err ) {
                last_jack_err = 0;
                throw MHA_Error(__FILE__,__LINE__,"Unable to activate JACK client (%s).",last_jack_err_msg);
            } else
                throw MHA_ErrorMsg("Unable to activate JACK client.");
        }
        b_prepared = true;
    }
    catch(...){
        // cleaning up:
        if( s_in )
            delete s_in;
        if( inch ){
            for(ch=0;ch<nchannels_in;ch++)
                if( inch[ch] )
                    delete inch[ch];
            delete [] inch;
            inch = NULL;
        }
        if( outch ){
            for(ch=0;ch<nchannels_out;ch++)
                if( outch[ch] )
                    delete outch[ch];
            delete [] outch;
            outch = NULL;
        }
        jack_client_close( jc );
        jc = NULL;
        throw;
    }
}

MHAJack::client_t::client_t(IOProcessEvent_t iproc_event,
                            void* iproc_handle,
                            IOStartedEvent_t istart_event,
                            void* istart_handle,
                            IOStoppedEvent_t istop_event,
                            void* istop_handle,
                            bool iuse_jack_transport)
    : num_xruns(0),
      fragsize(0),
      samplerate(1),
      proc_event(iproc_event),
      proc_handle(iproc_handle),
      start_event(istart_event),
      start_handle(istart_handle),
      stop_event(istop_event),
      stop_handle(istop_handle),
      s_in(NULL),
      s_out(NULL),
      inch(NULL),
      outch(NULL),
      jc(NULL),
      flags(MHAJACK_STOPPED),
      b_prepared(false),
      use_jack_transport(iuse_jack_transport),
      jstate_prev(JackTransportStopped),
      fail_on_async_jackerror(true)
{
}

void MHAJack::client_t::start(bool fail_on_async_jackerr_)
{
    fail_on_async_jackerror = fail_on_async_jackerr_;
    if( !b_prepared )
        throw MHA_Error(__FILE__,__LINE__,"The JACK client was not prepared for start.");
    if( use_jack_transport )
        jack_transport_start( jc );
    else
        internal_start();
}

std::vector<std::string> MHAJack::client_t::get_my_input_ports()
{
    std::vector<std::string> retv;
    if( inch )
        for(unsigned int k=0;k<nchannels_in;k++)
            if( inch[k] )
                retv.push_back(inch[k]->get_short_name());
    return retv;
}

std::vector<std::string> MHAJack::client_t::get_my_output_ports()
{
    std::vector<std::string> retv;
    if( outch )
        for(unsigned int k=0;k<nchannels_out;k++)
            if( outch[k] )
                retv.push_back(outch[k]->get_short_name());
    return retv;
}

void MHAJack::client_t::set_input_portnames(const std::vector<std::string>& names)
{
    input_portnames = names;
}

void MHAJack::client_t::set_output_portnames(const std::vector<std::string>& names)
{
    output_portnames = names;
}

void MHAJack::client_t::stop()
{
    if( use_jack_transport )
        jack_transport_stop( jc );
    else
        internal_stop();
}

void MHAJack::client_t::internal_start()
{
    flags = MHAJACK_FW_STARTED | MHAJACK_STARTING;
}

void MHAJack::client_t::internal_stop()
{
    flags &= ~MHAJACK_FW_STARTED;
}

void MHAJack::client_t::stopped(int proc_err,int io_err)
{
    flags = (flags | MHAJACK_STOPPED) & ~MHAJACK_FW_STARTED;
    if( stop_event )
        stop_event(stop_handle,proc_err,io_err);
}

/** 
    \brief This is the main processing callback. 

    Here happens double buffering and downsampling.
  
*/
int MHAJack::client_t::jack_proc_cb(jack_nframes_t n)
{
    if( !b_prepared )
        return 0;
    try{
        unsigned int kch;
        // if JACK transport is to be used, switch states according to
        // JACK transport state:
        if( use_jack_transport ){
            jack_transport_state_t jstate;
            jstate = jack_transport_query(jc,NULL);
            if( jstate != jstate_prev ){
                if( jstate == JackTransportRolling )
                    internal_start();
                else if( jstate == JackTransportStopped )
                    internal_stop();
                jstate_prev = jstate;
            }
        }
        // switch from "starting" state to "started" state:
        unsigned int lflags = flags;
        if( lflags & MHAJACK_STARTING ){
            if( start_event )
                start_event(start_handle);
            flags &= ~MHAJACK_STARTING;
        }
        // not running:
        if( !(lflags & MHAJACK_FW_STARTED) ){
            // switch from "stopping" state to "stopped" state:
            if( !(lflags & MHAJACK_STOPPED) )
                stopped(0,0);
            // if not running, then mute output:
            for( kch=0;kch<nchannels_out;kch++ )
                outch[kch]->mute(n);
            return 0;
        }
        if( s_in->num_frames != n )
            throw MHA_Error(__FILE__,__LINE__,
                            "Cannot handle JACK buffer size changes (size changed from %u to %u).",
                            s_in->num_frames,n);
        for( kch=0;kch<nchannels_in;kch++ )
            inch[kch]->read(s_in,kch);
        int err = 0;
        s_out = NULL;
        if( proc_event )
            err = proc_event(proc_handle,s_in,&s_out);
        // on error switch to stopped state:
        if( err != 0 ){
            stopped(err,0);
            return 0;
        }
        if( !s_out ){
            for( kch=0;kch<nchannels_out;kch++ )
                outch[kch]->mute(n);
            return 0;
        }
        if( s_out->num_frames != n )
            throw MHA_Error(__FILE__,__LINE__,
                            "Processing library returned invalid fragment size (expected %u, got %u).",
                            n,s_out->num_frames);
        if( s_out->num_channels != nchannels_out )
            throw MHA_Error(__FILE__,__LINE__,
                            "Processing library returned invalid number of channels (expected %u, got %u).",
                            nchannels_out,s_out->num_channels);
        // copy back to port:
        for( kch=0;kch<nchannels_out;kch++ )
            outch[kch]->write(s_out,kch);
        if( fail_on_async_jackerror ){
            // the next four lines are preventing jack2 from starting if
            // real-time settings are not correct:
            if( last_jack_err ){
                stopped(0,last_jack_err);
                return 0;
            }
        }
        return 0;
    }
    catch(const MHA_Error& e){
        strncpy(last_jack_err_msg,Getmsg(e),MAX_USER_ERR-1);
        last_jack_err = IO_ERROR_MHAJACKLIB;
        stopped(0,last_jack_err);
        return 0;
    }
}

int MHAJack::client_t::jack_proc_cb(jack_nframes_t n,void* h)
{
    return ((MHAJack::client_t*)h)->jack_proc_cb(n);
}

int MHAJack::client_t::jack_xrun_cb(void* h)
{
    return ((MHAJack::client_t*)h)->jack_xrun_cb();
}

/** 
    \brief Connect the input ports when connection variable is accessed
  
*/
void MHAJack::client_t::connect_input(const std::vector<std::string>& con)
{
    if(!b_prepared )
        return;
    if( !inch )
        throw MHA_ErrorMsg("Fatal error (inch pointer undefined)");
    for(unsigned int ch=0;ch<nchannels_in;ch++)
        if( inch[ch] && (ch < con.size()) ){
            if( con[ch] == ":" )
                inch[ch]->connect_to( "" );
            else
                inch[ch]->connect_to( con[ch].c_str() );
        }
}

/** 
    \brief Connect the output ports when connection variable is accessed
  
*/
void MHAJack::client_t::connect_output(const std::vector<std::string>& con)
{
    if(!b_prepared )
        return;
    if( !outch )
        throw MHA_ErrorMsg("Fatal error (outch pointer undefined)");
    for(unsigned int ch=0;ch<nchannels_out;ch++)
        if( outch[ch] && (ch < con.size()) ){
            if( con[ch] == ":" )
                outch[ch]->connect_to( "" );
            else
                outch[ch]->connect_to( con[ch].c_str() );
        }
}

/**
   \brief Get a list of Jack ports
   \param res Result string vector
   \param jack_flags Jack port flags (JackPortInput etc.)
 */
void MHAJack::client_t::get_ports(std::vector<std::string>& res,unsigned long jack_flags)
{
    res.clear();
    if( jc ){
        const char** jports;
        jports = jack_get_ports(jc,NULL,NULL,jack_flags);
        if( jports ){
            const char** jptmp;
            for( jptmp=jports;*jptmp;++jptmp ){
                res.push_back(*jptmp);
                //free(*jptmp);
            }
            free(jports);
        }
    }
}

MHAJack::client_noncont_t::client_noncont_t(const std::string& name_,bool use_jack_transport)
    : MHAJack::client_t(&proc,this,NULL,NULL,&IOStoppedEvent,this,use_jack_transport),
      b_stopped(false),
      pos(0), 
      sn_in(NULL), 
      sn_out(NULL), 
      name(name_)
{
}

void MHAJack::client_noncont_t::io(mha_wave_t* is_out,
                                   mha_wave_t* is_in,
                                   const std::vector<std::string>& p_out,
                                   const std::vector<std::string>& p_in,
                                   float* srate,
                                   unsigned int* fragsize)
{
    unsigned long timeout = 8000;// ms
    CHECK_VAR(is_in);
    CHECK_VAR(is_out);
    sn_in = is_in;
    sn_out = is_out;
    pos = 0;
    prepare(name,sn_in->num_channels,sn_out->num_channels);
    if( srate )
        *srate = get_srate();
    if( fragsize )
        *fragsize = get_fragsize();
    frag_out = new MHASignal::waveform_t(get_fragsize(),sn_out->num_channels);
    try{
        connect_input(p_in);
        connect_output(p_out);
        b_stopped = false;
        start();
        while( pos < std::max(sn_in->num_frames,sn_out->num_frames) ){
            usleep(100);
            if( b_stopped )
                throw MHA_Error(__FILE__,__LINE__,
                                "Processing was stopped before the data was completed.");
        }
        stop();
        while( !b_stopped ){
            usleep( 1000 );
            timeout --;
            if( !timeout )
                throw MHA_Error(__FILE__,__LINE__,"Stop request timed out.");
        }       
        release();
        delete frag_out;
        frag_out = NULL;
        sn_in = NULL;
        sn_out = NULL;
    }
    catch( ... ){
        release();
        delete frag_out;
        frag_out = NULL;
        sn_in = NULL;
        sn_out = NULL;
        throw;
    }
}

int MHAJack::client_noncont_t::proc(void* handle,mha_wave_t* sIn,mha_wave_t** sOut)
{
    if( handle )
        ((MHAJack::client_noncont_t*)(handle))->proc(sIn,sOut);
    return 0;
}

void MHAJack::client_noncont_t::IOStoppedEvent(void* handle,int a,int b)
{
    if( handle )
        ((MHAJack::client_noncont_t*)(handle))->IOStoppedEvent();
}

void MHAJack::client_noncont_t::IOStoppedEvent()
{
    b_stopped = true;
}

void MHAJack::client_noncont_t::proc(mha_wave_t* sIn,mha_wave_t** sOut)
{
    if( (!sn_in) || (!sn_out ) || (!frag_out) || b_stopped ){
        *sOut = frag_out;
        return;
    }
    CHECK_EXPR(sn_in->num_channels == sIn->num_channels);
    unsigned int ch;
    unsigned int k;
    unsigned int p_in_max = std::min(pos+get_fragsize(),sn_in->num_frames);
    unsigned int p_out_max = std::min(pos+get_fragsize(),sn_out->num_frames);
    for(ch=0;ch<sn_in->num_channels;ch++)
        for(k=pos;k<p_in_max;k++)
            value(sn_in,k,ch) = value(sIn,k-pos,ch);
    for(ch=0;ch<sn_out->num_channels;ch++){
        for(k=pos;k<p_out_max;k++)
            value(frag_out,k-pos,ch) = value(sn_out,k,ch);
        for(k=std::max(p_out_max,pos)-pos;k<get_fragsize();k++)
            value(frag_out,k,ch) = 0;
    }
    pos += get_fragsize();
    *sOut = frag_out;
}

void MHAJack::io(mha_wave_t* s_out,mha_wave_t* s_in,
                 const std::string& name,
                 const std::vector<std::string>& p_out,
                 const std::vector<std::string>& p_in,
                 float* srate,
                 unsigned int* fragsize,
                 bool use_jack_transport)
{
    CHECK_VAR(s_in);
    CHECK_VAR(s_out);
    MHAJack::client_noncont_t cl(name,use_jack_transport);
    cl.io(s_out,s_in,p_in,p_out);
    if( srate )
        *srate = cl.get_srate();
    if( fragsize )
        *fragsize = cl.get_fragsize();
}


/** 
    \brief Constructor for averaging client
  
    \param name_        Name of JACK client
    \param nrep_        Number of repetitions
*/
MHAJack::client_avg_t::client_avg_t(const std::string& name_,
                                    const unsigned int& nrep_)
    : MHAJack::client_t(&proc,this,NULL,NULL,&IOStoppedEvent,this),
      b_stopped(false),
      pos(0), 
      sn_in(NULL), 
      sn_out(NULL), 
      name(name_),
      nrep(nrep_),
      b_ready(true)
{
}

/** 
    \brief Recording function
  
    long-description

    \param is_out       Input (test) signal, which will be repeated
    \param is_in        System response (averaged, same length as input required)
    \param p_out        Ports to play back the test signal
    \param p_in         Ports to record from the system response
    \param srate        Pointer to sampling rate variable, will be filled with server sampling rate
    \param fragsize     Pointer to fragment size variable, will be filled with server fragment size
*/
void MHAJack::client_avg_t::io(mha_wave_t* is_out,
                               mha_wave_t* is_in,
                               const std::vector<std::string>& p_out,
                               const std::vector<std::string>& p_in,
                               float* srate,
                               unsigned int* fragsize)
{
    unsigned long timeout = 8000;// ms
    CHECK_VAR(is_in);
    CHECK_VAR(is_out);
    if( is_in->num_frames != is_out->num_frames )
        throw MHA_Error(__FILE__,__LINE__,
                        "Input and output signal must have the same number of frames.");
    sn_in = is_in;
    sn_out = is_out;
    pos = 0;
    n = 0;
    b_ready = false;
    prepare(name,sn_in->num_channels,sn_out->num_channels);
    if( srate )
        *srate = get_srate();
    if( fragsize )
        *fragsize = get_fragsize();
    frag_out = new MHASignal::waveform_t(get_fragsize(),sn_out->num_channels);
    try{
        connect_input(p_in);
        connect_output(p_out);
        b_stopped = false;
        start();
        while( !b_ready ){
            usleep(100);
            if( b_stopped )
                throw MHA_Error(__FILE__,__LINE__,
                                "Processing was stopped before the data was completed.");
        }
        stop();
        while( !b_stopped ){
            usleep( 1000 );
            timeout --;
            if( !timeout )
                throw MHA_Error(__FILE__,__LINE__,"Stop request timed out.");
        }       
        release();
        delete frag_out;
        *is_in *= 1.0f/nrep;
        frag_out = NULL;
        sn_in = NULL;
        sn_out = NULL;
    }
    catch( ... ){
        release();
        delete frag_out;
        frag_out = NULL;
        sn_in = NULL;
        sn_out = NULL;
        throw;
    }
}

int MHAJack::client_avg_t::proc(void* handle,mha_wave_t* sIn,mha_wave_t** sOut)
{
    if( handle )
        ((client_avg_t*)(handle))->proc(sIn,sOut);
    return 0;
}

void MHAJack::client_avg_t::IOStoppedEvent(void* handle,int a,int b)
{
    if( handle )
        ((client_avg_t*)(handle))->IOStoppedEvent();
}

void MHAJack::client_avg_t::IOStoppedEvent()
{
    b_stopped = true;
}

void MHAJack::client_avg_t::proc(mha_wave_t* sIn,mha_wave_t** sOut)
{
    if( (!sn_in) || (!sn_out ) || (!frag_out) || b_stopped || b_ready ){
        *sOut = frag_out;
        return;
    }
    clear(frag_out);
    CHECK_EXPR(sn_in->num_channels == sIn->num_channels);
    CHECK_EXPR(sIn->num_frames == get_fragsize() );
    unsigned int ch;
    unsigned int k;
    for( k=0; k < sIn->num_frames; k++ ){
        if( pos >= sn_in->num_frames ){
            pos = 0;
            n++;
            if( n > nrep ){
                b_ready = true;
                *sOut = frag_out;
                return;
            }
        }
        if( n )
            for(ch=0;ch<sn_in->num_channels;ch++)
                value(sn_in,pos,ch) += value(sIn,k,ch);
        for(ch=0;ch<sn_out->num_channels;ch++){
            value(frag_out,k,ch) = value(sn_out,pos,ch);
        }
        pos++;
    }
    *sOut = frag_out;
}

unsigned long MHAJack::client_t::get_xruns_reset()
{
    unsigned long ret = num_xruns;
    num_xruns = 0;
    return ret;
}

float MHAJack::client_t::get_cpu_load()
{
    if( jc )
        return jack_cpu_load(jc);
    else
        return 0;
}

/*
 * Local Variables:
 * compile-command: "make -C .."
 * mode: c++
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * coding: utf-8-unix
 * End:
 */
