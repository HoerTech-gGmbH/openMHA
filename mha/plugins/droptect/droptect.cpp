// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2013 2014 2015 2018 2019 2020 HörTech gGmbH
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
  /* Cumulative number of all dropouts since last reset, per channel */
  MHAParser::vint_mon_t dropouts;
  /* Number of consecutive dropouts, per channel. */
  MHAParser::vint_mon_t consecutive_dropouts;
  /* Number of blocks processed */
  MHAParser::int_mon_t blocks;
  /* Reset number of dropouts and blocks */
  MHAParser::bool_t reset;
  /* Level Threshold. Consider every frame below this a dropout */
  MHAParser::float_t threshold;
  /* Storage for power spectrum of the current block */
  MHASignal::waveform_t * current_powspec;
  /* Storage for filtered power spectrum */
  MHASignal::waveform_t * filtered_powspec;
  /* filter time constant */
  MHAParser::float_t tau;
  /* Internal channel-specific flags: filter contains past value above
   * threshold. Needed to avoid detecting a dropout in the first block. */
  std::vector<bool> filter_activated;
  /** The period of the process callback (duration of fragsize in seconds) */
  float period;
  /** User access to filtered spectrum */
  MHAParser::mfloat_mon_t filtered_powspec_mon;
  /** User access to broadband levels */
  MHAParser::vfloat_mon_t level_mon;
public:
  /** This constructor initializes the configuration language
   * variables and inserts them into the MHA configuration tree. */
  droptect_t(algo_comm_t & ac,
             const std::string & chain_name,
             const std::string & algo_name);

  /** Allocates and initializes storage for this algorithm.
   * @param signal_info contains fft length, number of channels,
   *                    fft length and hop size. */
  void prepare(mhaconfig_t & signal_info);

  /** Deallocates storage. */
  void release(void);

  /** Compares current spectrum against history. If spectral power has changed
   * or is below threshold, this is interpreted as dropout occurrence. */
  mha_spec_t * process(mha_spec_t * signal);
};

droptect_t::droptect_t(algo_comm_t & ac,
                       const std::string & chain_name,
                       const std::string & algo_name)
  : MHAPlugin::plugin_t<int>("Plugin detects dropouts in channels that"
                             " have a constant spectrum",ac),
    dropouts("Number of dropouts detected since last reset or start"),
    consecutive_dropouts("Number of consecutive dropouts."
                         " Resets to 0 each time there is no dropout."),
    blocks("Number of blocks processed since last reset or start"),
    reset("Setting to \"yes\" clears number of dropouts and blocks.\n"
          "Value is reset to \"no\" when the next spectrum is processed.","no"),
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
  insert_member(level_mon);
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
    dropouts.data.assign(signal_info.channels,0);
    consecutive_dropouts.data.assign(signal_info.channels,0);
    filter_activated.assign(signal_info.channels,false);
    period = signal_info.fragsize / signal_info.srate;
    filtered_powspec_mon.data.assign(signal_info.channels,
                                     std::vector<float>(signal_info.fftlen/2+1,
                                                        0.0f));
    level_mon.data.assign(signal_info.channels, -200.0f);
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
  // By using a copy of configuration variable "reset" we prevent concurrent
  // value changes during the processing of the different channels.
  const bool reset_copy = reset.data;
  if (reset_copy) {
    blocks.data = 0;
    reset.data = false;
  }
  const double lin_thresh = 2e-5 * pow(10.0,0.05*threshold.data);
  const double squared_thresh = lin_thresh * lin_thresh;
  current_powspec->powspec(*signal);
  const unsigned fftlen = input_cfg().fftlen;
  const float alpha = expf(-period/tau.data);
  for (unsigned channel = 0; channel < signal->num_channels; ++channel) {
    const double rmslevel_pa = MHASignal::rmslevel(*signal,channel,fftlen);
    level_mon.data[channel] = MHASignal::pa2dbspl(rmslevel_pa);
    if (reset_copy) {
      dropouts.data[channel] = 0;
    }
    // Detect dropouts:
    if (rmslevel_pa < lin_thresh) { // level criterium
      ++dropouts.data[channel];
      consecutive_dropouts.data[channel]++;
      printf("level too low\n");
    } else {
      if (filter_activated[channel]) { // specral shape criterium
        bool dropout = false;
        for (unsigned k = 0; k < signal->num_frames; ++k) {
          // The 3000 divisor excludes STFT bins with low levels from consider-
          // ation (35 dB below broadband threshold) to eliminate microphone
          // noise induced false positives.
          if ((value(current_powspec,k,channel) > squared_thresh / 3000 ||
                value(filtered_powspec,k,channel) > squared_thresh / 3000) &&

              // Detect dropout if only one of (reference,current) level is silence
              (value(current_powspec,k,channel) == 0 || // also protects from...
               value(filtered_powspec,k,channel) == 0 ||// ... division by 0:
               
               // Detect dropout if |reference-current| levels exceeds 6dB
               value(current_powspec,k,channel) / value(filtered_powspec,k,channel) < 0.5 ||
               value(current_powspec,k,channel) / value(filtered_powspec,k,channel) > 2)) {
            // Dropout detected, enter it into bookkeeping:
            dropouts.data[channel]++;
            consecutive_dropouts.data[channel]++;
            dropout = true;
            break; // Do not check remaining STFT bins when there is a dropout.
          }
        }
        if (!dropout) {
          // If there was no dropout, update the reference, export the powspec
          for (unsigned k = 0; k < signal->num_frames; ++k) {
            filtered_powspec->value(k,channel) *= alpha;
            filtered_powspec_mon.data[channel][k] = 
              (filtered_powspec->value(k,channel) +=
               current_powspec->value(k,channel) * (1-alpha));
            consecutive_dropouts.data[channel] = 0;
          }
        }
        else { // dropout.
          // If we get large number of consecutive dropouts, then the reason may be
          // that the reference spectrum has changed.  Update the reference spectrum.
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
        // Initialize the reference spectrum when processing the first block.
        for (unsigned k = 0; k < signal->num_frames; ++k) {
          filtered_powspec_mon.data[channel][k] = 
            filtered_powspec->value(k,channel) =
            current_powspec->value(k,channel);
        }
        filter_activated[channel] = true;
      }
    }
  }
  blocks.data++;
  return signal;
}

MHAPLUGIN_CALLBACKS(droptect,droptect_t,spec,spec)
MHAPLUGIN_DOCUMENTATION\
(droptect,
 "test-tool",
 "Plugin to detect dropouts in live audio processing setups. "
 " \\texttt{droptect} expects an input signal with a spectral shape "
 " that does not vary over time, e.g. a combination of different "
 " sinusoids."
 "\n\n"
 "Either feed such a signal from an external source into the sound "
 " card used by \\mha{}, or have \\mha{} create such a signal "
 " downstream of the droptect plugin (e.g. with \\texttt{sine}), "
 " and feed the sound card output back into the sound card input with an "
 " audio cable."
 "\n\n"
 "\\texttt{droptect} detects a dropout if\n"
 "\\begin{itemize}\n"
 "\\item The broadband level of the current STFT spectrum is below threshold, or\n"
 "\\item The level of any bin of the STFT spectrum differs by more than 6dB"
 "       from the average spectrum.\n"
 "\\end{itemize}\n"
 "STFT bins with very low level (35 dB below the broadband threshold) "
 " are excluded from the 6dB difference criterium to allow for soft "
 " microphone noise."
 "\n\n"
 "Detected dropouts are accumulated per audio channel and published in the "
 " monitor variable \\texttt{dropouts}. "
 " The count can be reset to 0 by assigning \"yes\" to \\texttt{reset}. "
 " Some false positive detected dropouts on startup of signal processing are "
 " expected.  Reset the dropout count after processing has started to remove "
 " these false positives."
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 2
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
