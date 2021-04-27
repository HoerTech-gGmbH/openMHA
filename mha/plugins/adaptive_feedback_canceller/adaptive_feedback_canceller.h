// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2017 2018 2020 2021 HörTech gGmbH
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

#ifndef ADAPTIVE_FEEDBACK_CANCELLER_H
#define ADAPTIVE_FEEDBACK_CANCELLER_H

#include "mha_plugin.hh"


class adaptive_feedback_canceller;

//runtime config
class adaptive_feedback_canceller_config {

public:
    adaptive_feedback_canceller_config(algo_comm_t &ac, const mhaconfig_t in_cfg, adaptive_feedback_canceller *afc);
    ~adaptive_feedback_canceller_config();

    mha_wave_t* process(mha_wave_t*s_Y, mha_real_t rho, mha_real_t c);
    void insert();

    //declare data necessary for processing state here

private:
    algo_comm_t ac;
    unsigned int ntaps;
    unsigned int frames;
    unsigned int channels;
    MHA_AC::waveform_t s_E;
    MHA_AC::waveform_t F;
    MHASignal::waveform_t Pu; ///< \brief Power of input signal delayline

    std::string name_d_;
    std::string name_lpc_;

    int n_no_update_;
    int no_iter;
    int iter;

    double PSD_val;
    
    MHASignal::waveform_t  v_G;

    MHASignal::waveform_t s_U;
    MHASignal::delay_t s_E_afc_delay;

    MHASignal::delay_t s_W;
    MHASignal::ringbuffer_t s_Wflt;
    MHASignal::delay_t s_U_delay;
    MHASignal::ringbuffer_t s_U_delayflt;
    MHASignal::waveform_t F_Uflt;
    MHASignal::delay_t s_Y_delay;
    MHASignal::ringbuffer_t s_Y_delayflt;
    MHASignal::ringbuffer_t UbufferPrew;
    mha_wave_t s_LPC;
    mha_wave_t UPrew;
    mha_wave_t YPrew;
    mha_wave_t EPrew;
    mha_wave_t UPrewW;

    mha_wave_t smpl;
    mha_wave_t *s_Usmpl;

};

class adaptive_feedback_canceller : public MHAPlugin::plugin_t<adaptive_feedback_canceller_config> {

public:
    adaptive_feedback_canceller(algo_comm_t ac,
                                const std::string & configured_name);
    ~adaptive_feedback_canceller();
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release(void) {/* Do nothing in release */}

    //declare MHAParser variables here
    MHAParser::float_t rho;
    MHAParser::float_t c;
    MHAParser::int_t ntaps;
    MHAParser::vfloat_t gains;
    MHAParser::string_t name_e;
    MHAParser::string_t name_f;
    MHAParser::string_t name_lpc;
    MHAParser::int_t lpc_order;
    MHAParser::vint_t afc_delay;
    MHAParser::vint_t delay_w;
    MHAParser::vint_t delay_d;
    MHAParser::int_t n_no_update;

private:
    void update_cfg();

    /* patch bay for connecting configuration parser
       events with local member functions: */
    MHAEvents::patchbay_t<adaptive_feedback_canceller> patchbay;

};

#endif // ADAPTIVE_FEEDBACK_CANCELLER_H

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
