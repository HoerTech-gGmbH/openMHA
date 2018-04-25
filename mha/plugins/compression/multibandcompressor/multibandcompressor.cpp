// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2013 2014 2015 2016 2018 HörTech gGmbH
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

#include "mha_toolbox.h"
#include "mha_plugin.hh"
#include "mha_fftfb.hh"
#include <math.h>
#include <fstream>
#include <iostream>
#include "mha_events.h"
#include "mhapluginloader.h"
#include "mha_algo_comm.hh"
#include "dc_afterburn.h"

#include "mha_defs.h"

namespace multibandcompressor {

class plugin_signals_t {
public:
    plugin_signals_t(unsigned int channels,unsigned int bands);
    void update_levels(MHAOvlFilter::fftfb_t*,mha_spec_t* s_in);
    void apply_gains(MHAOvlFilter::fftfb_t*,DynComp::dc_afterburn_t& burn,mha_spec_t* s_out);
private:
    MHASignal::waveform_t plug_level;
    MHASignal::waveform_t gain;
public:
    mha_wave_t* plug_output;
};

plugin_signals_t::plugin_signals_t(unsigned int channels,unsigned int bands)
    : plug_level(1,bands*channels),
      gain(bands,channels),
      plug_output(&plug_level)
{}

void plugin_signals_t::update_levels(MHAOvlFilter::fftfb_t*pFb,mha_spec_t* s_in)
{
    plug_output = &plug_level;
    pFb->get_fbpower(&gain,s_in);
    unsigned int band;
    unsigned int channel;
    for(band=0;band<gain.num_frames;band++)
        for(channel=0;channel<gain.num_channels;channel++){
            // Factor 2 because get_fbpower ignores the negative frequencies.
            gain.value(band,channel) = sqrt(2*gain.value(band,channel));
            plug_output->buf[band+channel*gain.num_frames] =
                gain.value(band,channel);
        }
}

void plugin_signals_t::apply_gains(MHAOvlFilter::fftfb_t* pFb,DynComp::dc_afterburn_t& burn, mha_spec_t* s_out)
{
    unsigned int band;
    unsigned int channel;
    mha_real_t Lin(0);
    for(band=0;band<gain.num_frames;band++)
        for(channel=0;channel<gain.num_channels;channel++){
            Lin = gain.value(band,channel);
            gain.value(band,channel) = 
                plug_output->buf[band+channel*gain.num_frames] /
                std::max(1e-20f,gain.value(band,channel));
            burn.burn(gain.value(band,channel),Lin,band,channel);
        }
    pFb->apply_gains(s_out,s_out,&gain);
}


class fftfb_plug_t : public MHAOvlFilter::fftfb_t {
public:
    fftfb_plug_t(MHAOvlFilter::fftfb_vars_t&,const mhaconfig_t& cfg,algo_comm_t ac,std::string alg);
    void insert();
private:
    /** vector of nominal center frequencies / Hz */
    MHA_AC::waveform_t cfv;
    /** vector of edge frequencies / Hz */
    MHA_AC::waveform_t efv;
    /** vector of band-weigths (sum of squared fft-bin-weigths)/num_frames */
    MHA_AC::waveform_t bwv;
};

fftfb_plug_t::fftfb_plug_t(MHAOvlFilter::fftfb_vars_t& vars,const mhaconfig_t& cfg,algo_comm_t ac,std::string alg)
    : MHAOvlFilter::fftfb_t(vars,cfg.fftlen,cfg.srate),
      cfv(ac,alg+"_cf",nbands(),1,false),
      efv(ac,alg+"_ef",nbands()+1,1,false),
      bwv(ac,alg+"_band_weights",nbands(),1,false)
{
    std::vector<mha_real_t> cfhz = get_cf_hz();
    std::vector<mha_real_t> efhz = get_ef_hz();
    unsigned int kfb, kfr;
    for(kfb=0;kfb<nbands();kfb++) {
        cfv[kfb] = cfhz[kfb];
        efv[kfb] = efhz[kfb];
        bwv[kfb] = 0.0f;
        for (kfr=bin1(kfb); kfr < bin2(kfb); kfr++) {
            bwv[kfb] += w(kfr,kfb) * w(kfr,kfb);
        }
        bwv[kfb] /= (cfg.fftlen/2+1);
    }
    if( nbands() )
        efv[nbands()] = efhz[nbands()];
}

void fftfb_plug_t::insert()
{
    cfv.insert();
    efv.insert();
    bwv.insert();
}

class interface_t : public MHAPlugin::plugin_t<fftfb_plug_t>,
                    public MHAOvlFilter::fftfb_vars_t {
public:
    interface_t(const algo_comm_t&,
                const std::string&,
                const std::string&);
    void prepare(mhaconfig_t&);
    void release();
    mha_spec_t* process(mha_spec_t*);
private:
    int num_channels;
    DynComp::dc_afterburn_t burn;
    MHAEvents::patchbay_t<interface_t> patchbay;
    void update_cfg();
    std::string algo;
    MHAParser::mhapluginloader_t plug;
    plugin_signals_t* plug_sigs;
};

/** \internal

    Default values are set and MHA configuration variables registered into the parser.

    \param ac_     algorithm communication handle
    \param th     chain name
    \param al     algorithm name
*/
interface_t::interface_t(const algo_comm_t& ac_,
                         const std::string& th,
                         const std::string& al)
    : MHAPlugin::plugin_t<fftfb_plug_t>("Multiband compressor framework based on level in overlapping filter bands.",ac_),
      MHAOvlFilter::fftfb_vars_t(static_cast<MHAParser::parser_t&>(*this)),
      num_channels(0),
      algo(al),
      plug(*this,ac),
      plug_sigs(NULL)
{
    set_node_id("multibandcompressor");
    insert_member(burn);
    std::string ch_name(al+"_nch");
    ac.insert_var_int(ac.handle, ch_name.c_str(), &num_channels);
}

void interface_t::prepare(mhaconfig_t& tf)
{
    if( tf.domain != MHA_SPECTRUM )
        throw MHA_ErrorMsg("Only spectral processing is supported.");
    num_channels = input_cfg().channels;
    update_cfg();
    poll_config();
    cfg->insert();
    mhaconfig_t gain_actor_cfg;
    memset(&gain_actor_cfg,0,sizeof(gain_actor_cfg));
    gain_actor_cfg.channels = input_cfg().channels*cfg->nbands();
    gain_actor_cfg.fragsize = 1;
    gain_actor_cfg.srate = input_cfg().srate/input_cfg().fragsize;
    gain_actor_cfg.domain = MHA_WAVEFORM;
    mhaconfig_t gain_actor_cfg_out(gain_actor_cfg);
    plug.prepare(gain_actor_cfg);
    try{
        PluginLoader::mhaconfig_compare(gain_actor_cfg_out,gain_actor_cfg,"Multibandcompressor:gain actor: ");
        plug_sigs = new plugin_signals_t(input_cfg().channels,cfg->nbands());
    }
    catch(...){
        plug.release();
        throw;
    }
}

void interface_t::release()
{
    plug.release();
    if( plug_sigs ){
        delete plug_sigs;
        plug_sigs = NULL;
    }
    burn.unset_fb_pars();
}

void interface_t::update_cfg()
{
    fftfb_plug_t* fb(new fftfb_plug_t(static_cast<MHAOvlFilter::fftfb_vars_t&>(*this),input_cfg(),ac,algo));
    push_config(fb);
    burn.set_fb_pars(fb->get_cf_hz(),input_cfg().channels,input_cfg().srate/input_cfg().fragsize);
}

mha_spec_t* interface_t::process(mha_spec_t* s)
{
    poll_config();
    if( plug_sigs ){
        burn.update_burner();
        plug_sigs->update_levels(cfg,s);
        plug.process(plug_sigs->plug_output,&(plug_sigs->plug_output));
        plug_sigs->apply_gains(cfg,burn,s);
    }
    return s;
}

}

MHAPLUGIN_CALLBACKS(multibandcompressor,multibandcompressor::interface_t,spec,spec)
MHAPLUGIN_DOCUMENTATION(multibandcompressor,"level compressor",
                        "multibandcompressor provides a complete framework\n"
                        "for dynamic range compression in multiple frequency\n"
                        "bands.\n"
                        "\n"
                        "It contains the same filterbank as the\n"
                        "fftfilterbank plugin (see there for documentation of\n"
                        "the filterbank) and combines the frequency bands\n"
                        "again after the compression.\n"
                        "\n"
                        "For the actual dynamic range compression,\n"
                        "multibandcompressor can load any other plugin using\n"
                        "the field plugin\\_name.  Common choices for this\n"
                        "plugin would be dc\\_simple or dc.\n"
                        "\n"
                        "Note that the dynamic range compression receives a\n"
                        "pseudo time signal where the sampling rate is the\n"
                        "rate of the block processing, i.e. in each channel\n"
                        "and band, there is exactly 1 signal sample for\n"
                        "every block.\n"
                        "These samples are a sparse, non-negative\n"
                        "representation of the actual signal in the\n"
                        "respective frequency band: The magnitude of each\n"
                        "sample is chosen by multibandcompressor so that\n"
                        "the level computed from this sparse signal is the\n"
                        "same as the level computed from the full signal for\n"
                        "this frequency band.\n"
                        "\n"
                        "The dynamic range compression will then apply gain\n"
                        "(or attenuation) to the sparse signal in each\n"
                        "frequency band.\n"
                        "The gain applied to the sparse\n"
                        "signal is measured by multibandcompressor\n"
                        "and eventually applied to the\n"
                        "respective full signal.\n"
                        "\n"
                        "Before the compressor gain is applied to\n"
                        "the full signal, it may be\n"
                        "modified by the after-burner built\n"
                        "into the multibandcompressor plugin\n"
                        "(sub-parser 'burn'): The purpose of\n"
                        "the after-burner is to enforce a\n"
                        "configurable Maximum Power Output\n"
                        "(MPO) for each frequency band, and to\n"
                        "compensate for drains and confluxes\n"
                        "of sound energy through vents and\n"
                        "open fittings.  Note that compensating\n"
                        "for drains in this way can easily\n"
                        "lead to feedback howling and should\n"
                        "be done with caution.  The\n"
                        "after-burner can be disabled\n"
                        "by setting burn.bypass=yes.\n"
                        )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// End:
