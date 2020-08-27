// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2007 2009 2010 2012 2013 2014 2015 2017 2018 HörTech gGmbH
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


/*
 * The simplest example of an \mha plugin.
 *
 * This plugin scales one channel of the input signal, working in the
 * time domain.
 */
#include "mha_plugin.hh"

/** This C++ class implements the simplest example plugin for the
 * step-by-step tutorial.  It inherits from MHAPlugin::plugin_t for
 * correct integration in the configuration language interface.  */
class example21_t : public MHAPlugin::plugin_t<int> {
public:
  /** Do-nothing constructor.  The constructor has to take these three
   * arguments, but it does not have to use them. However, the base
   * class has to be initialized. */
  example21_t(algo_comm_t & ac,
            const std::string & chain_name,
            const std::string & algo_name)
      : MHAPlugin::plugin_t<int>("",ac)
  {/* Do nothing in constructor */}

  /** Release may be empty */
  void release(void)
  {/* Do nothing in release */}

  /** Plugin preparation. This plugin checks that the input signal has the
   * waveform domain and contains at least one channel
   * @param signal_info 
   *   Structure containing a description of the form of the signal (domain,
   *   number of channels, frames per block, sampling rate.
   */
  void prepare(mhaconfig_t & signal_info)
  {
    if (signal_info.domain != MHA_WAVEFORM)
      throw MHA_Error(__FILE__, __LINE__,
                      "This plugin can only process waveform signals.");
    if (signal_info.channels < 1)
      throw MHA_Error(__FILE__,__LINE__,
                      "This plugin requires at least one input channel.");
  }

  /** Signal processing performed by the plugin.  
   * This plugin multiplies the signal in the first audio channel by
   * a factor 0.1. 
   * @param signal
   *   Pointer to the input signal structure.
   * @return
   *   Returns a pointer to the input signal structure,
   *   with a the signal modified by this plugin.
   *   (In-place processing)
   */
  mha_wave_t * process(mha_wave_t * signal)
  {
    unsigned int channel = 0; // channels and frames counting starts with 0
    float factor = 0.1f;
    unsigned int frame;

    // Scale channel number "channel" by "factor":
    for(frame = 0; frame < signal->num_frames; frame++) {
      // Waveform channels are stored interleaved.
      signal->buf[signal->num_channels * frame + channel] *= factor;
    }
    // Algorithms may process data in-place and return the input signal
    // structure as their output signal:
    return signal;
  }
};

/*
 * This macro connects the example21_t class with the \mha plugin C interface
 * The first argument is the class name, the other arguments define the 
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(example21,example21_t,wave,wave)

/*
 * This macro creates code classification of the plugin and for
 * automatic documentation.
 *
 * The first argument to the macro is a space separated list of
 * categories, starting with the most relevant category. The second
 * argument is a LaTeX-compatible character array with some detailed
 * documentation of the plugin.
 */
MHAPLUGIN_DOCUMENTATION\
(example21,
 "example level-modification audio-channels",
 "The {\\bf simplest} example of an \\mha{} plugin.\n\n"
 "This plugin scales one channel of the input signal, working in the "
 "time domain."
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
