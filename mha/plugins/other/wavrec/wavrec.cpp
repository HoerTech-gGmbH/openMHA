// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2010 2011 2012 2014 2015 2016 2018 HörTech gGmbH
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

#include <time.h>
#include <pthread.h>
#include "mha_plugin.hh"
#include "mha_fifo.h"
#include <sndfile.h>
#include <sys/time.h>

#define DEBUG(x) std::cerr << __FILE__ << ":" << __LINE__ << " " << #x << "=" << x << std::endl

class wavwriter_t {
public:
    wavwriter_t(bool active,const mhaconfig_t& cf,unsigned int fifosize,unsigned int minwrite,const std::string& prefix,bool use_date);
    ~wavwriter_t();
    void process(mha_wave_t*);
private:
    static void* write_thread(void* this_){((wavwriter_t*)this_)->write_thread();return NULL;};
    void write_thread();
    bool close_session;
    bool act_;
    mhaconfig_t cf_;
    SNDFILE* sf;
    mha_fifo_t<mha_real_t> fifo;
    unsigned int minw_;
    pthread_t writethread;
    float* data;
};

class wavrec_t : public MHAPlugin::plugin_t<wavwriter_t> {
public:
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t& cf);
    void release();
    wavrec_t(const algo_comm_t& iac,const std::string&,const std::string&);
private:
    void start_new_session();
    MHAParser::bool_t record;
    MHAParser::int_t fifolen;
    MHAParser::int_t minwrite;
    MHAParser::string_t prefix;
    MHAParser::bool_t use_date;
    MHAEvents::patchbay_t<wavrec_t> patchbay;
};

wavrec_t::wavrec_t(const algo_comm_t& iac,const std::string&,const std::string& algo_name)
    : MHAPlugin::plugin_t<wavwriter_t>("wav file recorder",iac),
      record("Record session. Each write access with argument \"yes\" will start a\n"
             "new file with current time and date as a name.","no"),
      fifolen("Length of FIFO in samples","262144","[2,]"),
      minwrite("Minimal write length (must be less then fifolen)","65536","[1,]"),
      prefix("Path (including path delimiter) and file prefix",""),
      use_date("Use date and time (yes), or only prefix (no)","yes")
{
    // make the plug-in findable via "?listid"
    set_node_id(algo_name);

    insert_member(fifolen);
    insert_member(minwrite);
    insert_member(prefix);
    insert_member(use_date);
    insert_member(record);
    patchbay.connect(&record.writeaccess,this,&wavrec_t::start_new_session);
}

mha_wave_t* wavrec_t::process(mha_wave_t* s)
{
    poll_config()->process(s);
    return s;
}

void wavrec_t::prepare(mhaconfig_t& cf)
{
    start_new_session();
}

void wavrec_t::release()
{
    push_config(new wavwriter_t(false,input_cfg(),8,2,"",true));
    push_config(new wavwriter_t(false,input_cfg(),8,2,"",true));
    poll_config();
    cleanup_unused_cfg();
}

void wavrec_t::start_new_session()
{
    push_config(new wavwriter_t(record.data,input_cfg(),fifolen.data,minwrite.data,prefix.data,use_date.data));
}

wavwriter_t::wavwriter_t(bool active,const mhaconfig_t& cf,unsigned int fifosize,unsigned int minwrite,const std::string& prefix, bool use_date)
    : close_session(false),
      act_(active),
      cf_(cf),
      sf(NULL),
      fifo(fifosize),
      minw_(minwrite),
      data(new float[fifosize])
{
    if(minw_ >= fifosize )
        throw MHA_Error(__FILE__,__LINE__,"minwrite must be less then fifosize (minwrite: %d, fifosize: %d)",minw_,fifosize);
    if( (cf_.channels == 0) || (cf_.srate==0) )
        act_ = false;
    if( act_ ){
        std::string fname;
        if( use_date ){
            time_t tm(time(NULL));
#ifdef _WIN32
            fname = ctime(&tm);
#else
            char timestr[28];
            fname = ctime_r(&tm,timestr);
#endif
            fname.erase(fname.size()-1,1);
            MHAParser::strreplace(fname,":","-");
            MHAParser::strreplace(fname,"Mon "," ");
            MHAParser::strreplace(fname,"Tue "," ");
            MHAParser::strreplace(fname,"Wed "," ");
            MHAParser::strreplace(fname,"Thu "," ");
            MHAParser::strreplace(fname,"Fri "," ");
            MHAParser::strreplace(fname,"Sat "," ");
            MHAParser::strreplace(fname,"Sun "," ");
            MHAParser::strreplace(fname," ","_");
        }
        fname = prefix+fname+".wav";
        SF_INFO sfinfo;
        memset(&sfinfo,0,sizeof(sfinfo));
        sfinfo.samplerate = (int)cf_.srate;
        sfinfo.channels = cf_.channels;
        sfinfo.format = SF_FORMAT_WAV|SF_FORMAT_FLOAT|SF_ENDIAN_FILE;
        sf = sf_open(fname.c_str(),SFM_WRITE,&sfinfo);
        if( !sf )
            throw MHA_Error(__FILE__,__LINE__,"Unable to create sound file %s: %s",fname.c_str(),sf_strerror(sf));
        pthread_create(&writethread,NULL,&wavwriter_t::write_thread,this);
    }
}

wavwriter_t::~wavwriter_t()
{
    if( act_ ){
        close_session = true;
        pthread_join(writethread,NULL);
    }
    if( sf )
        sf_close(sf);
    delete [] data;
}
    
void wavwriter_t::process(mha_wave_t* s)
{
    if( act_ )
        fifo.write(s->buf,std::min(fifo.get_available_space(),size(s)));
}

void wavwriter_t::write_thread()
{
    while(!close_session){
        mha_msleep(1);
        if( sf && (fifo.get_fill_count() > minw_) ){
            unsigned int frames = fifo.get_fill_count()/cf_.channels;
            fifo.read(data,frames*cf_.channels);
            sf_writef_float(sf,data,frames);
        }
    }
    if( sf && fifo.get_fill_count() ){
        unsigned int frames = fifo.get_fill_count()/cf_.channels;
        fifo.read(data,frames*cf_.channels);
        sf_writef_float(sf,data,frames);
    }
}


MHAPLUGIN_CALLBACKS(wavrec,wavrec_t,wave,wave)

/*
 * Local Variables:
 * compile-command: "make"
 * c-basic-offset: 4
 * End:
 */
