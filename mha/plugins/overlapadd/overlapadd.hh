// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2007 2008 2009 2010 2013 2014 2015 2017 2018 HörTech gGmbH
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

#ifndef OVERLAPADD_HH
#define OVERLAPADD_HH

#include <mha_plugin.hh>
#include <mha_signal.hh>
#include <mha_utils.hh>
#include <math.h>
#include <mha_events.h>
#include <mhapluginloader.h>
#include <mha_filter.hh>
#include <mha_windowparser.h>

// Forward declaration for unit test
class overlapadd_testing;

namespace overlapadd {

class overlapadd_t
{
public:
    overlapadd_t(mhaconfig_t spar_in,
                 mhaconfig_t spar_out,
                 float wexp,
                 float wndpos,
                 const MHAParser::window_t& window,
                 const MHAParser::window_t& zerowindow,
                 float& prescale_fac,float& postscale_fac);
    ~overlapadd_t();
    mha_spec_t* wave2spec(mha_wave_t*);
    mha_wave_t* spec2wave(mha_spec_t*);

private:
    friend class ::overlapadd_testing;
    void wave2spec_hop_forward(mha_wave_t *);
    void wave2spec_apply_window(void);
    mha_spec_t * wave2spec_compute_fft(void);

    mha_fft_t fft;
    MHAWindow::base_t prewnd;
    MHAWindow::base_t postwnd;
    MHASignal::waveform_t wave_in1;
    MHASignal::waveform_t wave_out1;
    MHASignal::spectrum_t spec_in;
    MHASignal::waveform_t calc_out;
    MHASignal::waveform_t out_buf;
    MHASignal::waveform_t write_buf;
    unsigned int n_zero;
    unsigned int n_pad1;
    unsigned int n_pad2;
};

class overlapadd_if_t : public MHAPlugin::plugin_t<overlapadd_t> {
public:
    overlapadd_if_t(const algo_comm_t&,const std::string&,const std::string&);
    ~overlapadd_if_t()=default;
    void prepare(mhaconfig_t&);
    void release();
    mha_wave_t* process(mha_wave_t*);
private:
    void update();
    /// \brief FFT length to be used, zero-padding is FFT length-wndlength
    MHAParser::int_t nfft;
    /// \brief Window length to be used (overlap is 1-fragsize/wndlength)
    MHAParser::int_t nwnd;
    /// \brief Relative position of zero padding (0 end, 0.5 center, 1 start)
    MHAParser::float_t wndpos;
    MHAParser::window_t window;
    MHAParser::float_t wndexp;
    MHAParser::window_t zerowindow;
    /// \brief Disallow window sizes that are not a multiple of the hop size
    ///        ("fragsize" in MHA) a by power of two
    MHAParser::bool_t strict_window_ratio;
    MHAParser::mhapluginloader_t plugloader;
    MHAParser::float_mon_t prescale;
    MHAParser::float_mon_t postscale;
    std::string algo;
    mhaconfig_t cf_in, cf_out;
    /** Lock/Unlock all configuration variables
     * @param b Desired lock state
     */
    void setlock(bool b){
        nfft.setlock(b);
        nwnd.setlock(b);
        wndpos.setlock(b);
        window.setlock(b);
        wndexp.setlock(b);
        zerowindow.setlock(b);
        strict_window_ratio.setlock(b);
    };
};
}
#endif

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
