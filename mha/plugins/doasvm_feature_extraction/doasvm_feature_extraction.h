// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2014 2018 2021 HörTech gGmbH
// Copyright © 2022 Hörzentrum Oldenburg gGmbH
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

#ifndef DOASVM_FEATURE_EXTRACTION_H
#define DOASVM_FEATURE_EXTRACTION_H

#include "mha_plugin.hh"
#include <mha_toolbox.h>
#include "hann.h"
#include "ifftshift.h"

class doasvm_feature_extraction;

//runtime config
class doasvm_feature_extraction_config {

public:
    doasvm_feature_extraction_config(MHA_AC::algo_comm_t & ac,
                                     const mhaconfig_t in_cfg,
                                     doasvm_feature_extraction *_doagcc);
    ~doasvm_feature_extraction_config();

    mha_wave_t* process(mha_wave_t*);

    //declare data necessary for processing state here
    doasvm_feature_extraction *doagcc;

    unsigned int wndlen;
    unsigned int fftlen;
    unsigned int G_length;
    unsigned int GCC_start;
    unsigned int GCC_end;

    MHA_AC::waveform_t vGCC_ac;

    mha_fft_t fft;
    mha_fft_t ifft;

    double hifftwin_sum;

    MHASignal::waveform_t proc_wave;
    MHASignal::waveform_t hwin;
    MHASignal::waveform_t hifftwin;
    MHASignal::waveform_t vGCC;

    MHASignal::spectrum_t in_spec;
    MHASignal::spectrum_t G;

};

class doasvm_feature_extraction : public MHAPlugin::plugin_t<doasvm_feature_extraction_config> {

public:
    doasvm_feature_extraction(MHA_AC::algo_comm_t & iac,
                              const std::string & configured_name);
    ~doasvm_feature_extraction();
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release(void) {/* Do nothing in release */}

    //declare MHAParser variables here
    MHAParser::int_t fftlen;
    MHAParser::int_t max_lag;
    MHAParser::int_t nupsample;
    MHAParser::string_t vGCC_name;

private:
    void update_cfg();

    /* patch bay for connecting configuration parser
       events with local member functions: */
    MHAEvents::patchbay_t<doasvm_feature_extraction> patchbay;

};

#endif // DOASVM_FEATURE_EXTRACTION_H

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
