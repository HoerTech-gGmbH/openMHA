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


#include "level_matching.hh"
#include "mha_signal.hh"

#include <cmath>
#define PATCH_VAR(var) patchbay.connect(&var.valuechanged, this, &level_matching::level_matching_t::update_cfg)
#define INSERT_PATCH(var) insert_member(var); PATCH_VAR(var)

namespace {
  void scale_channel(mha_wave_t& in_, unsigned ch_, mha_real_t scale_){
    for(unsigned fr=0U; fr<in_.num_frames; ++fr)
      value(in_,fr,ch_)*=scale_;
  }
  void scale_channel(mha_spec_t& in_, unsigned ch_, mha_real_t scale_){
    for(unsigned fr=0U; fr<in_.num_frames; ++fr)
      value(in_,fr,ch_)*=scale_;
  }
}

/** Channel pair ctor
 * @param idx_ Pair of channel indices
 * @param filter_rate_ Filter rate of low pass filter
 * @param mismatch_time_constant_ Time constant of low pass filter
 */
level_matching::channel_pair::channel_pair(const std::pair<int,int>& idx_, mha_real_t filter_rate_,
                                           mha_real_t mismatch_time_constant_):
  idx(idx_),
  mismatch(std::vector<mha_real_t>(1U,mismatch_time_constant_),filter_rate_,1.0f)
{}

/** Channel pair ctor
 * @param idx1 First channel index. Used as reference
 * @param idx2 Second channel index
 * @param filter_rate_ Filter rate of low pass filter
 * @param mismatch_time_constant_ Time constant of low pass filter
 */
level_matching::channel_pair::channel_pair(int idx1_, int idx2_,
                      mha_real_t filter_rate_,
                      mha_real_t mismatch_time_constant_):
  channel_pair({idx1_,idx2_}, filter_rate_, mismatch_time_constant_)
{}

/** Get last filter result
 * @return Last filter result
 */
mha_real_t level_matching::channel_pair::get_mismatch() const {return mismatch.get_last_output(0U);}


/** Calculates the filtered rms level mismatch
 * @param signal Input signal
 * @return Low-pass filtered mismatch ratio
 */
mha_real_t level_matching::channel_pair::update_mismatch(const mha_wave_t& signal_) {
  auto levels=
    std::make_pair<mha_real_t,mha_real_t>(MHASignal::pa2dbspl(std::fmax(2e-10f,MHASignal::rmslevel(signal_,idx.first))),
                                          MHASignal::pa2dbspl(std::fmax(2e-10f,MHASignal::rmslevel(signal_,idx.second))));
  // First mic is reference mic
  return mismatch(0U,levels.first-levels.second);
}

/** Calculates the filtered rms level mismatch
 * @param signal Input signal
 * @return Low-pass filtered mismatch ratio
 */
mha_real_t level_matching::channel_pair::update_mismatch(const mha_spec_t& signal_, const unsigned fftlen_) {

  auto levels=
    std::make_pair<mha_real_t,mha_real_t>(MHASignal::pa2dbspl(std::fmax(2e-10f,MHASignal::rmslevel(signal_,
                                                                                                   idx.first,
                                                                                                   fftlen_))),
                                          MHASignal::pa2dbspl(std::fmax(2e-10f,MHASignal::rmslevel(signal_,
                                                                                                   idx.second,
                                                                                                   fftlen_))));
  // First mic is reference mic
  return mismatch(0U,levels.first-levels.second);
}

/** RT-config c'tor
 * @param cfg_
 *   Structure containing a description of the form of the signal (domain,
 *   number of channels, frames per block, sampling rate.
 * @pre Size of param pairs must be two
 * @param pairs_ Matrix containing the channel indices of the microphone pairs. Two entries per row.
 * @param signal_tau Time constant of the signal low pass filter in seconds.
 * @param level_tau Time constant of the level mismatch averaging filter in seconds.
 */
level_matching::level_matching_config_t::level_matching_config_t(const mhaconfig_t& cfg_,
                                                                 const std::vector<std::vector<int>>& pairs_,
                                                                 mha_real_t signal_lp_fpass_, mha_real_t signal_lp_fstop_,
                                                                 unsigned signal_lp_order_,
                                                                 mha_real_t level_tau_,
                                                                 mha_real_t range_):
  tmp(cfg_),
  lp(cfg_.channels,{1},MHAFilter::fir_lp(signal_lp_fpass_, signal_lp_fstop_ , cfg_.srate , signal_lp_order_)),
  range(range_),
  fftlen(cfg_.fftlen)
{
  for(const auto& pair : pairs_){
    MHA_assert_equal(pair.size(),2U);
    pairings.emplace_back(pair[0],pair[1],cfg_.srate/cfg_.fragsize,level_tau_);
  }
}

//the actual processing implementation
mha_wave_t *level_matching::level_matching_config_t::process(mha_wave_t *s)
{
  lp.filter(static_cast<mha_wave_t*>(&tmp),s);
  for(auto& pair : pairings){
    auto mismatch=pair.update_mismatch(tmp);
    scale_channel(*s,pair.get_idx().second,MHASignal::db2lin(std::clamp(mismatch,-range,range)));
  }
  return s;
}
//the actual processing implementation
mha_spec_t *level_matching::level_matching_config_t::process(mha_spec_t *s)
{
  for(auto& pair : pairings){
    auto mismatch=pair.update_mismatch(*s,fftlen);
    scale_channel(*s,pair.get_idx().second,MHASignal::db2lin(std::clamp(mismatch,-range,range)));
  }
  return s;
}

/** Plugin interface ctor
 * @param ac 
 */
level_matching::level_matching_t::level_matching_t(algo_comm_t & ac,
                                                   const std::string &,
                                                   const std::string &)
  : MHAPlugin::plugin_t<level_matching_config_t>("Input level matching",ac)
{
  INSERT_PATCH(channels);
  INSERT_PATCH(lp_signal_fpass);
  INSERT_PATCH(lp_signal_fstop);
  INSERT_PATCH(lp_level_tau);
  INSERT_PATCH(range);
}

level_matching::level_matching_t::~level_matching_t() {}

/** Plugin preparation callback
 * @param signal_info
 *   Structure containing a description of the form of the signal (domain,
 *   number of channels, frames per block, sampling rate.
 */
void level_matching::level_matching_t::prepare(mhaconfig_t & signal_info)
{
  update_cfg();
}

void level_matching::level_matching_t::update_cfg()
{
  if ( is_prepared() ) {
    unsigned idx=0;
    for(const auto& pair:channels.data){
      if(pair.size()!=2)
        throw MHA_Error(__FILE__,__LINE__,"Channel matrix must have two elements per row!"
                        " Expected: %u. Found %zu in row %u",2U,pair.size(),idx);
      auto max_channel_idx=input_cfg().channels-1;
      std::for_each(pair.begin(),pair.end(),[&](unsigned idx_){
                                              if(idx_>max_channel_idx)
                                                throw MHA_Error(__FILE__,__LINE__,"Invalid channel index!"
                                                                " Number of channels: %u. Found index %u.",
                                                                max_channel_idx+1, idx_);
                                            });
      idx++;
    }
    level_matching::level_matching_config_t *
      config=new level_matching::level_matching_config_t(input_cfg(),channels.data,
                                                         lp_signal_fpass.data, lp_signal_fstop.data, lp_signal_order.data,
                                                         lp_level_tau.data, range.data);
    push_config( config );
  }
}

/**
 * Processing callback.
 * @param signal Input signal
 */
mha_wave_t * level_matching::level_matching_t::process(mha_wave_t * signal_)
{
  //this stub method defers processing to the configuration class
  return poll_config()->process( signal_ );
}

/**
 * Processing callback.
 * @param signal Input signal
 */
mha_spec_t * level_matching::level_matching_t::process(mha_spec_t * signal_)
{
  //this stub method defers processing to the configuration class
  return poll_config()->process( signal_ );
}

/*
 * This macro connects the plugin1_t class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(level_matching,level_matching::level_matching_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(level_matching,level_matching::level_matching_t,spec,spec)
/*
 * This macro creates code classification of the plugin and for
 * automatic documentation.
 *
 * The first argument to the macro is a space separated list of
 * categories, starting with the most relevant category. The second
 * argument is a LaTeX-compatible character array with some detailed
 * documentation of the plugin.
 */
MHAPLUGIN_DOCUMENTATION(level_matching_t,"level",
                        "This plugin implements automatic pairwaise matching of input levels. This algorithm can be used to"
                        " e.g.\\ compensate for microphone gain drift.\n"
                        " Microphone gain matching relies on the assumption that the input signal on"
                        " both microphones has the same level. This assumption breaks"
                        " down if the mic distance is small compared to the sound wavelength."
                        " To exclude high frequencies from the gain matching, if used in the time domain the signal"
                        " is filtered by a lowpass filter before the mismatch is calculated."
                        " In the spectral domain the user is responsible for the restriction of the matching to sensible"
                        " frequencies, e.g.\\ by usage of a fft filterbank and careful selection of"
                        " channels for the matching algorithm.\n"
                        " As gain drift usually happens on a time scale large compared to the block size the mismatch"
                        " is also lowpass filtered.\n"
                        "Currently this plugin can only match pairs of microphones."
                        " The microphone pairings are passed as a matrix, with each row containing"
                        " the indices of two microphones."
                        " The first entry is taken as reference mic. Its signal will not be changed."
                        " The signal of the other microphone will be scaled so that the average rms levels match."
                        )
