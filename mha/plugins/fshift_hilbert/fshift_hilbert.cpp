// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2008 2009 2010 2011 2013 2014 2015 2017 2018 2019 HörTech gGmbH
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
#include "mha_parser.hh"
#include "mha_defs.h"
#include "mha_events.h"
#include "mha_filter.hh"
#include <math.h>
#include <limits>
#include "sfftw.h"
#include "srfftw.h"

/** All types for the hilbert frequency shifter live in this namespace */
namespace fshift_hilbert {

    class hilbert_shifter_t : public MHASignal::spectrum_t
    {
    public:
        hilbert_shifter_t(unsigned int fftlen,
                          unsigned int channels,
                          mha_real_t srate,
                          unsigned int kmin,
                          unsigned int kmax,
                          std::vector<mha_real_t> dphi,
                          unsigned int frameshift,
                          unsigned int maxirslen,
                          unsigned int phasemode);
        ~hilbert_shifter_t();
        void process(mha_spec_t*);
    private:
        /** Part of the spectrum to be frequency shifted */
        MHASignal::spectrum_t fullspec;
        /** Analytic signal, defined as a(t)=x(t)+i*H(x(t)) */
        MHASignal::spectrum_t analytic;
        /** The frequency shifted signal in the time domain */
        MHASignal::waveform_t shifted;
        /** Helper varaible containing the coefficients used to
         * split the spectrum. Contains 1 for every fft bin to be
         * frequency shifted, 0 for all others */
        MHASignal::spectrum_t mixw_shift;
        /** Helper varaible containing the coefficients used to
         * split the spectrum. Contains 0 for every fft bin to be
         * frequency shifted, 1 for all others */
        MHASignal::spectrum_t mixw_ref;
        /** FFT plan for the transformation of fullspec into the time domain */
        fftw_plan plan_spec2analytic;
        /** MHA wrapper object for fftw */
        mha_fft_t mhafft;
        /** Vector holding one delta f value for every channel */
        MHASignal::waveform_t df;
        /** FFT frame that corresponds to f_min */
        unsigned int kmin;
        /** FFT frame that corresponds to f_max */
        unsigned int kmax;
        /** Total phase advance within one fragment */
        unsigned int frameshift;
        /** Phase advance per frame */
        std::vector<mha_complex_t> delta_phi;
        /** Sum of all phase advances */
        std::vector<mha_complex_t> delta_phi_total;
    };

    class frequency_translator_t : public MHAPlugin::plugin_t<hilbert_shifter_t> {
    public:
        frequency_translator_t(const algo_comm_t&,const std::string&,const std::string&);
        mha_spec_t* process(mha_spec_t*);
        void prepare(mhaconfig_t&);
        void release();
    private:
        void update();
        MHAEvents::patchbay_t<frequency_translator_t> patchbay;
        /** Vector containing the shift frequencies in Hz */
        MHAParser::vfloat_t df;
        /** Lower boundary for frequency shifter */
        MHAParser::float_t fmin;
        /** Upper boundary for frequency shifter */
        MHAParser::float_t fmax;
        /** Maximum length of cut off filter response */
        MHAParser::int_t irslen;
        /** Mode of gain smoothing */
        MHAParser::kw_t phasemode;
    };
}

fshift_hilbert::hilbert_shifter_t::hilbert_shifter_t(unsigned int fftlen,
                                                     unsigned int channels,
                                                     mha_real_t srate,
                                                     unsigned int kmin_,
                                                     unsigned int kmax_,
                                                     std::vector<mha_real_t> df_,
                                                     unsigned int frameshift_,
                                                     unsigned int maxirslen,
                                                     unsigned int phasemode)
: MHASignal::spectrum_t(fftlen/2+1,channels),
    fullspec(fftlen,channels),
    analytic(fftlen,channels),
    shifted(fftlen,channels),
    mixw_shift(fftlen/2+1,channels),
    mixw_ref(fftlen/2+1,channels),
    df(MHASignal::dupvec_chk(df_,channels)),
    kmin(kmin_),
    kmax(kmax_),
    frameshift(frameshift_),
    delta_phi(channels,{1,0}),
    delta_phi_total(channels,{1,0})
{
    mhafft = mha_fft_new(fftlen);
    plan_spec2analytic = fftw_create_plan( fftlen, FFTW_BACKWARD, FFTW_ESTIMATE );
    unsigned int k, ch;
    for(ch=0; ch<channels;ch++){
        df[ch] *= M_PI*2.0/srate;
        expi(delta_phi[ch],df[ch]);
        for(k=0;k<fftlen/2+1;k++){
            if( (k >= kmin) && (k <= kmax) )
                mixw_shift.value(k,ch) = mha_complex(1,0);
            else
                mixw_ref.value(k,ch) = mha_complex(1,0);
        }
    }
    MHAFilter::smoothspec_t* smsp;
    switch( phasemode ){
    case 0 : // none
        smsp = NULL;
        break;
    case 1 : // linear
        smsp = new MHAFilter::smoothspec_t(fftlen,channels,MHAWindow::fun_t(maxirslen,&MHAWindow::hanning,-1,1),false);
        break;
    case 2 : // minimal
        smsp = new MHAFilter::smoothspec_t(fftlen,channels,MHAWindow::fun_t(maxirslen,&MHAWindow::hanning,0,1),true);
        break;
    default:
        throw MHA_Error(__FILE__,__LINE__,"Invalid phase mode %u",phasemode);
    }
    if( smsp ){
        smsp->smoothspec( mixw_shift );
        smsp->smoothspec( mixw_ref );
        delete smsp;
    }
}

fshift_hilbert::hilbert_shifter_t::~hilbert_shifter_t()
{
    mha_fft_free(mhafft);
}

void fshift_hilbert::hilbert_shifter_t::process(mha_spec_t* s)
{
    unsigned int k, ch;
    clear(fullspec);
    //Split spectrum into the parts within kmin,kmax and the part without
    for(ch=0;ch<num_channels;ch++){
        for(k=0;k<s->num_frames;k++){
            fullspec(k,ch) = ::value(s,k,ch);
            fullspec(k,ch) *= 2;
            fullspec(k,ch) *= mixw_shift(k,ch);
            ::value(s,k,ch) *= mixw_ref(k,ch);
        }
        fullspec(0,ch) *= 0.5;
        if( 2*(s->num_frames-1) == fullspec.num_frames ) // even FFT length
            fullspec(s->num_frames-1,ch) *= 0.5;
    }
    //Create analytic signal
    for(ch=0;ch<num_channels;ch++){
        fftw_one(plan_spec2analytic,
                 (fftw_complex*)(&(fullspec.buf[fullspec.num_frames*ch])),
                 (fftw_complex*)(&(analytic.buf[analytic.num_frames*ch])));
    }
    //Perform the phase advance
    mha_complex_t reset={1,0};
    for(ch=0;ch<num_channels;ch++){
        for(k=0;k<shifted.num_frames;k++){
            //Semantically this is analytic(k,ch)*=delta_phi_total[ch], but we need only the real part
            shifted(k,ch) = delta_phi_total[ch].re*analytic(k,ch).re-delta_phi_total[ch].im*analytic(k,ch).im;
            delta_phi_total[ch]*=delta_phi[ch];
            //As fftlen>frameshift, we need to save the old phaseshift to ensure phase coherence
            if(k==frameshift-1){
                reset=delta_phi_total[ch];
            }
        }
        delta_phi_total[ch]=reset;
        delta_phi_total[ch]=delta_phi_total[ch]/abs(delta_phi_total[ch]);
    }
    //Inverse transform to obtain shifted spectrum
    mha_fft_wave2spec(mhafft,&shifted,this);
    *s += *this;
}

fshift_hilbert::frequency_translator_t::frequency_translator_t(const algo_comm_t& iac,const std::string& ith,const std::string& ial)
    : MHAPlugin::plugin_t<hilbert_shifter_t>("Pitch shifter",iac),
    df("frequency to shift the bins / Hz","40",""),
    fmin("lower boundary for frequency shifter","4000","[0,]"),
    fmax("upper boundary for frequency shifter","16000","[0,]"),
    irslen("Bandpass: maximum length of cut off filter response","1","[1,]"),
    phasemode("Bandpass: mode of gain smoothing","none","[none linear minimal]")
{
    insert_item("df",&df);
    insert_item("fmin",&fmin);
    insert_item("fmax",&fmax);
    insert_item("irslen",&irslen);
    insert_item("phasemode",&phasemode);
    patchbay.connect(&writeaccess,this,&fshift_hilbert::frequency_translator_t::update);
}

mha_spec_t* fshift_hilbert::frequency_translator_t::process(mha_spec_t* s)
{
    poll_config();
    cfg->process(s);
    return s;
}

void fshift_hilbert::frequency_translator_t::prepare(mhaconfig_t& tf)
{
    if( tf.domain != MHA_SPECTRUM )
        throw MHA_ErrorMsg("frequency_translator: Only spectral processing is supported.");
    if( tf.srate > 0 ){
        mha_real_t frate = 0.5*tf.srate;
        for(unsigned int k=0;k<df.data.size();k++){
            if( df.data[k] > frate )
                df.data[k] = frate;
            if( df.data[k] < -frate )
                df.data[k] = -frate;
        }
        char tmp[140];
        sprintf(tmp,"[%g,%g]",-frate,frate);
        df.set_range(tmp);
    }
    tftype = tf;
    update();
}

void fshift_hilbert::frequency_translator_t::release()
{
    df.set_range("");
}

void fshift_hilbert::frequency_translator_t::update()
{
    if( tftype.srate > 0 )
        push_config(new hilbert_shifter_t(
                                          tftype.fftlen,tftype.channels,tftype.srate,
                                          (unsigned int)(fmin.data*tftype.fftlen/tftype.srate),
                                          (unsigned int)(fmax.data*tftype.fftlen/tftype.srate),
                                          df.data,
                                          tftype.fragsize,
                                          irslen.data,
                                          phasemode.data.get_index()
                                          ));
}

MHAPLUGIN_CALLBACKS(fshift_hilbert,fshift_hilbert::frequency_translator_t,spec,spec)
MHAPLUGIN_DOCUMENTATION\
(fshift_hilbert,
 "feedback-suppression frequency-modification",
 " Performs a frequency shift on the selected frequency"
 " interval. "
 " The frequency band between (originally)"
 " \\texttt{fmin} and \\texttt{fmax} (frequencies in Hz)"
 " is shifted by \\texttt{df} (desired frequency change"
 " in Hz). "
 " Positive \\texttt{df} shifts the selected band to"
 " higher frequencies, negative \\texttt{df} shifts to"
 " lower frequencies."
 " \n\n"
 " The frequency shift on the sub-band is performed by"
 " splitting the input signal's spectrum into 2 parts:"
 " the band to be shifted, and the rest. "
 " The band to be shifted is multiplied in the time "
 " domain with a complex sinusoid of frequency "
 " \\texttt{df} Hz (see Wardle(1998)\\footnote{"
 "  Scott Wardle. A hilbert-transformer frequency shifter"
 "  Proc. DAFX98 Workshop on Digital Audio Effects, pages"
 "  25–29, Barcelona, 1998."
 " }) before it is recombined in the spectral domain with"
 " the unshifted signal part."
 " \n\n"
 " By default the shifted and the unshifted parts of the"
 " input signal are split at STFT bin boundaries.  The"
 " resulting rectangular transitions between shifted and"
 " unshifted parts can be smoothed if desired by setting"
 " \\texttt{phasemode} to \\texttt{linear} or"
 " \\texttt{minimal}, and choosing a longer impulse"
 " response length than the default of 1 sample. "
 " Our experience with hearing aid applications so far"
 " suggests that  smoothing these boundaries is not"
 " necessary."
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
