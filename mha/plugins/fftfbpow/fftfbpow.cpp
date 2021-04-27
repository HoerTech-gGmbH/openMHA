// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2009 2010 2013 2014 2015 2018 2019 HörTech gGmbH
// Copyright © 2021 HörTech gGmbH
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
#include "mha_fftfb.hh"
#include "mha_events.h"

/** Namespace for the fftfbpow plugin */
namespace fftfbpow {

  /** Run time configuration for the fftfbpow plugin */
  class fftfbpow_t : public MHAOvlFilter::fftfb_t {
  public:
    /** Constructor of the run time configuration
     * @param vars Set of configuration variables for FFT-based overlapping filters
     * @param nch Number of audio input channels
     * @param nfft Length of FFT
     * @param fs Sampling rate
     * @param ac AC space
     * @param name Configured name of plugin interface, used as prefix for AC variable names
     */
  fftfbpow_t(MHAOvlFilter::fftfb_vars_t& vars,unsigned int nch,unsigned int nfft,mha_real_t fs,algo_comm_t ac,std::string name);
    /// AC variable containing the estimated power in each frequency band.
    MHA_AC::waveform_t fbpow;
  };

  /** Interface class for fftfbpow plugin. */
  class fftfbpow_interface_t : public MHAPlugin::plugin_t<fftfbpow_t>, public MHAOvlFilter::fftfb_vars_t {
  public:
    /** Constructor with standard MHA constructor parameters
     * @param iac             Handle to algorithm communication variable space
     * @param configured_name Configured name of this plugin instance
     */
    fftfbpow_interface_t(algo_comm_t iac, const std::string & configured_name);
    /** Standard MHA plugin prepare function.
     * Ensures that the input is in the frequency domain, calls update_cfg()
     * and inserts fbpow into the AC space.
     * @param tf Incoming mha configuration structure, contains information about input signal
     */
    void prepare(mhaconfig_t& tf);
    /** Standard MHA plugin process fct. Polls new config and calls process() of
     * the runtime configuration.
     * @param s Input spectrum
     * @return Unchanged input spectrum
     */
    mha_spec_t* process(mha_spec_t* s);
  private:
    /** Constructs new runtime configuration in thread-safe manner */
    void update_cfg();
    /// Configured name of this plugin instance
    std::string name;
    /// Patchbay to connect to MHA configuration interface
    MHAEvents::patchbay_t<fftfbpow_interface_t> patchbay;
  };
}

fftfbpow::fftfbpow_interface_t::fftfbpow_interface_t(algo_comm_t iac,
                                                     const std::string & configured_name)
  : MHAPlugin::plugin_t<fftfbpow_t>("FFT based filterbank analysis with overlapping filters",iac),
  MHAOvlFilter::fftfb_vars_t(static_cast<MHAParser::parser_t&>(*this)),
  name(configured_name)
{
  patchbay.connect(&writeaccess,this,&fftfbpow_interface_t::update_cfg);
}

void fftfbpow::fftfbpow_interface_t::prepare(mhaconfig_t& tf)
{
  if( tf.domain != MHA_SPECTRUM )
    throw MHA_ErrorMsg("fftfbpow: Only spectral processing is supported.");
  tftype = tf;
  update_cfg();
  poll_config();
  cfg->fbpow.insert();
}

mha_spec_t* fftfbpow::fftfbpow_interface_t::process(mha_spec_t* s){
  poll_config();
  cfg->fbpow.insert();
  cfg->get_fbpower_db(&(cfg->fbpow),s);
  return s;
}

void fftfbpow::fftfbpow_interface_t::update_cfg()
{
  if( is_prepared() )
    push_config(new fftfbpow_t(static_cast<MHAOvlFilter::fftfb_vars_t&>(*this),tftype.channels,tftype.fftlen,tftype.srate,ac,name));
}


fftfbpow::fftfbpow_t::fftfbpow_t(MHAOvlFilter::fftfb_vars_t& vars,unsigned int nch,unsigned int nfft,mha_real_t fs,algo_comm_t ac,std::string name)
  : MHAOvlFilter::fftfb_t(vars,nfft,fs),
  fbpow(ac,name,nbands(),nch,false)
{
}

MHAPLUGIN_CALLBACKS(fftfbpow,fftfbpow::fftfbpow_interface_t,spec,spec)
MHAPLUGIN_DOCUMENTATION\
(fftfbpow,
 "filterbank feature-extraction level-meter",
 "This plugin implements a filterbank based on FFT spectrum. The power\n"
 "in each filter bank channel is calculated and stored into an AC\n"
 "variable. The input signal is passed through unmodified.\n"
 "\n"
 "For details on the filter shapes, please see description of"
 " plugin {\\tt fftfilterbank} (section\n"
 "\\ref{plug:fftfilterbank} on page \\pageref{plug:fftfilterbank}).\n"
 "\n"
 "\n"
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
