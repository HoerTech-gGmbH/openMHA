// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2010 2012 2013 2014 2015 2017 HörTech gGmbH
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

namespace matrixmixer {

class cfg_t {
    public:
        cfg_t(std::vector<std::vector<float> > imixer,
              unsigned int ci,
              unsigned int co,
              unsigned int fragsize,
          unsigned int nfft);
    mha_wave_t* process(mha_wave_t*);
    mha_spec_t* process(mha_spec_t*);
private:
    MHASignal::waveform_t m;
    MHASignal::waveform_t wout;
    MHASignal::spectrum_t sout;
};

class matmix_t : public MHAPlugin::plugin_t<cfg_t> {
public:
    matmix_t(const algo_comm_t&,
             const std::string&,
             const std::string&);
    void prepare(mhaconfig_t&);
    mha_wave_t* process(mha_wave_t*);
    mha_spec_t* process(mha_spec_t*);
private:
    void update_m();
    MHAEvents::patchbay_t<matmix_t> patchbay;
    MHAParser::mfloat_t mixer;
    unsigned int ci;
    unsigned int co;
};

matmix_t::matmix_t(const algo_comm_t& iac,
                   const std::string&,
                   const std::string&)
    : MHAPlugin::plugin_t<cfg_t>("Matrix mixer",iac),
      mixer("mixer matrix, one row vector for each output channel","[[1 0];[0 1]]"),
      ci(0),
      co(0)
{
    insert_item("m",&mixer);
    patchbay.connect(&mixer.writeaccess,this,&matmix_t::update_m);
}

void matmix_t::prepare(mhaconfig_t& tf)
{
    co = mixer.data.size();
    ci = mixer.data[0].size();
    if( tf.channels != ci )
        throw MHA_Error(__FILE__,__LINE__,
                        "Matrixmixer: The mixer matrix has %d inputs, but the plugin received %d.",
                        ci,tf.channels);
    tf.channels = co;
    tftype = tf;
    update_m();
}

mha_wave_t* matmix_t::process(mha_wave_t* s)
{
    poll_config();
    return cfg->process(s);
}

mha_spec_t* matmix_t::process(mha_spec_t* s)
{
    poll_config();
    return cfg->process(s);
}

mha_wave_t* cfg_t::process(mha_wave_t* s)
{
    unsigned int ki,ko,kfr;
    if( s->num_frames != wout.num_frames )
        throw MHA_Error(__FILE__,__LINE__,
                        "matrixmixer: Invalid input fragment size (%d, expected %d).",
                        s->num_frames,wout.num_frames);
    if( s->num_channels != m.num_frames )
        throw MHA_Error(__FILE__,__LINE__,
                        "matrixmixer: Invalid input channel count (%d, expected %d).",
                        s->num_channels,m.num_frames);
    clear(wout);
    for(ko=0; ko<m.num_channels; ko++)
        for(ki=0; ki<m.num_frames; ki++)
            if( m(ki,ko) != 0 )
                for(kfr=0; kfr<wout.num_frames;kfr++)
                    wout(kfr,ko) += value(s,kfr,ki) * m(ki,ko);
    return &wout;
}

mha_spec_t* cfg_t::process(mha_spec_t* s)
{
    mha_complex_t temp;
    unsigned int ki,ko,kfr;
    if( s->num_frames != sout.num_frames )
        throw MHA_Error(__FILE__,__LINE__,
                        "matrixmixer: Invalid input fragment size (%d, expected %d).",
                        s->num_frames,wout.num_frames);
    if( s->num_channels != m.num_frames )
        throw MHA_Error(__FILE__,__LINE__,
                        "matrixmixer: Invalid input channel count (%d, expected %d).",
                        s->num_channels,m.num_frames);
    clear(sout);
    for(ko=0; ko<m.num_channels; ko++)
        for(ki=0; ki<m.num_frames; ki++)
            if( m(ki,ko) != 0 )
                for(kfr=0; kfr<sout.num_frames;kfr++){
        temp = value(s,kfr,ki);
        temp *= m(ki,ko);
                    sout(kfr,ko) += temp;
    }
    return &sout;
}

cfg_t::cfg_t(std::vector<std::vector<float> > imixer,
             unsigned int ci,
             unsigned int co,
             unsigned int fragsize,
             unsigned int nfft)
    : m(ci,co),
      wout(fragsize,co),
      sout(nfft/2+1,co)
{
    unsigned int ki, ko;
    if( co != imixer.size() ){
        throw MHA_Error(__FILE__,__LINE__,
                        "Mismatching number of input channels (co:%d, m:%d).",
                        co,imixer.size());
    }
    for(ko = 0; ko < co; ko++)
        if( imixer[ko].size() != ci ){
            throw MHA_Error(__FILE__,__LINE__,
                            "Mismatching number of input channels (ci:%d, m[%d]:%d).",
                            ci,ko,imixer[ko].size());
        }
    for(ko=0;ko<co;ko++)
        for(ki=0;ki<ci;ki++)
            m(ki,ko) = imixer[ko][ki];
}

void matmix_t::update_m(void)
{
    unsigned int lci = 1, lco = 1;
    if( (ci == 0) && (co == 0) ){
        lco = mixer.data.size();
        if( lco )
            lci = mixer.data[0].size();
    }else{
        lci = ci;
        lco = co;
    }
    push_config(new cfg_t(mixer.data,lci,lco,tftype.fragsize,tftype.fftlen));
}

}

MHAPLUGIN_CALLBACKS(matrixmixer,matrixmixer::matmix_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(matrixmixer,matrixmixer::matmix_t,spec,spec)
MHAPLUGIN_DOCUMENTATION(matrixmixer,"signalflow","")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
