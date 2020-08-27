// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2007 2009 2010 2012 2013 2014 2015 2017 2018
//             2020 HörTech gGmbH
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
 * The purpose of this example is to show the developer how to conduct a
 * C++-Unit-Test on MHA Plugins. Therefore this example7 code is the same
 * as example1 with the difference that it is split into a .cpp and a .hh
 * file in order to provide the class declaration to the Unit-Tests.
 */

#include "example7.hh"
#include "mha_plugin.hh"

example7_t::example7_t(algo_comm_t & ac,
                       const std::string & chain_name,
                       const std::string & algo_name)
    : MHAPlugin::plugin_t<int>("",ac)
  {}

void example7_t::release(void)
  {}

void example7_t::prepare(mhaconfig_t & signal_info)
  {
    if (signal_info.domain != MHA_WAVEFORM)
      throw MHA_Error(__FILE__, __LINE__,
                      "This plugin can only process waveform signals.");
    if (signal_info.channels < 1)
      throw MHA_Error(__FILE__,__LINE__,
                      "This plugin requires at least one input channel.");
  }

mha_wave_t * example7_t::process(mha_wave_t * signal)
  {
    unsigned int channel = 0;
    float factor = 0.1f;
    unsigned int frame;

    for(frame = 0; frame < signal->num_frames; frame++) {
        signal->buf[signal->num_channels * frame + channel] *= factor;
    }
    return signal;
  }

MHAPLUGIN_CALLBACKS(example7,example7_t,wave,wave)

MHAPLUGIN_DOCUMENTATION\
(example1,
 "example level-modification audio-channels unit-testing",
 "The is again example1 but split into .hh and .cpp-file in order to "
 "provide the class declaration to the Unit-Test"
 "This plugin scales one channel of the input signal, working in the "
 "time domain."
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
