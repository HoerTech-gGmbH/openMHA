// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2018 2019 2021 HörTech gGmbH
// Copyright © 2022 Hörzentrum Oldenburg gGmbH
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

#include "fshift.hh"
#include <cmath>

int fshift::fft_find_bin(mha_real_t frequency, unsigned fftlen, mha_real_t srate){
  if(frequency<-srate/2 or frequency>srate/2){
    throw MHA_Error(__FILE__,__LINE__,"fft_find_bin: Frequency %.0f is out of range [0, %0.f].",frequency,srate/2);
  }
  return round(frequency*fftlen/srate);
}

fshift::fshift_config_t::fshift_config_t(fshift_t const * const plug)
  :  kmin(fft_find_bin(plug->fmin(),plug->input_cfg().fftlen,plug->input_cfg().srate)),
     kmax(fft_find_bin(plug->fmax(),plug->input_cfg().fftlen,plug->input_cfg().srate)),
     df(fft_find_bin(plug->df(),plug->input_cfg().fftlen,plug->input_cfg().srate)),
     delta_phi{std::cos(plug->input_cfg().fragsize*plug->df()*static_cast<mha_real_t>(M_PI)*2.0f/plug->input_cfg().srate),
               std::sin(plug->input_cfg().fragsize*plug->df()*static_cast<mha_real_t>(M_PI)*2.0f/plug->input_cfg().srate)},
     delta_phi_total{1.0f,0.0f}
{}

mha_spec_t *fshift::fshift_config_t::process(mha_spec_t *in)
{
  if(df==0){
    return in;
  }
  for(unsigned ch=0U; ch<in->num_channels; ++ch){
    if(df<0){
      for(unsigned fr=kmin; fr<kmax; ++fr){
        int idx=fr+df;
        if(idx<static_cast<int>(in->num_frames) and idx>=0){
          value(in,idx,ch)+=value(in,fr,ch)*delta_phi_total;
          value(in, fr, ch)={0,0};
        }
      }
    }
    else{
      for(unsigned fr=kmax; (fr+1)>kmin; --fr){
        int idx=fr+df;
        if(idx<static_cast<int>(in->num_frames) and idx>=0){
          value(in,idx,ch)+=value(in,fr,ch)*delta_phi_total;
          value(in, fr, ch)={0,0};
        }
      }
    }
  }
  delta_phi_total*=delta_phi;
  delta_phi_total=delta_phi_total/abs(delta_phi_total);
  return in;
}

/** Constructs our plugin. */
fshift::fshift_t::fshift_t(MHA_AC::algo_comm_t & iac, const std::string &)
       : MHAPlugin::plugin_t<fshift_config_t>("",iac),
  m_fmin("lower boundary for frequency shifter in Hz","4000","[0,]"),
  m_fmax("upper boundary for frequency shifter in Hz","16000","[0,]"),
  m_df("shift frequency in Hz","40","")
{
  insert_item("fmin",&m_fmin);
  patchbay.connect(&m_fmin.valuechanged, this, &fshift::fshift_t::update_cfg);
  insert_item("fmax",&m_fmax);
  patchbay.connect(&m_fmax.valuechanged, this, &fshift::fshift_t::update_cfg);
  insert_item("df",&m_df);
  patchbay.connect(&m_df.valuechanged, this, &fshift::fshift_t::update_cfg);
}

fshift::fshift_t::~fshift_t() {}

/** Plugin preparation.
 *  An opportunity to validate configuration parameters before instantiating a configuration.
 * @param signal_info
 *   Structure containing a description of the form of the signal (domain,
 *   number of channels, frames per block, sampling rate.
 */
void fshift::fshift_t::prepare(mhaconfig_t & signal_info)
{
  if (signal_info.domain != MHA_SPECTRUM)
    throw MHA_Error(__FILE__, __LINE__,
                    "This plugin can only process spectrum signals.");


  
  mha_real_t frate = 0.5*signal_info.srate;
  if( m_df.data > frate )
    m_df.data= frate;
  if( m_df.data < -frate )
    m_df.data = -frate;
  if(m_fmax.data > frate)
    m_fmax.data=frate;
  /* make sure that a valid runtime configuration exists: */
  update_cfg();
}

void fshift::fshift_t::update_cfg()
{
    if ( is_prepared() ) {

        //when necessary, make a new configuration instance
        //possibly based on changes in parser variables
        fshift_config_t *config;
        config = new fshift_config_t( this );
        push_config( config );
    }
}

/**
 * Checks for the most recent configuration and defers processing to it.
 */
mha_spec_t * fshift::fshift_t::process(mha_spec_t * signal)
{
    //this stub method defers processing to the configuration class
    return poll_config()->process( signal );
}

/*
 * This macro connects the plugin1_t class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(fshift,fshift::fshift_t,
                    spec,spec)

/*
 * This macro creates code classification of the plugin and for
 * automatic documentation.
 *
 * The first argument to the macro is a space separated list of
 * categories, starting with the most relevant category. The second
 * argument is a LaTeX-compatible character array with some detailed
 * documentation of the plugin.
 */
MHAPLUGIN_DOCUMENTATION
(fshift::fshift_t,
 "feedback-suppression frequency-modification",
 " Performs a quantized frequency shift on the selected frequency"
 " interval. "
 " The frequency band between (originally)"
 " \\texttt{fmin} and \\texttt{fmax} (frequencies in Hz)"
 " is shifted by \\texttt{df} (desired frequency change"
 " in Hz). "
 " Positive \\texttt{df} shifts the selected band to"
 " higher frequencies, negative \\texttt{df} shifts to"
 " lower frequencies."
 " \n\n"
 " The shifted and the unshifted parts of the"
 " input signal are split at the STFT bin boundaries nearest to the."
 " The frequency shift \\texttt{df} is rounded to the nearest bin as well."
 " The parts of the spectrum that would be shifted below 0 Hz or above the Nyquist"
 " frequency are discarded. "
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 2
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
