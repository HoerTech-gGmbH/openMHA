// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2013 2014 2015 2018 HörTech gGmbH
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
#include <stdio.h>

/** Detect dropouts in a signal with a constant spectrum. */
class droptect_t : public MHAPlugin::plugin_t<int> {
  /* Number of dropouts, per channel */
  MHAParser::vint_mon_t dropouts;
  /* Number of consecutive dropouts, per channel */
  MHAParser::vint_mon_t consecutive_dropouts;
  /* Number of blocks processed */
  MHAParser::int_mon_t blocks;
  /* Reset number of dropouts and blocks */
  MHAParser::bool_t reset;
  /* Level Threshold. Consider every frame below this a dropout */
  MHAParser::float_t threshold;
  /* Room for power spectrum of the current block */
  MHASignal::waveform_t * current_powspec;
  /* Room for filtered power spectrum */
  MHASignal::waveform_t * filtered_powspec;
  /* filter fime constant */
  MHAParser::float_t tau;
  /* flag: filter contains value above threshold */
  std::vector<bool> filter_activated;
  /** The period of the process callback */
  float period;
  /** User access to filtered spectrum */
  MHAParser::mfloat_mon_t filtered_powspec_mon;
  MHAParser::float_mon_t level_mon;
public:
  /** This constructor initializes the configuration language
   * variables and inserts them into the MHA configuration tree. */
  droptect_t(algo_comm_t & ac,
             const std::string & chain_name,
             const std::string & algo_name);

  void prepare(mhaconfig_t & signal_info);

  void release(void);
  mha_spec_t * process(mha_spec_t * signal);
};

droptect_t::droptect_t(algo_comm_t & ac,
                       const std::string & chain_name,
                       const std::string & algo_name)
  : MHAPlugin::plugin_t<int>("Plugin detects dropouts in channels that"
                             " have a constant spectrum",ac),
    dropouts("Number of dropouts detected since last reset or start"),
    consecutive_dropouts("Number of consecutive dropouts"),
    blocks("Number of blocks processed since last reset or start"),
    reset("Setting to yes clears number of dropouts and blocks","no"),
    threshold("Threshold level in dB. All blocks below this threshold are"
              " considered to be dropouts", "50","[,]"),
    current_powspec(0),
    filtered_powspec(0),
    tau("Time constant for filtering power spectra","0.2","[,]"),
    period(0),
    filtered_powspec_mon("Floating average of power spectrum"),
    level_mon("current level")
{
  insert_member(dropouts); insert_member(consecutive_dropouts);
  insert_member(blocks);   insert_member(reset);
  insert_member(threshold);insert_member(tau);
  insert_member(filtered_powspec_mon);
  insert_member(level_mon); level_mon.data = -200;
  dropouts.data.resize(0);
  consecutive_dropouts.data.resize(0);
  blocks.data = 0;
  filter_activated.resize(0);
  filtered_powspec_mon.data.resize(0);
}

void droptect_t::prepare(mhaconfig_t & signal_info)
{
    if (signal_info.domain != MHA_SPECTRUM)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin can only process spectrum signals.");
    current_powspec = new MHASignal::waveform_t(signal_info.fftlen/2+1,
                                                signal_info.channels);
    filtered_powspec = new MHASignal::waveform_t(signal_info.fftlen/2+1,
                                                 signal_info.channels);
    blocks.data = 0;
    dropouts.data.resize(0);
    consecutive_dropouts.data.resize(0);
    filter_activated.resize(0);
    dropouts.data.resize(signal_info.channels,0);
    consecutive_dropouts.data.resize(signal_info.channels,0);
    filter_activated.resize(signal_info.channels,false);
    period = signal_info.fragsize / signal_info.srate;
    filtered_powspec_mon.data.resize(0);
    filtered_powspec_mon.data.resize(signal_info.channels,
                                     std::vector<float>(signal_info.fftlen/2+1,
                                                        0.0f));
}

void droptect_t::release(void)
{
  delete current_powspec;
  current_powspec = 0;
  delete filtered_powspec;
  filtered_powspec = 0;
}

mha_spec_t * droptect_t::process(mha_spec_t * signal)
{
  if (reset.data) {
    blocks.data = 0;
  }
  double lin_thresh = 2e-5 * pow(10.0,0.05*threshold.data);
  double quad_thresh = lin_thresh * lin_thresh;
  current_powspec->powspec(*signal);
  unsigned fftlen = input_cfg().fftlen;
  float alpha = expf(-period/tau.data);
  for (unsigned channel = 0; channel < signal->num_channels; ++channel) {
    double level = MHASignal::rmslevel(*signal,channel,fftlen);
    level_mon.data = 20*log10(level / 2e-5);
    if (reset.data) dropouts.data[channel] = 0;
    if (level < lin_thresh) {
      ++dropouts.data[channel];
      consecutive_dropouts.data[channel]++;
      printf("level too low\n");
    } else {
      if (filter_activated[channel]) {
        bool dropout = false;
        for (unsigned k = 0; k < signal->num_frames; ++k) {
          if ((value(current_powspec,k,channel) > quad_thresh / 3000 ||
                value(filtered_powspec,k,channel) > quad_thresh / 3000) &&
               (value(current_powspec,k,channel) == 0 ||
                value(filtered_powspec,k,channel) == 0 ||
                value(current_powspec,k,channel) / value(filtered_powspec,k,channel) < 0.5 ||
                value(current_powspec,k,channel) / value(filtered_powspec,k,channel) > 2)) {
            dropouts.data[channel]++;
            consecutive_dropouts.data[channel]++;
            dropout = true;
            printf("level differs in channel %d bin %d: filtered=%g,current=%g\n",channel,k,filtered_powspec->value(k,channel),current_powspec->value(k,channel));
            break;
          }
        }
        if (!dropout)
          for (unsigned k = 0; k < signal->num_frames; ++k) {
            filtered_powspec->value(k,channel) *= alpha;
            filtered_powspec_mon.data[channel][k] = 
              (filtered_powspec->value(k,channel) +=
               current_powspec->value(k,channel) * (1-alpha));
            consecutive_dropouts.data[channel] = 0;
          }
        else { // dropout
          if (consecutive_dropouts.data[channel] > 10) {
            for (unsigned k = 0; k < signal->num_frames; ++k) {
              filtered_powspec_mon.data[channel][k] = 
                filtered_powspec->value(k,channel) =
                current_powspec->value(k,channel);
            }
            consecutive_dropouts.data[channel] = 0;
          }
        }
      } else {
        for (unsigned k = 0; k < signal->num_frames; ++k) {
          filtered_powspec_mon.data[channel][k] = 
            filtered_powspec->value(k,channel) =
            current_powspec->value(k,channel);
        }
        filter_activated[channel] = true;
      }
    }
  }
  reset.data = false;
  blocks.data++;
  return signal;
}

MHAPLUGIN_CALLBACKS(droptect,droptect_t,spec,spec)
MHAPLUGIN_DOCUMENTATION(droptect,"analysis","")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
