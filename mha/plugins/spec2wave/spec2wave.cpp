// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2009 2010 2013 2014 2015 2017 2018 2019 HörTech gGmbH
// Copyright © 2020 2021 HörTech gGmbH
// Copyright © 2022 2024 Hörzentrum Oldenburg gGmbH
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

#include "mha_defs.h"
#include "mha_error.hh"
#include "mha_parser.hh"
#include "mha.hh"
#include "mha_plugin.hh"
#include "mha_signal.hh"
#include "mha_events.h"
#include <math.h>
#include "windowselector.h"

inline unsigned int max(unsigned int a, unsigned int b)
{
    if( a > b )
        return a;
    return b;
}

inline unsigned int min(unsigned int a, unsigned int b)
{
    if( a < b )
        return a;
    return b;
}

/// Class for storing two hanning ramps: a rising ramp a and a falling ramp b.
class hanning_ramps_t {
public:
    /// Initialize two arrays with hanning ramp factors.
    /// @param len_a
    ///       Length of the rising ramp a from 0 (inclusive) to 1 (exclusive).
    /// @param len_b
    ///       Length of the falling ramp b from 1 (inclusive) to 0 (exclusive).
    hanning_ramps_t(unsigned int len_a, unsigned int len_b);
    ~hanning_ramps_t();
    /// Apply the hanning ramps to a waveform.
    /// @param b The waveform to which the ramps are applied. ramp_a is applied
    ///          to the first len_a frames of b, ramp_b is applied to the last
    ///          len_b frames of b. Ramps are applied to all channels. If the
    ///          waveform has fewer frames than len_a or len_b, then the
    ///          respective ramp is truncated.
    ///          The audio samples stored by b are modified in place.
    void operator()(MHASignal::waveform_t& b);
private:
    /// Length of the rising ramp a.
    unsigned int len_a;
    /// Length of the falling ramp b.
    unsigned int len_b;
    /// Factors of the rising hanning ramp from 0 (inclusive) to 1 (exclusive).
    mha_real_t *ramp_a;
    /// Factors of the falling hanning ramp from 1 (inclusive) to 0(exclusive).
    mha_real_t *ramp_b;
};

/// Runtime config class for plugin spec2wave, the counterpart of wave2spec.
class spec2wave_t {
public:
    /// Constructor.
    /// @param nfft_ FFT length.
    /// @param nwnd_ Overlap-add analysis window length.
    /// @param nwndshift_ Overlap-add hop size. Needed to compute scaling.
    /// @param nch Number of audio channels.
    /// @param ramplen A user-configurable factor which is applied to the
    ///                length of the zero paddings in samples before and
    ///                after the analysis window. Rising and falling hanning
    ///                ramps are computed for the resulting ramp lengths in
    ///                member ramps.
    /// @param postwin Post windowing function. This window is applied in
    ///                addition to the hanning ramps. It is applied to the
    ///                whole output waveform computed by the inverse FFT before
    ///                the signal is overlap-added to the previous output.
    spec2wave_t(unsigned nfft_, unsigned nwnd_, unsigned nwndshift_,
                unsigned nch, mha_real_t ramplen,
                const MHAWindow::base_t& postwin);
    ~spec2wave_t();
    mha_wave_t* process(mha_spec_t*);
private:
    mha_fft_t ft; //!< FFT class
    unsigned int npad1; //!< length of zero padding before window
    unsigned int npad2; //!< length of zero padding after window
    hanning_ramps_t ramps;
    MHASignal::waveform_t calc_out;
    MHASignal::waveform_t out_buf;
    MHASignal::waveform_t write_buf;  
    mha_real_t sc;
    unsigned int nfft;
    unsigned int nwndshift;
    MHAWindow::base_t postwindow;
};

class spec2wave_if_t : public MHAPlugin::plugin_t<spec2wave_t> {
public:
    spec2wave_if_t(MHA_AC::algo_comm_t & iac,
                   const std::string & configured_name);
    void prepare(mhaconfig_t&);
    void release();
    mha_wave_t* process(mha_spec_t*);
private:
    void update();
    void setlock(bool b);
    MHAParser::float_t ramplen;
    windowselector_t window_config;
};

/**********************************************************************/

hanning_ramps_t::hanning_ramps_t(unsigned int la,unsigned int lb)
    : len_a(la),
      len_b(lb),
      ramp_a(new mha_real_t[max(1,len_a)]),
      ramp_b(new mha_real_t[max(1,len_b)])
{
    unsigned int k;
    // Hanning ramp of length len_a which runs from 0 inclusive to 1 exclusive.
    for(k=0; k<len_a; k++ )
        ramp_a[k] = 0.5 * (1 - cos( M_PI * k / len_a ) );
    // Hanning ramp of length len_b which runs from 1 inclusive to 0 exclusive.
    for(k=0; k<len_b; k++ )
        ramp_b[k] = 0.5 * (1 + cos( M_PI * k / len_b ) );
}

hanning_ramps_t::~hanning_ramps_t(void)
{
    delete [] ramp_a;
    delete [] ramp_b;
}

void hanning_ramps_t::operator()(MHASignal::waveform_t& b)
{
    unsigned int k;
    for(k=0; k<min(len_a,b.num_frames);k++)
        b.scale_frame(k,ramp_a[k]);
    // index of first frame of b to which ramp_b is applied
    unsigned int kb = max(b.num_frames,len_b)-len_b;
    for(k=kb;k<b.num_frames;k++)
        b.scale_frame(k,ramp_b[k-kb]);
}

/**********************************************************************/

spec2wave_t::spec2wave_t(unsigned int nfft_,unsigned int nwnd_,unsigned int nwndshift_,unsigned int nch,mha_real_t ramplen,const MHAWindow::base_t& postwin)
    : npad1((unsigned int)(floor(0.5*(nfft_ - nwnd_)))),
      npad2(nfft_ - nwnd_ - npad1),
      ramps((unsigned int)(ramplen*npad1),(unsigned int)(ramplen*npad2)),
      calc_out(nfft_,nch),
      out_buf(nfft_,nch),
      write_buf(nwndshift_,nch),
      sc(2.0*(mha_real_t)nwndshift_/(mha_real_t)nwnd_*sqrt(0.375*nwnd_/nfft_)),
      nfft(nfft_),
      nwndshift(nwndshift_),
      postwindow(postwin)
{
    postwindow *= sc;
    ft = mha_fft_new( nfft );
}

spec2wave_t::~spec2wave_t()
{
    mha_fft_free( ft );
}

mha_wave_t* spec2wave_t::process(mha_spec_t* spec_in)
{
    unsigned int k;
    // Perform inverse FFT. Store waveform in calc_out.
    mha_fft_spec2wave( ft, spec_in, &calc_out );
    // Apply postwindowing to calc_out. Default postwindow is rect (no effect),
    postwindow(calc_out); // but postwindow also includes the needed scaling.

    // Apply hanning ramps to calc_out, to the originally zero-padded regions.
    // Signal can leak out to these regions by signal modification in the
    // frequency domain. Applying hanning ramps reduces artifacts from temporal
    // aliasing, but can not eliminate them.
    ramps(calc_out);
    // Shift the historic output buffer by nwndshift frames to the left.
    out_buf.copy_from_at(0,nfft-nwndshift,out_buf,nwndshift);
    // Clear the rightmost nwndshift frames of out_buf.
    for(k=nfft-nwndshift; k<nfft; k++)
        out_buf.assign_frame(k,0);
    // Overlap the current output with the historic output.
    out_buf += calc_out;
    // Write the finished samples to the waveform output buffer of the plugin.
    write_buf.copy_from_at(0,nwndshift,out_buf,0);
    // Return the output buffer.
    return &write_buf;
}

spec2wave_if_t::spec2wave_if_t(MHA_AC::algo_comm_t & iac, const std::string &)
    : MHAPlugin::plugin_t<spec2wave_t>("spectrum to waveform iFFT plugin\n"
                                       "Performs inverse FFT, "
                                       "hanning ramping in zero-padding regions,\n"
                                       "postwindowing, overlap-add, and "
                                       "normalization.\n"
                                       "Note that normalization only works"
                                       " for mod(wndlen,fragsize)=0.\n"
                                       "Also note that hanning ramps only"
                                       " work for wndpos=0.5.\n"
                                       "Always set ramplen=0 here if wndpos!="
                                       "0.5 in the corresponding wave2spec.",
                                       iac),
      ramplen("Relative length of post windowing hanning ramps"
              " (for centered analysis window).\n"
              "0: no hanning ramping is applied.\n"
              "1: hanning ramping is applied to the full zero-paddings before"
              " and after/n"
              "   the analysis window. Assumes symmetric zero-padding.\n"
              "values between 0 and 1 shorten the ramps and move them away"
              " from the center.",
              "1",
              "[0,1]"),
      window_config("rect")
{
    insert_item("ramplen",&ramplen);
    window_config.insert_items(this);
}

void spec2wave_if_t::prepare(mhaconfig_t& t)
{
    try{
        setlock(true);
        if( t.domain != MHA_SPECTRUM )
            throw MHA_ErrorMsg("spec2wave: Spectral input is required.");
        t.domain = MHA_WAVEFORM;
        tftype = t;
        update();
        poll_config();
    }
    catch(MHA_Error& e){
        setlock(false);
        throw;
    }
}

void spec2wave_if_t::release(){
    setlock(false);
}

mha_wave_t* spec2wave_if_t::process(mha_spec_t* spec_in)
{
    return cfg->process(spec_in);
}

void spec2wave_if_t::update()
{
    if (is_prepared()) {
        if( tftype.fftlen ) {
            const MHAWindow::base_t& postwindow =
                window_config.get_window_data(tftype.fftlen);
            push_config(new spec2wave_t(tftype.fftlen,
                                        tftype.wndlen,
                                        tftype.fragsize,
                                        tftype.channels,
                                        ramplen.data,
                                        postwindow));
        } else {
            throw MHA_ErrorMsg("unsuitable fftlen == 0");
        }
    }
}

void spec2wave_if_t::setlock(bool b){
    window_config.setlock(b);
    ramplen.setlock(b);
}
MHAPLUGIN_CALLBACKS(spec2wave,spec2wave_if_t,spec,wave)
MHAPLUGIN_DOCUMENTATION\
(spec2wave,
 "signal-transformation overlap-add",
 "This plugin calculates the inverse FFT and overlap add\n"
 "resynthesis. This plugin is the counterpart of the wave2spec plugin.\n"
 "FFT length, window length, and hop size are taken from signal metadata\n"
 "published by the upstream wave2spec plugin.\n"
 "\n"
 "This plugin first computes the inverse Fourier transform, then the\n"
 "hanning window ramps are applied to the zero-padded regions before and\n"
 "after the analysis window, assuming a centered position of the analysis window in\n"
 "the FFT buffer. If hanning ramping of these regions is not desired, the\n"
 "hanning ramping can be deactivated by setting ramplen to 0.\n"
 "\n"
 "Setting ramplen to values between 0.0 and 1.0 shortens the duration of\n"
 "the hanning ramps while moving them away from the center.\n"
 "\n"
 "Additionally, a postwindowing function can be configured which is\n"
 "applied to the whole output waveform before overlap-adding. The\n"
 "default postwindow shape is rectangular, which means no effect.\n"
 "Note that the postwindow settings have no effect on the hanning-ramping\n"
 "described above, which is only controlled by the ramplen setting.\n"
 "\n"
 "Finally, overlap-add is performed to produce the next output waveform\n"
 "chunk."
)

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
