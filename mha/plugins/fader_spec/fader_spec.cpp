// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2008 2009 2010 2013 2014 2015 2018 2019 2020 HörTech gGmbH
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

#include "mha_parser.hh"
#include "mha_plugin.hh"
#include "mha_defs.h"
#include "mha_events.h"
#include "mha_signal.hh"

class spec_fader_t {
public:
    spec_fader_t(unsigned int ch,mha_real_t fr,MHAParser::vfloat_t& ng,MHAParser::float_t& t);
    ~spec_fader_t(){
        memset(gains,0,sizeof(gains[0])*nch);
        delete [] gains;
    };
    unsigned int nch;
    mha_real_t* gains;
    unsigned int fr;
};

spec_fader_t::spec_fader_t(unsigned int ch,mha_real_t _fr,
                           MHAParser::vfloat_t& ng,MHAParser::float_t& t)
    :nch(ch),
     gains(NULL),
     fr((unsigned int)(t.data*_fr))
{
    if( nch == 0 )
        nch = ng.data.size();
    if( ng.data.size() != nch )
        throw MHA_Error(__FILE__,__LINE__,
                        "mismatching size of gains vector and channel number (%zu gains, %u channels)",
                        ng.data.size(),nch);
    gains = new mha_real_t[nch];
    for(unsigned int k=0;k<nch;k++)
        gains[k] = ng.data[k];
}

class fader_if_t : public MHAPlugin::plugin_t<spec_fader_t> 
{
public:
    fader_if_t(const algo_comm_t&,const std::string&,const std::string&);
    mha_spec_t* process(mha_spec_t*);
    void prepare(mhaconfig_t&);
private:
    void update_cfg();
    MHAEvents::patchbay_t<fader_if_t> patchbay;
    MHAParser::float_t tau;
    MHAParser::vfloat_t newgains;
    mha_real_t* actgains;
};

fader_if_t::fader_if_t(const algo_comm_t& iac,const std::string&,const std::string&)
    : MHAPlugin::plugin_t<spec_fader_t>("fader",iac),
      tau("fader duration in seconds","1","[0,["),
      newgains("","[1 1]"),
      actgains(NULL)
{
    insert_item("tau",&tau);
    insert_item("gains",&newgains);
    patchbay.connect(&newgains.writeaccess,this,&fader_if_t::update_cfg);
}

void fader_if_t::prepare(mhaconfig_t& tf)
{
    if( tf.domain != MHA_SPECTRUM )
        throw MHA_ErrorMsg("fader: Only spectral processing is supported.");
    tftype = tf;
    if( actgains )
        delete [] actgains;
    actgains = new mha_real_t[tftype.channels];
    memset(actgains,0,sizeof(actgains[0])*tftype.channels);
    update_cfg();
}

mha_spec_t* fader_if_t::process(mha_spec_t* s)
{
    poll_config();
    unsigned int kch,kfr,chofs;
    if( cfg->fr ){
        for(kch=0;kch<s->num_channels;kch++)
            actgains[kch] += (cfg->gains[kch]-actgains[kch])/(mha_real_t)(cfg->fr);
        cfg->fr--;
    }
    else {
        // ensure target gains are reached even if tau < frameperiod
        std::copy(cfg->gains, cfg->gains + s->num_channels, actgains);
    }
    for(kch=0;kch<s->num_channels;kch++){
        chofs = kch*s->num_frames;
        for(kfr=0;kfr<s->num_frames;kfr++){
            s->buf[kfr+chofs].re *= actgains[kch];
            s->buf[kfr+chofs].im *= actgains[kch];
        }
    }
    return s;
}

void fader_if_t::update_cfg(void)
{
    push_config(new spec_fader_t(tftype.channels,
                                 tftype.srate/mha_min_1(tftype.fragsize),
                                 newgains,
                                 tau));
}


MHAPLUGIN_CALLBACKS(fader_spec,fader_if_t,spec,spec)
MHAPLUGIN_DOCUMENTATION\
(fader_spec,
 "data-flow audio-channels cross-fade  level-modification",
 "")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
