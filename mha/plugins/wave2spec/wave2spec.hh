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

/** Runtime configuration class for plugin wave2spec.
 * Manages window shift, windowing, zero-padding, and FFT.
 * Inserts current window shape and current STFT spectrum
 * into AC space. */
class wave2spec_t : public MHA_AC::spectrum_t {
public:
    /** Constructor computes window and zeropadding, allocates storage and
     * FFT plan.
     * @param nfft FFT length
     * @param nwnd_ window length in samples
     * @param nwndshift_ window shift (hop size) in samples
     * @param nch number of audio channels
     * @param wndpos for cases nfft > nwnd_, where to place the window inside
     *               the FFT buffer: 0 = at start, 1 = at end, 0.5 = centered.
     *               Position is rounded to full samples and determines
     *               zero-padding
     * @param window Analysis window shape
     * @param ac algorithm communication storage accessor 
     * @param algo configured name of this plugin, used to name the AC 
     *             variables published by wave2spec */
    wave2spec_t(unsigned int nfft,
                unsigned int nwnd_,
                unsigned int nwndshift_,
                unsigned int nch,
                mha_real_t wndpos,
                const MHAWindow::base_t& window,
                algo_comm_t ac,
                std::string algo);
    /** Insert two AC variables into AC space: 
     * - <configured_name>: Contains the current STFT spectrum.
     * - <configured_name>_wnd: Contains the window shape as individual weights.
     */
    void publish_ac_variables();
    /** Perform signal shift, windowing, zero-padding and FFT.
     * @param wave_in latest block of audio signal (hop size samples per channel)
     * @return pointer to current STFT spectrum. Storage is managed by this
     *         object. Downstream plugins may modify the signal in place. */
    mha_spec_t* process(mha_wave_t* wave_in);
    /** Destructor removes AC variables from AC space and deallocates memory */
    ~wave2spec_t();

    /** Getter method to read zeropadding computed for the STFT configuration
     * parameters.  Result is only valid after prepare() has been called.
     * @return Computed zeropadding before or after the analysis window
     *         in number of samples.
     * @param after When false, return length of zeropadding before
     *              the analysis window.  When true, return length of
     *              zeropadding in samples after the analysis window.
     */
    unsigned get_zeropadding(bool after) const {return after ? npad2 : npad1;}
private:
    /** Applies analysis window weights to current input signal and writes
     * windowed signal to correct place in FFT buffer.  Ensures zero-padding
     * regions contain only zeros.  To be invoked before applying FFT.
     * @param[out] dest waveform buffer with FFT length audio samples, 
     *                  completely overwritten by this method
     * @param[in] src waveform buffer with window length audio samples, these
     *                samples are written to dest after window shape has been
     *                applied to the individual samples. */
    void calc_pre_wnd(MHASignal::waveform_t&dest,const MHASignal::waveform_t&src);
    /** window length */
    unsigned int nwnd;
    /** window shift or hop size */
    unsigned int nwndshift;
    /** FFT instance used for transformation */
    mha_fft_t ft;
    unsigned int npad1; //!< length of zero padding before window
    unsigned int npad2; //!< length of zero padding after window
    /** Analysis window */
    MHAWindow::base_t window;
    /** waveform buffer with FFT length samples per channel */
    MHASignal::waveform_t calc_in;
    /** waveform buffer with window length samples per channel */
    MHASignal::waveform_t in_buf;
    /** spectrum buffer containing only the positive frequency bins */
    MHASignal::spectrum_t spec_in;
    /** name of window shape AC variable */
    std::string ac_wndshape_name;
};

/** Plugin wave2spec interface class, uses wave2spec_t as runtime
 * configuration */
class wave2spec_if_t : public MHAPlugin::plugin_t<wave2spec_t> {
public:
    /** Constructor of wave2spec plugin, sets up configuration variables 
     * and callbacks.
     * @param iac algorithm communication storage accessor
     * @param ialg configured name of this plugin, used to name the AC 
     *             variables published by wave2spec */
    wave2spec_if_t(const algo_comm_t&iac,const std::string&,const std::string&ialg);
    /** prepare for signal processing
     * @param[in,out] t signal dimenstions, modified by prepare as determined by
     *                  the STFT configuration */
    void prepare(mhaconfig_t&t);
    /** Unprepare signal processing
     */
    void release();
    /** processing callback used for domain transformation
     * @param wave_in latest block of audio signal (hop size samples per channel)
     * @param sout output spectrum pointer */
    void process(mha_wave_t* wave_in,mha_spec_t** sout);
    /** processing callback used if output of original waveform is requested.
     * The STFT spectrum is computed and can only be accessed by downstream
     * plugins through the AC variable published by this plugin.
     * @param wave_in latest block of audio signal (hop size samples per channel)
     * @param sout output waveform pointer (FFT length samples per channel) */
    void process(mha_wave_t* wave_in,mha_wave_t** sout);
private:
    /** Create a new runtime configuration from configuration parameters
     * when the plugin is prepared, or when the window position or other
     * window parameters change.
     * @throw MHA_Error if the configuration change is not compatible with the
     *                  current input and FFT length constraints. */
    void update();
    /** FFT length selector */
    MHAParser::int_t nfft;
    /** Window length selector */
    MHAParser::int_t nwnd;
    /** Window position selector */
    MHAParser::float_t wndpos;
    windowselector_t window_config;
    /** Switch to disallow window sizes that are not a
        multiple of the fragsize a by power of two. */
    MHAParser::bool_t strict_window_ratio;
    /** Switch to select return domain */
    MHAParser::bool_t return_wave;
    /** configured name this plugin, used to name the AC variables */
    std::string algo;
    MHAParser::vfloat_mon_t zeropadding = {
        "Zeropadding in samples before and after the analysis window"
    };
    /** Lock/Unlock all configuration variables
     * @param b Desired lock state
     */
    void setlock(bool b);
};

#endif // WAVE2SPEC_HH

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
