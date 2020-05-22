// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2007 2008 2009 2010 2013 2014 2015 2017 2018 HörTech gGmbH
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

#include "overlapadd.hh"

namespace overlapadd {

overlapadd_t::overlapadd_t(mhaconfig_t spar_in,
                           mhaconfig_t spar_out,
                           float wexp,
                           float wndpos,
                           const MHAParser::window_t& window,
                           const MHAParser::window_t& zerowindow,
                           float& prescale_fac,float& postscale_fac)
    : fft(mha_fft_new(spar_in.fftlen)),
      prewnd((spar_in.wndlen)),
      postwnd((spar_in.fftlen)),
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
        throw MHA_Error(__FILE__,__LINE__,"overlap add sub-plugins are not allowed to change fragment size from %u to %u.",
                        spar_in.fragsize,spar_out.fragsize);
    if( spar_in.wndlen != spar_out.wndlen )
        throw MHA_Error(__FILE__,__LINE__,"overlap add sub-plugins are not allowed to change window length from %u to %u.",
                        spar_in.wndlen,spar_out.wndlen);
    if( spar_in.fftlen != spar_out.fftlen )
        throw MHA_Error(__FILE__,__LINE__,"overlap add sub-plugins are not allowed to change FFT length from %u to %u.",
                        spar_in.fftlen,spar_out.fftlen);

    if (wexp != 1.0f && wexp != 0.0f && spar_in.fftlen != spar_in.wndlen)
        throw MHA_Error(__FILE__,__LINE__,
                        "window exponent (%f) other than exactly 1.0 or 0.0"
                        " only makes sense when window length (%u) is equal to"
                        " fft length (%u)",
                        wexp, spar_in.wndlen, spar_in.fftlen);

    if (wexp == 0.0f) {
        // exponent is zero, fill window with all ones
        prewnd.assign(1.0f);
    } else {
        prewnd.copy(window.get_window(spar_in.wndlen));
        // apply the exponent
        prewnd ^= wexp;
     }

    if (wexp == 1.0f) {
        // post window exponent 1-wexp is zero, fill post window with all ones
        postwnd.assign(1.0f);
    } else {
        postwnd.copy(window.get_window(spar_in.fftlen));
        // apply the remainder of the exponent
        postwnd ^= 1.0-wexp;
    }

    if( n_pad1 ){
        MHAWindow::base_t pad1wnd(zerowindow.get_window(n_pad1,-1,0));
        pad1wnd.ramp_begin(postwnd);
    }
    if( n_pad2 ){
        MHAWindow::base_t pad2wnd(zerowindow.get_window(n_pad2,0,1));
        pad2wnd.ramp_end(postwnd);
    }
    const float rms_of_window = sqrtf(prewnd.sumsqr() / spar_in.wndlen);
    const float zeropadding_compensation = sqrtf(float(spar_in.fftlen)
                                                 / spar_in.wndlen);
    const float scale_fac = zeropadding_compensation / rms_of_window;
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

void overlapadd_t::wave2spec_hop_forward(mha_wave_t *s)
{
    timeshift(wave_in1,-s->num_frames);
    assign(range(wave_in1,wave_in1.num_frames-s->num_frames,s->num_frames),*s);
}
void overlapadd_t::wave2spec_apply_window(void)
{
    assign(range(wave_out1,0,n_pad1),0.0f);
    assign(range(wave_out1,n_pad1,wave_in1.num_frames),wave_in1);
    assign(range(wave_out1,n_pad1+wave_in1.num_frames,n_pad2),0.0f);
    mha_wave_t windowed_in = range(wave_out1,n_pad1,wave_in1.num_frames);
    prewnd(windowed_in);
}
mha_spec_t * overlapadd_t::wave2spec_compute_fft(void)
{
    mha_fft_wave2spec(fft,&wave_out1,&spec_in);
    return &spec_in;
}

mha_spec_t* overlapadd_t::wave2spec(mha_wave_t* s)
{
    wave2spec_hop_forward(s);
    wave2spec_apply_window();
    return wave2spec_compute_fft();
}

mha_wave_t* overlapadd_t::spec2wave(mha_spec_t* s)
{
    mha_fft_spec2wave(fft,s,&calc_out);
    postwnd(calc_out);
    timeshift(out_buf,-write_buf.num_frames);
    out_buf += calc_out;
    assign(write_buf,range(out_buf,0,write_buf.num_frames));
    return &write_buf;
}

overlapadd_if_t::overlapadd_if_t(const algo_comm_t& iac,const std::string&,const std::string& ialg)
    : MHAPlugin::plugin_t<overlapadd_t>(
        "Waveform to spectrum overlap add and FFT method.\n\n"
        "Audio data is collected up to wndlen, then windowed by\n"
        "the given window function, zero padded up to fftlength\n"
        "(symmetric zero padding or asymmetric zero padding possible),\n"
        "and Fast-Fourier-transformed.\n"
        "The configuration variables are locked in the prepare call and must be unlocked by release"
        " before a change is possible.",iac),
      nfft("FFT length","512","[1,]"),
      nwnd("window length/samples","400","[1,]"),
      wndpos("window position\n(0 = beginning, 0.5 = symmetric zero padding, 1 = end)","0.5","[0,1]"),
      window("window type"),
      wndexp("window exponent to be applied to all elements of window function","1"),
      zerowindow("zero padding post window type"),
      strict_window_ratio("Disallow window sizes that are not a multiple of the"
                          " hop size (fragsize) by a power of two.","yes"),
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
    insert_item("strict_window_ratio",&strict_window_ratio);
    insert_member(prescale);
    insert_member(postscale);
}

void overlapadd_if_t::prepare(mhaconfig_t& t)
{
    try{
        setlock(true);
        if( t.domain != MHA_WAVEFORM )
            throw MHA_ErrorMsg("overlapadd: waveform input is required.");
        t.fftlen = nfft.data;
        t.wndlen = nwnd.data;
        if (strict_window_ratio.data)
            if (t.wndlen==t.fragsize or
                !MHAUtils::is_multiple_of_by_power_of_two(t.wndlen,t.fragsize))
                throw MHA_Error(__FILE__,__LINE__,
                                "The ratio between the hop size (\"fragsize\", %u) "
                                "and the window length (%u) must be a power of two.",
                                t.fragsize, t.wndlen);
        if( t.fragsize > t.wndlen )
            throw MHA_Error(__FILE__,__LINE__,
                            "overlapadd: The hop size (\"fragsize\", %u) is greater than the window length (%u).",
                            t.fragsize, t.wndlen);
        if( t.fftlen < t.wndlen )
            throw MHA_Error(__FILE__,__LINE__,
                            "overlapadd: Invalid FFT length %u (less than window length %u).",
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
    }
    catch(MHA_Error& e){
        setlock(false);
    }
}

void overlapadd_if_t::release()
{
    setlock(false);
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
    spec = cfg->wave2spec(wave_in);
    plugloader.process(spec,&spec);
    return cfg->spec2wave(spec);
}

}

MHAPLUGIN_CALLBACKS(overlapadd,overlapadd::overlapadd_if_t,wave,wave)
MHAPLUGIN_DOCUMENTATION\
(overlapadd,
 "plugin-arrangement signal-transformation overlap-add",
 "\n"
 "The plugin 'overlapadd' transforms fragmented waveform audio data into short\n"
 "time Fourier transformed (STFT) audio data.\n"
 "Both the forward and the inverse transform are performed.\n"
 "Another plugin which processes the STFT spectra must be loaded by\n"
 "setting \\texttt{plugin\\_name}.\n"
 "\n"
 "The overlap-add mechanism\n"
 "is similar to that from Allen (1977): First the waveform signal\n"
 "is windowed by a window function. The default window shape is the\n"
 "Hanning window, but\n"
 "other pre-defined and user-defined window shapes can be selected. In each\n"
 "processing frame, the window is shifted by the fragment size of the\n"
 "input waveform. Missing parts of the signal are taken from the past.\n"
 "The windowed signal is padded with zeros on both sides up to the FFT\n"
 "length to avoid aliasing when filters are applied in the frequency\n"
 "domain.\\footnote{The impulse response of the applied filter can have the\n"
 "length of the zero padding; if the impulse response is longer, later\n"
 "parts of the impulse response will be mapped to the beginning of the\n"
 "fragment (temporal aliasing).  Linear phase filters (real gains in the\n"
 "frequency domain) produce symmetric impulse responses and therefore\n"
 "require symmetric zero padding.}  The zero padded signal is then fast\n"
 "Fourier transformed. Parameters are FFT length $N$, window length $M$\n"
 "and the fragment size $P$.  Typical values for the window length are\n"
 "$M=2P$ or $M=4P$. The default Hanning window is $w_1(k)\n"
 "= \\frac12(1-\\cos(2\\pi k/M))$, the windowed signal is\n"
 "\\begin{equation} x_w(m,k) = w_1(k) \\cdot x(m\\cdot P + k),\n"
 "\\end{equation} with $k=0,\\dots,M-1$ and the fragment index $m$.\n"
 "\n"
 "After processing and inverse Fourier transformation, ramps can be \n"
 "applied to the signal to avoid discontinuities in case of temporal\n"
 "aliasing, and thus reducing the artifacts.  These ramps\n"
 "are a applied to the zero-padding regions.  The shape of\n"
 "the ramps is determined by the window shape {\\em zerownd.type}.\n"
 "Common choices are Hanning ramps or rectangular ramps (i.e. no\n"
 "ramps, the default).\n"
 "This allows an exact reproduction in those cases where the local\n"
 "impulse response of the filter (represented by all algorithms between\n"
 "FFT and inverse FFT) is shorter than the zero padding length.  The\n"
 "windowing in both stages of the overlap-add mechanism is shown in\n"
 "Fig.\\ \\ref{fig:overlapadd} for $M=2P$ (50\\% overlap).\n"
 "\n"
 "The total delay between input and output of a real-time system with\n"
 "fragment size $P$ and an overlap-add based linear-phase filter is the\n"
 "window length plus half the zero-padding length, or $M+(N-M)/2$, plus\n"
 "an additional delay needed for the signal processing plus a delay\n"
 "generated by the AD/DA converters (e.g., anti-aliasing filter delay).\n"
 "In an offline system, the complete input signal is available in\n"
 "advance, and thus the delay of the overlap-add method is determined\n"
 "only by the relative shift between output and input signal, which is\n"
 "$(M+N)/2-P$ (equal to $N/2$ in case of 50\\% overlap, i.e.\\ $M=2P$).\n"
 "Contrary to a real-time system, the delay of an offline system depends\n"
 "on the amount of overlap.\n"
 "\n"
 "\\MHAfigure{Windowing in the overlap-add method with 50\\% overlap and"
 " zero-padding.\n"
 "In the upper panel, the windowed input signal before applying the FFT\n"
 "is schematically plotted.\n"
 "In the lower panel, the same time interval after inverse FFT is shown.\n"
 "The shaded segment is the fragment which is read from the input stream\n"
 "(upper panel) and written to the output stream (lower panel) in one\n"
 "processing cycle.\n"
 "The delay between input and output signal is the length of leading zeros\n"
 "plus the window length.\n"
 "}{overlapadd}\n"
 "\\MHAfigure{Windowing in the overlap-add method, as in"
 " Fig.\\ \\ref{fig:overlapadd},\n"
 "but with post-windowing and without zero-padding.\n"
 "In this setup, $W^\\alpha$ is applied before FFT and $W^{1-\\alpha}$"
 " is used for post-windowing.\n"
 "The delay between input and output signal is the window length.\n"
 "}{overlapadd_nozero}\n"
 "\n"
 "The spectral signal produced by this plugin is subject to the\n"
 "following scaling:\n"
 "The attenuation effect of applying the analysis window "
 "is compensated by "
 "dividing by the RMS (root mean square) of the window. "
 "To account for the zero-padding, which would reduce the RMS of the "
 "signal block\\footnote{The same sum of squared samples would be divided "
 "by fftlen instead of wndlen to compute the mean after zero-padding}, "
 "the signal is multiplied with $\\sqrt{\\mathrm{fftlen} / \\mathrm{wndlen}}$. "
 "Finally, the forward FFT operation in the MHA will apply a factor "
 "$1 / \\sqrt{\\mathrm{fftlen}}$ "
 "so that the sum of squared magnitudes of the spectral "
 "bins produces the correct level in Pascal. "
 "\n\n"
 "The purpose of the scaling described in the previous paragraph is to enable"
 " spectral algorithms to determine the physical level of the signal in the "
 "current STFT block without having to apply correction factors for window "
 "shape, zero-padding, overlap, FFT length, etc."
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
