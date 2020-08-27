// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2005 2006 2009 2010 2013 2014 2015 2016 HörTech gGmbH
// Copyright © 2018 2019 2020 HörTech gGmbH
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
#include "mha_signal.hh"
#include "mha_parser.hh"
#include "mha_defs.h"
#include "mha_events.h"

namespace equalize {

  class cfg_t {
  public:
    cfg_t(int infft,int inchannels,std::vector<std::vector<float> > ifgains);
    cfg_t(const cfg_t&)=delete;
    cfg_t& operator=(const cfg_t&)=delete;
    ~cfg_t();
    int num_bins;
    int nchannels;
    mha_real_t* fftgains;
  };

  class freqgains_t : public MHAPlugin::plugin_t<cfg_t> {
  public:
    freqgains_t(const algo_comm_t& iac,const std::string&,const std::string&);
    mha_spec_t* process(mha_spec_t*);
    void prepare(mhaconfig_t&);
  private:
    void update_gains();
    void update_id();
    MHAParser::mfloat_t fftgains;
    MHAParser::string_t id;
    MHAEvents::patchbay_t<freqgains_t> patchbay;
  };


  /********************************************************************/

  freqgains_t::freqgains_t(
                           const algo_comm_t& iac,const std::string&,const std::string&)
    : MHAPlugin::plugin_t<cfg_t>("Equalizer plugin applies configurable gains to all bins of the spectrum",iac),
      fftgains("gains in FFT resolution (FFT length/2+1 entries required per row)\n"
               " as linear factors, one row per audio channel","[[]]","[0,["),
      id("Access to the id feature of the equalize plugin.  Usually the"
         " equalize plugin exposes no plugin id.  This variable allows to set"
         " a plugin id.  If set to \"equalize\", then this plugin can be"
         " fitted with a linear hearing aid fitting rule", "")
  {
    insert_item("gains",&fftgains);
    insert_item("id", &id);
    patchbay.connect(&fftgains.writeaccess,this,&freqgains_t::update_gains);
    patchbay.connect(&id.writeaccess,this,&freqgains_t::update_id);
  }

  void freqgains_t::prepare(mhaconfig_t& tf)
  {
    if( tf.domain != MHA_SPECTRUM )
      throw MHA_ErrorMsg("equalize: Only spectral processing is supported.");
    tftype = tf;
    update_gains();
  }

  mha_spec_t* freqgains_t::process(mha_spec_t* s)
  {
    poll_config();
    unsigned int k;
    for(k=0;k < s->num_channels * s->num_frames; k++){
      s->buf[k].re *= cfg->fftgains[k];
      s->buf[k].im *= cfg->fftgains[k];
    }
    return s;
  }

  void freqgains_t::update_gains(void)
  {
    if( tftype.channels )
      push_config(new cfg_t(tftype.fftlen,tftype.channels,fftgains.data));
  }

  void freqgains_t::update_id(void)
  {
    set_node_id(id.data);
  }

  cfg_t::cfg_t(int infft,int inchannels,std::vector<std::vector<float> > ifgains)
    : num_bins(infft/2+1),
      nchannels(inchannels),
      fftgains(NULL)
  {
    int k, ch;
    if( nchannels <= 0 )
      throw MHA_ErrorMsg("Invalid number of channels.");
    if( (int)ifgains.size() != nchannels )
      throw MHA_Error(__FILE__,__LINE__,
                      "The gain matrix needs %d channels, found %zu.",
                      nchannels,ifgains.size());
    for(ch=0;ch<nchannels;ch++){
      if( (int)ifgains[ch].size() != num_bins )
        throw MHA_Error(__FILE__,__LINE__,
                        "The gain matrix needs %d entries per channel, found %zu.",
                        num_bins,ifgains[ch].size());
    }
    fftgains = new float[num_bins*nchannels];
    for(ch=0;ch<nchannels;ch++){
      for(k=0;k<num_bins;k++)
        fftgains[ch*num_bins+k] = ifgains[ch][k];
    }
  }

  cfg_t::~cfg_t()
  {
    delete [] fftgains;
  }

}

MHAPLUGIN_CALLBACKS(equalize,equalize::freqgains_t,spec,spec)
MHAPLUGIN_DOCUMENTATION(equalize,"filter level-modification","High resolution gain structure. This plugin allows to apply a"
                        " bin-wise gain to every bin of the spectrum.")

// Local variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
