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

#include "dc.hh"

using namespace dc;

dc_if_t::dc_if_t(MHA_AC::algo_comm_t & iac, const std::string & configured_name)
   : MHAPlugin::plugin_t<dc_t>("dynamic compression",iac),
    dc_vars_t(static_cast<MHAParser::parser_t&>(*this)),
    algo(configured_name)
{
    patchbay.connect(&writeaccess,this,&dc_if_t::update);
}

void dc_if_t::update()
{
    if( is_prepared() ){
        float filter_rate;
        if( tftype.domain == MHA_WAVEFORM )
            filter_rate = tftype.srate;
        else
            filter_rate = tftype.srate / tftype.fragsize;

        if(cfg && tftype.channels==cfg->get_nch()) {
            auto rmslevel_state=cfg->get_rmslevel_filter_state();
            auto attack_state=cfg->get_attack_filter_state();
            auto decay_state=cfg->get_decay_filter_state();
            push_config(new dc_t(static_cast<const dc_vars_t&>(*this),
                                 filter_rate,
                                 tftype.channels,
                                 ac,
                                 tftype.domain,tftype.fftlen,
                                 broadband_audiochannels, algo,
                                 rmslevel_state, attack_state, decay_state));
        }
        else {
            push_config(new dc_t(static_cast<const dc_vars_t&>(*this),
                                 filter_rate,
                                 tftype.channels,
                                 ac,
                                 tftype.domain,tftype.fftlen,
                                 broadband_audiochannels, algo));
        }
    }
}

/** The dynamic compressor implemented by plugin \c dc was created to perform
 * multi-band dynamic compression as found in hearing aids.  In hearing aid
 * simulation tasks, openMHA will normally simulate either one or two hearing
 * aids (i.e., aid one or two ears).
 * 
 * The filter bank which splits the broadband input signal from left and/or
 * right microphone is not part of plugin dc.  openMHA researchers can use
 * a filterbank plugin of their choice, e.g. \c fftfilterbank.
 * Filter banks split broadband signal channels into multiple
 * narrow-band signal channels:  A filter bank with 10 frequency bands
 * would split one broadband signal channel into 10 narrow-band signal
 * channels, and the dc plugin will then only see these 10 narrow-band
 * audio signal channels.  The same filter bank would split 2 broadband
 * channels into 20 narrow-band channels, i.e. 10 channels per ear.
 * 
 * For hearing aid fitting, the fitting rule should be able to detect if
 * the dc plugin fits one hearing aid or two hearing aids.  The dc plugin
 * can normally not derive this information from the incoming audio signal
 * dimensions alone, but the filterbank that was used to split the broadband
 * signals into narrow-band signal knows the original number of broadband
 * channels.  MHA filterbank plugins therefore publish an AC variable
 * containing the original number of broadband audio channels,
 * which were present before the filterbank split the signal into frequency
 * bands.
 * 
 * This function accesses the AC space and reads the number of broadband audio
 * channels from the AC variable \c acname, which must be given by the
 * configuration and should identify the AC variable published by the filter
 * bank for this purpose.
 * 
 * If \c acname is empty, then the function parameter \c totalchannels is
 * returned instead, assuming that there was no filterbank and all input
 * channels that the \c dc plugin receives are broadband audio channels.
 * 
 * @param totalchannels   The number of audio channels that dc receives from
 *        the MHA (MHA informs plugins about the number of input signal
 *        channels with the prepare() callback).  If the \c dc plugin processes
 *        the output signal of a filter bank, then this will be the equal to
 *        the number of broadband input channels multiplied by the number of
 *        filter bank bands.  If not, this will already be the number of
 *        broadband input channels.
 * @param acname   If non-empty, name of an AC variable that contains the
 *        number of broadband input processed by an up-stream filter bank
 *        plugin.  An empty acname denotes that no filter bank is present and
 *        that the \c dc plugin directly processes broadband audio signals.
 * @param ac    Algorithm communication variable space, needed to access
 *        AC variable \c acname.
 * @return The number of broadband audio channels from which the input signal
 *        of \c dc was generated.  This will usually be 1 or 2 for normal
 *        hearing aid simulation task, but the \c dc plugin is not restricted
 *        to process only 1 or 2 broadband signals, therefore other return
 *        values are possible, but then hearing aid fitting rules querying
 *        the dc plugin may become confused.
 * @warning Since this function accesses the AC variable space, it may only
 *        be called in situation where a plugin is allowed to interact with
 *        the AC variable space.  This function should be called from prepare()
 */
static unsigned int get_audiochannels(unsigned int totalchannels,
                                      std::string acname,
                                      MHA_AC::algo_comm_t & ac)
{
    if( acname.size() ){
        totalchannels = MHA_AC::get_var_int(ac,acname);
        if( totalchannels == 0 )
            throw MHA_Error(__FILE__,__LINE__,"dc got 0 channels from AC variable \"%s\".",acname.c_str());
    }else{
        if( totalchannels == 0 )
            throw MHA_Error(__FILE__,__LINE__,"dc got 0 channels from prepare configuration.");
    }
    return totalchannels;
}

void dc_if_t::prepare(mhaconfig_t& tf)
{
    tftype = tf;
    broadband_audiochannels = get_audiochannels(tftype.channels,chname.data,ac);
    bands_per_channel = tftype.channels / broadband_audiochannels;
    if (bands_per_channel * broadband_audiochannels != tftype.channels)
        throw MHA_Error(__FILE__,__LINE__, "Mismatching channel configuration "
                        "(%u bands_per_channel, %u broadbandaudio channels, %u"
                        " total compression bands)", bands_per_channel,
                        broadband_audiochannels, tftype.channels);
    update();
    poll_config();
    cfg->explicit_insert();
    input_level.data.resize(tf.channels);
    filtered_level.data.resize(tf.channels);
    center_frequencies.data.resize(bands_per_channel);
    edge_frequencies.data.resize(bands_per_channel+1);
    band_weights.data.resize(bands_per_channel);
    cf_name = filterbank.data + "_cf";
    ef_name = filterbank.data + "_ef";
    bw_name = filterbank.data + "_band_weights";
    update_monitors();

    // Filterbank layout must not change during processing.
    filterbank.setlock(true);
    chname.setlock(true);
}

void dc_if_t::release()
{
    filterbank.setlock(false);
    chname.setlock(false);
}

mha_wave_t* dc_if_t::process(mha_wave_t* s_in)
{
    poll_config();
    mha_wave_t * s_out = cfg->process(s_in);
    update_monitors();
    return s_out;
}

mha_spec_t* dc_if_t::process(mha_spec_t* s_in)
{
    poll_config();
    mha_spec_t * s_out;
    s_out = cfg->process(s_in);
    update_monitors();
    return s_out;
}

dc_vars_validator_t::dc_vars_validator_t(dc_vars_t & v,
                                         unsigned int s,
                                         mha_domain_t domain)
{
    if( !s )
        throw MHA_Error(__FILE__,__LINE__,
                        "Invalid (zero) channel count.");
#define DUPVEC(x) v.x.data = MHASignal::dupvec_chk(v.x.data,s)
    DUPVEC(gtmin);
    DUPVEC(gtstep);
    if( domain == MHA_WAVEFORM )
        DUPVEC(taurmslevel);
    DUPVEC(tauattack);
    DUPVEC(taudecay);
#undef DUPVEC
    if( v.gtdata.data.size() != s )
        throw MHA_Error(__FILE__,__LINE__,
                        "Invalid gaintable size (found %zu, expected %u).",
                        v.gtdata.data.size(),s);
    if( v.gtmin.data.size() != s )
        throw MHA_Error(__FILE__,__LINE__,
                        "Invalid gaintable minimum vector size (found %zu, expected %u).",
                        v.gtmin.data.size(),s);
    if( v.gtstep.data.size() != s )
        throw MHA_Error(__FILE__,__LINE__,
                        "Invalid gaintable stepsize vector size (found %zu, expected %u).",
                        v.gtstep.data.size(),s);
    if( (domain == MHA_WAVEFORM) && (v.taurmslevel.data.size() != s) )
        throw MHA_Error(__FILE__,__LINE__,
                        "Invalid rms level time constant vector size (found %zu, expected %u).",
                        v.taurmslevel.data.size(),s);
    if( v.tauattack.data.size() != s )
        throw MHA_Error(__FILE__,__LINE__,
                        "Invalid attack time constant vector size (found %zu, expected %u).",
                        v.tauattack.data.size(),s);
    if( v.taudecay.data.size() != s )
        throw MHA_Error(__FILE__,__LINE__,
                        "Invalid decay time constant vector size (found %zu, expected %u).",
                        v.taudecay.data.size(),s);
    if( v.offset.data.size() != s and v.offset.data.size() )
        throw MHA_Error(__FILE__,__LINE__,"Invalid level offset vector size (found %zu, expected %u or 0).",
                        v.offset.data.size(),s);
}

dc_t::dc_t(dc_vars_t vars,
           mha_real_t filter_rate,
           unsigned int nch_,
           MHA_AC::algo_comm_t & ac,
           mha_domain_t domain,
           unsigned int fftlen_,
           unsigned naudiochannels_,
           const std::string& algo,
           const std::vector<mha_real_t>& rmslevel_state,
           const std::vector<mha_real_t>& attack_state,
           const std::vector<mha_real_t>& decay_state
           )
    :
    dc_vars_validator_t(vars, nch_, domain),
    offset(vars.offset.data),
    rmslevel([&](){
            if(rmslevel_state.size())
                return MHAFilter::o1flt_lowpass_t(vars.taurmslevel.data, filter_rate, rmslevel_state);
            else
                return MHAFilter::o1flt_lowpass_t(vars.taurmslevel.data, filter_rate, pow(10,65*0.1)*4e-10);
        }()),
    attack([&](){if(attack_state.size())
                return MHAFilter::o1flt_lowpass_t(vars.tauattack.data, filter_rate, attack_state);
            else
                return MHAFilter::o1flt_lowpass_t(vars.tauattack.data, filter_rate, 65);
        }()),
    decay([&](){if(decay_state.size())
                return MHAFilter::o1flt_maxtrack_t(vars.taudecay.data, filter_rate, decay_state);
            else
                return MHAFilter::o1flt_maxtrack_t(vars.taudecay.data, filter_rate, 65);
        }()),
    bypass(vars.bypass.data),
    log_interp(vars.log_interp.data),
    naudiochannels(naudiochannels_),
    nbands(nch_ / naudiochannels),
    nch(nch_),
    level_in_db(ac,algo+"_l_in",nbands,naudiochannels,false),
    level_in_db_adjusted(ac,algo+"_l_in_adj",nbands,naudiochannels,false),
    fftlen(fftlen_)
{
    if( nbands * naudiochannels != nch )
        throw MHA_Error(__FILE__,__LINE__,
                        "Mismatching channel configuration (%u bands, %u input channels, %u audio channels)",
                        nbands,nch,naudiochannels);
    unsigned int k,klev;
    gt.resize(nch);
    mha_real_t inlev;
    for(k=0; k<nch; k++){
        gt[k].clear();
        inlev = vars.gtmin.data[k];
        if (log_interp){
            for(klev=0;klev<vars.gtdata.data[k].size();klev++){
                gt[k].add_entry(vars.gtdata.data[k][klev]);
                inlev += vars.gtstep.data[k];
            }
        }
        else {
            for(klev=0;klev<vars.gtdata.data[k].size();klev++){
                gt[k].add_entry(MHASignal::db2lin(vars.gtdata.data[k][klev]));
                inlev += vars.gtstep.data[k];
            }
        }
        gt[k].set_xmin( vars.gtmin.data[k] );
        gt[k].set_xmax( inlev );
        gt[k].prepare();
    }
}

void dc_if_t::update_monitors()
{
    unsigned int band, ch;
    const unsigned int fbands = cfg->get_nbands();
    const unsigned int naudiochannels = tftype.channels / fbands;
    const mha_real_t * cf = 0, * ef = 0, * bw = 0;
    MHA_AC::comm_var_t v;
    if (ac.is_var(cf_name)) {
        v = ac.get_var(cf_name);
        if (v.data_type == MHA_AC_MHAREAL && v.num_entries == fbands)
            cf = static_cast<mha_real_t *>(v.data);
    }
    if (ac.is_var(ef_name)) {
        v = ac.get_var(ef_name);
        if (v.data_type == MHA_AC_MHAREAL && v.num_entries == (fbands+1U))
            ef = static_cast<mha_real_t *>(v.data);
    }
    if (ac.is_var(bw_name)) {
        v = ac.get_var(bw_name);
        if (v.data_type == MHA_AC_MHAREAL && v.num_entries == fbands)
            bw = static_cast<mha_real_t *>(v.data);
    }
    for (band = 0; band < fbands; ++band) {
        if (cf) center_frequencies.data[band] = cf[band];
        if (ef) edge_frequencies.data[band] = ef[band];
        if (bw) band_weights.data[band] = bw[band];
        for (ch = 0; ch < naudiochannels; ++ch) {
            input_level.data[band + ch*fbands] =
                cfg->get_level_in_db().value(band, ch);
            filtered_level.data[band + ch*fbands] =
                cfg->get_level_in_db_adjusted().value(band, ch);
        }
    }
    if (ef) edge_frequencies.data[band] = ef[band];
}

void dc_t::explicit_insert()
{
    level_in_db.insert();
    level_in_db_adjusted.insert();
}

mha_wave_t* dc_t::process(mha_wave_t* s)
{
    explicit_insert();
    mha_real_t level_in, gain;
    unsigned int k, ch, kfb, idx, ch_idx = 0;
    if( s->num_channels != gt.size() )
        throw MHA_Error(__FILE__,__LINE__,
                        "The audio channel number changed from %zu to %u.",
                        gt.size(), s->num_channels);
    for(k=0;k<s->num_frames;k++){
        for(kfb=0;kfb<nbands;kfb++){
            for(ch=0;ch<naudiochannels;ch++){
                ch_idx = kfb + nbands*ch;
                idx = k*s->num_channels + ch_idx;
                level_in = rmslevel(ch_idx, s->buf[idx]*s->buf[idx]);
                level_in_db.value(kfb,ch) = MHASignal::pa22dbspl(level_in);
                level_in_db_adjusted.value(kfb,ch)=decay(ch_idx,attack(ch_idx, MHASignal::pa22dbspl(level_in)));
                if (bypass) continue;
                gain = gt[ch_idx].interp( level_in_db_adjusted.value(kfb,ch) + (offset.size() ? offset[ch_idx] : 0));
                if(log_interp)
                    gain = MHASignal::db2lin(gain);
                if( gain < 0 )
                    gain = 0;
                s->buf[idx] *= gain;
            }
        }
    }
    return s;
}

mha_spec_t* dc_t::process(mha_spec_t* s)
{
    explicit_insert();
    mha_real_t level_in, gain;
    unsigned int k, ch, kfb, ch_idx;
    if( s->num_channels != gt.size() )
        throw MHA_Error(__FILE__,__LINE__,
                        "The audio channel number changed from %zu to %u.",
                        gt.size(), s->num_channels);
    for(kfb=0;kfb<nbands;kfb++){
        for(ch=0;ch<naudiochannels;ch++){
            ch_idx = kfb + nbands*ch;
            level_in = MHASignal::colored_intensity(*s, ch_idx, fftlen, 0);
            level_in_db.value(kfb,ch) = MHASignal::pa22dbspl(level_in);
            level_in_db_adjusted.value(kfb,ch)=decay(ch_idx,attack(ch_idx,MHASignal::pa22dbspl(level_in)));
        }
    }
    // apply gains:
    if (bypass) return s;
    for(ch=0;ch<naudiochannels;ch++)
        for(kfb=0;kfb<nbands;kfb++){
            ch_idx = kfb + nbands*ch;
            gain = gt[ch_idx].interp(level_in_db_adjusted.value(kfb,ch) + (offset.size() ? offset[ch_idx] : 0));
            if (log_interp)
                gain = MHASignal::db2lin(gain);
            if( gain < 0 )
                gain = 0;
            for(k=0;k<s->num_frames;k++){
                value(s,k,ch_idx) *= gain;
            }
        }
    return s;
}

dc_vars_t::dc_vars_t(MHAParser::parser_t& p)
    : gtdata("gaintable data with gains in dB.  Each "
             "row in this matrix contains gains for one\n"
             "channel or band.  Internally, the "
             "dB gains of this table are converted to\n"
             "linear gain factors, and interpolated and "
             "extrapolated linearly between mesh\n"
             "points.","[[]]"),
      gtmin("a vector containing one entry for each "
            "channel/band which is the input sound\n"
            "level in dB SPL for which first gain "
            "in the corresponding row of gain table\n"
            "gtdata is applied to amplify the signal","[]"),
      gtstep("input sound level difference in dB between "
             "table columns in the corresponding row\n"
             "of gain table gtdata.  I.e. the first "
             "entry in each gtdata row is applied\n"
             "when input level is gtmin dB, the second "
             "entry is applied when input level is\n"
             "gtmin+gtstep dB, etc.  A small step "
             "size (e.g. 1 dB) is recommended to avoid\n"
             "undesired effects of the linear interpolation","[]"),
      taurmslevel("RMS level averaging time constant in s","[]"),
      tauattack("attack time constant in s","[]"),
      taudecay("decay time constant in s","[]"),
      offset("level offset for each band in dB. If this vector is non-empty,\n"
             "the computed input level are adjusted by the offset values\n"
             "in this vector before the gains are looked up in the gaintable.","[]"),
      filterbank("Name of fftfilterbank plugin."
                 "  Used to extract frequency information.",
                 "fftfilterbank"),
      chname("name of audio channel number variable (empty: broadband)",""),
      bypass("bypass dynamic compression", "no"),
      log_interp("use logarithmic interpolation of gaintable entries","no"),
      clientid("Client ID of last fit",""),
      gainrule("Gain rule of last fit",""),
      preset("Preset name of last fit",""),
      input_level("input level of last block / dB SPL"),
      filtered_level("input level of last block after time-constant filters / dB SPL"),
      center_frequencies("nominal center frequencies of filterbank bands"),
      edge_frequencies("edge frequencies of filterbank bands"),
      band_weights("Weights of the individual frequency bands.\n"
                   "Computed as (sum of squared fft-bin-weigths) / num_frames.")
{
    p.set_node_id("dc");
    p.insert_member(gtmin);
    p.insert_member(gtstep);
    p.insert_member(gtdata);
    p.insert_item("tau_rmslev",&taurmslevel);
    p.insert_item("tau_attack",&tauattack);
    p.insert_item("tau_decay",&taudecay);
    p.insert_item("level_offset",&offset);
    p.insert_item("fb", &filterbank);
    p.insert_member(chname);
    p.insert_member(bypass);
    p.insert_member(log_interp);
    p.insert_member(clientid);
    p.insert_member(gainrule);
    p.insert_member(preset);
    p.insert_item("level_in", &input_level);
    p.insert_item("level_in_filtered", &filtered_level);
    p.insert_item("cf", &center_frequencies);
    p.insert_item("ef", &edge_frequencies);
    p.insert_member(band_weights);
}


MHAPLUGIN_CALLBACKS(dc,dc::dc_if_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(dc,dc::dc_if_t,spec,spec)
MHAPLUGIN_DOCUMENTATION\
(dc,
 "compression level-modification",
 "\\MHAfigure{Input-output function of one channel in the dc dynamic"
 " compression algorithm.}{dc_in_out}"
 "The plugin {\\em dc} is a multiband dynamic range compression plugin.\n"
 "One compression function (input-output function) is applied to\n"
 "each audio channel.\n"
 "Frequency-dependent compression can be achieved by using the\n"
 "{\\em fftfilterbank} plugin in conjunction with this plugin, which\n"
 "separates broadband audio channels into multiple frequency bands.\n"
 "\n"
 "The input-level dependent gain function is configured by a gain table"
 " containing the gain values in dB applied in different \n"
 " channels and frequency bands.  When a gain table is read by the {\\em dc}"
 " plugin the gains contained in the gain table are converted from dB gains"
 " to linear factors."
 " The variable \\texttt{log\\_interp} controls if the gain values are interpolated linearly"
 " or logarithmically, the default is linear interpolation."
 " For input levels outside of the range covered by the gain table an extrapolation"
 " depending on the variable \\texttt{log\\_interp} on the two nearest points is used."
 "\n\n"
 "The linear interpolation of gains originally given in dB can have undesired "
 "interpolation effects especially for large step sizes \\texttt{gtstep}. "
 "For step sizes \\texttt{gtstep} significantly larger than 1dB, these undesired "
 "interpolation effects should be avoided by switching \\texttt{log\\_interp}. "
 "We provide the mfile tool \\texttt{dc\\_plot\\_io.m} "
 "which can be used to plot the resulting input/output "
 "characteristic resulting from a {\\em dc} gain table configuration with "
 "Octave/Matlab, refer to the inline help in that mfile."
 "\n\n"
 "The following configuration fragment e.g. reproduces the I/O characteristic "
 "shown in \\figref{dc_in_out}. To keep the example readable, a gtstep size of "
 "4 dB was used, reducing the amount of numbers to give here and avoiding "
 "fractional dB gains. An actual configuration should use a step size of 1 dB "
 "and not avoid fractions.  The fitting GUI can be used to configure the dc "
 "plugins, and it uses a 1 dB step size. Refer to the fitting GUI manual "
 "\\href{http://www.openmha.org/docs/openMHA\\_gui\\_manual.pdf}"
 "{openMHA\\_gui\\_manual.pdf}.\n"
 "\n"
 "\\begin{verbatim}\n"
 "algos = [fftfilterbank dc combinechannels]\n"
 "fftfilterbank.ftype = center\n"
 "fftfilterbank.f = [200 2000]\n"
 "fftfilterbank.ovltype = rect\n"
 "dc.gtmin=[16 16]\n"
 "dc.gtstep=[4 4]\n"
 "dc.gtdata = [[37 40 39 38 37 36 35 34 33 32 31 30 26 22 18 14 10 6 2 -2 -2];...\n"
 "             [37 40 39 38 37 36 35 34 33 32 31 30 26 22 18 14 10 6 2 -2 -2];]\n"
 "dc.tau_attack = [0.005 0.005]\n"
 "dc.tau_decay =  [0.060 0.060]\n"
 "dc.combinechannels.name = fftfilterbank_channels\n"
 "\\end{verbatim}\n"
 "In this configuration it is assumed that one audio channel is "
 "configured. All variables of {\\em dc} have two entries, one for "
 "each frequency band.\n"
 "This configuration applies 40 dB gain at 20 dB SPL input level, and 30 dB at "
 "60 dB input level. Above 60 dB input level, it limits the output level to 90 "
 "dB SPL until the output level is softer than the input level.  At low input "
 "levels below 20 dB SPL, it applies a noise gate.  This example configuration "
 "is not a fitting recommendation. Established fitting rules should be used "
 "to derive individual fittings for test subjects depending on the individual "
 "hearing impairment."
 "\n\n"
 "{\\em dc} allows to specify band-specific input level adjustments that are"
 " applied to the measured input levels in the respective bands through the configuration"
 " variable \\texttt{level\\_offset}. "
 " Using this variable, it is e.g. possible to compute the"
 " gain table in LTASS levels rather than physical band levels.\n\n"
 "The input level"
 " (abscissa of the input-output function, cf. \\figref{dc_in_out})\n"
 "is determined by an attack- and release-filter of the short time"
 " RMS level $L_{st}$, given in dB (SPL)."
 " The attack filter $L_a$ is a first order low pass filter."
 " The release filter $L_{in}$ is a maximum tracker, i.e.\n"
 "\\begin{eqnarray}\n"
 "L_{a} &=& \\left\\langle 20\\log_{10}(L_{st}) \\right\\rangle_{\\tau_{attack}}\\\\\n"
 "L_{in} &=& {\\textrm{max}}(L_{a},\\left\\langle L_{a} \\right\\rangle_{\\tau_{release}})\n"
 "\\end{eqnarray}\n"
 "The gain table is given as a matrix with $n_{f} \\cdot n_{ch}$ rows,"
 " where $n_{f}$ is\n"
 "the number of frequency bands and $n_{ch}$ is the number of channels."
 " The order of row indices is \n"
 "$0...n_{f}...n_{f} \\cdot n_{ch}$. The x-values for the n-th column"
 " of the gain table are given as $x_n=gtmin+gtstep\\cdot n$."
 )
// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
