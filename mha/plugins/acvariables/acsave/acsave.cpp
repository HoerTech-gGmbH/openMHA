// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2005 2006 2007 2009 2010 2012 2014 2015 HörTech gGmbH
// Copyright © 2017 HörTech gGmbH
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

#include "mha_plugin.hh"
#include "mha_signal.hh"
#include "mha_defs.h"
#include "mha_events.h"
#include <math.h>
#include <stdio.h>
#include "mha_os.h"

#define ACSAVE_FMT_TXT 0
#define ACSAVE_SFMT_TXT "txt"
#define ACSAVE_FMT_MAT4 1
#define ACSAVE_SFMT_MAT4 "mat4"
#define ACSAVE_FMT_M 2
#define ACSAVE_SFMT_M "m"

namespace acsave {

class save_var_t {
public:
    save_var_t(const std::string&,int,const algo_comm_t&);
    ~save_var_t();
    void store_frame();
    void save_txt(FILE*,unsigned int);
    void save_mat4(FILE*,unsigned int);
    void save_m(FILE*,unsigned int);
    double* data;
private:
    std::string name;
    unsigned int nframes;
    unsigned int ndim;
    unsigned int maxframe;
    algo_comm_t ac;
    unsigned int framecnt;
    bool b_complex;
};

class cfg_t {
public:
    cfg_t(const algo_comm_t& iac,unsigned int imax_frames,std::vector<std::string>& var_names);
    ~cfg_t();
    void store_frame();
    void flush_data(const std::string&,unsigned int);
private:
    algo_comm_t ac;
    unsigned int nvars;
    save_var_t** varlist;
    unsigned int rec_frames;
    unsigned int max_frames;
};

class acsave_t : public MHAPlugin::plugin_t<cfg_t> {
    typedef std::vector<save_var_t*> varlist_t;
public:
    acsave_t(const algo_comm_t&,const std::string&,const std::string&);
    void prepare(mhaconfig_t&);
    void release();
    mha_spec_t* process(mha_spec_t*);
    mha_wave_t* process(mha_wave_t*);
    void event_start_recording();
    void event_stop_and_flush();
private:
    void process();
    MHAParser::bool_t bflush;
    MHAParser::kw_t fileformat;
    MHAParser::string_t fname;
    MHAParser::float_t reclen;
    MHAParser::vstring_t variables;
    varlist_t varlist;
    std::string chain;
    std::string algo;
    bool b_prepared;
    bool b_flushed;
    MHAEvents::patchbay_t<acsave_t> patchbay;
};

cfg_t::cfg_t(const algo_comm_t& iac,
             unsigned int imax_frames,
             std::vector<std::string>& varnames) :
    ac(iac),
    nvars(0),
    varlist(NULL),
    rec_frames(0),
    max_frames(imax_frames)
{
    //DEBUG(imax_frames);
    //DEBUG(varnames.size());
    if( !varnames.size() ){
        int get_entries_error_code;
        unsigned int cstr_len = 512;
        std::string entr;
        do {
            cstr_len <<= 1;
            if (cstr_len > 0x100000)
                throw MHA_ErrorMsg("list of all ac variables is longer than 1MB."
                                 " You should select a subset using vars.");
            char* temp_cstr;
            temp_cstr = new char[cstr_len];
            temp_cstr[0] = 0;
            get_entries_error_code = 
                ac.get_entries(ac.handle, temp_cstr, cstr_len);
            entr = temp_cstr;
            delete [] temp_cstr; temp_cstr = 0;
        } while (get_entries_error_code == -3);
        if (get_entries_error_code == -1)
            throw MHA_ErrorMsg("Bug: ac handle used is invalid");
        
        entr = std::string("[") + entr + std::string("]");
        std::vector<std::string> entrl;
        MHAParser::StrCnv::str2val(entr,entrl);
        varnames = entrl;
    }
    nvars = varnames.size();
    //DEBUG(nvars);
    //mha_debug("%s:%d nvars = %d\n",__FILE__,__LINE__,nvars);
    if( !nvars )
        return;
    unsigned int k;
    varlist = new save_var_t*[nvars];
    for(k=0;k<nvars;k++){
        varlist[k] = new save_var_t(varnames[k],max_frames,ac);
    }
}

cfg_t::~cfg_t()
{
    unsigned int k;
    if( varlist ){
        for(k=0;k<nvars;k++){
            delete varlist[k];
        }
        delete [] varlist;
    }
}

//! 
/*!

This function is called in the processing thread.

*/
void cfg_t::store_frame()
{
    //DEBUG(rec_frames);
    //DEBUG(max_frames);
    if( rec_frames >= max_frames )
        return;
    unsigned int k;
    for(k=0;k<nvars;k++){
        varlist[k]->store_frame();
    }
    rec_frames++;
}

//! 
/*!
  This function is called in the configuration thread.

  \param filename       Output file name
  \param fmt            Output file format
*/
void cfg_t::flush_data(const std::string& filename,unsigned int fmt)
{
    if( !filename.size() )
        return;
    unsigned int tobesaved = rec_frames;
    max_frames = 0;
    FILE* fh;
    if( !(fh = fopen(filename.c_str(),"wb") ) ){
        throw MHA_Error(__FILE__,__LINE__,
                        "Unable to open file \"%s\".",filename.c_str());
    }
    unsigned int k;
    switch( fmt ){
        case ACSAVE_FMT_TXT :
            for(k=0;k<nvars;k++)
                varlist[k]->save_txt(fh,tobesaved);
            break;
        case ACSAVE_FMT_MAT4 :
            for(k=0;k<nvars;k++)
                varlist[k]->save_mat4(fh,tobesaved);
            break;
        case ACSAVE_FMT_M :
            for(k=0;k<nvars;k++)
                varlist[k]->save_m(fh,tobesaved);
            break;
    }
    fclose(fh);
    // delete all data, free the memory: This is allowed, because new
    // data is only stored after a new "reclen=..." or "filename="
    // command, i.e. a new constructor call.
    if( varlist ){
        for(k=0;k<nvars;k++){
            delete varlist[k];
            varlist[k] = NULL;
        }
        delete [] varlist;
        varlist = NULL;
    }
}

/****************************************************************/
/*                                                            ***/
/* save                                                       ***/
/*                                                            ***/
/****************************************************************/

acsave_t::acsave_t(const algo_comm_t& iac,const std::string& ith,const std::string& ial)
    : MHAPlugin::plugin_t<cfg_t>(
        "Save chain data to text or Matlab 4 files.\n\n"
        "Usage:\n\n"
        "  1. set up file name and type\n\n"
        "  2. set the maximal length. This will start\n"
        "     the recording.\n\n"
        "  3. Set \"flush\" to yes to save recorded frames.\n"
        "     This will overwrite previously written data.\n\n"
        "File name and type can be changed at any time and has to be valid\n"
        "when sending the flush command. Changing the list of variables\n"
        "also starts the recording with the currently configured recording\n"
        "length (previously recorded data might be overwritten). Issueing\n"
        "the 'flush' command frees allocated memory.",
        iac),
      bflush("flush the buffers to disk","no"),
      fileformat("file format of output file","txt","[txt mat4 m]"),
      fname("output file name",""),
      reclen("maximal recording length in seconds","10","[0,]"),
      variables("list of variables to be saved (empty: save all)","[]"),
      chain(ith),
      algo(ial),
      b_prepared(false),
      b_flushed(false)
{
    varlist.clear();
    insert_item("fileformat",&fileformat);
    insert_item("name",&fname);
    insert_item("reclen",&reclen);
    insert_item("flush",&bflush);
    insert_item("vars",&variables);
    patchbay.connect(&bflush.writeaccess,this,&acsave_t::event_stop_and_flush);
    patchbay.connect(&reclen.writeaccess,this,&acsave_t::event_start_recording);
    patchbay.connect(&variables.writeaccess,this,&acsave_t::event_start_recording);
}

void acsave_t::event_stop_and_flush()
{
    if( bflush.data && (!b_flushed)){
        cfg_t* lcfg = cfg;
        if( lcfg ){
            lcfg->flush_data(fname.data,fileformat.data.get_index());
        }
    }
    bflush.data = false;
    b_flushed = true;
}

void acsave_t::event_start_recording()
{
    if( !b_prepared )
        return;
    unsigned int recframes = mha_min_1(
        (unsigned int)(tftype.srate/(float)tftype.fragsize * (float)reclen.data)
        );
    push_config(new cfg_t(ac,recframes,variables.data));
    b_flushed = false;
}

void acsave_t::prepare(mhaconfig_t& tf)
{
    tftype = tf;
    b_prepared = true;
    event_start_recording();
}

void acsave_t::release()
{
    bflush.data = true;
    event_stop_and_flush();
}

mha_spec_t* acsave_t::process(mha_spec_t* s)
{
    process();
    return s;
}

mha_wave_t* acsave_t::process(mha_wave_t* s)
{
    process();
    return s;
}

void acsave_t::process()
{
    poll_config();
    cfg->store_frame();
}

typedef struct {
    int32_t t;
    int32_t rows;
    int32_t cols;
    int32_t imag;
    int32_t namelen;
} mat4head_t;

save_var_t::save_var_t(const std::string& nm,int n,const algo_comm_t& iac)
    : data(NULL),name(nm),nframes(n),ndim(0), maxframe(0), ac(iac), framecnt(0), b_complex(false)
{
    comm_var_t v;
    if( ac.get_var(ac.handle,name.c_str(),&v) )
        throw MHA_Error(__FILE__,__LINE__,
                        "No such variable: \"%s\"",name.c_str());
    switch( v.data_type ){
    case MHA_AC_INT :
    case MHA_AC_FLOAT :
    case MHA_AC_DOUBLE :
    case MHA_AC_MHAREAL :
        ndim = v.num_entries;
        break;
    case MHA_AC_MHACOMPLEX :
        ndim = v.num_entries;
        b_complex = true;
        break;
    default:
        ndim = 0;
    }
    //DEBUG(ndim);
    //DEBUG(nframes);
    //mha_debug("%s:%d name=%s ndim=%d\n",__FILE__,__LINE__,name.c_str(),ndim);
    if( ndim * nframes > 0 ){
        if( b_complex ){
            data = new double[ndim*nframes*2];
            memset(data,0,sizeof(double)*ndim*nframes*2);
        }else{
            data = new double[ndim*nframes];
            memset(data,0,sizeof(double)*ndim*nframes);
        }
    }
}

save_var_t::~save_var_t()
{
    if( data )
        delete [] data;
}

void save_var_t::store_frame()
{
    //DEBUG(data);
    if( !data )
        return;
    //DEBUG(framecnt);
    //DEBUG(nframes);
    if( framecnt >= nframes )
        return;
    comm_var_t v;
    int err;
    if( (err = ac.get_var(ac.handle,name.c_str(),&v)) )
        throw MHA_ErrorMsg(ac.get_error(err));
    unsigned int local_ndim = v.num_entries;
    if( local_ndim > ndim )
        local_ndim = ndim;
    unsigned int k;
    switch( v.data_type ){
        case MHA_AC_INT :
            for( k=0;k<local_ndim;k++)
                data[k*nframes+framecnt] = ((int*)v.data)[k];
            for( k=local_ndim; k<ndim; k++)
                data[k*nframes+framecnt] = 0;
            break;
        case MHA_AC_FLOAT :
            for( k=0;k<local_ndim;k++)
                data[k*nframes+framecnt] = ((float*)v.data)[k];
            for( k=local_ndim; k<ndim; k++)
                data[k*nframes+framecnt] = 0;
            break;
        case MHA_AC_DOUBLE :
            for( k=0;k<local_ndim;k++)
                data[k*nframes+framecnt] = ((double*)v.data)[k];
            for( k=local_ndim; k<ndim; k++)
                data[k*nframes+framecnt] = 0;
            break;
        case MHA_AC_MHAREAL :
            for( k=0;k<local_ndim;k++)
                data[k*nframes+framecnt] = ((mha_real_t*)v.data)[k];
            for( k=local_ndim; k<ndim; k++)
                data[k*nframes+framecnt] = 0;
            break;
        case MHA_AC_MHACOMPLEX :
            for( k=0;k<local_ndim;k++){
                data[2*(k*nframes+framecnt)] = ((mha_complex_t*)v.data)[k].re;
                data[2*(k*nframes+framecnt)+1] = ((mha_complex_t*)v.data)[k].im;
            }
            for( k=local_ndim; k<ndim; k++){
                data[2*(k*nframes+framecnt)] = 0;
                data[2*(k*nframes+framecnt)+1] = 0;
            }
            break;
        default:
            return;
    }
    //DEBUG(maxframe);
    framecnt++;
    if( framecnt > maxframe )
        maxframe = framecnt;
}

void save_var_t::save_txt(FILE* fh,unsigned int writeframes)
{
    if( !data )
        return;
    if( writeframes > maxframe )
        writeframes = maxframe;
    fprintf(fh,"# %s\n",name.c_str());
    unsigned int kfr, kd;
    for(kfr=0;kfr<writeframes;kfr++)
        for(kd=0;kd<ndim;kd++){
            if( b_complex )
                fprintf(fh,"(%g;%g)%s",
                        data[2*(kd*nframes+kfr)],
                        data[2*(kd*nframes+kfr)+1],
                        (kd<ndim-1)?"\t":"\n");
            else
                fprintf(fh,"%g%s",data[kd*nframes+kfr],(kd<ndim-1)?"\t":"\n");
        }
    fprintf(fh,"\n\n");
}

void save_var_t::save_m(FILE* fh,unsigned int writeframes)
{
    if( !data )
        return;
    if( writeframes > maxframe )
        writeframes = maxframe;
    fprintf(fh,"%% data for variable %s\n",name.c_str());
    fprintf(fh,"%s = [ ...\n",name.c_str());
    unsigned int kfr, kd;
    for(kfr=0;kfr<writeframes;kfr++)
        for(kd=0;kd<ndim;kd++){
            if( b_complex )
                fprintf(fh,"(%g+i*%g)%s",
                        data[2*(kd*nframes+kfr)],
                        data[2*(kd*nframes+kfr)+1],
                        (kd<ndim-1)?" ":"; ...\n");
            else
                fprintf(fh,"%g%s",data[kd*nframes+kfr],(kd<ndim-1)?" ":"; ...\n");
        }
    fprintf(fh,"\n];\n\n");
}

void save_var_t::save_mat4(FILE* fh,unsigned int writeframes)
{
    //DEBUG(writeframes);
    std::string local_name = name;
    unsigned int maxlen = 19;
    if( local_name.size() > maxlen )
        local_name.erase(maxlen,local_name.size()-maxlen);
    if( !data )
        return;
    if( writeframes > maxframe )
        writeframes = maxframe;
    //DEBUG(maxframe);
    CHECK_VAR(fh);
    // fill the matlab header and write to disk:
    mat4head_t m4h;
    m4h.t = 0000;
    m4h.rows = writeframes;
    m4h.cols = ndim;
    m4h.imag = b_complex;
    m4h.namelen = local_name.size()+1;
    fwrite(&m4h,sizeof(m4h),1,fh);
    // create the variable name data and write to disk:
    char* cstr = new char[m4h.namelen];
    strncpy(cstr,local_name.c_str(),m4h.namelen-1);
    cstr[m4h.namelen-1] = 0;
    fwrite(cstr,1,m4h.namelen,fh);
    delete [] cstr;
    // copy the matrix and write to disk:
    double* newdata = new double[writeframes];
    unsigned int kdim,kfr;
    // first, save real part:
    //DEBUG(ndim);
    for(kdim=0;kdim<ndim;kdim++){
        for(kfr=0;kfr<writeframes;kfr++){
            if( b_complex )
                newdata[kfr] = data[2*(kdim*nframes+kfr)];
            else
                newdata[kfr] = data[kdim*nframes+kfr];
        }
        fwrite(newdata,sizeof(double),writeframes,fh);
    }
    // now save imaginary part, iv available:
    if( b_complex )
        for(kdim=0;kdim<ndim;kdim++){
            for(kfr=0;kfr<writeframes;kfr++)
                newdata[kfr] = data[2*(kdim*nframes+kfr)+1];
            fwrite(newdata,sizeof(double),writeframes,fh);
        }
    // ok, saved.
    delete [] newdata;
}

}

MHAPLUGIN_CALLBACKS(acsave,acsave::acsave_t,spec,spec)
MHAPLUGIN_PROC_CALLBACK(acsave,acsave::acsave_t,wave,wave)
MHAPLUGIN_DOCUMENTATION(acsave,
        "AC-variables",
        "The 'acsave' plugin can save numeric algorithm communication variables\n"
        "(AC variables) into files. The files can have plain text, MATLAB 4.x\n"
        "or MATLAB script format. Each signal frame represents a row. The\n"
        "number of columns is gathered at preparation time. If a variable size\n"
        "is increased after preparation, only the part available at preparation\n"
        "time is stored. If the size is decreased, it is zero-padded to the\n"
        "original size.\n"
        "\n"
        "To save the data to disk, first set up file name and type. Then\n"
        "setting the maximal length will start the recording. At any time, set\n"
        "'flush' to yes in order to save the recorded frames. This will\n"
        "overwrite previously written data.\n"
        "\n"
        "File name and type can be changed at any time and have to be valid\n"
        "when sending the flush command.\n")
    

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
