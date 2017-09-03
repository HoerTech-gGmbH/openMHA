// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2007 2008 2009 2010 2013 2014 2015 2017 HörTech gGmbH
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

#include <mha_plugin.hh>
#include <mha_signal.hh>
#include <math.h>
#include <mha_events.h>
#include <mhapluginloader.h>
#include <mha_filter.hh>
#include <mha_windowparser.h>

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
    mha_spec_t* ola1(mha_wave_t*);
    mha_wave_t* ola2(mha_spec_t*);
private:
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

overlapadd_t::overlapadd_t(mhaconfig_t spar_in,
                           mhaconfig_t spar_out,
                           float wexp,
                           float wndpos,
                           const MHAParser::window_t& window,
                           const MHAParser::window_t& zerowindow,
                           float& prescale_fac,float& postscale_fac)
    : fft(mha_fft_new(spar_in.fftlen)),
      prewnd(window.get_window(spar_in.wndlen)),
      postwnd(window.get_window(spar_in.fftlen)),
      wave_in1(spar_in.wndlen,spar_in.channels),
      wave_out1(spar_in.fftlen,spar_in.channels),
      spec_in(spar_in.fftlen/2+1,spar_in.channels),
      calc_out(spar_out.fftlen,spar_out.channels),
      out_buf(spar_out.fftlen,spar_out.channels),
      write_buf(spar_out.fragsize,spar_out.channels),
      n_zero(spar_in.fftlen-spar_in.wndlen),
      n_pad1((unsigned int)(wndpos*n_zero)),
      n_pad2(n_zero-n_pad1)
{
    if( spar_in.fragsize != spar_out.fragsize )
        throw MHA_Error(__FILE__,__LINE__,"overlap add sub-plugins are not allowed to change fragment size from %d to %d.",spar_in.fragsize,spar_out.fragsize);
    if( spar_in.wndlen != spar_out.wndlen )
        throw MHA_Error(__FILE__,__LINE__,"overlap add sub-plugins are not allowed to change window length from %d to %d.",spar_in.wndlen,spar_out.wndlen);
    if( spar_in.fftlen != spar_out.fftlen )
        throw MHA_Error(__FILE__,__LINE__,"overlap add sub-plugins are not allowed to change FFT length from %d to %d.",spar_in.fftlen,spar_out.fftlen);
    prewnd ^= wexp;
    postwnd ^= 1.0-wexp;
    if( n_pad1 ){
        MHAWindow::base_t pad1wnd(zerowindow.get_window(n_pad1,-1,0));
        pad1wnd.ramp_begin(postwnd);
    }
    if( n_pad2 ){
        MHAWindow::base_t pad2wnd(zerowindow.get_window(n_pad2,0,1));
        pad2wnd.ramp_end(postwnd);
    }
    float scale_fac = sqrt(spar_in.fftlen/prewnd.sumsqr());
    float scale_postfac = scale_fac * spar_in.wndlen / (2.0f*spar_in.fragsize);

    switch( window.get_type() ){
    case MHAParser::window_t::wnd_hann :
        break;
    case MHAParser::window_t::wnd_rect :
        scale_postfac *= 2;
        break;
    case MHAParser::window_t::wnd_hamming :
        scale_postfac *= 1.08;
        break;
    case MHAParser::window_t::wnd_blackman :
        if( spar_in.wndlen/spar_in.fragsize == 2 )
            scale_postfac *= 0.847585;
        else
            scale_postfac *= 0.84;
        break;
    case MHAParser::window_t::wnd_bartlett :
        break;
    case MHAParser::window_t::wnd_user :
        break;
    }
    prewnd *= scale_fac;
    postwnd *= 1.0f/scale_postfac;
    prescale_fac = scale_fac;
    postscale_fac = 1.0f/scale_postfac;
}

overlapadd_t::~overlapadd_t()
{
    mha_fft_free(fft);
}

mha_spec_t* overlapadd_t::ola1(mha_wave_t* s)
{
    timeshift(wave_in1,-s->num_frames);
    assign(range(wave_in1,wave_in1.num_frames-s->num_frames,s->num_frames),*s);
    assign(range(wave_out1,0,n_pad1),0.0f);
    assign(range(wave_out1,n_pad1,wave_in1.num_frames),wave_in1);
    assign(range(wave_out1,n_pad1+wave_in1.num_frames,n_pad2),0.0f);
    mha_wave_t windowed_in = range(wave_out1,n_pad1,wave_in1.num_frames);
    prewnd(windowed_in);
    mha_fft_wave2spec(fft,&wave_out1,&spec_in);
    return &spec_in;
}

mha_wave_t* overlapadd_t::ola2(mha_spec_t* s)
{
    mha_fft_spec2wave(fft,s,&calc_out);
    postwnd(calc_out);
    timeshift(out_buf,-write_buf.num_frames);
    out_buf += calc_out;
    assign(write_buf,range(out_buf,0,write_buf.num_frames));
    return &write_buf;
}

class overlapadd_if_t : public MHAPlugin::plugin_t<overlapadd_t> {
public:
    overlapadd_if_t(const algo_comm_t&,const std::string&,const std::string&);
    ~overlapadd_if_t();
    void prepare(mhaconfig_t&);
    void release();
    mha_wave_t* process(mha_wave_t*);
private:
    void update();
    MHAEvents::patchbay_t<overlapadd_if_t> patchbay;
    /// \brief FFT length to be used, zero-padding is FFT length-wndlength
    MHAParser::int_t nfft;
    /// \brief Window length to be used (overlap is 1-fragsize/wndlength)
    MHAParser::int_t nwnd;
    /// \brief Relative position of zero padding (0 end, 0.5 center, 1 start)
    MHAParser::float_t wndpos;
    MHAParser::window_t window;
    MHAParser::float_t wndexp;
    MHAParser::window_t zerowindow;
    MHAParser::mhapluginloader_t plugloader;
    MHAParser::float_mon_t prescale;
    MHAParser::float_mon_t postscale;
    std::string algo;
    mhaconfig_t cf_in, cf_out;
};

overlapadd_if_t::overlapadd_if_t(const algo_comm_t& iac,const std::string&,const std::string& ialg)
    : MHAPlugin::plugin_t<overlapadd_t>(
        "Waveform to spectrum overlap add and FFT method.\n"
        "Audio data is collected up to wndlen, than windowed with\n"
        "the given window function, zero padded up to fftlength\n"
        "(symmetric zero padding or asymmetric zero padding possible),\n"
        "and Fast-Fourier-transformed.\n"
        "All parameter changes take effect after the next prepare call.",iac),
      nfft("FFT length","512","[1,]"),
      nwnd("window length/samples","400","[1,]"),
      wndpos("window position\n(0 = beginning, 0.5 = symmetric zero padding, 1 = end)","0.5","[0,1]"),
      window("window type"),
      wndexp("window exponent to be applied to all elements of window function","1"),
      zerowindow("zero padding post window type"),
      plugloader(*this,iac),
      prescale("scaling factor (pre-scaling)"),
      postscale("scaling factor (post-scaling)"),
      algo(ialg),
      cf_in(tftype),cf_out(tftype)
{
    zerowindow.parse("type=rect");
    insert_item("fftlen",&nfft);
    window.insert_item("len",&nwnd);
    window.insert_item("pos",&wndpos);
    window.insert_item("exp",&wndexp);
    insert_item("wnd",&window);
    insert_item("zerownd",&zerowindow);
    insert_member(prescale);
    insert_member(postscale);
    patchbay.connect(&wndexp.writeaccess,this,&overlapadd_if_t::update);
    patchbay.connect(&wndpos.writeaccess,this,&overlapadd_if_t::update);
    patchbay.connect(&window.writeaccess,this,&overlapadd_if_t::update);
    patchbay.connect(&zerowindow.writeaccess,this,&overlapadd_if_t::update);
}

overlapadd_if_t::~overlapadd_if_t()
{
}

void overlapadd_if_t::prepare(mhaconfig_t& t)
{
    if( t.domain != MHA_WAVEFORM )
        throw MHA_ErrorMsg("overlapadd: waveform input is required.");
    t.fftlen = nfft.data;
    t.wndlen = nwnd.data;
    if( t.fragsize > t.wndlen )
        throw MHA_Error(__FILE__,__LINE__,
                        "overlapadd: The fragment size (%d) is greater than the window size (%d).",
                        t.fragsize, t.wndlen);
    if( t.fftlen < t.wndlen )
        throw MHA_Error(__FILE__,__LINE__,
                        "overlapadd: Invalid FFT length %d (less than window length %d).",
                        t.fftlen, t.wndlen );
    cf_in = tftype = t;
    t.domain = MHA_SPECTRUM;
    plugloader.prepare(t);
    if( t.domain != MHA_SPECTRUM )
        throw MHA_Error(__FILE__,__LINE__,"The processing plugin did not return spectral output.");
    /* prepare */
    t.domain = MHA_WAVEFORM;
    cf_out = t;
    update();
    poll_config();
}

void overlapadd_if_t::release()
{
    plugloader.release();
}

void overlapadd_if_t::update()
{
    if( (cf_in.fftlen > 0) &&
        (cf_in.wndlen > 0) &&
        (cf_in.fragsize > 0) && 
        (cf_in.channels > 0) ){
        push_config(new overlapadd_t(cf_in,cf_out,
                                     wndexp.data,
                                     wndpos.data,
                                     window,
                                     zerowindow,prescale.data,
                                     postscale.data));
    }
}

mha_wave_t* overlapadd_if_t::process(mha_wave_t* wave_in)
{
    poll_config();
    mha_spec_t* spec;
    spec = cfg->ola1(wave_in);
    plugloader.process(spec,&spec);
    return cfg->ola2(spec);
}

}

MHAPLUGIN_CALLBACKS(overlapadd,overlapadd::overlapadd_if_t,wave,wave)
MHAPLUGIN_DOCUMENTATION(overlapadd,"overlapadd",
            "\n"
            "This plugin transforms fragmented waveform data into short time\n"
            "Fourier transformed audio (STFT), and after processing by spectral\n"
            "processing plugins back to the time domain. This overlap-add mechanism\n"
            "is similar to that from Allen (1977): First, the waveform signal\n"
            "is windowed with a window function, e.g., a Hanning window. In each\n"
            "processing frame, the window is shifted by the fragment size of the\n"
            "input waveform. Missing parts of the signal are taken from the past.\n"
            "The windowed signal is padded with zeros on both sides up to the FFT\n"
            "length, to avoid aliasing when filters are applied in the frequency\n"
            "domain.  The impulse response of the applied filter can have the\n"
            "length of the zero padding; if the impulse response is longer, later\n"
            "parts of the impulse response will be mapped to the beginning of the\n"
            "fragment (temporal aliasing).  Linear phase filters (real gains in the\n"
            "frequency domain) produce symmetric impulse responses and therefore\n"
            "require symmetric zero padding.  The zero padded signal is now fast\n"
            "Fourier transformed. Parameters are FFT length $N$, window length $M$\n"
            "and the fragment size $P$.  Typical values for the window length are\n"
            "$M=2P$ or $M=4P$. The Hanning window used in the first step is $w_1(k)\n"
            "= \\frac12(1-\\cos(2\\pi k/M))$, the windowed signal is\n"
            "\\begin{equation} x_w(m,k) = w_1(k) \\cdot x(m\\cdot P + k),\n"
            "\\end{equation} with $k=0,\\dots,M-1$ and the fragment index $m$.\n"
            "\n"
            "After processing and inverse Fourier transformation, Hanning ramps are\n"
            "applied to the signal to avoid discontinuities in case of temporal\n"
            "aliasing, and thus reducing the artifacts.  The length of the Hanning\n"
            "ramps are a fraction $p$ of half the zero-padding length $(N-M)/2$.\n"
            "If $p=1$, the entire zero-padded parts are smoothed with Hanning\n"
            "ramps.  $p=0$ means, that no Hanning ramps are applied to the signal.\n"
            "This allows an exact reproduction in those cases, where the local\n"
            "impulse response of the filter (represented by all algorithms between\n"
            "FFT and inverse FFT) is shorter than the zero padding length.  The\n"
            "windowing in both stages of the overlap-add mechanism is plotted in\n"
            "Fig.\\ \\ref{fig:overlapadd} for $M=2P$ (50\% overlap).\n"
            "\n"
            "The total delay between input and output of a real-time system with\n"
            "fragment size $P$ and an overlap-add based linear-phase filter, is the\n"
            "window length plus half the zero-padding length, or $M+(N-M)/2$, plus\n"
            "an additional delay needed for the signal processing, and plus a delay\n"
            "generated by the AD/DA converters (e.g., anti-aliasing filter delay).\n"
            "In an offline system, the complete input signal is available in\n"
            "advance, and thus the delay of the overlap-add method is determined\n"
            "only by the relative shift between output and input signal, which is\n"
            "$(M+N)/2-P$ (equal to $N/2$ in case of 50\\% overlap, i.e.\\ $M=2P$).\n"
            "Contrary to a real-time system, the delay of an offline system depends\n"
            "on the amount of overlap.\n"
            "\n"
            "\\MHAfigure{Windowing in the overlap-add method with 50\\% overlap and zero-padding.\n"
            "In the upper panel, the windowed input signal before applying the FFT\n"
            "is schematically plotted.\n"
            "In the lower panel, the same time interval after inverse FFT is shown.\n"
            "The shaded segment is the fragment which is read from the input stream\n"
            "(upper panel) and written to the output stream (lower panel) in one\n"
            "processing cycle.\n"
            "The delay between input and output signal is the length of leading zeros\n"
            "plus the window length.\n"
            "}{overlapadd}\n"
            "\\MHAfigure{Windowing in the overlap-add method, as in Fig.\\ \\ref{fig:overlapadd},\n"
            "but with post-windowing and without zero-padding.\n"
            "In this setup, $W^\\alpha$ is applied before FFT and $W^{1-\\alpha}$ is used for post-windowing.\n"
            "The delay between input and output signal is the window length.\n"
            "}{overlapadd_nozero}\n"
            "\n"
    )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
