// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2009 2010 2013 2014 2015 2017 2018 HörTech gGmbH
// Copyright © 2019 2020 2021 HörTech gGmbH
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

#include "mha_plugin.hh"
#include "mha_utils.hh"

namespace rmslevel {

    enum class UNIT {SPL=0, HL=1};

    /**
     * Rmslevel plugin. Measures sound for each block and publishes
     * measured result in monitor variables and AC variables.
     */
    class rmslevel_if_t : public MHAPlugin::plugin_t<UNIT> {
    public:

        /** Constructor of rmslevel plugin
         * \param iac Algorithm communication variable space, used to store
         *            extracted levels
         * \param configured_name Configured name of this plugins: either
         *            "rmslevel" or the name explicitly given after the colon.
         *            Used as prefix for all published AC variables. */
        rmslevel_if_t(algo_comm_t iac, const std::string & configured_name);

        /** Extract level from current STFT spectrum.
         * \param s input spectrum, not modified by this method. */
        mha_spec_t* process(mha_spec_t*s);

        /** Extract level from current time signal block.
         * \param s input audio block, not modified by this method. */
        mha_wave_t* process(mha_wave_t*s);

        /** Prepare rmslevel plugin for signal processing: Resize and
         * reinitialize monitor variables according to number of audio
         * channels specified in parameter, publish applicable monitor
         * variables as AC variables (depends on signal domain).
         * \param signal_dimensions: Audio signal metadata, not modified by
         *        this method. */
        void prepare(mhaconfig_t & signal_dimensions) override;

        /** Release removes published AC variables from AC space. */
        void release() override;
    private:

        /** Called on write access to the configuration variable \c unit. */
        void update();

        /** Configuration language event dispatcher. */
        MHAEvents::patchbay_t<rmslevel_if_t> patchbay;

        /** Sound power. */
        MHAParser::vfloat_mon_t level = {"RMS level in W/m^2"};

        /** Sound pressure level. */
        MHAParser::vfloat_mon_t level_db = {"RMS level in dB"};

        /** Peak amplitude. */
        MHAParser::vfloat_mon_t peak = {"peak amplitude in Pa"};

        /** dB value corresponding to peak amplitude. */
        MHAParser::vfloat_mon_t peak_db = {"peak amplitude in dB"};

        /** AC variable name for \c level. */
        const std::string level_acname;

        /** AC variable name for \c level_db. */
        const std::string level_db_acname;

        /** AC variable name for \c peak. */
        const std::string peak_acname;

        /** AC variable name for \c peak_db. */
        const std::string peak_db_acname;

        /** Configuration variable for selecting result dB scale */
        MHAParser::kw_t unit = {
          "Compute measured levels in dB(SPL) or dB(HL).\n"
          "When processing time domain signal, only dB(SPL) is supported.",
          "spl", "[spl hl]"
        };

        /** freq_offsets provides the conversion of dB(SPL) to dB(HL)
         * for every frequency bin in the stft used by coloured_intensity.
         * Unused when not in spectral domain and unit=hl.
         */
        std::vector<mha_real_t> freq_offsets;

        /** (Re-)insert AC variables for spectral processing into AC space.
         * Needs to be called during prepare() and at the end of every
         * invocation of process() when signal domain is MHA_SPECTRUM. */
        void insert_ac_variables_levels();

        /** (Re-)insert AC variables for waveform processing into AC space.
         * Needs to be called during prepare() and at the end of every
         * invocation of process() when signal domain is MHA_WAVEFORM. */
        void insert_ac_variables_peaks_and_levels();

        /** (Re-)insert a single AC variable. Helper method used by
         * insert_ac_variables_levels and insert_ac_variables_peaks_and_levels.
         * The stride of the AC variable will be set to v.size().
         * @param v Vector of floats to insert into the AC space. Its memory
         *          at v.data() must be valid until the next call to process()
         *          or release() (whichever occurs earlier).  Values may be
         *          accessed or altered by other plugins.
         * @param acname Name of the AC variable in the AC space.
         */
        void insert_ac_variable_float_vector(std::vector<float> & v,
                                             const std::string & acname);

        /** Remove AC variables from AC space. Called from release(). */
        void remove_ac_variables();
    };

    rmslevel_if_t::rmslevel_if_t(algo_comm_t iac, const std::string & configured_name)
        : plugin_t<UNIT>(
              "This plugin measures block based levels.  Results are\n"
              "published in monitor variables and in these AC variables\n"
              "(replace 'rmslevel' with the configured plugin name):\n\n"
              "  rmslevel_level_db\n"
              "  rmslevel_peak_db\n"
              "  rmslevel_level\n"
              "  rmslevel_peak\n"
              " The \'peak\' variables are only"
              " available during waveform processing.",
              iac),
          level_acname(configured_name + "_level"),
          level_db_acname(configured_name + "_level_db"),
          peak_acname(configured_name + "_peak"),
          peak_db_acname(configured_name + "_peak_db"),
          unit("Use dB(SPL) or dB(HL)", "spl", "[spl hl]")
    {
        insert_member(unit);
        insert_member(level);
        insert_member(level_db);
        insert_member(peak);
        insert_member(peak_db);
        patchbay.connect(&unit.writeaccess, this, &rmslevel_if_t::update);
    }

    mha_spec_t* rmslevel_if_t::process(mha_spec_t* s)
    {
        poll_config(); // makes dB unit available in *cfg
        for(unsigned int ch=0U; ch<s->num_channels;ch++){
            level.data[ch] = std::max(2e-10f,
                MHASignal::rmslevel(*s,ch,tftype.fftlen));
            switch(*cfg) {
            case UNIT::SPL:
                level_db.data[ch] = MHASignal::pa2dbspl(level.data[ch]);
                break;
            case UNIT::HL:
                level_db.data[ch] = std::max(-100.0f,MHASignal::pa22dbspl(
                        MHASignal::colored_intensity(
                            *s,ch,tftype.fftlen,freq_offsets.data())));
                break;
            default:
                throw MHA_Error(__FILE__,__LINE__, "Internal error: "
                                "Unknown unit for dB. Use dB(SPL) or dB(HL)");
            }
        }
        insert_ac_variables_levels();
        return s;
    }

    mha_wave_t* rmslevel_if_t::process(mha_wave_t* s)
    {
        if (*poll_config() != UNIT::SPL)
            throw MHA_Error(__FILE__, __LINE__, "Internal error: dB unit must "
                            "be dB(SPL) when processing waveform signal");
        for(unsigned ch=0U; ch<s->num_channels;ch++){
            level.data[ch] = std::max(MHASignal::rmslevel(*s,ch),2e-10f);
            peak.data[ch] = std::max(MHASignal::maxabs(*s,ch),2e-10f);
            level_db.data[ch] = MHASignal::pa2dbspl(level.data[ch]);
            peak_db.data[ch] = MHASignal::pa2dbspl(peak.data[ch]);
        }
        insert_ac_variables_peaks_and_levels();
        return s;
    }

    void rmslevel_if_t::update(){
        if(!is_prepared())
            return;
        const UNIT u = static_cast<UNIT>(unit.data.get_index());
        if (tftype.domain == MHA_WAVEFORM  and  u == UNIT::HL)
            throw MHA_Error(__FILE__,__LINE__,"rmslevel: Conversion to dB(HL) is only supported in frequency domain.");
        push_config(new UNIT(u));
    }

    void rmslevel_if_t::prepare(mhaconfig_t& tf)
    {
        tftype = tf;

        freq_offsets.resize(tftype.fftlen/2+1);
        for(unsigned idx=0U; idx<tftype.fftlen/2+1; ++idx) {
            const auto freq =
                MHASignal::bin2freq(idx,tftype.fftlen, tftype.srate);
            // factor two because coloured intensity expects squared weights
            freq_offsets[idx] = MHASignal::db2lin(2*MHAUtils::spl2hl(freq));
        }

        // All results are filled with tf.channels NaN values
        // before the first block is processed.
        constexpr float NaN = std::numeric_limits<float>::quiet_NaN();
        const std::vector<float> init_vector(tftype.channels, NaN);

        // All following assignments resize and initialize with NaN
        level.data = init_vector;
        level_db.data = init_vector;
        peak.data = init_vector;    // will stay NaN during spectrum processing
        peak_db.data = init_vector; // will stay NaN during spectrum processing
        update();
        if (tftype.domain == MHA_WAVEFORM)
            insert_ac_variables_peaks_and_levels();
        else
            insert_ac_variables_levels();
    }

    void rmslevel_if_t::release()
    {
        remove_ac_variables();
    }

    void rmslevel_if_t::insert_ac_variables_levels()
    {
        insert_ac_variable_float_vector(level.data, level_acname);
        insert_ac_variable_float_vector(level_db.data, level_db_acname);
    }
    void rmslevel_if_t::insert_ac_variables_peaks_and_levels()
    {
        insert_ac_variable_float_vector(peak.data, peak_acname);
        insert_ac_variable_float_vector(peak_db.data, peak_db_acname);
        insert_ac_variables_levels(); // reuse existing method for level vars
    }
    void rmslevel_if_t::
    insert_ac_variable_float_vector(std::vector<float> & v,
                                    const std::string & acname)
    {
        comm_var_t cv;
        cv.data_type = MHA_AC_FLOAT,
        cv.num_entries = v.size(),
        cv.stride = v.size(),
        cv.data = v.data();

        // Check that v.size() did not overflow cv.num_entries nor cv.stride
        if (cv.num_entries != v.size() || cv.stride != cv.num_entries)
            throw MHA_Error(__FILE__,__LINE__, "Vector %s has too many "
                            "(%zu) elements for an AC variable",
                            acname.c_str(), v.size());
        ac.handle->insert_var(acname, cv);
    }
    void rmslevel_if_t::remove_ac_variables()
    {
        ac.handle->remove_ref(peak.data.data());
        ac.handle->remove_ref(peak_db.data.data());
        ac.handle->remove_ref(level.data.data());
        ac.handle->remove_ref(level_db.data.data());
    }

    MHAPLUGIN_CALLBACKS(rmslevel,rmslevel_if_t,spec,spec)
    MHAPLUGIN_PROC_CALLBACK(rmslevel,rmslevel_if_t,wave,wave)
    MHAPLUGIN_DOCUMENTATION\
    (rmslevel,
     "level-meter feature-extraction",
     "This plugin computes the rms level and peak level of the current fragment and provides them as AC and monitor "
     " variables rms level in $W/m^2$ and peak level in Pascal. \n"
     "The values are provided in linear (variable names: level and peak) and logarithmic scale (level\\_db and peak\\_db)."
     " The default unit for the logarithmic scale is dB(SPL), \n"
     "but conversion to dB(HL) as per ISO 389-7:2005 (freefield) can be activated in the spectral domain. The correction values"
     " for frequencies above 16 kHz are extrapolated.")
}
// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
