// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2007 2008 2009 2010 2013 2014 2015 2017 2018 2019 HörTech gGmbH
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

#include <limits>
#include "mha_plugin.hh"
#include "mha_filter.hh"

namespace dc_simple {

using namespace MHAPlugin;

void test_fail(const std::vector<float>& v, unsigned int s, const std::string& name)
{
    if( (v.size() != s) && (v.size() != 1))
        throw MHA_Error(__FILE__,__LINE__,
                        "Invalid %s vector size (found %zu, expected %u).",
                        name.c_str(),v.size(),s);
}

std::vector<float> force_resize(const std::vector<float>& v, unsigned int s, const std::string& name)
{
    std::vector<float> retv(v);
    if( (retv.size() != s) ){
        if( retv.size() == 1 ){
            for( unsigned int k=1;k<s;k++ )
                retv.push_back(v[0]);
        }else
            throw MHA_Error(__FILE__,__LINE__,
                            "Invalid %s vector size (found %zu, expected %u).",
                            name.c_str(),retv.size(),s);
    }
    return retv;
}

mha_real_t not_zero(mha_real_t x,const std::string& comment = "")
{
    if( x == 0 )
        throw MHA_Error(__FILE__,__LINE__,"Value is zero. %s",comment.c_str());
    return x;
}

class dc_vars_t {
public:
    dc_vars_t(MHAParser::parser_t&);
    MHAParser::vfloat_t g50;
    MHAParser::vfloat_t g80;
    MHAParser::vfloat_t maxgain;
    MHAParser::vfloat_t expansion_threshold;
    MHAParser::vfloat_t expansion_slope;
    MHAParser::vfloat_t limiter_threshold;
    MHAParser::vfloat_t tauattack;
    MHAParser::vfloat_t taudecay;
    MHAParser::bool_t bypass;
};

class dc_vars_validator_t {
public:
    dc_vars_validator_t(const dc_vars_t& v,unsigned int s);
};

class level_smoother_t : private dc_vars_validator_t {
public:
    level_smoother_t(const dc_vars_t& vars,
                     mha_real_t filter_rate,
                     mhaconfig_t buscfg);
    mha_wave_t* process(mha_spec_t*);
    mha_wave_t* process(mha_wave_t*);
private:
    MHAFilter::o1flt_lowpass_t attack;
    MHAFilter::o1flt_maxtrack_t decay;
    unsigned int nbands;
    unsigned int fftlen;
    MHASignal::waveform_t level_wave;
    MHASignal::waveform_t level_spec;
};

class dc_t : private dc_vars_validator_t {
public:
    dc_t(const dc_vars_t& vars,
         mha_real_t filter_rate,
         unsigned int nch,
         unsigned int fftlen);
    mha_spec_t* process(mha_spec_t*, mha_wave_t* level_db);
    mha_wave_t* process(mha_wave_t*, mha_wave_t* level_db);
private:
    class line_t {
    public:
        line_t(mha_real_t x1,mha_real_t y1,mha_real_t x2,mha_real_t y2);
        line_t(mha_real_t x1,mha_real_t y1,mha_real_t slope);
        inline mha_real_t operator()(mha_real_t x){return m*x+y0;};
    private:
        mha_real_t m, y0;
    };
    std::vector<mha_real_t> expansion_threshold;
    std::vector<mha_real_t> limiter_threshold;
    std::vector<line_t> compression;
    std::vector<line_t> expansion;
    std::vector<line_t> limiter;
    std::vector<mha_real_t> maxgain;
    unsigned int nbands;
public:
    std::vector<float> mon_l, mon_g;
};

dc_t::line_t::line_t(mha_real_t x1,mha_real_t y1,mha_real_t x2,mha_real_t y2)
    : m((y2-y1) / not_zero(x2-x1,"(line_t::line_t)")),y0(y1 - m*x1)
{
}

dc_t::line_t::line_t(mha_real_t x1,mha_real_t y1,mha_real_t m_)
    : m(m_),y0(y1 - m*x1)
{
}

typedef MHAPlugin::plugin_t<dc_t> DC;
typedef MHAPlugin::config_t<level_smoother_t> LEVEL;

class dc_if_t : public DC, public LEVEL, public dc_vars_t {
public:
    dc_if_t(const algo_comm_t& ac_,
            const std::string& th_,
            const std::string& al_);
    void prepare(mhaconfig_t& tf);
    void release();
    mha_spec_t* process(mha_spec_t*);
    mha_wave_t* process(mha_wave_t*);
private:
    void update_dc();
    void update_level();
    void has_been_modified(){modified.data = 1;};
    void read_modified(){modified.data = 0;};
    void update_level_mon();
    void update_gain_mon();
    MHAParser::string_t clientid;
    MHAParser::string_t gainrule;
    MHAParser::string_t preset;
    MHAParser::int_mon_t modified;
    MHAParser::vfloat_mon_t mon_l, mon_g;
    MHAParser::string_t filterbank;
    MHAParser::vfloat_mon_t center_frequencies;
    MHAParser::vfloat_mon_t edge_frequencies;
    MHAEvents::patchbay_t<dc_if_t> patchbay;
    bool prepared;
};

dc_if_t::dc_if_t(const algo_comm_t& ac_,
                 const std::string& th_,
                 const std::string& al_)
    : DC("Simple dynamic compression scheme",ac_),
      dc_vars_t(static_cast<MHAParser::parser_t&>(*this)),
      clientid("Client ID of last fit",""),
      gainrule("Gain rule of last fit",""),
      preset("Preset name of last fit",""),
      modified("Flag if configuration has been modified"),
      mon_l("Input level in dB"),
      mon_g("Applied gain in dB"),
      filterbank("Name of fftfilterbank plugin.\n"
                 "Used to extract frequency information.",""),
      center_frequencies("center frequencies of the frequency bands [Hz]"),
      edge_frequencies("edge frequencies of the frequency bands [Hz]"),
      prepared(false)
{
    set_node_id("dc_simple");
    patchbay.connect(&g50.writeaccess,this,&dc_if_t::update_dc);
    patchbay.connect(&g80.writeaccess,this,&dc_if_t::update_dc);
    patchbay.connect(&maxgain.writeaccess,this,&dc_if_t::update_dc);
    patchbay.connect(&expansion_threshold.writeaccess,this,&dc_if_t::update_dc);
    patchbay.connect(&expansion_slope.writeaccess,this,&dc_if_t::update_dc);
    patchbay.connect(&limiter_threshold.writeaccess,this,&dc_if_t::update_dc);
    patchbay.connect(&tauattack.writeaccess,this,&dc_if_t::update_level);
    patchbay.connect(&taudecay.writeaccess,this,&dc_if_t::update_level);
    insert_item("clientid",&clientid);
    insert_item("gainrule",&gainrule);
    insert_item("preset",&preset);
    insert_item("modified",&modified);
    insert_item("level",&mon_l);
    insert_item("gain",&mon_g);
    patchbay.connect(&writeaccess,this,&dc_if_t::has_been_modified);
    patchbay.connect(&modified.readaccess,this,&dc_if_t::read_modified);
    patchbay.connect(&mon_l.prereadaccess,this,&dc_if_t::update_level_mon);
    patchbay.connect(&mon_g.prereadaccess,this,&dc_if_t::update_gain_mon);
    insert_item("filterbank",&filterbank);
    insert_item("cf", &center_frequencies);
    insert_item("ef", &edge_frequencies);
}

void dc_if_t::update_level_mon()
{
    mon_l.data.clear();
    if( DC::cfg )
        mon_l.data = DC::cfg->mon_l;
}

void dc_if_t::update_gain_mon()
{
    mon_g.data.clear();
    if( DC::cfg ){
        mon_g.data.resize(DC::cfg->mon_g.size());
        for(unsigned int k=0;k<mon_g.data.size();k++){
            mon_g.data[k] = 20*log10(DC::cfg->mon_g[k]);
        }
    }
}

void dc_if_t::update_dc()
{
    clientid.data = "";
    gainrule.data = "";
    preset.data = "";
    if( prepared ){
        float frate = tftype.srate;
        if( tftype.domain == MHA_SPECTRUM )
            frate = tftype.srate / not_zero(tftype.fragsize,"(dc_if_t::update_dc)");
        DC::push_config(new dc_t(static_cast<const dc_vars_t&>(*this),
                                 frate,
                                 tftype.channels,
                                 tftype.fftlen));
    }
}

void dc_if_t::update_level()
{
    clientid.data = "";
    gainrule.data = "";
    preset.data = "";
    if( prepared ){
        float frate = tftype.srate;
        if( tftype.domain == MHA_SPECTRUM )
            frate = tftype.srate / not_zero(tftype.fragsize,"(dc_if_t::update_dc)");
        LEVEL::push_config(new level_smoother_t(static_cast<const dc_vars_t&>(*this),
                                                frate,
                                                tftype));
    }
}

void dc_if_t::prepare(mhaconfig_t& tf)
{
    try{
        prepared = true;
        tftype = tf;
        if( tftype.channels == 0 )
            throw MHA_Error(__FILE__,__LINE__,"This plugin cannot handle zero input channels.");
        std::string tmp_clientid = clientid.data;
        std::string tmp_gainrule = gainrule.data;
        std::string tmp_preset = preset.data;
        update_dc();
        update_level();
        clientid.data = tmp_clientid;
        gainrule.data = tmp_gainrule;
        preset.data = tmp_preset;
        if( filterbank.data.size() ){
            center_frequencies.data = MHA_AC::get_var_vfloat(ac, filterbank.data + "_cf");
            edge_frequencies.data = MHA_AC::get_var_vfloat(ac, filterbank.data + "_ef" );
        }else{
            center_frequencies.data.clear();
            edge_frequencies.data.clear();
        }
    }
    catch(...){
        prepared = false;
        throw;
    }
}

void dc_if_t::release()
{
    prepared = false;
}

mha_spec_t* dc_if_t::process(mha_spec_t* s)
{
    if( bypass.data )
        return s;
    return DC::poll_config()->process(s,LEVEL::poll_config()->process(s));
}

mha_wave_t* dc_if_t::process(mha_wave_t* s)
{
    if( bypass.data )
        return s;
    return DC::poll_config()->process(s,LEVEL::poll_config()->process(s));
}

dc_vars_validator_t::dc_vars_validator_t(const dc_vars_t& v,unsigned int s)
{
    if( !s )
        throw MHA_Error(__FILE__,__LINE__,
                        "Invalid (zero) channel count.");
    test_fail(v.g50.data,s,"g50");
    test_fail(v.g80.data,s,"g80");
    test_fail(v.expansion_threshold.data,s,"expansion threshold");
    test_fail(v.expansion_slope.data,s,"expansion slope");
    test_fail(v.limiter_threshold.data,s,"limiter threshold");
    test_fail(v.tauattack.data,s,"attack time constant");
    test_fail(v.taudecay.data,s,"decay time constant");
}

level_smoother_t::level_smoother_t(const dc_vars_t& vars,
                                   mha_real_t filter_rate,
                                   mhaconfig_t buscfg)
    : dc_vars_validator_t(vars, buscfg.channels),
      attack(force_resize(vars.tauattack.data,buscfg.channels,"tau_attack"), filter_rate, 65),
      decay(force_resize(vars.taudecay.data,buscfg.channels,"tau_decay"), filter_rate, 65),
      nbands(buscfg.channels),
      fftlen(buscfg.fftlen),
      level_wave(buscfg.fragsize,buscfg.channels),
      level_spec(1,buscfg.channels)
{
}

dc_t::dc_t(const dc_vars_t& vars,
           mha_real_t filter_rate,
           unsigned int nch,
           unsigned int fftlen_)
    : dc_vars_validator_t(vars, nch),
      expansion_threshold(force_resize(vars.expansion_threshold.data,nch,"expansion_threshold")),
      limiter_threshold(force_resize(vars.limiter_threshold.data,nch,"limiter_threshold")),
      maxgain(force_resize(vars.maxgain.data,nch,"maxgain")),
      nbands(nch)
{
    std::vector<float> expansion_slope(force_resize(vars.expansion_slope.data,nch,"expansion_slope"));
    std::vector<float> g50(force_resize(vars.g50.data,nch,"g50"));
    std::vector<float> g80(force_resize(vars.g80.data,nch,"g80"));
    compression.clear();
    expansion.clear();
    for(unsigned int k=0;k<nbands;k++){
        compression.push_back(line_t(50,g50[k],80,g80[k]));
        expansion.push_back(line_t(expansion_threshold[k],compression[k](expansion_threshold[k]),expansion_slope[k]-1));
        limiter.push_back(line_t(limiter_threshold[k],compression[k](limiter_threshold[k]),-1.0f));
    }
    mon_l.resize(nbands);
    mon_g.resize(nbands);
}

mha_wave_t* level_smoother_t::process(mha_wave_t* s)
{
    unsigned int t, k;
    for(k=0;k<nbands;k++)
        for(t=0;t<s->num_frames;t++)
            level_wave(t,k) = decay(k,MHASignal::pa2dbspl(attack(k,fabsf(value(s,t,k)))));
    return &level_wave;
}

mha_wave_t* dc_t::process(mha_wave_t* s, mha_wave_t* level_db)
{
    if( s->num_channels != nbands )
        throw MHA_ErrorMsg("dc_simple: unexpected change of channel number.");
    unsigned int t, k;
    mha_real_t l(0), g(0);
    for(k=0;k<nbands;k++){
        for(t=0;t<s->num_frames;t++){
            l = value(level_db,t,k);
            if( l <= expansion_threshold[k] )
                g = expansion[k](l);
            else if( l <= limiter_threshold[k] )
                g = compression[k](l);
            else
                g = limiter[k](l);
            g = std::min(g,maxgain[k]);
            g = pow(10.0,0.05*g);
            value(s,t,k) *= g;
        }
        mon_l[k] = l;
        mon_g[k] = g;
    }
    return s;
}

mha_wave_t* level_smoother_t::process(mha_spec_t* s)
{
    for(unsigned int k=0;k<s->num_channels;k++)
        level_spec.buf[k] = decay(k,attack(k,MHASignal::pa2dbspl(std::max(1e-10f,MHASignal::rmslevel(*s,k,fftlen)))));
    return &level_spec;
}

mha_spec_t* dc_t::process(mha_spec_t* s, mha_wave_t* level_db)
{
    if( s->num_channels != nbands )
        throw MHA_ErrorMsg("dc_simple: unexpected change of channel number.");
    unsigned int bin, k;
    mha_real_t l, g;
    for(k=0;k<nbands;k++){
        l = level_db->buf[k];
        if( l <= expansion_threshold[k] )
            g = expansion[k](l);
        else if( l <= limiter_threshold[k] )
            g = compression[k](l);
        else
            g = limiter[k](l);
        g = std::min(g,maxgain[k]);
        g = pow(10.0,0.05*g);
        for(bin=0;bin<s->num_frames;bin++){
            value(s,bin,k) *= g;
        }
        mon_l[k] = l;
        mon_g[k] = g;
    }
    return s;
}

dc_vars_t::dc_vars_t(MHAParser::parser_t& p)
    : g50("Gain in dB at 50 dB input level","[0]","[-80,80]"),
      g80("Gain in dB at 80 dB input level","[0]","[-80,80]"),
      maxgain("Maximal amplification in dB","[80]"),
      expansion_threshold("expansion threshold in dB","[0]",""),
      expansion_slope("expansion slope of input-output function in dB/dB","[1]","]0,10]"),
      limiter_threshold("limiter threshold in dB","[100]"),
      tauattack("attack time constant in s","[0.005]","[0,]"),
      taudecay("decay time constant in s","[0.05]","[0,]"),
      bypass("bypass dynamic compression","no")
{
    p.insert_item("g50",&g50);
    p.insert_item("g80",&g80);
    p.insert_item("maxgain",&maxgain);
    p.insert_item("expansion_threshold",&expansion_threshold);
    p.insert_item("expansion_slope",&expansion_slope);
    p.insert_item("limiter_threshold",&limiter_threshold);
    p.insert_item("tau_attack",&tauattack);
    p.insert_item("tau_decay",&taudecay);
    p.insert_item("bypass",&bypass);
}

}

MHAPLUGIN_CALLBACKS(dc_simple,dc_simple::dc_if_t,spec,spec)
MHAPLUGIN_PROC_CALLBACK(dc_simple,dc_simple::dc_if_t,wave,wave)
MHAPLUGIN_DOCUMENTATION\
(dc_simple,
 "compression level-modification",
 "The plugin {\\em dc\\_simple} is a multiband dynamic compression.\n"
 "One compression function (input-output function) is applied to\n"
 "each audio channel; multiple frequency bands can be used via the\n"
 "{\\em fftfilterbank} plugin. The level dependent gain function\n"
 "is determined by the gains at 50 and 80 dB (G50 and G80). To reduce\n"
 "noise, an expansion is applied below a noise gate level. See also\n"
 "\\figref{dc_simple_in_out}.\n"
 "\n"
 "If spectral processing is used, the input level"
 " ($x$-axis of the input-output function)\n"
 "is determined by an attack- and release-filter of the short time"
 " RMS level $L_{st}$ given in dB (SPL)"
 "The attack filter is a first order low pass filter."
 " The release filter is a maximum tracker, i.e.\n"
 "\\begin{eqnarray}\n"
 "L_{a} &=& \\left\\langle 20\\log_{10}(L_{st}) \\right\\rangle_{\\tau_{attack}}\\\\\n"
 "L_{in} &=& {\\textrm{max}}(L_{a},\\left\\langle L_{a} \\right\\rangle_{\\tau_{release}})\n"
 "\\end{eqnarray}\n"
 "The input level is divided into three sections. In each section the input"
 " level $L_{in}$ is transformed linearly into a gain $G$ on a log-log scale:\n"
 "$G_{dB} = (m-1) L_{in} + n$, where $m$ is the slope of the"
 " input-output function, and $n$ is an offset.\n"
 "Between expansion threshold and limiter threshold,"
 " $m$ and $n$ are given by the gain at 50 and 80 dB.\n"
 "In the section below the expansion threshold,"
 " $m$ is the expansion slope, and above the limiter threshold, $m$ is zero.\n"
 "$n$ is chosen to result in a continuous input-output function.\n"
 "\n"
 "All variables are vectors with one entry for\n"
 "each input channel (number of audio channels times number of frequency\n"
 "bands).\n"
 "\n"
 "\\MHAfigure{Input-output function of one channel in the"
 " {\\em dc\\_simple} dynamic compression algorithm.}{dc_simple_in_out}\n"
 "\n"
 "An example configuration of a chain with dynamic compression could be:\n"
 "\n"
 "\\begin{verbatim}\n"
 "algos = [fftfilterbank dc_simple combinechannels]\n"
 "fftfilterbank.ftype = center\n"
 "fftfilterbank.f = [250 1000 4000]\n"
 "fftfilterbank.ovltype = rect\n"
 "fftfilterbank.fscale = bark\n"
 "dc_simple.g50 = [10 25 40 11 31 55]\n"
 "dc_simple.g80 = [5 15 10 5 21 19]\n"
 "dc_simple.expansion_threshold = [20 20 20 20 20 20]\n"
 "dc_simple.expansion_slope = [4 4 4 4 4 4]\n"
 "dc_simple.limiter_threshold = [120 120 120 120 120 120]\n"
 "dc_simple.tau_attack = [0.005 0.005 0.005 0.005 0.005 0.005]\n"
 "dc_simple.tau_decay = [0.015 0.015 0.015 0.015 0.015 0.015]\n"
 "combinechannels.name = fftfilterbank_channels\n"
 "\\end{verbatim}\n"
 "\n"
 "In this configuration it is assumed that two audio channels are\n"
 "configured, i.e.\\ all variables of {\\em dc\\_simple} have three entries\n"
 "for the first audio channel and three for the second audio channel.\n"
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
