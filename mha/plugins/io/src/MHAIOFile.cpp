// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2009 2011 2013 2014 2015 2016 2017 HörTech gGmbH
// Copyright © 2018 2019 2020 HörTech gGmbH
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
#include <sndfile.h>

#define DEBUG(x) std::cerr << __FILE__ << ":" << __LINE__ << " " << #x " = " << x << std::endl
/**
    \brief File IO
*/
class io_file_t : public MHAParser::parser_t
{
public:
    io_file_t(int fragsize, 
              float samplerate,
              IOProcessEvent_t proc_event,
              void* proc_handle,
              IOStartedEvent_t start_event,
              void* start_handle,
              IOStoppedEvent_t stop_event,
              void* stop_handle);
    ~io_file_t();
    void prepare(int,int);
    void start();
    void stop();
    void release();
private:
    void stopped(int,int);
    void setlock(bool locked);
    int fragsize;
    float samplerate;
    int nchannels_in;
    int nchannels_file_in;
    int nchannels_out;
    IOProcessEvent_t proc_event;
    void* proc_handle;
    IOStartedEvent_t start_event;
    void* start_handle;
    IOStoppedEvent_t stop_event;
    void* stop_handle;
    MHAParser::string_t filename_input;
    MHAParser::string_t filename_output;
    MHAParser::kw_t output_sample_format;
    MHAParser::int_t startsample;
    MHAParser::int_t length;
    MHAParser::bool_t strict_channel_match;
    MHAParser::bool_t strict_srate_match;
    MHASignal::waveform_t* s_in;
    MHASignal::waveform_t* s_file_in;
    mha_wave_t* s_out;
    bool b_prepared;
    SNDFILE* sf_in;
    SNDFILE* sf_out;
    SF_INFO sfinf_in;
    SF_INFO sfinf_out;
    sf_count_t total_read;
};

/** lock or unlock all parser variables. Used in prepare/release.
 * @param locked When true, locks. When false, unlocks. */
void io_file_t::setlock(bool locked)
{
    filename_input.setlock(locked);
    filename_output.setlock(locked);
    output_sample_format.setlock(locked);
    startsample.setlock(locked);
    length.setlock(locked);
    strict_channel_match.setlock(locked);
    strict_srate_match.setlock(locked);
}

/** 
    \brief Remove FILE client and deallocate internal ports and buffers
  
*/
void io_file_t::release()
{
    setlock(false);
    b_prepared = false;
    sf_close( sf_in );
    sf_close( sf_out );
    delete s_in;
    delete s_file_in;
    s_in = NULL;
    s_file_in = NULL;
}

/** 
    \brief Allocate buffers, activate FILE client and install internal ports
  
*/
void io_file_t::prepare(int nch_in,int nch_out)
{
    sf_in = NULL;
    if( filename_input.data.size() == 0 )
        throw MHA_Error(__FILE__,__LINE__,"Input filename not provided.");
    sf_out = NULL;
    if( filename_output.data.size() == 0)
        throw MHA_Error(__FILE__,__LINE__,"Output filename not provided.");
    s_in = NULL;
    s_file_in = NULL;
    total_read = 0;
    try{
        nchannels_in = nch_in;
        nchannels_out = nch_out;
        sf_in = sf_open( filename_input.data.c_str(), SFM_READ, &sfinf_in );
        nchannels_file_in = sfinf_in.channels;

        if( !sf_in )
            throw MHA_Error(__FILE__,__LINE__,"Unable to open \"%s\" for reading.",filename_input.data.c_str());
        if( strict_channel_match.data && (sfinf_in.channels != nchannels_in) )
            throw MHA_Error(__FILE__,__LINE__,"The input file has %d channels, but %d are required.",
                            sfinf_in.channels, nchannels_in );
        if( strict_srate_match.data && (sfinf_in.samplerate != samplerate) )
            throw MHA_Error(__FILE__,__LINE__,"The input file has %d Hz samplerate, but %g Hz is required.",
                            sfinf_in.samplerate, samplerate);
        sfinf_out = sfinf_in;
        sfinf_out.channels = nchannels_out;
        int count(0);
        sf_command(NULL, SFC_GET_FORMAT_SUBTYPE_COUNT, &count, sizeof(int));
        for(int k=0;k<count;k++){
            SF_FORMAT_INFO      format_info;
            format_info.format = k ;
            sf_command(NULL, SFC_GET_FORMAT_SUBTYPE, &format_info, sizeof(format_info));
            std::string fmtname(format_info.name);
            MHAParser::strreplace(fmtname," ","_");
            if( output_sample_format.isval(fmtname) ){
                sfinf_out.format = (sfinf_out.format & (SF_FORMAT_TYPEMASK|SF_FORMAT_ENDMASK))|format_info.format;
            }
        }

        sf_out = sf_open( filename_output.data.c_str(), SFM_WRITE, &sfinf_out );

        if( !sf_out )
            throw MHA_Error(__FILE__,__LINE__,"Unable to open \"%s\" for writing.",filename_output.data.c_str());
        sf_command(sf_out, SFC_SET_CLIPPING, NULL, SF_TRUE);
        s_in = new MHASignal::waveform_t(fragsize,nchannels_in);
        s_file_in = new MHASignal::waveform_t(fragsize,nchannels_file_in);
        if( startsample.data )
            sf_seek(sf_in,startsample.data,SEEK_SET);
        b_prepared = true;
        setlock(true);
    }
    catch(...){
        if( sf_in )
            sf_close(sf_in);
        if( sf_out )
            sf_close(sf_out);
        if( s_in )
            delete s_in;
        if( s_file_in )
            delete s_file_in;
        throw;
    }
}

io_file_t::io_file_t(int ifragsize, 
                     float isamplerate,
                     IOProcessEvent_t iproc_event,
                     void* iproc_handle,
                     IOStartedEvent_t istart_event,
                     void* istart_handle,
                     IOStoppedEvent_t istop_event,
                     void* istop_handle)
    : MHAParser::parser_t("Sound file IO client.\n\n"
                          "Read from input files and write to files of same format."),
      fragsize(ifragsize),
      samplerate(isamplerate),
      proc_event(iproc_event),
      proc_handle(iproc_handle),
      start_event(istart_event),
      start_handle(istart_handle),
      stop_event(istop_event),
      stop_handle(istop_handle),
      filename_input("Input sound file name",""),
      filename_output("Output sound file name",""),
      output_sample_format("Output sample format, or 'input' to copy from input file","input","[input]"),
      startsample("First sample to be processed.","0","[0,]"),
      length("Number of samples to be processed by one start command, or zero for all.","0","[0,]"),
      strict_channel_match("Require same channel count in MHA and sound file.","yes"),
      strict_srate_match("Require same sample rate in MHA and sound file.","yes"),
      s_in(NULL),
      s_file_in(NULL),
      s_out(NULL),
      b_prepared(false)
{
    insert_item("in",&filename_input);
    insert_item("out",&filename_output);
    insert_member(output_sample_format);
    insert_member(startsample);
    insert_member(length);
    insert_member(strict_channel_match);
    insert_member(strict_srate_match);
    int count(0);
    sf_command(NULL, SFC_GET_FORMAT_SUBTYPE_COUNT, &count, sizeof(int));
    for(int k=0;k<count;k++){
        SF_FORMAT_INFO  format_info;
        format_info.format = k ;
        sf_command(NULL, SFC_GET_FORMAT_SUBTYPE, &format_info, sizeof(format_info));
        std::string fmtname(format_info.name);
        MHAParser::strreplace(fmtname," ","_");
        output_sample_format.data.add_entry(fmtname);
    }
}

void io_file_t::start()
{
    if( !b_prepared )
        throw MHA_Error(__FILE__,__LINE__,"The FILE client was not prepared for start.");
    start_event(start_handle);
    int read_cnt;
    do{
        clear(s_in);
        clear(s_file_in);
        // read from file and pad with zeros, if not enough data:
        read_cnt = sf_readf_float( sf_in, s_file_in->buf, s_file_in->num_frames );
        if (read_cnt == 0) 
            break;
        for(int lch=0;lch<std::min(nchannels_in,nchannels_file_in);lch++)
            s_in->copy_channel(*s_file_in,lch,lch);
        total_read += read_cnt;
        // process the data:
        int err = proc_event( proc_handle, s_in, &s_out );
        // on error switch to stopped state:
        if( err != 0 ){
            stopped(err,0);
            return;
        }
        sf_writef_float( sf_out, s_out->buf, read_cnt );
    }while( (read_cnt == (int)s_in->num_frames) && ((length.data==0)||(total_read<length.data)) );
    stopped(0,0);
}

void io_file_t::stop()
{
    total_read = 0;
}

void io_file_t::stopped(int proc_err,int io_err)
{
    if( stop_event )
        stop_event(stop_handle,proc_err,io_err);
}

io_file_t::~io_file_t()
{
}

#define ERR_SUCCESS 0
#define ERR_IHANDLE -1
#define ERR_USER -1000

#define MAX_USER_ERR 0x500
static char user_err_msg[MAX_USER_ERR];

extern "C" {
#ifdef MHA_STATIC_PLUGINS
#define IOInit               MHA_STATIC_MHAIOFile_IOInit
#define IOPrepare            MHA_STATIC_MHAIOFile_IOPrepare
#define IOStart              MHA_STATIC_MHAIOFile_IOStart
#define IOStop               MHA_STATIC_MHAIOFile_IOStop
#define IORelease            MHA_STATIC_MHAIOFile_IORelease
#define IOSetVar             MHA_STATIC_MHAIOFile_IOSetVar
#define IOStrError           MHA_STATIC_MHAIOFile_IOStrError
#define IODestroy            MHA_STATIC_MHAIOFile_IODestroy
#define dummy_interface_test MHA_STATIC_MHAIOFile_dummy_interface_test
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
            io_file_t* cl = new io_file_t(fragsize,samplerate,proc_event,proc_handle,start_event,start_handle,stop_event,stop_handle);
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
            ((io_file_t*)handle)->prepare(nch_in,nch_out);
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
            ((io_file_t*)handle)->start();
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
            ((io_file_t*)handle)->stop();
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
            ((io_file_t*)handle)->release();
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
            ((io_file_t*)handle)->parse(command,retval,maxretlen);
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
        delete (io_file_t*)handle;
    }

    void dummy_interface_test(void){
#ifdef MHA_STATIC_PLUGINS
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOFile_,IOInit);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOFile_,IOPrepare);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOFile_,IOStart);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOFile_,IOStop);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOFile_,IORelease);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOFile_,IOSetVar);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOFile_,IOStrError);
        MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOFile_,IODestroy);
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
