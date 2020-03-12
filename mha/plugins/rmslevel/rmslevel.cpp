// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2009 2010 2013 2014 2015 2017 2018 HörTech gGmbH
// Copyright © 2019 2020 HörTech gGmbH
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
#include "mha_tablelookup.hh"
#include "mha_utils.hh"
#include <math.h>
#include <unordered_map>
#include <memory>
#include <utility>
#include <string>
#include <vector>
#include <algorithm>

namespace rmslevel {

    enum class UNIT {SPL=0, HL=1};

    class mon_t : public MHA_AC::waveform_t, private MHAParser::vfloat_mon_t {
    public:
        mon_t(unsigned int nch,const std::string& name, algo_comm_t ac,const std::string& base,
              MHAParser::parser_t& p, const std::string& help);
        mon_t(const mon_t&)=delete;
        mon_t(mon_t &&)=delete;

        mon_t& operator=(const mon_t&) = delete;
        mon_t& operator=(mon_t&&) = delete;
        void store();
    };

    mon_t::mon_t(unsigned int nch,const std::string& name,algo_comm_t ac,const std::string& base,
                 MHAParser::parser_t& p,const std::string& help)
        : MHA_AC::waveform_t(ac,base+"_"+name,1,nch,false),
          MHAParser::vfloat_mon_t(help)
    {
        p.force_remove_item(name);
        p.insert_item(name,this);
       
        data.resize(nch);
    }

    void mon_t::store()
    {
        for(unsigned int k=0;k<get_size();k++)
            data[k] = buf[k];
    }

    /**
     * Run-time configuration class of the rmslevel plugin
     */
    class rmslevel_t {
    public:
        /**
         * C'tor of the runtime configuration class
         * @param tf Input signal configuration
         * @param ac AC space, needed to publish ac variables
         * @param name Configured name of the plugin within the parser structure
         * @param p Instance of the parent interface class, needed to publish monitor variables
         * @param unit_ Configured dB scale
         * @param num_channels_ Number of monitor channels if channel summation shall be used, zero otherwise
         */
        rmslevel_t(const mhaconfig_t& tf ,algo_comm_t ac,const std::string& name,MHAParser::parser_t& p,
                   UNIT unit_);
        ~rmslevel_t(){};
        mha_spec_t* process(mha_spec_t*);
        mha_wave_t* process(mha_wave_t*);
        void fill(mha_spec_t*);
        void sum_and_fill(mha_spec_t*);
        void fill(mha_wave_t*);
        void sum_and_fill(mha_wave_t*);
        void insert();
    private:
        // A map is potentially slower but leads to better code readability.
        std::unordered_map<std::string,std::unique_ptr<mon_t> > monitors;
        unsigned int fftlen;
        UNIT unit;
        mha_domain_t domain;
        /** freq_offsets provides the conversion of dB(SPL) to dB(HL)
         * for every frequency bin in the stft used by coloured_intensity.
         * Unused when not in spectral domain and unit=hl.
         */
        std::vector<mha_real_t> freq_offsets;
    };

    /**
     * Interface class of the rmslevel plugin
     */
    class rmslevel_if_t : public MHAPlugin::plugin_t<rmslevel_t> {
    public:
        rmslevel_if_t(const algo_comm_t&,const std::string&,const std::string&);
        mha_spec_t* process(mha_spec_t*);
        mha_wave_t* process(mha_wave_t*);
        void prepare(mhaconfig_t&);
    private:
        void update();
        MHAEvents::patchbay_t<rmslevel_if_t> patchbay;
        std::string name;
        MHAParser::kw_t unit;
    };

    void rmslevel_t::insert()
    {
        for(auto& mon : monitors) {
            mon.second->insert();
        }
    }

    rmslevel_t::rmslevel_t(const mhaconfig_t& tf,algo_comm_t ac,const std::string& name,MHAParser::parser_t& p,
                           UNIT unit_)
        : fftlen(tf.fftlen),
          unit(unit_),
          domain(tf.domain),
          freq_offsets(fftlen/2+1)
    {
        for(unsigned idx=0U; idx<fftlen/2+1; ++idx){
            // factor two needed because coloured intensity expects squared weights
            freq_offsets[idx]=MHASignal::db2lin(2*MHAUtils::spl2hl(MHASignal::bin2freq(idx,tf.fftlen, tf.srate)));
        }
        monitors.emplace(std::make_pair("level",std::make_unique<mon_t>(tf.channels,
                                                                        "level",ac,name,p,"RMS level in W/m^2")));
        monitors.emplace(std::make_pair("level_db",std::make_unique<mon_t>(tf.channels,
                                                                           "level_db",ac,name,p,"RMS level in dB")));
        if(domain==MHA_WAVEFORM){
            monitors.emplace(std::make_pair("peak",std::make_unique<mon_t>(tf.channels,
                                                                           "peak",ac,name,p,"peak amplitude in Pa")));
            monitors.emplace(std::make_pair("peak_db",std::make_unique<mon_t>(tf.channels,
                                                                              "peak_db",ac,name,p,"peak amplitude in dB")));
        }
    }

    rmslevel_if_t::rmslevel_if_t(const algo_comm_t &iac, const std::string &ith,
                                 const std::string &ial)
        : plugin_t<rmslevel_t>(
              "This algorithm displays block based RMS level informations.\n"
              "Results are stored in these AC variables (replace 'rmslevel'\n"
              "by the configured plugin name):\n\n"
              "  rmslevel_level_db\n"
              "  rmslevel_peak_db\n"
              "  rmslevel_level\n"
              "  rmslevel_peak\n"
              " The \'peak\' variables are only"
              " available during waveform processing.",
              iac),
          name(ial), unit("Use dB(SPL) or dB(HL)", "spl", "[spl hl]")
    {
      insert_item("unit", &unit);
      patchbay.connect(&unit.writeaccess, this, &rmslevel_if_t::update);
    }

    mha_spec_t* rmslevel_if_t::process(mha_spec_t* s)
    {
        poll_config();
        return cfg->process(s);
    }

    mha_wave_t* rmslevel_if_t::process(mha_wave_t* s)
    {
        poll_config();
        return cfg->process(s);
    }

    mha_spec_t* rmslevel_t::process(mha_spec_t* s)
    {
        for(unsigned int ch=0U; ch<s->num_channels;ch++){
            (*monitors["level"])[ch] = std::max(MHASignal::rmslevel(*s,ch,fftlen),2e-10f);
            if(unit==UNIT::SPL){
                (*monitors["level_db"])[ch] = MHASignal::pa2dbspl((*monitors["level"])[ch]);
            }
            else if(unit==UNIT::HL){
                (*monitors["level_db"])[ch] =
                    MHASignal::pa22dbspl(std::max(MHASignal::colored_intensity(*s,ch,fftlen,freq_offsets.data()),2e-10f));
            }
            else {
                throw MHA_Error(__FILE__,__LINE__,"Bug: Unknown unit for dB\nUse dB(SPL) or dB(HL)");
            }
        }
        for(auto& mon: monitors){
            mon.second->store();
        }
        insert();
        return s;
    }

    mha_wave_t* rmslevel_t::process(mha_wave_t* s)
    {
        for(unsigned ch=0U; ch<s->num_channels;ch++){
            (*monitors["level"])[ch] = std::max(MHASignal::rmslevel(*s,ch),2e-10f);
            (*monitors["peak"])[ch] = std::max(MHASignal::maxabs(*s,ch),2e-10f);
            (*monitors["level_db"])[ch] = MHASignal::pa2dbspl((*monitors["level"])[ch]);
            (*monitors["peak_db"])[ch] = MHASignal::pa2dbspl((*monitors["peak"])[ch]);
        }
        for(auto& mon: monitors){
            mon.second->store();
        }
        return s;
    }

    void rmslevel_if_t::update(){
        if(!is_prepared())
            return;
        if( tftype.domain == MHA_WAVEFORM and static_cast<UNIT>(unit.data.get_index()) == UNIT::HL )
            throw MHA_Error(__FILE__,__LINE__,"rmslevel: Conversion to dB(HL) is only supported in frequency domain.");
        auto cfg=new rmslevel_t(tftype,ac,name,static_cast<MHAParser::parser_t&>(*this),
                                static_cast<UNIT>(unit.data.get_index()));
        cfg->insert();
        push_config(cfg);
    }

    void rmslevel_if_t::prepare(mhaconfig_t& tf)
    {
        tftype = tf;
        update();
    }

    MHAPLUGIN_CALLBACKS(rmslevel,rmslevel_if_t,spec,spec)
    MHAPLUGIN_PROC_CALLBACK(rmslevel,rmslevel_if_t,wave,wave)
    MHAPLUGIN_DOCUMENTATION\
    (rmslevel,
     "level-meter feature-extraction",
     "This plugin computes the rms level and peak level of the current fragment and provides them as AC and monitor "
     " variables rms level in $W/m^2$ and peak level in Pascal. \n"
     "The values are provided in linear (variable names: level and peak) and logarithmic scale (level\\_db and peak\\_db)."
     " The default unit for the logarithmic scale is dB(SPL), \n"
     "but conversion to dB(HL) as per ISO 389-7:2005 (freefield) can be activated in the spectral domain. The correction values"
     " for frequencies above 16 kHz are extrapolated.")
}
// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
