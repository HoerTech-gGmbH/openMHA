// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2010 2011 2013 2016 2017 HörTech gGmbH
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

#ifndef DC_AFTERBURN_H
#define DC_AFTERBURN_H

#include "mha_parser.hh"
#include "mha_filter.hh"
#include "mha_defs.h"

namespace DynComp {

    /**
       \brief Variables for dc_afterburn_t class.
    */
    class dc_afterburn_vars_t : public MHAParser::parser_t {
    public:
        dc_afterburn_vars_t();
        MHAParser::vfloat_t f;
        MHAParser::vfloat_t drain;
        MHAParser::vfloat_t conflux;
        MHAParser::vfloat_t maxgain;
        MHAParser::vfloat_t mpo;
        MHAParser::float_t taugain;
        MHAParser::kw_t commit;
        MHAParser::bool_t bypass;
    };

/**
   \brief Real-time class for after burn effect.

   The constructor processes the parameters and creates pre-processed
   variables for efficient realtime processing.
*/
    class dc_afterburn_rt_t {
    public:
        dc_afterburn_rt_t(const std::vector<float>& cf,unsigned int channels, float srate, const dc_afterburn_vars_t& vars);
        /**
           \brief gain modifier method (afterburn).

           \param Gin Linear gain.
           \param Lin Input level (Pascal).
           \param band Filter band number.
           \param channel Channel number.

           Output level for MPO is estimated by Gin * Lin.
        */
        inline void burn(float& Gin, float Lin, unsigned int band, unsigned int channel)
            {
                // gain modifiers:
                // drain: if drain is low, amplify signal
                // conflux: subtract conflux from insertion gain (linear domain)
                // maxgain: do not apply more than maxgain
                // mpo: reduce gain if Lin is above mpo
                // low pass: apply to gain modifiers (not to original gain)

                float Gain_vent = Gin*drain_inv[band]-conflux[band];
                float Gain_limited = std::min(maxgain[band],std::max(0.0f,Gain_vent));
                float Gain_mpo = Gain_limited / std::max(1.0f,Gin*Lin*mpo_inv[band]);
                float Gain_modifier = lp[channel](band,Gain_mpo / std::max(1e-20f,Gin));
                Gin *= Gain_modifier;
            };
    private:
        std::vector<float> drain_inv;
        std::vector<float> conflux;
        std::vector<float> maxgain;
        std::vector<float> mpo_inv;
        std::vector<MHAFilter::o1flt_lowpass_t> lp;
    };

/**
   \brief Afterburn class, to be defined as a member of compressors.
*/
    class dc_afterburn_t : public dc_afterburn_vars_t, protected MHAPlugin::config_t<dc_afterburn_rt_t> {
    public:
        dc_afterburn_t();
        void set_fb_pars(const std::vector<float>& cf, unsigned int channels, float srate);
        void unset_fb_pars();
        void update_burner(){poll_config();};
        inline void burn(float& Gin, float Lin, unsigned int band, unsigned int channel) 
            {
                if( bypass.data )
                    return;
                MHA_assert( cfg );
                cfg->burn(Gin,Lin,band,channel);
            };
    private:
        void update();
        MHAEvents::patchbay_t<dc_afterburn_t> patchbay;
        std::vector<float> _cf;
        unsigned int _channels;
        float _srate;
        bool commit_pending;
        bool fb_pars_configured;
    };

}

#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * compile-command: "make -C .."
 * coding: utf-8-unix
 * End:
 */
