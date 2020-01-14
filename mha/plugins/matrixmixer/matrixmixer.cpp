// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2010 2012 2013 2014 2015 2017 2018 2019 HörTech gGmbH
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
    : MHAPlugin::plugin_t<cfg_t>("Matrix mixer plugin, can mix multiple input"
                                 " channels into\n"
                                 "any number of output channels"
                                 " with configurable weights.",iac),
      mixer("Mixer matrix, one row vector for each output channel.\n"
            "The number of columns must match the number of input channels.",
            "[[1 0];[0 1]]"),
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
                        "Matrixmixer: The mixer matrix has %u inputs, but the plugin received %u.",
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
                        "matrixmixer: Invalid input fragment size (%u, expected %u).",
                        s->num_frames,wout.num_frames);
    if( s->num_channels != m.num_frames )
        throw MHA_Error(__FILE__,__LINE__,
                        "matrixmixer: Invalid input channel count (%u, expected %u).",
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
                        "matrixmixer: Invalid input fragment size (%u, expected %u).",
                        s->num_frames,wout.num_frames);
    if( s->num_channels != m.num_frames )
        throw MHA_Error(__FILE__,__LINE__,
                        "matrixmixer: Invalid input channel count (%u, expected %u).",
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
                        "Mismatching number of input channels (co:%u, m:%zu).",
                        co,imixer.size());
    }
    for(ko = 0; ko < co; ko++)
        if( imixer[ko].size() != ci ){
            throw MHA_Error(__FILE__,__LINE__,
                            "Mismatching number of input channels (ci:%u, m[%u]:%zu).",
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
MHAPLUGIN_DOCUMENTATION\
(matrixmixer,
 "data-flow audio-channels",
 "The \\texttt{matrixmixer} plugin can combine the signal from multiple input "
 "channels into any number of output channels, with defined mixing weights."
 "\n\n"
 "Example: To combine the two channels of a stereo signal into a single (mono) "
 "channel, configure the \\texttt{matrixmixer} plugin configuration variable "
 "\\texttt{m} as"
 "\n\n"
 "\\verb|m = [[1.0 1.0]]|"
 "\n\n"
 "which causes the first and the second channel to be multiplied with a "
 "weight of 1 before they are mixed (by adding them together) to form a "
 "single output channel."
 "\n\n"
 "It is also possible to mix the channels with weights different from 1:"
 "\n\n"
 "\\verb|m = [[1 0.5]]|"
 "\n\n"
 "This attenuates the second channel by multiplying all samples in that channel "
 "with 0.5 before mixing it with the first channel.  "
 "The configuration variable \\texttt{m} expects a matrix of float values.  "
 "The examples above showed a matrix with only one row, which resulted in only "
 "one output channel being produced by the \\texttt{matrixmixer} plugin. "
 "To produce more output channels, more rows "
 "(separated by semicolons)\\footnote{In \\mha{} configuration, a matrix is "
 "  specified as a vector of vectors, where the subsequent row vectors are "
 "  separated by semicolons.  For details, refer to the subsection on "
 "  multidimensional variables in the \\mha{} application manual.} "
 "can be specified for matrix \\texttt{m}:"
 "\n\n"
 "\\verb|m = [[1 0];[0 1]]|"
 "\n\n"
 "This is the identity matrix for two channels. "
 "This matrix does not change the signal. "
 "\n\n"
 "The following setting demonstrates how matrixmixer can be used to change "
 "the order of audio channels in a multi-channel signal. "
 "This example swaps the first two channels:"
 "\n\n"
 "\\verb|m = [[0 1];[1 0]]|"
 "\n\n"
 "The next setting creates a 4-channel signal output from a stereo signal, "
 "where the first two channels are the original stereo channels, the third is "
 "the sum of the two stereo channels, and the fourth output channel is the "
 "difference of the two stereo channels"
 "\\footnote{The combination of the sum and the difference of the two channels"
 "  of a stereo signal is known as the mid-side signal and used for stereo"
 "  transmission in FM radio.  We combine it here with the orignial stereo"
 "  signal for the sole purpose of demonstrating the creation of more output"
 "  channels than input channels with the \\texttt{matrixmixer} plugin.}:"
 "\n\n"
 "\\verb|m = [[0 1];[1 0];[1 1];[1 -1]]|"
 "\n\n"
 "The following example duplicates a single input channel to two output"
 " channels:"
 "\n\n"
 "\\verb|m = [[1];[1]]|"
 "\n\n"
 "To summarize, you need to configure the variable \\texttt{m} "
 "with a matrix with float values. "
 "The matrix needs to have as many columns as the \\texttt{matrixmixer} "
 "receives input channels, and as many rows as you want \\texttt{matrixmixer} "
 "to produce output channels."
 "\n\n"
 "Example configurations and example input files are contained in the "
 "matrixmixer examples directory.  "
 "Please refer to the README file in this directory for an explanation of "
 "the different examples."
 "\n\n"
 "A matlab/octave test exercising the matrixmixer plugin in six different "
 "configurations can be found in the mhatest directory in file "
 "\\texttt{test\\_matrixmixer.m}. "
 "This test file is executed together with the other "
 "system-level tests when invoking \\texttt{make test}."
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
