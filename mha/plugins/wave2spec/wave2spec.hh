// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2008 2009 2010 2013 2014 2015 2018 HörTech gGmbH
// Copyright © 2019 HörTech gGmbH
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

#ifndef WAVE2SPEC_HH
#define WAVE2SPEC_HH

#define MHAPLUGIN_OVERLOAD_OUTDOMAIN

#include <mha_plugin.hh>
#include <mha_signal.hh>
#include <mha_utils.hh>
#include <math.h>
#include <mha_defs.h>
#include <mha_events.h>
#include "windowselector.h"

class wave2spec_t : public MHA_AC::spectrum_t {
public:
    wave2spec_t(unsigned int nfft,
                unsigned int nwnd_,
                unsigned int nwndshift_,
                unsigned int nch,
                mha_real_t wndpos,
                const MHAWindow::base_t& window,
                algo_comm_t ac,
                std::string algo);
    mha_spec_t* process(mha_wave_t*);
    ~wave2spec_t();
private:
    void calc_pre_wnd(MHASignal::waveform_t&,const MHASignal::waveform_t&);
    unsigned int nwnd;
    unsigned int nwndshift;
    mha_fft_t ft;       //!< FFT class
    unsigned int npad1; //!< length of zero padding before window
    unsigned int npad2; //!< length of zero padding after window
    MHAWindow::base_t window;
    MHASignal::waveform_t calc_in;
    MHASignal::waveform_t in_buf;
    MHASignal::spectrum_t spec_in;      //!< non-interleaved, complex, fftlen
};

class wave2spec_if_t : public MHAPlugin::plugin_t<wave2spec_t> {
public:
    wave2spec_if_t(const algo_comm_t&,const std::string&,const std::string&);
    void prepare(mhaconfig_t&);
    void process(mha_wave_t*,mha_spec_t**);
    void process(mha_wave_t*,mha_wave_t**);
private:
    void update();
    MHAEvents::patchbay_t<wave2spec_if_t> patchbay;
    MHAParser::int_t nfft;
    MHAParser::int_t nwnd;
    MHAParser::float_t wndpos;
    windowselector_t window_config;
    /** Switch to disallow window sizes that are not a
        multiple of the fragsize a by power of two. */
    MHAParser::bool_t strict_window_ratio;
    MHAParser::bool_t return_wave;
    std::string algo;
};

#endif // WAVE2SPEC_HH

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
