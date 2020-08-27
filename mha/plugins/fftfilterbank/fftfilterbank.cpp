// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2009 2010 2011 2013 2014 2015 2016 HörTech gGmbH
// Copyright © 2017 2018 2019 2020 HörTech gGmbH
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

#include "mha_toolbox.h"
#include "mha_plugin.hh"
#include "mha_fftfb.hh"
#include <math.h>
#include <fstream>
#include <iostream>
#include "mha_events.h"

namespace fftfilterbank {

class fftfb_plug_t : public MHAOvlFilter::overlap_save_filterbank_analytic_t {
public:
    fftfb_plug_t(MHAOvlFilter::overlap_save_filterbank_t::vars_t&,mhaconfig_t chcfg,algo_comm_t ac,std::string alg,bool return_imag);
    mha_spec_t* process(mha_spec_t*);
    mha_wave_t* process(mha_wave_t*);
    void insert();
private:
    MHAOvlFilter::fftfb_ac_info_t fb_acinfo;
    MHASignal::spectrum_t s_out;
    MHA_AC::waveform_t imag;
    bool return_imag_;
};

class fftfb_interface_t : public MHAPlugin::plugin_t<fftfb_plug_t>,
                    public MHAOvlFilter::overlap_save_filterbank_t::vars_t {
public:
    fftfb_interface_t(const algo_comm_t& ac,
                const std::string& th,
                const std::string& al);
    void prepare(mhaconfig_t&);
    void release();
    mha_spec_t* process(mha_spec_t*);
    mha_wave_t* process(mha_wave_t*);
private:
    MHAParser::bool_t return_imag;
    MHAEvents::patchbay_t<fftfb_interface_t> patchbay;
    void update_cfg();
    MHA_AC::int_t nchannels;
    std::string algo;
    bool prepared;
    unsigned int nbands;
};

/** 
Default values are set and MHA configuration variables registered into the parser.

\param ac     algorithm communication handle
\param th     chain name
\param al     algorithm name
*/
fftfb_interface_t::fftfb_interface_t(const algo_comm_t& ac,
                         const std::string& th,
                         const std::string& al)
    : MHAPlugin::plugin_t<fftfb_plug_t>("FFT based filterbank with overlapping filters",ac),
      MHAOvlFilter::overlap_save_filterbank_t::vars_t(static_cast<MHAParser::parser_t&>(*this)),
      return_imag("Return imaginary part? Results are stored in AC variable '<plugname>_imag'.","no"),
      nchannels(ac,al+"_nchannels"),
      algo(al),
      prepared(false),
      nbands(0)
{
    insert_member(return_imag);
    set_node_id("fftfilterbank");
    patchbay.connect(&writeaccess,this,&fftfb_interface_t::update_cfg);
}

//! Prepare all variables for processing.
/*!

In this function, all variables are initialised and the filter
shapes for each band are calculated. The filter shapes \f$W(f)\f$
are defined as

\f[
W(f) = W(T(S(f))) = W(x),\quad x = T(S(f))=T(\hat f),
\f]

\f$W(x)\f$ beeing a symmetric window function in the interval
\f$[-1,1]\f$ and \f$S(f)\f$ the transformation from the linear scale
to the given frequency scale (see functions in FreqScaleFun). The
function \f$T(\hat f)\f$ transforms the frequency range between the
center frequencies \f$[\hat f_{k-1},\hat f_k]\f$ and \f$[\hat
f_k,\hat f_{k+1}]\f$ into the interval \f$[-1,0]\f$ and \f$[0,1]\f$,
respectively. This function is realised by the function linscale().

\param tf     Channel configuration

*/
void fftfb_interface_t::prepare(mhaconfig_t& tf)
{
    prepared = true;
    try{
        tftype = tf;
        fftlen.setlock(true);
        if( tf.domain == MHA_SPECTRUM )
            fftlen.data = tftype.fftlen;
        nchannels.data = tf.channels;
        fftfb_plug_t* NewConfig = new fftfb_plug_t(static_cast<MHAOvlFilter::overlap_save_filterbank_t::vars_t&>(*this),tftype,ac,algo,return_imag.data);
        nbands = NewConfig->nbands();
        push_config(NewConfig);
        poll_config();
        cfg->insert();
        tf.channels *= nbands;
    }
    catch(MHA_Error&e){
    fftlen.setlock(false);
        prepared = false;
        throw e;
    }
}

void fftfb_interface_t::release()
{
    fftlen.setlock(false);
    prepared = false;
}

void fftfb_interface_t::update_cfg()
{
    if( prepared ){
        fftfb_plug_t* NewConfig = new fftfb_plug_t(static_cast<MHAOvlFilter::overlap_save_filterbank_t::vars_t&>(*this),tftype,ac,algo,return_imag.data);
        if( NewConfig->nbands() != nbands ){
            unsigned int newnbands(NewConfig->nbands());
            delete NewConfig;
            throw MHA_Error(__FILE__,__LINE__,"Filterbank size cannot change at runtime from %u to %u bands.",
                            nbands,newnbands);
        }
        push_config(NewConfig);
    }
}

mha_spec_t* fftfb_interface_t::process(mha_spec_t* s)
{
    return poll_config()->process(s);
}

mha_wave_t* fftfb_interface_t::process(mha_wave_t* s)
{
    return poll_config()->process(s);
}

fftfb_plug_t::fftfb_plug_t(MHAOvlFilter::overlap_save_filterbank_t::vars_t& vars,mhaconfig_t chcfg,algo_comm_t ac,std::string alg,bool return_imag)
    : MHAOvlFilter::overlap_save_filterbank_analytic_t(vars,chcfg),
      fb_acinfo(*this,ac,alg),
      s_out(chcfg.fftlen/2+1,chcfg.channels*nbands()),
      imag(ac,alg+"_imag",chcfg.fragsize,chcfg.channels*nbands(),false),
      return_imag_(return_imag)
{
}

void fftfb_plug_t::insert()
{
    fb_acinfo.insert();
    if( return_imag_ )
        imag.insert();
}

mha_spec_t* fftfb_plug_t::process(mha_spec_t* s)
{
    unsigned int ch, kfb, kfr, oidx, iidx;
    memset(s_out.buf,0,sizeof(s_out.buf[0])*s_out.num_frames*s_out.num_channels);
    insert();
    for(ch=0;ch<s->num_channels;ch++){
        for(kfb=0; kfb<nbands(); kfb++){
            for(kfr=bin1(kfb); kfr < bin2(kfb); kfr++){
                oidx = (ch*nbands()+kfb)*s->num_frames+kfr;
                iidx = ch*s->num_frames+kfr;
                s_out.buf[oidx].re = w(kfr,kfb) * s->buf[iidx].re;
                s_out.buf[oidx].im = w(kfr,kfb) * s->buf[iidx].im;
            }
        }
    }
    return &s_out;
}

mha_wave_t* fftfb_plug_t::process(mha_wave_t* s)
{
    insert();
    if( return_imag_ ){
        mha_wave_t* sRe;
        mha_wave_t* sIm;
        filter_analytic(s,&sRe,&sIm);
        imag.copy(*sIm);
        return sRe;
    }else{
        mha_wave_t* sRet;
        filter(s,&sRet);
        return sRet;
    }
}

}

MHAPLUGIN_CALLBACKS(fftfilterbank,fftfilterbank::fftfb_interface_t,spec,spec)
MHAPLUGIN_PROC_CALLBACK(fftfilterbank,fftfilterbank::fftfb_interface_t,wave,wave)
MHAPLUGIN_DOCUMENTATION\
(fftfilterbank,
 "filterbank",
 "This plugin implements a linear phase filterbank based on FFT spectrum."
 " Each filter\n"
 "bank channel is stored into an own audio channel. The number of output\n"
 "channels of this plugin is the number of frequency bands times the\n"
 "number of input channels.\n"
 "\n"
 "Please use the iFFT plugin {\\em spec2wave}"
 " (p. \\pageref{plug:spec2wave}) to get the\n"
 "waveform signal of the filterbank output. The {\\em matrixmixer}\n"
 "(p. \\pageref{plug:matrixmixer})\n"
 "plugin or {\\em combinechannels}"
 " (p. \\pageref{plug:combinechannels}) can be used for resynthesis.\n"
 "\n"
 "The filters are calculated by applying filter weights to each FFT\n"
 "bin. These weights (filter shapes) depend on the settings of the\n"
 "{\\tt ftype} variable. If {\\tt center} is selected, the frequency\n"
 "interval between the lower neighbour center frequency and the desired\n"
 "center frequency is mapped to the interval [-1,0] and between the\n"
 "desired center frequency and the upper neightbour to the interval\n"
 "[0,1]. These mappings are linear on the given frequency scale so that\n"
 "a value of 0.5 denotes the middle between two neighboured center\n"
 "frequencies on the given frequency scale. The filter weights are\n"
 "calculated with the configured crossing function on this interval, see\n"
 "next figure for details. Please note that the filters are not\n"
 "necessarily symmetric (symmetry is achieved only if the center\n"
 "frequencies are equally spaced on the desired frequency scale). The\n"
 "lowest and highest filter channels include the full range from zero to\n"
 "the center frequency or from the center frequency to the nyquist\n"
 "frequency, respectively.\n"
 "\n"
 "If {\\tt edge} is selected, then the frequency axis is transformed to be\n"
 "linear on the desired frequency scale. The interval between two edge\n"
 "frequencies is mapped to [-0.5,0.5]. Now, the filter shape function\n"
 "(rectangular, linear/sawtooth, hanning) is applied to the frequency\n"
 "axis. This results in symmetric filters on the desired frequency\n"
 "scale.\n"
 "\n"
 "\\MHAfigure{Schematic plot of overlapping filters}{ovlfftfilter_shapes}\n"
 "\\MHAfigure{Example filter shapes with center frequencies configured}"
 "{fftfb_shapes_center}\n"
 "\\MHAfigure{Example filter shapes with edge frequencies configured}"
 "{fftfb_shapes_edge}\n"
 )

    // Local Variables:
    // compile-command: "make"
    // c-basic-offset: 4
    // indent-tabs-mode: nil
    // coding: utf-8-unix
    // End:
