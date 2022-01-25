// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2008 2009 2010 2013 2011 2014 2015 HörTech gGmbH
// Copyright © 2016 2017 2018 2019 2020 2021 HörTech gGmbH
// Copyright © 2021 2022 Hörzentrum Oldenburg gGmbH
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

#ifndef DC_H
#define DC_H

#include "mha.hh"
#include "mha_tablelookup.hh"
// Override deprecated warning for base_t copy ctor
// Usage here is okay because we only want the underlying
// data of the configuration variables
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#include "mha_filter.hh"
#include "mha_plugin.hh"
#pragma GCC diagnostic pop
#include <vector>
#include <string>

/** Namespace containing all classes of the \c dc plugin which performs
 * dynamic compression. */
namespace dc {

  /** Collection of configuration variables of the \c dc plugin.
   * The dc plugin interface class \c dc_if_t inherits from \c dc_vars_t.
   * This class is also used to pass a copy of all configuration variables
   * to the constructor of the runtime configuration class \c dc_t. */
  class dc_vars_t{
  public:
    /** Constructor initializes the configuration language variables' data
     * members and inserts them into the parser p.
     * @param p The dc plugin interface instance into which to insert the
     *          configuration variables. */
    explicit dc_vars_t(MHAParser::parser_t& p);
    /** Default copy constructor is used to pass a copy of all configuration
     * variables to the runtime configuration constructor \c dc_t::dc_t. */
    dc_vars_t(const dc_vars_t &) = default;
    /** Gain table with gains in dB. */
    MHAParser::mfloat_t gtdata;
    /** Narrow-band input levels in dB SPL specifying for each row of \c gtdata
     * to which input level the first gain in this row applies. */
    MHAParser::vfloat_t gtmin;
    /** Input level increment between elements for each row of \c gtdata. */
    MHAParser::vfloat_t gtstep;
    /** Low-pass filter time constants for time-domain envelope extraction. */
    MHAParser::vfloat_t taurmslevel;
    /** Low-pass filter time constants for level extraction when level rises.*/
    MHAParser::vfloat_t tauattack;
    /** Low-pass filter time constants for level extraction when level falls.*/
    MHAParser::vfloat_t taudecay;
    /** Row-specific input level corrections in dB to apply before gain lookup
     * in \c gtdata. */
    MHAParser::vfloat_t offset;
    /** Configured name of filterbank plugin placed upstream of this \c dc
     * plugin.  Used to lookup the AC variables published by the filterbank. */
    MHAParser::string_t filterbank;

    /** Name of the AC variable containing the number of audiochannels before
     * the filterbank. */
    MHAParser::string_t chname;
    /** Switch for bypassing the dynamic compression. */
    MHAParser::bool_t bypass;
    /** Interpolate gain table in dBs (vs. interpolating linear factors). */
    MHAParser::bool_t log_interp;
    /** Metadata: Some ID of the hearing impaired subject. */
    MHAParser::string_t clientid;
    /** Metadata: Some name of the gain rule that was used to compute gtdata.*/
    MHAParser::string_t gainrule;
    /** Metadata: Some name given to the current setting. */
    MHAParser::string_t preset;

    /** Narrow-band input levels of current block before attack/decay filter.*/
    MHAParser::vfloat_mon_t input_level;
    /** Narrow-band input levels of current block after attack/decay filter. */
    MHAParser::vfloat_mon_t filtered_level;
    /** Center frequencies of upstream filterbank. */
    MHAParser::vfloat_mon_t center_frequencies;
    /** Edge frequencies of upstream filterbank. */
    MHAParser::vfloat_mon_t edge_frequencies;
    /** Center frequencies of upstream filterbank. */
    MHAParser::vfloat_mon_t band_weights;
  };

  /** Consistency checker.  The runtime configuration class \c dc_t inherits
   * from this class. */
  class dc_vars_validator_t {
    public:
        /** Expands vectors in v, checks for consistency.
         * @param v Reference to \c dc_t's copy of the configuration variables.
         * @param s Total number of compression bands: bands x channels.
         * @param domain \c MHA_WAVEFORM or \c MHA_SPECTRUM. */
        dc_vars_validator_t(dc_vars_t & v,
                            unsigned int s,
                            mha_domain_t domain);
  };

  /** Runtime configuration class of dynamic compression plugin \c dc. */
  class dc_t : private dc_vars_validator_t {
    public:
        /** Constructor.
         * @param vars A copy of all configuration language variables of \c dc.
         * @param filter_rate The rate in Hz with which the level filters of
         *                    plugin \c dc are called.  For waveform processing
         *                    this is equal to the audio sampling rate.  For
         *                    spectral processing, this is equal to the audio
         *                    block rate.
         * @param nch_ Total number of compression bands: bands x channels.
         * @param ac Algorithm communication variable space.  The constructor
         *           will not interact with it.
         * @param domain \c MHA_WAVEFORM or \c MHA_SPECTRUM.
         * @param fftlen FFT length used for STFT processing, in samples.
         * @param naudiochannels_  Number of broadband audio channels
         *                         (before the upstream filterbank).
         * @param configured_name The configured name of this plugin in the MHA
         *                        configuration.  Used to derive the names of
         *                        the AC variables published by this plugin.
         * @param rmslevel_state Start state of rmslevel filters.
         * @param attack_state Start state of attack level filters.
         * @param decay_state Start state of decay level filters. */
        dc_t(dc_vars_t vars,
             mha_real_t filter_rate,
             unsigned int nch_,
             algo_comm_t ac,
             mha_domain_t domain,
             unsigned int fftlen,
             unsigned int naudiochannels_,
             const std::string& configured_name,
             const std::vector<mha_real_t>& rmslevel_state={},
             const std::vector<mha_real_t>& attack_state={},
             const std::vector<mha_real_t>& decay_state={}
             );
        /** Process method extracts band-specific input levels using the
         * rmslevel, attack and decay filters on each input sample, looks up
         * the gains and applies them to each sample of the signal in place.
         * @param s_in Latest block of time-domain input signal.
         * @return s_in after modifying the signal in place. */
        mha_wave_t* process(mha_wave_t* s_in);
        /** Process method extracts band-specific input levels using the
         * attack and decay filters on the latest STFT spectrum, looks up
         * the gains and applies them in place.
         * @param s_in Latest spectrum of the STFT input signal.
         * @return s_in after modifying the signal in place. */
        mha_spec_t* process(mha_spec_t* s_in);

        /* (Re)inserts AC variables into AC space. */
        void explicit_insert();

        /** @return Number of frequency bands per broadband input channels. */
        unsigned get_nbands() const {return nbands;}
        /** @return Number of frequency bands times broadband input channels.*/
        unsigned get_nch() const {return nch;}
        /** @return Const reference to the Matrix of input levels as
         *          computed before processed by the attack/decay filter. */
        const MHASignal::waveform_t & get_level_in_db() const
        {return level_in_db;}
        /** @return Const reference to the Matrix of input levels after
         *          being filtered by the attack/decay filter. */
        const MHASignal::waveform_t & get_level_in_db_adjusted() const
        {return level_in_db_adjusted;}

        /** @return Filter states of first-order rmslevel low-pass filters. */
        std::vector<mha_real_t> get_rmslevel_filter_state() const {
            return rmslevel.flatten();
        }
        /** @return Filter states of first-order attack low-pass filters. */
        std::vector<mha_real_t> get_attack_filter_state() const {
            return attack.flatten();
        }
        /** @return Filter states of max-tracking decay low-pass filters. */
        std::vector<mha_real_t> get_decay_filter_state() const {
            return decay.flatten();
        }

    private:
        /** Dynamic compression gains. If \c log_interp is true, then they are
         * stored as dB gains, otherwise they are stored as linear gains. */
        std::vector<MHATableLookup::linear_table_t> gt;
        /** band-specific dB offsets added to measured input levels before
         * gain lookup is performed. */
        std::vector<mha_real_t> offset;
        /** Envelope extraction filters used in waveform processing. */
        MHAFilter::o1flt_lowpass_t rmslevel;
        /** Attack filters used in input level estimation. */
        MHAFilter::o1flt_lowpass_t attack;
        /** Maximum-tracking decay filters used in input level estimation. */
        MHAFilter::o1flt_maxtrack_t decay;
        /** Dynamic compression is not applied if bypass == true. */
        bool bypass;
        /// Flag whether gain table interpolation should be done in dB domain.
        bool log_interp;
        /// Number of broadband audio channels (before the upstream filterbank)
        unsigned int naudiochannels;
        /** Number of bands per broadband audio channel. */
        unsigned int nbands;
        /** \c nbands * \c naudiochannels */
        unsigned int nch;
        /** Matrix of latest input levels before attack/decay filter. */
        MHA_AC::waveform_t level_in_db;
        /** Matrix of latest input levels after attack/decay filter. */
        MHA_AC::waveform_t level_in_db_adjusted;
        /** FFT length in samples, required for computing levels correctly. */
        unsigned int fftlen;
    };

  /** Plugin interface class of the dynamic compression plugin \c dc. */
  class dc_if_t : public MHAPlugin::plugin_t<dc_t>, public dc_vars_t {
    public:
        /** Standard MHA plugin constructor.
         * @param iac Algorithm communication variable space.
         * @param configured_name The name given to this plugin by the
         *                        configuration. */
        dc_if_t(algo_comm_t iac, const std::string & configured_name);
        /** Prepare plugin for signal processing.
         * @param tf Input signal dimensions.  They are not modified. */
        void prepare(mhaconfig_t& tf);
        /** Release plugin from signal processing. */
        void release();
        /** Process method extracts band-specific input levels using the
         * rmslevel, attack and decay filters on each input sample, looks up
         * the gains and applies them to each sample of the signal in place.
         * @param s_in Latest block of time-domain input signal.
         * @return s_in after modifying the signal in place. */
        mha_wave_t* process(mha_wave_t* s_in);
        /** Process method extracts band-specific input levels using the
         * attack and decay filters on the latest STFT spectrum, looks up
         * the gains and applies them in place.
         * @param s_in Latest spectrum of the STFT input signal.
         * @return s_in after modifying the signal in place. */
        mha_spec_t* process(mha_spec_t* s_in);
    private:
        /** Called from within the processing routines: updates the monitor
         * variables. */
        void update_monitors();

        /** Called by MHA configuration change event mechanism: creates new
         * runtime configuration */
        void update();
        /** Configured name of this plugin, used as prefix for names of
         * published AC variables. */
        std::string algo;
        /** Connects configuration events to callbacks. */
        MHAEvents::patchbay_t<dc_if_t> patchbay;
        /** Number of broadband audio channels (before the filterbank).  This
         * value is filled in during prepare(). */
        unsigned broadband_audiochannels = {0U};
        /** Number of frequency bands per broadband audio channel.
         * Initialized during prepare(). */
        unsigned bands_per_channel = {0U};
        /** Name of AC variable containing the filterbank centre frequencies
         * in Hz.  Initialized during prepare(). */
        std::string cf_name;
        /** Name of AC variable containing the filterbank edge frequencies
         * in Hz.  Initialized during prepare(). */
        std::string ef_name;
        /** Name of the AC variable containing the filterbank band weights.
         * Initialized during prepare(). */
        std::string bw_name;
    };
}


#endif
