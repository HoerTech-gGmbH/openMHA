// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2009 2010 2013 2014 2015 2018 2020 HörTech gGmbH
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

/**
  \file   gtfb_simd.cpp
  \brief  Gammatone Filterbank Analyzer Plugin using SIMD

  A single-instruction-multiple-data (SIMD) implementation of a
  gammatone filterbank (GTFB)

  Not all functions in this file are actually used.  Read the functions
  in this file as a path to convert an algorithm, here complex-valued
  gammatone filtering as introduced in Hohmann 2002, from a
  single-instruction-single-data (SISD) implementation that uses complex
  arithmetic operations to a SIMD implementation of the same, splitting
  the complex arithmetic operations into their defining real operations,
  i.e (a+b).real==a.real+b.real, (a+b).imag==a.imag+b.imag,
  (a*b).real==a.real*b.real-a.imag*b.imag,
  (a*b).imag==a.real*b.imag+a.imag*b.real.
*/
/*
   State of implementation
   - The simd code is implemented but untested
   - ?vectorize rinput filling?
   - performance testing against gtfb_analyzer and mex function (the latter is 
     optimized with loop unrolling)
*/


#include "mha_plugin.hh"
#include "mha_signal.hh"
#include "mha_parser.hh"
#include "mha_defs.h"
#include <math.h>
#include <time.h>
#include "mha_events.h"

// Setting these bits in the MXCSR register avoids problems with denormals
#define MXCSR_DAZ (1 << 6)      /* Enable denormals are zero mode */
#define MXCSR_FTZ (1 << 15)     /* Enable flush to zero mode */

/** Filters one sample per band, using SISD operations and the mha_complex
 * operations.  To process more than one sample, the function must be
 * called repeatedly in correct order (from oldest sample to newest sample),
 * and the calling function must preserve the filter state (parameter states).
 *
 * This function is not actually used in this plugin, but can be used for
 * testing.  It implements the Hohmann 2002 filtering in the most readable
 * form, and is translated towards a SIMD implementation in the following
 * functions.
 * @param bands
 *   Number of total bands to compute (i.e. input_channels * num_frequencies)
 * @param order
 *   Gammatone filter order
 * @param inputs
 *   Pointer to array of complex input samples, only 1 sample per band
 * @param outputs
 *   Pointer to array with space for output samples, 1 complex sample per band
 * @param coefficients
 *   Pointer to array of recursive filter coefficient, 1 coefficient per band.
 *   The same coefficient is reused for all filter orders.
 * @param states
 *   Pointer to array of complex filter states.  Array size is bands*order.
 *   Initialize all elements with zeros before filtering the first sample.
 *   Filter state values will be modified by the function.  For filtering the
 *   next sample, this function needs the filter state array from filtering the
 *   previous sample again, unmodified.
 *   The filter state of band b, order o can be found at index [b+o*bands] */
void filter_sisd_complex(const unsigned bands, const unsigned order,
                         const mha_complex_t * inputs, mha_complex_t * outputs,
                         const mha_complex_t * coefficients,
                         mha_complex_t * states)
{
  /* pseudo code for vector operation
  for bandv = band % 4
      cplusv @= cinputv = inputs[bandv]
      ccoeffv = coefficients[bandv]
      for stage = 0...order
          cstatev = states[bandv, stage]
          cstatev *= coeffv // could be eliminated by table
          cstatev += @cplusv
          cplusv @= statev
      outputs[bandv] = cplusv
  */
  for (unsigned band = 0; band < bands; ++band) {
    const mha_complex_t * cplus = &inputs[band];
    const mha_complex_t & ccoeff = coefficients[band];
    for (unsigned stage = 0; stage < order; ++stage) {
      mha_complex_t & cstate = states[band + stage*bands];
      cstate *= ccoeff;
      cstate += *cplus;
      cplus = &cstate;
    }
    outputs[band] = *cplus;
  }
}

/** Filters one sample per band, using SISD operations and real
 * operations (operating on real and imaginary part as necessary).
 * To process more than one sample, the function must be
 * called repeatedly in correct order (from oldest sample to newest sample),
 * and the calling function must preserve the filter state (parameter states).
 *
 * This function is not actually used in this plugin, but can be used for
 * testing.  It reimplements filter_sisd_complex, but expands the complex
 * operations into real arithmetics.
 * @param bands
 *   Number of total bands to compute (i.e. input_channels * num_frequencies)
 * @param order
 *   Gammatone filter order
 * @param inputs
 *   Pointer to array of complex input samples, only 1 sample per band
 * @param outputs
 *   Pointer to array with space for output samples, 1 complex sample per band
 * @param coefficients
 *   Pointer to array of recursive filter coefficient, 1 coefficient per band.
 *   The same coefficient is reused for all filter orders.
 * @param states
 *   Pointer to array of complex filter states.  Array size is bands*order.
 *   Initialize all elements with zeros before filtering the first sample.
 *   Filter state values will be modified by the function.  For filtering the
 *   next sample, this function needs the filter state array from filtering the
 *   previous sample again, unmodified.
 *   The filter state of band b, order o can be found at index [b+o*bands] */
inline
void filter_sisd_real(const unsigned bands, const unsigned order,
                      const mha_complex_t * inputs, mha_complex_t * outputs,
                      const mha_complex_t * coefficients,
                      mha_complex_t * states)
{
  // Intermediate storage for the real part of the state during multiplication
  mha_real_t tmp;
  for (unsigned band = 0; band < bands; ++band) {
    const mha_real_t * rplus = &inputs[band].re;
    const mha_real_t * iplus = &inputs[band].im;
    const mha_real_t & rcoeff = coefficients[band].re;
    const mha_real_t & icoeff = coefficients[band].im;
    for (unsigned stage = 0; stage < order; ++stage) {
      mha_real_t & rstate = states[band + stage*bands].re;
      mha_real_t & istate = states[band + stage*bands].im;
      // The following three lines implement complex multiplication
      // (cstate *= ccoeff;) and complex addition (cstate += *cplus;)
      tmp = rstate * rcoeff - istate * icoeff + *rplus;
      istate = rstate * icoeff + istate * rcoeff + *iplus;
      rstate = tmp;
      // Summand during next order is this orders output: (cplus = &state;)
      rplus = &rstate;
      iplus = &istate;
    }
    // Copy last filter order's output to caller's output location.
    outputs[band].re = *rplus;
    outputs[band].im = *iplus;
  }
} 

/** Checks alignment of pointer address.
 * @param ptr pointer to check
 * @param alignment required alignment
 * @throw MHA_Error if ptr is not aligned as required. */
#define check_alignment(ptr,alignment) do { \
  if (((size_t)(ptr)) % (alignment)) \
      throw MHA_Error(__FILE__,__LINE__,"Incorrect alignment of %s", #ptr); \
} while(0)

/** Filters one sample per band, using simd operations on float32
 * To process more than one sample, the function must be
 * called repeatedly in correct order (from oldest sample to newest sample),
 * and the calling function must preserve the filter state (parameters
 * rstates and istates).
 *
 * This reimplements filter_sisd_real, but uses the CPU's vector registers
 * for the arithmetics, and is actually used by this plugin.
 * @param bands
 *   Number of total bands to compute (i.e. input_channels * num_frequencies)
 *   bands is also the size of the arrays pointed to by rinputs, iinputs,
 *   routputs, ioutputs, rcoefficients, icoefficients.
 * @param order
 *   Gammatone filter order
 * @param rinputs
 *   Pointer to array of the real part of input samples
 * @param iinputs
 *   Pointer to array of the imaginary part of input samples
 * @param routputs
 *   Pointer to array with space for the real parts of the output samples
 * @param routputs
 *   Pointer to array with space for the real parts of the output samples
 * @param rcoefficients
 *   Pointer to array of the real parts of the recursive filter coefficients
 * @param icoefficients
 *   Pointer to array of the imaginary parts of the filter coefficients
 * @param rstates
 *   Pointer to array of real parts of filter states. Array size is
 *   bands*order.  Initialize all elements with zeros before filtering the
 *   first sample.
 *   Filter state values will be modified by the function.  For filtering the
 *   next sample, this function needs the filter state arrays from filtering the
 *   previous sample again, unmodified. The real part of the filter state of
 *   band b, order o can be found at index [b+o*bands]
 * @param istates
 *   Pointer to array of imaginary parts of filter states. Array size is
 *   bands*order.  Initialize all elements with zeros before filtering the
 *   first sample.
 *   Filter state values will be modified by the function.  For filtering the
 *   next sample, this function needs the filter state arrays from filtering the
 *   previous sample again, unmodified. The imaginary part of the filter state
 *   of band b, order o can be found at index [b+o*bands] */
inline
void filter_simd(const unsigned bands, const unsigned order,
                 const mha_real_t * rinputs, const mha_real_t * iinputs,
                 mha_real_t * routputs, mha_real_t * ioutputs,
                 const mha_real_t * rcoefficients,
                 const mha_real_t * icoefficients,
                 mha_real_t * rstates, mha_real_t * istates)
{
  // check alignment
  check_alignment(rinputs,16); check_alignment(iinputs,16);
  check_alignment(routputs,16); check_alignment(ioutputs,16);
  check_alignment(rcoefficients,16); check_alignment(icoefficients,16);
  check_alignment(rstates,16); check_alignment(istates,16);
  if (bands % 4)
    throw MHA_ErrorMsg("number of bands must be multiple of 4");

  // data type grouping 4 floats for SIMD operations
  typedef float v4sf __attribute__ ((vector_size(4*sizeof(float))));

  // abbreviations for SIMD arithmetic operations
#define add4f(a,b) __builtin_ia32_addps(a,b)
#define sub4f(a,b) __builtin_ia32_subps(a,b)
#define mul4f(a,b) __builtin_ia32_mulps(a,b)

  // SIMD data used during computation
  v4sf rtmp, itmp, *rstate, *istate;
  for (unsigned band = 0; band < bands; band += 4) {
    // copy input data
    auto rplus = reinterpret_cast<const v4sf*>(&rinputs[band]);
    auto iplus = reinterpret_cast<const v4sf*>(&iinputs[band]);
    auto rcoeff = reinterpret_cast<const v4sf*>(&rcoefficients[band]);
    auto icoeff = reinterpret_cast<const v4sf*>(&icoefficients[band]);
    for (unsigned stage = 0; stage < order; ++stage) {
      rstate = reinterpret_cast<v4sf*>(&rstates[band + stage*bands]);
      istate = reinterpret_cast<v4sf*>(&istates[band + stage*bands]);

      // computes: rtmp = *rstate * *rcoeff - *istate * *icoeff + *rplus;
      rtmp = add4f(sub4f(mul4f(*rstate, *rcoeff),
                         mul4f(*istate, *icoeff)),
                   *rplus);
      // computes: itmp = *rstate * *icoeff + *istate * *rcoeff + *iplus;
      itmp = add4f(add4f(mul4f(*rstate, *icoeff),
                         mul4f(*istate, *rcoeff)),
                   *iplus);
      *rstate = rtmp;
      *istate = itmp;

      // summand for next order is this order's output
      rplus = rstate;
      iplus = istate;
    }
    // copy output data
    *reinterpret_cast<v4sf*>(&routputs[band]) = *rplus;
    *reinterpret_cast<v4sf*>(&ioutputs[band]) = *iplus;
  }
} 



/**\internal
 * Configuration for Gammatone Filterbank SIMD Analyzer.
 */
class gtfb_simd_cfg_t {

    unsigned order;             /**<  The order of the gammatone filters.*/
    unsigned bands;             /**<  Number of frequency bands per channel */
    unsigned channels;          /**< Number of input audio channels */
    unsigned bandsXchannels;    /**< Product of bands and channels */

    /// Combination of normalization and phase correction factor.
    std::vector<mha_complex_t> norm_phase;

    /// input signal (1 sample) multiplied with norm_phase
    float * rinputs; // bandsXchannels entries
    float * iinputs;
    
    /// The complex coefficients of the gammatone filter bands.
    float * rcoefficients; // bandsXchannels entries
    float * icoefficients; 

    /**\internal Storage for Filter state.
     * Holds channels() * bands() * order complex filter states.
     * Layout: state[stage * bandsXchannels + bands()*channel+band]
     */
    float * rstates;
    float * istates;

    /// output signal buffer, used by s_out.  Contains bandsXchannels * fragsize
    /// *2 entries.  all real parts come before all complex parts.  Order is:
    /// channel 0 band 0 real, channel 0 band 1 real, ..., channel 1 band 0 real
    /// channel 1 band 1 real, ... channel 0 band 0 imag ... then next sample
    float * sout_buf;

    /// Large float array. All previous pointers point into this array.
    /// Contains 3 unused entries to be able to adjust for alignment.
    float * large_array; // bandsXchannels*(fragsize + order + 2)*2 + 3 entries
    
    /**\internal Storage for the (complex) output signal.  Each of the
     * real input audio channels is split into frequency bands with
     * complex time signal output.  The split complex time signal is
     * again stored in a mha_wave_t buffer.  Each complex time signal
     * is stored as adjacent real and imaginary channels.  Complex
     * output from one source channel is stored in adjacent complex
     * output channels.
     */
    mha_wave_t s_out;
public:
    /// Each band is split into this number of bands.
    unsigned get_bands() const {return bands;}
    /// The number of separate audio channels.
    unsigned get_channels() const {return channels;}
    /// The number of frames in one chunk.
    unsigned get_frames() const {return s_out.num_frames;}

    // Delete (move)-assignment operators, copy and move constructors
    // As gtfb_simd_cfg_t has manual memory management, these special
    // functions should either be provided or disabled 
    gtfb_simd_cfg_t(gtfb_simd_cfg_t&&)=delete;
    gtfb_simd_cfg_t& operator=(gtfb_simd_cfg_t&&)=delete;
    gtfb_simd_cfg_t(const gtfb_simd_cfg_t&)=delete;
    gtfb_simd_cfg_t& operator=(const gtfb_simd_cfg_t&)=delete;

    /**\internal
     * Create a configuration for Gammatone Filterbank Analyzer.
     * @param ch     Number of Audio channels.
     * @param frames Number of Audio frames per chunk.
     * @param ord    The order of the gammatone filters.
     * @param _coeff Complex gammatone filter coefficients.
     * @param _norm_phase Normalization and phase correction factors.
     */
    gtfb_simd_cfg_t(unsigned ch, unsigned frames, unsigned ord,
                    const std::vector<mha_complex_t> & _coeff,
                    const std::vector<mha_complex_t> & _norm_phase)
        : order(ord),
          bands(_coeff.size()),
          channels(ch),
          bandsXchannels(bands*channels),
          norm_phase(_norm_phase)
    {
        if (bandsXchannels % 4)
            throw MHA_Error(__FILE__,__LINE__,
                            "Product of channels and bands has to be a multiple"
                            " of 4, is %u", bandsXchannels);
        if (_coeff.size() != norm_phase.size())
            throw MHA_Error(__FILE__,__LINE__,
                            "Number (%zu) of coefficients differs from number "\
                            "(%zu) of normalization/phase-correction factors",
                            _coeff.size(), norm_phase.size());
        large_array = new float[bandsXchannels*(frames + order + 2)*2 + 3];
        rinputs = (float*)((size_t(large_array+3) / 16) * 16);
        iinputs = rinputs + bandsXchannels;
        rcoefficients = iinputs + bandsXchannels;
        icoefficients = rcoefficients + bandsXchannels; 
        rstates = icoefficients + bandsXchannels;
        istates = rstates + bandsXchannels * order;
        sout_buf = istates + bandsXchannels * order;

        s_out.num_channels = bandsXchannels * 2;
        s_out.num_frames = frames;
        s_out.channel_info = 0;
        s_out.buf = sout_buf;

        for (unsigned channel = 0; channel < channels; ++channel)
            for (unsigned band = 0; band < bands; ++band) {
                rcoefficients[channel * bands + band] = _coeff[band].re;
                icoefficients[channel * bands + band] = _coeff[band].im;
            }
                
    }
    ~gtfb_simd_cfg_t()
    {
        delete [] large_array;
        s_out.buf = sout_buf = istates = rstates = large_array = 
            rinputs = iinputs = rcoefficients = icoefficients = 0;
    }
    inline mha_wave_t * process(mha_wave_t *s_in)
    {
        if (s_in->num_channels != channels || s_in->num_frames != get_frames())
            throw MHA_ErrorMsg("Input signal: unexpected frame or channel count");
        unsigned outband;
        float * input_sample;
        float * routputs = sout_buf;
        float * ioutputs = sout_buf + bandsXchannels;
        for (unsigned frame = 0; frame < get_frames();
             ++frame,
                 (routputs += bandsXchannels * 2),
                 (ioutputs += bandsXchannels * 2)) {
            for (unsigned channel = outband = 0; channel < channels; ++channel){
                input_sample = &value(s_in, frame, channel);
                for (unsigned band = 0; band < bands; ++band, ++outband) {
                    rinputs[outband] = *input_sample * norm_phase[band].re;
                    iinputs[outband] = *input_sample * norm_phase[band].im;
                }
            }
            filter_simd(bandsXchannels, order,
                        rinputs, iinputs,
                        routputs, ioutputs,
                        rcoefficients, icoefficients,
                        rstates, istates);
        }
        return &s_out;
    }
};

class gtfb_simd_t : public MHAPlugin::plugin_t<gtfb_simd_cfg_t> {
public:
    gtfb_simd_t(const algo_comm_t&,
                    const std::string& thread_name,
                    const std::string& algo_name);
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
private:
    void update_cfg();
    MHAEvents::patchbay_t<gtfb_simd_t> patchbay;
    bool prepared;
    MHAParser::int_t order;
    MHAParser::vcomplex_t coeff;
    MHAParser::vcomplex_t norm_phase;
};

/********************************************************************/

using MHAParser::StrCnv::val2str;

void gtfb_simd_t::update_cfg()
{
    if (prepared) {
        gtfb_simd_cfg_t * c =
            new gtfb_simd_cfg_t(tftype.channels,
                                tftype.fragsize,
                                order.data,
                                coeff.data,
                                norm_phase.data);
        push_config(c);
    }
}

gtfb_simd_t::gtfb_simd_t(const algo_comm_t& iac,
                         const std::string& thread_name,
                         const std::string& algo_name)
    : MHAPlugin::plugin_t<gtfb_simd_cfg_t>("Gammatone Filterbank Analyzer",
                                               iac),
      prepared(false),
      order("Order of gammatone filters", "4", "[0,["),
      coeff("Filter coefficients of gammatone filters", "[]"),
      norm_phase("Normalization & phase correction factors", "[]")
{
    insert_item("coeff", &coeff);
    patchbay.connect(&coeff.writeaccess,this,&gtfb_simd_t::update_cfg);
    insert_item("norm_phase", &norm_phase);
    patchbay.connect(&norm_phase.writeaccess,this,&gtfb_simd_t::update_cfg);
    insert_item("order",&order);
    patchbay.connect(&order.writeaccess,this,&gtfb_simd_t::update_cfg);
}

void gtfb_simd_t::prepare(mhaconfig_t& tf)
{
    if (prepared) 
        throw MHA_ErrorMsg("gtfb_simd_t::prepare is called a second time");
    if( tf.domain != MHA_WAVEFORM)
        throw MHA_ErrorMsg("gtfb_analyzer: Only waveform input can be processed.");
    tftype = tf;
    tf.channels *= coeff.data.size() * 2;
    prepared = true;

    // treat subnormals as zero
    unsigned int mxcsr = __builtin_ia32_stmxcsr ();
    mxcsr |= MXCSR_DAZ | MXCSR_FTZ;
    __builtin_ia32_ldmxcsr (mxcsr);

    update_cfg();
}

mha_wave_t* gtfb_simd_t::process(mha_wave_t* s)
{
    poll_config();
    return cfg->process(s);
}

MHAPLUGIN_CALLBACKS(gtfb_simd,gtfb_simd_t,wave,wave)
MHAPLUGIN_DOCUMENTATION
(gtfb_simd,"filterbank",
 "gtfb\\_simd implements the same gammatone filterbank as plugin gtfb\\_analyzer.\n"
 "The gammatone filtering is performed using built-in vector operations of x86."
 "The total number of bands (audio channels x filterbank frequencies) has to be"
 "a multiple of 4."
 "\n\n"
 "This plugin should be regarded as a proof-of-concept how Single-Instruction-"
 "Multiple-Data (SIMD) can be used inside openMHA.  For practical gammatone "
 "filtering applications, the plugins gtfb\\_analyzer and "
 "gtfb\\_simple\\_bridge should be used instead." )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
