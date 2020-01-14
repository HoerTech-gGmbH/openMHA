// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2009 2010 2013 2014 2015 2018 2019 2020 HörTech gGmbH
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

namespace shadowfilter_end {

class cfg_t {
public:
    cfg_t(int nfft_, algo_comm_t ac_, std::string name_);
    mha_spec_t* process(mha_spec_t*);
private:
    algo_comm_t ac;
    std::string name;
    int nfft;
    int ntracks;
    int nch_out;
    mha_spec_t in_spec;
    MHASignal::spectrum_t out_spec;
    MHA_AC::spectrum_t gains;
};

cfg_t::cfg_t(int nfft_, algo_comm_t ac_, std::string name_)
    : ac(ac_),
      name(name_),
      nfft(nfft_),
      ntracks(MHA_AC::get_var_int(ac,name+"_ntracks")),
      nch_out(MHA_AC::get_var_int(ac,name+"_nch")),
      in_spec(MHA_AC::get_var_spectrum(ac,name)),
      out_spec(nfft/2+1,in_spec.num_channels),
      gains(ac,name+"_gains",nfft/2+1,nch_out,true)
{
    if( (int)in_spec.num_frames != nfft/2+1 )
        throw MHA_ErrorMsg("Mismatching signal dimension.");
}

mha_spec_t* cfg_t::process(mha_spec_t* s)
{
    in_spec = MHA_AC::get_var_spectrum(ac,name);
    if( (int)in_spec.num_frames != nfft/2+1 )
        throw MHA_ErrorMsg("Mismatching signal dimension.");
    if( in_spec.num_frames != out_spec.num_frames )
        throw MHA_ErrorMsg("Mismatching signal dimension.");
    if( in_spec.num_channels != out_spec.num_channels )
        throw MHA_ErrorMsg("Mismatching signal dimension.");
    if( nch_out != (int)s->num_channels )
        throw MHA_Error(__FILE__,__LINE__,
                        "Error in shadowfilter_end: The input has %u channels but was configured to have %d.",
                        s->num_channels,nch_out);
    if( nch_out > (int)in_spec.num_channels )
        throw MHA_ErrorMsg("More input channels than output channels.");
    mha_real_t a2;
    unsigned int ktr;
    mha_complex_t sp;
    for(unsigned k=0;k<size(gains);k++){
        a2 = in_spec.buf[k].re * in_spec.buf[k].re + in_spec.buf[k].im * in_spec.buf[k].im;
        if (a2 == 0) {
            // avoid having NaN gains if input spectrum bins are zero
            set(gains.buf[k], 1);
        }
        else {
            gains.buf[k].re = (in_spec.buf[k].re * s->buf[k].re + in_spec.buf[k].im * s->buf[k].im) / a2;
            gains.buf[k].im = (in_spec.buf[k].re * s->buf[k].im - in_spec.buf[k].im * s->buf[k].re) / a2;
        }
        out_spec.buf[k] = s->buf[k];
        for(ktr=1;(int)ktr<ntracks;ktr++){
            sp = in_spec.buf[k+size(gains)*ktr];
            out_spec.buf[k+size(gains)*ktr].re = sp.re * gains.buf[k].re - sp.im * gains.buf[k].im;
            out_spec.buf[k+size(gains)*ktr].im = sp.re * gains.buf[k].im + sp.im * gains.buf[k].re;
        }
    }
    return &out_spec;
}

class shadowfilter_end_t : public MHAPlugin::plugin_t<cfg_t> {
public:
    shadowfilter_end_t(const algo_comm_t&,const std::string&,const std::string&);
    mha_spec_t* process(mha_spec_t*);
    void prepare(mhaconfig_t&);
private:
    MHAParser::string_t basename;
};

shadowfilter_end_t::shadowfilter_end_t(
                                       const algo_comm_t& iac,
                                       const std::string& ith,
                                       const std::string& ial)
    : MHAPlugin::plugin_t<cfg_t>("Compute spectral gains seen since shadowfilter_begin, apply gains to other tracks",iac),
      basename("configuration name of shadowfilter_begin","shadowfilter_begin")
{
    insert_item("basename",&basename);
}

mha_spec_t* shadowfilter_end_t::process(mha_spec_t* s)
{
    poll_config();
    return cfg->process(s);
}

void shadowfilter_end_t::prepare(mhaconfig_t& tf)
{
    try{
        basename.setlock(true);
        if( tf.domain != MHA_SPECTRUM )
            throw MHA_ErrorMsg("shadowfilter_end: Only spectral processing is supported.");
        if( (int)tf.channels != MHA_AC::get_var_int(ac,basename.data+"_nch") )
            throw MHA_ErrorMsg("shadowfilter_end: Invalid number of input channels.");
        mha_spec_t spec = MHA_AC::get_var_spectrum(ac,basename.data);
        tf.channels = spec.num_channels;
        tftype = tf;
        push_config(new cfg_t(tftype.fftlen,ac,basename.data));
    }
    catch(...){
        basename.setlock(false);
        throw;
    }
}

}

MHAPLUGIN_CALLBACKS(shadowfilter_end,shadowfilter_end::shadowfilter_end_t,spec,spec)
MHAPLUGIN_DOCUMENTATION\
(shadowfilter_end,
 "data-flow feature-extraction filter",
 "See section \\ref{plug:shadowfilter_begin} for a description of the\n"
 "shadow filter method. The 'shadowfilter\\_end' plugin creates an AC\n"
 "variable shadowfilter\\_gains, which contains the complex gains created\n"
 "by the algorithm.\n"
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// End:
