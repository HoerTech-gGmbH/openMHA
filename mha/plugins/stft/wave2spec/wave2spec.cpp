// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2008 2009 2010 2014 2015 2018 HörTech gGmbH
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

#define MHAPLUGIN_OVERLOAD_OUTDOMAIN

#include <mha_plugin.hh>
#include <mha_signal.hh>
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
    MHAParser::bool_t return_wave;
    std::string algo;
};

void wave2spec_t::calc_pre_wnd(MHASignal::waveform_t& dest,const MHASignal::waveform_t& src)
{
    if( dest.num_frames != npad1 + nwnd + npad2 )
        throw MHA_ErrorMsg("destination has wrong length");
    if( src.num_frames != nwnd )
        throw MHA_Error(__FILE__,__LINE__,"Source has wrong length: %d (window length is %d)",src.num_frames,nwnd);
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
      spec_in(nfft/2+1,nch)
{
    window *= sqrt((float)nfft/window.sumsqr());
    ft = mha_fft_new( nfft );
    if (window.num_channels != 1U)
        throw MHA_Error(__FILE__,__LINE__,
                        "The wave2spec:%s analysis window storage should have"
                        " only 1 channel, has %u",
                        algo.c_str(), window.num_channels);
    comm_var_t cv = {MHA_AC_MHAREAL, window.num_frames, 1U, window.buf};
    int err = ac.insert_var(ac.handle,(algo + "_wnd").c_str(), cv);
    if (err)
        throw MHA_Error(__FILE__,__LINE__,
                        "Unable to insert wave2spec window into AC space as '%s':\n%s",
                        (algo + "_wnd").c_str(), ac.get_error(err));
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
                        "The window shift (%d) is greater than the window size (%d).",
                        __FILE__,__LINE__,nwndshift,nwnd);
    in_buf.copy_from_at(in_buf.num_frames - nwndshift,nwndshift,*wave_in,0);
    calc_pre_wnd(calc_in,in_buf);
    in_buf.copy_from_at(0,in_buf.num_frames-nwndshift,in_buf,nwndshift);
    mha_fft_wave2spec( ft, &calc_in, &spec_in );
    copy(spec_in);
    return &spec_in;
}

wave2spec_if_t::wave2spec_if_t(const algo_comm_t& iac,const std::string&,const std::string& ialg)
    : MHAPlugin::plugin_t<wave2spec_t>(
        "Waveform to spectrum overlap add and FFT method.\n\n"
        "Audio data is collected up to wndlen, then windowed with\n"
        "the given window function, zero padded up to fftlen\n"
        "(symmetric zero padding or asymmetric zero padding possible),\n"
        "and fast-Fourier-transformed (FFT).",iac),
      nfft("FFT lengths","512","[1,]"),
      nwnd("window length/samples","400","[1,]"),
      wndpos("window position\n(0 = beginning, 0.5 = symmetric zero padding, 1 = end)","0.5","[0,1]"),
      window_config("hanning"),
      return_wave("return input waveform signal, store spectrum only to AC","no"),
      algo(ialg)
{
    insert_item("fftlen",&nfft);
    insert_item("wndlen",&nwnd);
    insert_item("wndpos",&wndpos);
    window_config.insert_items(this);
    insert_item("return_wave",&return_wave);
    patchbay.connect(&wndpos.writeaccess,this,&wave2spec_if_t::update);
    patchbay.connect(&window_config.updated,this,&wave2spec_if_t::update);
}

void wave2spec_if_t::prepare(mhaconfig_t& t)
{
    if( t.domain != MHA_WAVEFORM )
        throw MHA_ErrorMsg("wave2spec: waveform input is required.");
    if( return_wave.data )
        t.domain = MHA_WAVEFORM;
    else
        t.domain = MHA_SPECTRUM;
    t.fftlen = nfft.data;
    t.wndlen = nwnd.data;
    if( t.fragsize > t.wndlen )
        throw MHA_Error(__FILE__,__LINE__,
                        "wave2spec: The fragment size (%d) is greater than the window size (%d).",
                        t.fragsize, t.wndlen);
    if( t.fftlen < t.wndlen )
        throw MHA_Error(__FILE__,__LINE__,
                        "wave2spec: Invalid FFT length %d (less than window length %d).",
                        t.fftlen, t.wndlen );
    tftype = t;
    update();
    poll_config();
    cfg->insert();
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
    poll_config();
    *sout = cfg->process(wave_in);
}

void wave2spec_if_t::process(mha_wave_t* wave_in,mha_wave_t** sout)
{
    poll_config();
    cfg->process(wave_in);
    *sout = wave_in;
}

MHAPLUGIN_CALLBACKS(wave2spec,wave2spec_if_t,wave,spec)
MHAPLUGIN_PROC_CALLBACK(wave2spec,wave2spec_if_t,wave,wave)

MHAPLUGIN_DOCUMENTATION\
(wave2spec, "overlapadd",
 "This plugin performs the domain transformation from time-domain "
 "waveform signal to short-time Fourier transform (STFT) signal in "
 "the spectral domain.  "
 
 "This plugin can be used as the analysis part of a complete "
 "overlap-add procedure.  "

 "Audio signal data is collected up to the length of the analysis "
 "window. "

 "The hop-size is equal to the audio block size that this "
 "plugin receives. "
 
 "Window size and FFT length are configurable through the "
 "configuration variables of this plugin. "

 "\n\n"

 "The shape of the analysis window is also configurable: "
 "Several pre-defined window shapes can be selected, and also "
 "a completely user-defined window shape. "

 "In addition, a configurable exponent can be applied to all samples "
 "of the window before it is used. "

 "\n\n"
 
 "During processing, the input data samples are multiplied with the "
 "samples of the analysis window, zero "
 "padded to the FFT length and Fourier transformed. "

 "Please note that "
 "usually the window shift is less than the window length. "

 "For this reason, the short time fourier transform does not exactly "
 "correspond to the current input waveform block: the analysis "
 "window contains samples from the current as well as from previous invocation(s). "
 
 "The absolute window shift is identical to the fragment size, e.g.\\ to "
 "achieve a window shift of 50\\%, configure a fragment size of \\verb!wndlen!/2."

 "\n\n"

 "A copy of the output spectrum is stored in the AC space in a variable\n"
 "of same name as the configured plugin name.  To access the spectrum in "
 "AC space from the code of self-developed other plugins, \n"
 "the function \\verb!MHA_AC::get_var_spectrum()! can be used.\n"
 "See the \\mha developer manual or the\n"
 "header file {\\tt mha\\_algo\\_comm.h} for details. "

 "\n\n"
 
 "Please refer to section \\ref{plug:overlapadd} for a description of the "
 "overlap-add method that is also followed by this plugin, "
 "\\verb!wave2spec!."

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
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
