// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2005 2006 2009 2010 2013 2014 2015 2018 HörTech gGmbH
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

#include <math.h>
#include "mha_defs.h"
#include "mha_plugin.hh"
#include "mha_filter.hh"

class cfg_t {
public:
    cfg_t(mha_real_t tau_attack,mha_real_t tau_decay,unsigned int nch,mha_real_t start_limit,mha_real_t slope_db,mha_real_t fs);
    mha_real_t start_lin;
    mha_real_t alpha;
    MHAFilter::o1flt_lowpass_t attack;
    MHAFilter::o1flt_maxtrack_t decay;
};

class softclip_t : public MHAPlugin::plugin_t<cfg_t> {
public:
    softclip_t(const algo_comm_t&,
               const std::string&,
               const std::string&);
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void update();
private:
    mhaconfig_t tftype;
    MHAParser::float_t attack;
    MHAParser::float_t decay;
    MHAParser::float_t start_limit;
    MHAParser::float_t slope_db;
    MHAEvents::patchbay_t<softclip_t> patchbay;
};

softclip_t::softclip_t(const algo_comm_t& iac,
                       const std::string& chain,
                       const std::string& name)
    : MHAPlugin::plugin_t<cfg_t>(
        "The softclipper implements a broad band dynamic\n"
        "compression above a given level (Compression limiting).",iac),
      attack("time constant of attack filter","0.002","[0,["),
      decay("time constant of decay filter","0.05","[0,["),
      start_limit("entry point of time domain soft clipping (dB)","110.0"),
      slope_db("slope of input-output table above start (dB/dB)","0.125","[0,]")
{
    tftype.channels = 0;
    tftype.srate = 0;
    insert_item("tau_decay",&decay);
    insert_item("tau_attack",&attack);
    insert_item("start",&start_limit);
    insert_item("slope",&slope_db);
    patchbay.connect(&writeaccess,this,&softclip_t::update);
}

cfg_t::cfg_t(mha_real_t tau_attack,mha_real_t tau_decay,unsigned int nch,mha_real_t start_limit,mha_real_t slope_db,mha_real_t fs)
    : start_lin(2e-5*pow(10.0, 0.05*start_limit)),
      alpha(slope_db - 1.0),
      attack(std::vector<mha_real_t>(nch,tau_attack),fs,start_lin),
      decay(std::vector<mha_real_t>(nch,tau_decay),fs,start_lin)
{
}

mha_wave_t* softclip_t::process(mha_wave_t* s)
{
    unsigned int k;
    unsigned int ch;
    mha_real_t val;
    poll_config();
    for( k=0; k<s->num_frames; k ++ ){
        for( ch=0;ch<s->num_channels;ch++ ){
            val = cfg->decay( ch, cfg->attack( ch, fabs(value(s,k,ch))));
            if( val > cfg->start_lin )
                value(s,k,ch) *= pow( val/cfg->start_lin, cfg->alpha );
        }
    }
    return s;
}

void softclip_t::prepare(mhaconfig_t& tf)
{
    if( tf.domain != MHA_WAVEFORM )
        throw MHA_ErrorMsg("Softclip: Waveform input is required.");
    tftype = tf;
    update();
}

void softclip_t::update()
{
    if( tftype.channels )
        push_config(new cfg_t(attack.data,decay.data,tftype.channels,start_limit.data,slope_db.data,tftype.srate));
}
 
MHAPLUGIN_CALLBACKS(softclip,softclip_t,wave,wave)
    MHAPLUGIN_DOCUMENTATION(softclip,"level compression","")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
