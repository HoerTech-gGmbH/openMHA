// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2018 2021 HörTech gGmbH
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

#ifndef FSHIFT_H
#define FSHIFT_H
#include "mha_plugin.hh"

/** All types for the fshift plugin live in this namespace */
namespace fshift{


  /** Finds bin number of FFT bin nearest to the given frequency.
   * @param frequency The frequency for which to look. Has to be in range [-srate/2,+srate/2]
   * @param fftlen Length of the FFT.
   * @param srate Sampling rate of the waveform from which the FFT originates.
   * @returns Bin number of the FFT bin corresponding to frequency
   */
  int fft_find_bin(mha_real_t frequency, unsigned fftlen, mha_real_t srate);

  class fshift_t;

  /** fshift runtime config class */
  class fshift_config_t {

  public:
    /** C'tor of the fshift plugin runtime configuration class.
     * @param plug ptr to the plugin interface class. Configuration
     * information is given this way to keep the argument list small.
     */
    explicit fshift_config_t(fshift_t const * const plug);
    ~fshift_config_t()=default;

    mha_spec_t* process(mha_spec_t*);

  private:
    /** FFT bin corresponding to fmin */
    const unsigned int kmin;
    /** FFT bin corresponding to fmax */
    const unsigned kmax;
    /** Frequency shift expressed in FFT bins */
    const int df;
    /** Phase advance per fft frame */
    const mha_complex_t delta_phi;
    /** Sum of all phase advances */
    mha_complex_t delta_phi_total;
  };

  /** fshift plugin interface class */
  class fshift_t : public MHAPlugin::plugin_t<fshift_config_t> {
  public:
    fshift_t(algo_comm_t iac, const std::string & configured_name);
    ~fshift_t();
    mha_spec_t* process(mha_spec_t*);
    void prepare(mhaconfig_t&);
    void release(void) {/* Do nothing in release */}

    mha_real_t fmin() const {
      return m_fmin.data;
    }

    mha_real_t fmax() const {
      return m_fmax.data;
    }

    mha_real_t df() const {
      return m_df.data;
    }

  private:
    void update_cfg();
    /** patch bay for connecting configuration parser
        events with local member functions: */
    MHAEvents::patchbay_t<fshift_t> patchbay;
    /*lower boundary for frequency shifter */
    MHAParser::float_t m_fmin;
    /** upper boundary for frequency shifter */
    MHAParser::float_t m_fmax;
    /** Shift frequency in Hz */
    MHAParser::float_t m_df;
  };
}
#endif // FSHIFT_H

// Local Variables:
// compile-command: "make"
// c-basic-offset: 2
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
