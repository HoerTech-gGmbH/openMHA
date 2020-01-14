// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2009 2010 2013 2014 2015 2018 2019 2020 HörTech gGmbH
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

#include "mha_plugin.hh"
#include "mha_signal.hh"
#include "mha_parser.hh"
#include "mha_defs.h"
#include <math.h>
#include <time.h>
#include "mha_events.h"
#include "mha_filter.hh"


/**
   \file   gtfb_analyzer.cpp
   \brief  Gammatone Filterbank Analyzer Plugin
*/

namespace gtfb_analyzer {
/// Configuration for Gammatone Filterbank Analyzer.
struct gtfb_analyzer_cfg_t {

    /// The order of the gammatone filters.
    unsigned order;

    /// The complex coefficients of the gammatone filter bands.
    std::vector<mha_complex_t> coeff;

    /// Combination of normalization and phase correction factor.
    std::vector<mha_complex_t> norm_phase;

    /** Storage for the (complex) output signal.  Each of the
     * real input audio channels is split into frequency bands with
     * complex time signal output.  The split complex time signal is
     * again stored in a mha_wave_t buffer.  Each complex time signal
     * is stored as adjacent real and imaginary channels.  Complex
     * output from one source channel is stored in adjacent complex
     * output channels.
     *
     * Example: If the input has 2 channels ch0 ch1, and gtfb_analyzer
     * splits into 3 bands b0 b1 b2, then the order of output channels
     * in s_out is:
     * ch0_b0_real ch0_b0_imag ch0_b1_real ch0_b1_imag ch0_b2_real ch0_b2_imag
     * ch1_b0_real ch1_b1_imag ch1_b1_real ch1_b1_imag ch1_b2_real ch1_b2_imag
     */
    mha_wave_t s_out;

    /** Storage for Filter state.
     * Holds channels() * bands() * order complex filter states.
     * Layout: state[(bands()*channel+band)*order+stage]
     */
    std::vector<mha_complex_t> state;
    /// Each band is split into this number of bands.
    unsigned bands() const {return coeff.size();}
    /// The number of separate audio channels.
    unsigned channels() const {return s_out.num_channels / bands() / 2;}
    /// The number of frames in one chunk.
    unsigned frames() const {return s_out.num_frames;}
    /// Returns pointer to filter states for that band
    mha_complex_t * states(unsigned channel, unsigned band)
    { return &state[(bands()*channel+band)*order]; }

    /**
     * Create a configuration for Gammatone Filterbank Analyzer.
     * @param ch     Number of Audio channels.
     * @param frames Number of Audio frames per chunk.
     * @param ord    The order of the gammatone filters.
     * @param _coeff Complex gammatone filter coefficients.
     * @param _norm_phase Normalization and phase correction factors.
     */
    gtfb_analyzer_cfg_t(unsigned ch, unsigned frames, unsigned ord,
                        const std::vector<mha_complex_t> & _coeff,
                        const std::vector<mha_complex_t> & _norm_phase)
        : order(ord),
          coeff(_coeff),
          norm_phase(_norm_phase),
          state(_coeff.size() * ch * ord, mha_complex(0))
    {
        if (coeff.size() != norm_phase.size())
            throw MHA_Error(__FILE__,__LINE__,
                            "Number (%zu) of coefficients differs from number "\
                            "(%zu) of normalization/phase-correction factors",
                            coeff.size(), norm_phase.size());
        s_out.num_channels = ch * coeff.size() * 2;
        s_out.num_frames = frames;
        s_out.channel_info = 0;
        s_out.buf = new mha_real_t[ch * bands() * frames * 2];
    }
    ~gtfb_analyzer_cfg_t()
    {delete [] s_out.buf; s_out.buf = 0;}
    mha_complex_t & cvalue(unsigned frame, unsigned channel, unsigned band)
    {
        return *(reinterpret_cast<mha_complex_t *>(s_out.buf) +
                 (frame * channels() * bands() + channel * bands() + band));
    }
};

/// Gammatone Filterbank Analyzer Plugin
class gtfb_analyzer_t : public MHAPlugin::plugin_t<gtfb_analyzer_cfg_t> {
public:
    gtfb_analyzer_t(const algo_comm_t&,
                    const std::string& thread_name,
                    const std::string& algo_name);
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
private:
    void update_cfg();
    MHAEvents::patchbay_t<gtfb_analyzer_t> patchbay;
    bool prepared;
    MHAParser::int_t order;
    MHAParser::vcomplex_t coeff;
    MHAParser::vcomplex_t norm_phase;
};
}
/********************************************************************/

using MHAParser::StrCnv::val2str;

void gtfb_analyzer::gtfb_analyzer_t::update_cfg()
{
    if (prepared) {
        gtfb_analyzer_cfg_t * c =
            new gtfb_analyzer_cfg_t(tftype.channels,
                                    tftype.fragsize,
                                    order.data,
                                    coeff.data,
                                    norm_phase.data);
        push_config(c);
    }
}

gtfb_analyzer::gtfb_analyzer_t::gtfb_analyzer_t(
                                 const algo_comm_t& iac,
                                 const std::string& thread_name,
                                 const std::string& algo_name)
    : MHAPlugin::plugin_t<gtfb_analyzer_cfg_t>("Gammatone Filterbank Analyzer",
                                               iac),
    prepared(false),
    order("Order of gammatone filters", "4", "[0,["),
    coeff("Filter coefficients of gammatone filters", "[]"),
    norm_phase("Normalization & phase correction factors", "[]")
{
    insert_item("coeff", &coeff);
    patchbay.connect(&coeff.writeaccess,this,&gtfb_analyzer::gtfb_analyzer_t::update_cfg);
    insert_item("norm_phase", &norm_phase);
    patchbay.connect(&norm_phase.writeaccess,this,&gtfb_analyzer::gtfb_analyzer_t::update_cfg);
    insert_item("order",&order);
    patchbay.connect(&order.writeaccess,this,&gtfb_analyzer::gtfb_analyzer_t::update_cfg);
}

void gtfb_analyzer::gtfb_analyzer_t::prepare(mhaconfig_t& tf)
{
    if (prepared)
        throw MHA_ErrorMsg("gtfb_analyzer::gtfb_analyzer_t::prepare is called a second time");
    if( tf.domain != MHA_WAVEFORM)
        throw MHA_ErrorMsg("gtfb_analyzer: Only waveform input can be processed.");
    tftype = tf;
    tf.channels *= coeff.data.size() * 2;
    prepared = true;
    update_cfg();
}

/**
 * Filters a complex input sample with the given filter coefficient.
 * No normalization takes place. The implementation is
 * tail-recursive and to exploit compiler optimization.
 *
 * @param input The complex input sample
 *
 * @param coeff The complex filter coefficient
 *
 * @param states Pointer to the array of complex filter states.
 *
 * @param orders The filter order
 *
 * @return A const ref to the filtered sample
 *
 */
static inline const mha_complex_t &
filter_complex(const mha_complex_t & input,
               const mha_complex_t & coeff,
               mha_complex_t * states,
               unsigned orders)
{
    if (orders == 0) return input;
    return filter_complex(((*states) *= coeff) += input,
                          coeff, states+1, orders-1);
}

/**
 * Filters a real input sample with the given filter coefficient and
 * applies the given normalization with phase correction.
 *
 * @param input The real input sample
 *
 * @param tmp_complex A reference to a mha_complex_t used for intermediate
 * results. No assumptions should be made about the state of tmp_complex
 * after the return of filter_real. This is an optimization to reduce the number
 * of dtor/ctor calls of mha_complex_t
 *
 * @param coeff The complex filter coefficient
 *
 * @param states Pointer to the array of complex filter states.
 *
 * @param orders The filter order
 *
 * @param normphase Normalization coefficient including the phase correction
 *
 * @return A const ref to the filtered sample
 *
 */
static inline const mha_complex_t &
filter_real(mha_real_t input,
            mha_complex_t & tmp_complex,
            const mha_complex_t & coeff,
            mha_complex_t * states,
            unsigned orders,
            const mha_complex_t & normphase)
{
    return filter_complex((tmp_complex = normphase) *= input,
                          coeff, states, orders);
}

mha_wave_t* gtfb_analyzer::gtfb_analyzer_t::process(mha_wave_t* s)
{
    poll_config();
    mha_complex_t tmp_complex = {0,0};
    for (unsigned frame = 0; frame < s->num_frames; ++frame) {
        for (unsigned channel = 0; channel < s->num_channels; ++channel) {
            for (unsigned band = 0; band < cfg->bands(); ++band) {
                cfg->cvalue(frame, channel, band) =
                    filter_real(value(s,frame,channel),
                                tmp_complex,
                                cfg->coeff[band],
                                cfg->states(channel,band),
                                cfg->order,
                                cfg->norm_phase[band]);
                MHAFilter::make_friendly_number(cfg->cvalue(frame, channel, band));
            }
        }
    }
    return &cfg->s_out;
}

MHAPLUGIN_CALLBACKS(gtfb_analyzer,gtfb_analyzer::gtfb_analyzer_t,wave,wave)
MHAPLUGIN_DOCUMENTATION\
(gtfb_analyzer,
 "filterbank",
 "Implements a complex-valued gammatone filterbank using"
 " cascaded first-order filters as described in"
 " Hohmann(2002)\\footnote{"
 " Volker Hohmann,"
 " Frequency analysis and synthesis using a Gammatone"
 " filterbank."
 " Acta Acustica united with Acustica 88(3),"
 " pp. 433-442, 2002."
 "}, and Herzke and Hohmann(2007)\\footnote{"
 " Tobias Herzke and Volker Hohmann,"
 " Improved Numerical Methods for Gammatone Filterbank"
 " Analysis and Synthesis."
 " Acta Acustica united with Acustica 93(3), "
 " pp. 498-500, 2007."
 "}.  Set the parameter order to the desired gammatone"
 " filter order. The coeff is a vector of complex filter"
 " coefficients, one for each filterbank frequency band."
 " The complex coefficients need to be computed outside"
 " of the MHA, e.g. with the help of the matlab"
 " implementation of the gammatone filterbank which can"
 " be downloaded from"
 " https://uol.de/mediphysik/downloads/.  Similarly, the"
 " combination of normalization factors and phases also"
 " have to be computed outside of the MHA, e.g. also"
 " with the same matlab implementation of this gammatone"
 " filterbank."
 "\n\n"
 " The output signal produced by this plugin contains"
 " the complex output signal produced by the cascaded"
 " gammatone filters in each band. Because the MHA time "
 " domain signal representation does not support storing"
 " of complex values, real and imaginary parts are"
 " stored in different output channels."
 "\n\n"
 " Example: If the input has 2 channels (ch0, ch1),"
 " and \\texttt{gtfb\\_analyzer} splits into 3 bands (b0, b1, b2),"
 " then the order of output channels produced by"
 " \\texttt{gtfb\\_analyzer} is:"
 " ch0\\_b0\\_real, ch0\\_b0\\_imag, ch0\\_b1\\_real, ch0\\_b1\\_imag,"
 " ch0\\_b2\\_real, ch0\\_b2\\_imag, ch1\\_b0\\_real, ch1\\_b1\\_imag,"
 " ch1\\_b1\\_real, ch1\\_b1\\_imag, ch1\\_b2\\_real, ch1\\_b2\\_imag"
 )


// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
