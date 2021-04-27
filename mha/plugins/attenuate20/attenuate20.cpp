// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2020 2021 HörTech gGmbH
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

class attenuate20_t : public MHAPlugin::plugin_t<int> {
public:
  attenuate20_t(algo_comm_t iac, const std::string & configured_name)
      : MHAPlugin::plugin_t<int>("This plugin attenuates by 20dB",iac)
  {(void)configured_name;}
  void release(void) override
  {}
  void prepare(mhaconfig_t & signal_info) override
  {
    if (signal_info.domain != MHA_WAVEFORM)
      throw MHA_Error(__FILE__, __LINE__,"can only process waveform");
  }
  mha_wave_t * process(mha_wave_t * signal)
  {
    // -20dB = factor 0.1
    MHASignal::for_each(signal,[](mha_real_t sample){return sample * 0.1f;});
    return signal;
  }
};
MHAPLUGIN_CALLBACKS(attenuate20,attenuate20_t,wave,wave)
MHAPLUGIN_DOCUMENTATION(attenuate20, "example level-modification",
 "Plugin attenuate20 attenuates the input signal by 20dB.")
