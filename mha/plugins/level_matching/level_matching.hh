// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2019 HörTech gGmbH
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

#ifndef LEVEL_MATCHING_H
#define LEVEL_MATCHING_H

#include "mha_plugin.hh"
#include "mha_filter.hh"
namespace level_matching {

  class channel_pair {
  public:
    channel_pair(const std::pair<int,int>& idx_, mha_real_t filter_rate_,
                 mha_real_t mismatch_time_constant_);
    channel_pair(int idx1_, int idx2_,
                 mha_real_t filter_rate_,
                 mha_real_t mismatch_time_constant_);
    mha_real_t update_mismatch(const mha_wave_t& signal_);
    mha_real_t update_mismatch(const mha_spec_t& signal_, unsigned fftlen_);
    mha_real_t get_mismatch() const;
    const std::pair<int,int>& get_idx() const {return idx;}
  private:
    /// Indices of channels
    std::pair<int,int> idx;
    /// Low-pass filtered level mismatch
    MHAFilter::o1flt_lowpass_t mismatch;
  };

  //runtime config
  class level_matching_config_t {
  public:
    level_matching_config_t(const mhaconfig_t& cfg_, const std::vector<std::vector<int>>& pairs_,
                            mha_real_t signal_lp_fpass_, mha_real_t signal_fstop_, unsigned signal_lp_order_,
                            mha_real_t level_tau_,
                            mha_real_t range_);
    ~level_matching_config_t()=default;

    mha_wave_t* process(mha_wave_t*);
    mha_spec_t* process(mha_spec_t*);

  private:
    /// Temporary storage for input signal to use waveform_t helper functions.
    MHASignal::waveform_t tmp;
    /// Channel pairings
    std::vector<channel_pair> pairings;
    /// Nth-order low pass filter used to exclude high frequencies from level matching
    MHAFilter::iir_filter_state_t lp;
    /// Maximum matching range. Maximum adjustment done.
    mha_real_t range;
    /// fftlen in spectral domain
    unsigned fftlen;
  };

  class level_matching_t : public MHAPlugin::plugin_t<level_matching_config_t> {

  public:
    level_matching_t(algo_comm_t & ac,const std::string & chain_name,
                     const std::string & algo_name);
    ~level_matching_t();
    mha_wave_t* process(mha_wave_t*);
    mha_spec_t* process(mha_spec_t*);
    void prepare(mhaconfig_t&);
    void release(void) {/* Do nothing in release */}

  private:
    void update_cfg();
    MHAParser::mint_t channels={"channels","[[0 1]]"};
    MHAParser::float_t range={"Maximum matching range in dB","4","[0,["};
    MHAParser::float_t lp_signal_fpass={"Upper edge of the lp pass band for the signal in Hz","4000","[0,["};
    MHAParser::float_t lp_signal_fstop={"Stop band lower edge frequency for the signal in Hz","8000","[0,["};
    MHAParser::float_t lp_signal_order={"Signal lp order","24","[1,["};
    MHAParser::float_t lp_level_tau={"Low pass time constant for the mismatch in s","1","[0,["};
    /* patch bay for connecting configuration parser
       events with local member functions: */
    MHAEvents::patchbay_t<level_matching_t> patchbay;

  };
} //namespace level_matching
#endif // LEVEL_MATCHING_H
