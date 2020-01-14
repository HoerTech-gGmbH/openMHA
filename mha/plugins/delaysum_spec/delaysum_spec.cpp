// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2007 2010 2013 2014 2015 2016 2017 2018 2019 HörTech gGmbH
// Copyright © 2020 HörTech gGmbH
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

#include <stdio.h>
#include "mha_plugin.hh"
#include "mha_events.h"

namespace delaysum_spec {
  class delaysum_t {
  public:
    delaysum_t(std::vector<float> groupdelay, std::vector<float> gain, unsigned int nChannels, unsigned int nFFT, float fs);
    mha_spec_t* process(mha_spec_t*);
  private:
    MHASignal::spectrum_t scale;
    MHASignal::spectrum_t output;
  };

  class delaysum_spec_if_t : public MHAPlugin::plugin_t<delaysum_t> {
  public:
    delaysum_spec_if_t(const algo_comm_t&,const std::string&,const std::string&);
    mha_spec_t* process(mha_spec_t*);
    void prepare(mhaconfig_t&);
  private:
    void update_cfg();
    MHAParser::vfloat_t groupdelay;
    MHAParser::vfloat_t gain;
    MHAEvents::patchbay_t<delaysum_spec_if_t> patchbay;
  };

  delaysum_t::delaysum_t(std::vector<float> groupdelay, std::vector<float> gain,
                         unsigned int nChannels, unsigned int nFFT, float fs)
    : scale(nFFT/2+1, nChannels), output(nFFT/2+1, 1)
  {
    if( groupdelay.size()!= nChannels )  {
      throw MHA_Error(__FILE__,__LINE__,
                      "Invalid channel number %zu (%u channels configured).",
                      groupdelay.size(),nChannels);};

    if( gain.size()!= nChannels )  {
      throw MHA_Error(__FILE__,__LINE__,
                      "Invalid channel number %zu (%u channels configured).",
                      gain.size(),nChannels);};
    float df=fs/nFFT;
    for (unsigned int ch = 0; ch<nChannels; ch++)
      {
        float dPhi = -2*M_PI*groupdelay[ch]*df;
        for (unsigned int fr = 0; fr<nFFT/2+1; fr++)
          {
            expi(scale(fr,ch), fr*dPhi, gain[ch]);
          }
      }

  }

  mha_spec_t* delaysum_t::process(mha_spec_t* spec)
  {
    *spec *=scale;
    for(unsigned int fr = 0; fr < spec->num_frames; fr++){
      output(fr,0) =  mha_complex(0,0);

      for(unsigned int ch = 0; ch< spec->num_channels; ch++){
        output(fr,0) += value(spec,fr,ch);
      }
    }
    return &output;
  }

  delaysum_spec_if_t::delaysum_spec_if_t(
                                         const algo_comm_t& iac,
                                         const std::string&,const std::string&)
    : MHAPlugin::plugin_t<delaysum_t>("simple delay and sum with single channel output",iac),
    groupdelay("Group delay in seconds. Positive values represent a delay. One entry for each audio channel","[0 0]","[,]"),
    gain("weights of channels.  Each entry is multiplied to its\n"
         "respective channel.  Needs one entry per channel.","[1 1]","[,]")
  {

    insert_item("groupdelay",&groupdelay);
    insert_item("gain",&gain);

    patchbay.connect(&groupdelay.writeaccess,this,&delaysum_spec_if_t::update_cfg);
    patchbay.connect(&gain.writeaccess,this,&delaysum_spec_if_t::update_cfg);

  }

  mha_spec_t* delaysum_spec_if_t::process(mha_spec_t* spec)
  {
    return poll_config()->process(spec);
  }

  void delaysum_spec_if_t::prepare(mhaconfig_t& signal_info)
  {
    if( signal_info.domain != MHA_SPECTRUM )
      throw MHA_Error(__FILE__,__LINE__,
                      "delaysum: Only spectral processing is supported.");
    tftype = signal_info;
    signal_info.channels = 1;
    update_cfg();
  }

  void delaysum_spec_if_t::update_cfg()
  {
    if( tftype.channels )
      push_config(new delaysum_t(groupdelay.data, gain.data, tftype.channels, tftype.fftlen, tftype.srate));
  }
} // namespace delaysum_spec
MHAPLUGIN_CALLBACKS(delaysum_spec,delaysum_spec::delaysum_spec_if_t,spec,spec)
MHAPLUGIN_DOCUMENTATION(delaysum_spec,"beamforming directional multichannel","This plugin allows to delay and "
 "sum multiple input channels using individual "
 "delays and weights. After each channel is delayed "
 "it is multiplied with the given weight and then "
 "added to the single output channel.")
