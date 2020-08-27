// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2008 2009 2010 2013 2014 2015 2018 HörTech gGmbH
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

#include "wave2spec.hh"

void wave2spec_t::calc_pre_wnd(MHASignal::waveform_t& dest,const MHASignal::waveform_t& src)
{
    if( dest.num_frames != npad1 + nwnd + npad2 )
        throw MHA_ErrorMsg("destination has wrong length");
    if( src.num_frames != nwnd )
        throw MHA_Error(__FILE__,__LINE__,"Source has wrong length: %u (window length is %u)",src.num_frames,nwnd);
    if( dest.num_channels != src.num_channels )
        throw MHA_ErrorMsg("channel count mismatch");
    unsigned int k;
    dest.copy_from_at(npad1,nwnd,src,0);
    for( k=0; k<nwnd; k++ )
        dest.scale_frame(npad1+k,window[k]);
    for( k=0; k<npad1; k++ )
        dest.assign_frame(k,0);
    for( k=npad1 + nwnd; k<dest.num_frames; k++ )
        dest.assign_frame(k,0);
}

wave2spec_t::wave2spec_t(unsigned int nfft,
                         unsigned int nwnd_,
                         unsigned int nwndshift_,
                         unsigned int nch,
                         mha_real_t wndpos,
                         const MHAWindow::base_t& window_,
                         algo_comm_t ac,
                         std::string algo)
    : MHA_AC::spectrum_t(ac,algo,nfft/2+1,nch,false),
      nwnd(nwnd_),
      nwndshift(nwndshift_),
      npad1((unsigned int)(floor(wndpos*(nfft-nwnd)))),
      npad2(nfft-nwnd-npad1),
      window(window_),
      calc_in(nfft,nch),
      in_buf(nwnd,nch),
      spec_in(nfft/2+1,nch),
      ac_wndshape_name(algo + "_wnd")
{
    const float rms_of_window = sqrtf(window.sumsqr() / nwnd);
    const float zeropadding_compensation = sqrtf(float(nfft) / nwnd);
    const float normalization_factor = zeropadding_compensation / rms_of_window;
    window *= normalization_factor;
    ft = mha_fft_new( nfft );
    if (window.num_channels != 1U)
        throw MHA_Error(__FILE__,__LINE__,
                        "The wave2spec:%s analysis window storage should have"
                        " only 1 channel, has %u",
                        algo.c_str(), window.num_channels);
}

void wave2spec_t::publish_ac_variables()
{
    // insert spectrum with name <configured_name>
    this->MHA_AC::spectrum_t::insert(); // would throw in case of error

    // insert window shape with name <configured_name>_wnd
    comm_var_t cv = {MHA_AC_MHAREAL, window.num_frames, 1U, window.buf};
    int err = ac.insert_var(ac.handle,ac_wndshape_name.c_str(), cv);
    if (err) // insert_var returns non-zero error code in case of error
        throw MHA_Error(__FILE__,__LINE__,
                        "Unable to insert wave2spec window into AC space as '%s':\n%s",
                        ac_wndshape_name.c_str(), ac.get_error(err));
}

wave2spec_t::~wave2spec_t()
{
    mha_fft_free( ft );
    ac.remove_ref(ac.handle, window.buf);
}

mha_spec_t* wave2spec_t::process(mha_wave_t* wave_in)
{
    if( nwndshift > nwnd )
        throw MHA_Error(__FILE__,__LINE__,
                        "The window shift (%u) is greater than the window size (%u).",
                        nwndshift,nwnd);
    in_buf.copy_from_at(in_buf.num_frames - nwndshift,nwndshift,*wave_in,0);
    calc_pre_wnd(calc_in,in_buf);
    in_buf.copy_from_at(0,in_buf.num_frames-nwndshift,in_buf,nwndshift);
    mha_fft_wave2spec( ft, &calc_in, &spec_in );
    copy(spec_in);
    publish_ac_variables(); // AC vars must be updated in every process callback
    return &spec_in;
}

wave2spec_if_t::wave2spec_if_t(const algo_comm_t& iac,const std::string&,const std::string& ialg)
    : MHAPlugin::plugin_t<wave2spec_t>(
        "Waveform to spectrum overlap add and FFT method.\n\n"
        "Audio data is collected up to wndlen, then windowed by\n"
        "the given window function, zero padded up to fftlen\n"
        "(symmetric zero padding or asymmetric zero padding possible),\n"
        "and fast-Fourier-transformed.\n"
        "The configuration variables are locked during processing.",iac),
      nfft("FFT lengths","512","[1,]"),
      nwnd("window length/samples","400","[1,]"),
      wndpos("window position\n(0 = beginning, 0.5 = symmetric zero padding, 1 = end)","0.5","[0,1]"),
      window_config("hanning"),
      strict_window_ratio("Disallow window sizes that are not a multiple of the"
                          " hop size (fragsize) by power of two.","yes"),
      return_wave("return input waveform signal, store spectrum only to AC","no"),
      algo(ialg)
{
    insert_item("fftlen",&nfft);
    insert_item("wndlen",&nwnd);
    insert_item("wndpos",&wndpos);
    window_config.insert_items(this);
    insert_item("strict_window_ratio", &strict_window_ratio);
    insert_item("return_wave",&return_wave);
    insert_member(zeropadding);
}

void wave2spec_if_t::prepare(mhaconfig_t& t)
{
    try{
        setlock(true);
        if( t.domain != MHA_WAVEFORM )
            throw MHA_ErrorMsg("wave2spec: waveform input is required.");
        if( return_wave.data )
            t.domain = MHA_WAVEFORM;
        else
            t.domain = MHA_SPECTRUM;
        t.fftlen = nfft.data;
        t.wndlen = nwnd.data;
        if (strict_window_ratio.data)
            if (t.wndlen==t.fragsize ||
                !MHAUtils::is_multiple_of_by_power_of_two(t.wndlen,t.fragsize))
                throw MHA_Error(__FILE__,__LINE__,
                                "The ratio between the hop size (\"fragsize\", %u) "
                                "and the window length (%u) must be a power of two.",
                                t.fragsize, t.wndlen);
        if( t.fragsize > t.wndlen )
            throw MHA_Error(__FILE__,__LINE__,
                            "wave2spec: The fragment size (%u) is greater than the window size (%u).",
                            t.fragsize, t.wndlen);
        if( t.fftlen < t.wndlen )
            throw MHA_Error(__FILE__,__LINE__,
                            "wave2spec: Invalid FFT length %u (less than window length %u).",
                            t.fftlen, t.wndlen );
        tftype = t;
        update();
        poll_config();
        cfg->publish_ac_variables();
        zeropadding.data.resize(2);
        zeropadding.data[0] = cfg->get_zeropadding(false);
        zeropadding.data[1] = cfg->get_zeropadding(true);
    }
    catch(MHA_Error& e){
        setlock(false);
    }
}

void wave2spec_if_t::release()
{
    setlock(false);
}

void wave2spec_if_t::update()
{
    if (is_prepared()) {
        if( (tftype.fftlen > 0) &&
            (tftype.wndlen > 0) &&
            (tftype.fragsize > 0) &&
            (tftype.channels > 0) )  {
            push_config(new wave2spec_t(tftype.fftlen,
                                        tftype.wndlen,
                                        tftype.fragsize,
                                        tftype.channels,
                                        wndpos.data,
                                        window_config.get_window_data(tftype.wndlen),
                                        ac,
                                        algo));
        }
        else
            throw MHA_ErrorMsg("unsuitable stft parameter with value < 1 "
                               "(fftlen, wndlen, fragsize, or channel)");
    }
}

void wave2spec_if_t::process(mha_wave_t* wave_in,mha_spec_t** sout)
{
    *sout = cfg->process(wave_in);
}

void wave2spec_if_t::process(mha_wave_t* wave_in,mha_wave_t** sout)
{
    cfg->process(wave_in);
    *sout = wave_in;
}

void wave2spec_if_t::setlock(bool b) {
    nfft.setlock(b);
    nwnd.setlock(b);
    wndpos.setlock(b);
    strict_window_ratio.setlock(b);
    return_wave.setlock(b);
    window_config.setlock(b);
}

MHAPLUGIN_CALLBACKS(wave2spec,wave2spec_if_t,wave,spec)
MHAPLUGIN_PROC_CALLBACK(wave2spec,wave2spec_if_t,wave,wave)

MHAPLUGIN_DOCUMENTATION\
(wave2spec,
 "signal-transformation overlap-add",
 "The plugin 'wave2spec' transforms time-domain "
 "waveform signal to short-time Fourier transform (STFT) signal.  "
 
 "It can be used as the analysis part of a complete "
 "overlap-add procedure.  "

 "Audio signal data is collected up to the length of the analysis "
 "window. "

 "The hop-size is equal to the audio block size that this "
 "plugin receives. "
 
 "Window size and FFT length are configurable through the "
 "configuration variables. "

 "\n\n"

 "Several pre-defined window shapes as well as user-defined "
 "window shapes are supported. "

 "In addition, a configurable exponent can be applied to the "
 "window samples. "

 "\n\n"
 
 "During processing, the input data samples are multiplied with the "
 "samples of the analysis window, zero "
 "padded to the FFT length, and Fourier transformed. "

 "For this reason, the short time fourier transform does not exactly "
 "correspond to the current input waveform block: the analysis "
 "window contains samples from the current as well as from previous invocation(s). "
 
 "The absolute window shift is identical to the fragment size, e.g.\\ to "
 "achieve a window shift of 50\\%, configure a fragment size of \\verb!wndlen!/2."

 "\n\n"

 "A copy of the output spectrum is stored in the AC space in a variable\n"
 "of same name as the configured plugin name.  To access the spectrum in "
 "AC space, \n"
 "the function \\verb!MHA_AC::get_var_spectrum()! can be used.\n"
 "See the \\mha{} developer manual or the\n"
 "header file {\\tt mha\\_algo\\_comm.h} for details. "

 "\n\n"
 
 "See section \\ref{plug:overlapadd} for a description of the "
 "overlap-add method that is also followed by this plugin. "

 "\n\n"

 "Example configurations for the \\verb!wave2spec! plugin are available in the "
 "short-time-fourier-transform examples directory, and in the matlab/octave "
 "tests exercising this plugin in the \\verb!mhatest! directory.  "
 "These test files are executed together with the other "
 "system-level tests when invoking \\texttt{make test}.  "
 "Please note that you need to have the signal processing package "
 "installed in order to sucessfully execute all tests for this "
 "plugin.\\footnote{In octave, the package can be installed with "
 "\\texttt{pkg install -forge control signal} from within octave.}"

 "\n\n"

 "The plugin performs the following scaling of the signal: "
 "The effect on the level of applying the analysis window to "
 "the input signal is compensated by "
 "dividing by the RMS (root mean square) of the window. "
 "To account for the zero-padding, which would reduce the RMS of the "
 "signal block\\footnote{The same sum of squared samples would be divided "
 "by fftlen instead of wndlen to compute the mean after zero-padding}, "
 "the signal is multiplied with $\\sqrt{\\mathrm{fftlen} / \\mathrm{wndlen}}$. "
 "Finally, the forward FFT operation in the MHA will apply a factor "
 "$1 / \\sqrt{\\mathrm{fftlen}}$ "
 "so that algorithms that compute signal level do not have to know the fftlen, "
 "but can simply sum squared magnitudes of the STFT bins to compute the RMS "
 "of the current block in Pascal. "
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
