// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2008 2009 2010 2011 2014 2015 2016 HörTech gGmbH
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

#include <limits>
#include <algorithm>
#include "mha_plugin.hh"
#include "mha_tablelookup.hh"
#include "mha_filter.hh"

namespace dc {

    using namespace MHAPlugin;

    class wideband_inhib_vars_t;

    class wb_inhib_cfg_t {
    public:
        wb_inhib_cfg_t(const wideband_inhib_vars_t& vars);
        std::vector<float> weights;
        float dl_map_min;
        float dl_map_max;
        float dl_diff;
        float l_min;
        std::vector<std::vector<float> > g_scale;
    };

    class wideband_inhib_vars_t : public MHAParser::parser_t, public MHAPlugin::config_t<wb_inhib_cfg_t> {
    public:
        wideband_inhib_vars_t();
        MHAParser::vfloat_t weights;
        MHAParser::float_t dl_map_min;
        MHAParser::float_t dl_map_max;
        MHAParser::float_t l_min;
        MHAParser::mfloat_t g_scale;
        void setchannels(unsigned int ch,unsigned int bnds){channels=ch;bands=bnds;update();};// no real-time function
        MHAEvents::patchbay_t<wideband_inhib_vars_t> patchbay;
        wb_inhib_cfg_t* current() {return poll_config();};
        void update();
        unsigned int channels;
        unsigned int bands;
    };

    class dc_vars_t {
    public:
        dc_vars_t(MHAParser::parser_t&);
        MHAParser::bool_t powersum;
        MHAParser::mfloat_t gtdata;
        MHAParser::vfloat_t gtmin;
        MHAParser::vfloat_t gtstep;
        MHAParser::vfloat_t taurmslevel;
        MHAParser::vfloat_t tauattack;
        MHAParser::vfloat_t taudecay;
        MHAParser::string_t filterbank;
        std::string cf_name, ef_name, bw_name;
        MHAParser::string_t chname;
        MHAParser::bool_t bypass;
        MHAParser::string_t clientid;
        MHAParser::string_t gainrule;
        MHAParser::string_t preset;
        MHAParser::int_mon_t modified;
        MHAParser::mfloat_t max_level_difference;
        MHAParser::vfloat_mon_t input_level;
        MHAParser::vfloat_mon_t filtered_level;
        MHAParser::vfloat_mon_t center_frequencies;
        MHAParser::vfloat_mon_t edge_frequencies;
        MHAParser::vfloat_mon_t band_weights;
        MHAParser::bool_t use_wbinhib;
    };

    class dc_vars_validator_t {
    public:
        dc_vars_validator_t(dc_vars_t & v,
                            unsigned int s,
                            mha_domain_t domain);
    };

    class dc_t : private dc_vars_validator_t {
    public:
        dc_t(dc_vars_t vars,
             mha_real_t filter_rate,
             unsigned int nch,
             algo_comm_t ac,
             mha_domain_t domain,
             unsigned int fftlen,
             std::string algo);
        mha_wave_t* process(mha_wave_t*);
        mha_spec_t* process(mha_spec_t*, wb_inhib_cfg_t* wbinhib);

        void explicit_insert();

        /** Number of frequency bands accessor. */
        unsigned get_nbands() const {return nbands;}
        const MHASignal::waveform_t & get_level_in_db() const
        {return level_in_db;}
        const MHASignal::waveform_t & get_level_in_db_adjusted() const
        {return level_in_db_adjusted;}
    private:
        std::vector<MHATableLookup::linear_table_t> gt;
        MHAFilter::o1flt_lowpass_t rmslevel;
        MHAFilter::o1flt_lowpass_t attack;
        MHAFilter::o1flt_maxtrack_t decay;
        bool powersum;
        bool bypass;
        unsigned int naudiochannels;
        unsigned int nbands;
        MHA_AC::waveform_t level_in_db;
        MHA_AC::waveform_t level_in_db_adjusted;
        MHA_AC::waveform_t inhib_gain;
        MHASignal::waveform_t max_level_difference;
        unsigned int k_nyquist;
    };

    class dc_if_t : public plugin_t<dc_t>, public dc_vars_t {
    public:
        dc_if_t(const algo_comm_t& ac_,
                const std::string& th_,
                const std::string& al_);
        void prepare(mhaconfig_t& tf);
        mha_wave_t* process(mha_wave_t*);
        mha_spec_t* process(mha_spec_t*);
    private:
        /** Called from within the processing routines: updates the monitor
         * variables. */
        void update_monitors();

        /** Called by MHA configuration change event mechanism: creates new
         * runtime configuration */
        void update();

        std::string algo;
        wideband_inhib_vars_t wbinhib;
        MHAEvents::patchbay_t<dc_if_t> patchbay;
    };

    dc_if_t::dc_if_t(const algo_comm_t& ac_,
                     const std::string& th_,
                     const std::string& al_)
        : plugin_t<dc_t>("dynamic compression",ac_),
        dc_vars_t(static_cast<MHAParser::parser_t&>(*this)),
        algo(al_)
    {
        patchbay.connect(&writeaccess,this,&dc_if_t::update);
        insert_member(wbinhib);
    }

    void dc_if_t::update()
    {
        if( tftype.channels ){
            float filter_rate;
            if( tftype.domain == MHA_WAVEFORM )
                filter_rate = tftype.srate;
            else
                filter_rate = tftype.srate / tftype.fragsize;
            push_config(new dc_t(static_cast<const dc_vars_t&>(*this),
                                 filter_rate,
                                 tftype.channels,
                                 ac,
                                 tftype.domain,tftype.fftlen,algo));
        }
    }

    void dc_if_t::prepare(mhaconfig_t& tf)
    {
        tftype = tf;
        update();
        poll_config();
        cfg->explicit_insert();
        size_t fbands = cfg->get_nbands();
        wbinhib.setchannels(tf.channels/fbands,fbands);
        input_level.data.resize(tf.channels);
        filtered_level.data.resize(tf.channels);
        center_frequencies.data.resize(fbands);
        edge_frequencies.data.resize(fbands+1);
        band_weights.data.resize(fbands);
        cf_name = filterbank.data + "_cf";
        ef_name = filterbank.data + "_ef";
        bw_name = filterbank.data + "_band_weights";
        update_monitors();
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
        if( use_wbinhib.data )
            s_out = cfg->process(s_in,wbinhib.current());
        else
            s_out = cfg->process(s_in,NULL);
        update_monitors();
        return s_out;
    }

    dc_vars_validator_t::dc_vars_validator_t(dc_vars_t& v,
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
                            "Invalid gaintable size (found %d, expected %d).",
                            v.gtdata.data.size(),s);
        if( v.gtmin.data.size() != s )
            throw MHA_Error(__FILE__,__LINE__,
                            "Invalid gaintable minimum vector size (found %d, expected %d).",
                            v.gtmin.data.size(),s);
        if( v.gtstep.data.size() != s )
            throw MHA_Error(__FILE__,__LINE__,
                            "Invalid gaintable stepsize vector size (found %d, expected %d).",
                            v.gtstep.data.size(),s);
        if( (domain == MHA_WAVEFORM) && (v.taurmslevel.data.size() != s) )
            throw MHA_Error(__FILE__,__LINE__,
                            "Invalid rms level time constant vector size (found %d, expected %d).",
                            v.taurmslevel.data.size(),s);
        if( v.tauattack.data.size() != s )
            throw MHA_Error(__FILE__,__LINE__,
                            "Invalid attack time constant vector size (found %d, expected %d).",
                            v.tauattack.data.size(),s);
        if( v.taudecay.data.size() != s )
            throw MHA_Error(__FILE__,__LINE__,
                            "Invalid decay time constant vector size (found %d, expected %d).",
                            v.taudecay.data.size(),s);
    }

    unsigned int get_audiochannels(unsigned int totalchannels,std::string acname,algo_comm_t ac)
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

    dc_t::dc_t(dc_vars_t vars,
               mha_real_t filter_rate,
               unsigned int nch,
               algo_comm_t ac,
               mha_domain_t domain,
               unsigned int fftlen,
               std::string algo)
        : dc_vars_validator_t(vars, nch, domain),
          rmslevel(vars.taurmslevel.data, filter_rate, pow(10,65*0.1) * 4e-10),
          attack(vars.tauattack.data, filter_rate, 65),
          decay(vars.taudecay.data, filter_rate, 65),
          powersum(vars.powersum.data),
          bypass(vars.bypass.data),
          naudiochannels(get_audiochannels(nch,vars.chname.data,ac)),
          nbands(nch / naudiochannels),
          level_in_db(ac,algo+"_l_in",nbands,naudiochannels,false),
          level_in_db_adjusted(ac,algo+"_l_in_adj",nbands,naudiochannels,false),
          inhib_gain(ac,algo+"_inhibg",nbands,naudiochannels,false),
          max_level_difference(nbands-1,naudiochannels)
    {
        if( nbands * naudiochannels != nch )
            throw MHA_Error(__FILE__,__LINE__,
                            "Mismatching channel configuration (%d bands, %d input channels, %d audio channels)",
                            nbands,nch,naudiochannels);

        unsigned int k,klev, kfb;
        // max_level_difference is only used in multi-band compressors:
        if( nbands > 1 ){
            // no values given, use 1000 dB as default:
            if( (vars.max_level_difference.data.size() == 0) ||
                (vars.max_level_difference.data[0].size() == 0) ){
                for( k=0; k<naudiochannels; k++ )
                    for(kfb=0;kfb<nbands-1;kfb++)
                        max_level_difference.value(kfb,k) = 100000;
            }else{
                if( vars.max_level_difference.data.size() != naudiochannels )
                    throw MHA_Error(__FILE__,__LINE__,
                                    "max_level_difference needs %d rows\n"
                                    "(one for each audio channel)",
                                    naudiochannels);
                for( k=0; k<naudiochannels; k++ ){
                    if( vars.max_level_difference.data[k].size() != (nbands-1) )
                        throw MHA_Error(__FILE__,__LINE__,
                                        "max_level_difference needs %d columns\n"
                                        "(nbands-1, found %d)",
                                        nbands-1,
                                        vars.max_level_difference.data[k].size());
                    for(kfb=0;kfb<nbands-1;kfb++)
                        max_level_difference.value(kfb,k) = vars.max_level_difference.data[k][kfb];
                }
            }
        }
        gt.resize(nch);
        mha_real_t inlev;
        for(k=0; k<nch; k++){
            gt[k].clear();
            inlev = vars.gtmin.data[k];
            for(klev=0;klev<vars.gtdata.data[k].size();klev++){
                gt[k].add_entry(pow(10,0.05*vars.gtdata.data[k][klev]));
                inlev += vars.gtstep.data[k];
            }
            gt[k].set_xmin( vars.gtmin.data[k] );
            gt[k].set_xmax( inlev );
            gt[k].prepare();
        }
        if( 2*(unsigned int)(fftlen/2) == fftlen )
            k_nyquist = fftlen/2;
        else
            k_nyquist = fftlen/2+1;
    }

    void dc_if_t::update_monitors()
    {
        unsigned int band, ch;
        const unsigned int fbands = cfg->get_nbands();
        const unsigned int naudiochannels = tftype.channels / fbands;
        const mha_real_t * cf = 0, * ef = 0, * bw = 0;
        comm_var_t v;
        if (ac.get_var(ac.handle, cf_name.c_str(), &v) == 0
            && v.data_type == MHA_AC_MHAREAL
            && v.num_entries == fbands)
            cf = (mha_real_t *)v.data;
        if (ac.get_var(ac.handle, ef_name.c_str(), &v) == 0
            && v.data_type == MHA_AC_MHAREAL
            && v.num_entries == (fbands+1U))
            ef = (mha_real_t *)v.data;
        if (ac.get_var(ac.handle, bw_name.c_str(), &v) == 0
            && v.data_type == MHA_AC_MHAREAL
            && v.num_entries == (fbands))
            bw = (mha_real_t *)v.data;
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
        inhib_gain.insert();
    }

    mha_wave_t* dc_t::process(mha_wave_t* s)
    {
        explicit_insert();
        mha_real_t level_in, gain;
        unsigned int k, ch, kfb, idx, ch_idx = 0;
        if( s->num_channels != gt.size() )
            throw MHA_Error(__FILE__,__LINE__,
                            "The audio channel number changed from %d to %d.",
                            gt.size(), s->num_channels);
        for(k=0;k<s->num_frames;k++){
            for(kfb=0;kfb<nbands;kfb++){
                level_in = 0;
                if( powersum ){
                    for(ch=0;ch<naudiochannels;ch++){
                        idx = k*s->num_channels + kfb + nbands*ch;
                        level_in += s->buf[idx] * s->buf[idx];
                    }
                }
                for(ch=0;ch<naudiochannels;ch++){
                    ch_idx = kfb + nbands*ch;
                    idx = k*s->num_channels + ch_idx;
                    if( !powersum )
                        level_in = s->buf[idx]*s->buf[idx];
                    level_in_db.value(kfb,ch) =
                        rmslevel( ch_idx, level_in ) / 4e-10;
                    if( level_in_db.value(kfb,ch) < 1e-10 )
                        level_in_db.value(kfb,ch) = 1e-10;
                    level_in_db.value(kfb,ch) = decay(ch_idx, attack( ch_idx, 10.0*log10( level_in_db.value(kfb,ch) ) ) );

                    if (bypass) continue;

                    gain = gt[ch_idx].interp( level_in_db.value(kfb,ch) );
                    if( gain < 0 )
                        gain = 0;
                    s->buf[idx] *= gain;
                }
            }
        }
        return s;
    }

    mha_spec_t* dc_t::process(mha_spec_t* s, wb_inhib_cfg_t* wbinhib)
    {
        level_in_db.insert();
        level_in_db_adjusted.insert();
        mha_real_t level_in, gain;
        unsigned int k, ch, kfb, ch_idx;
        if( s->num_channels != gt.size() )
            throw MHA_Error(__FILE__,__LINE__,
                            "The audio channel number changed from %d to %d.",
                            gt.size(), s->num_channels);
        mha_real_t pscale = 1;
        for(kfb=0;kfb<nbands;kfb++){
            level_in = 0;
            if( powersum )
                for(ch=0;ch<naudiochannels;ch++){
                    ch_idx = kfb + nbands*ch;
                    for(k=0;k<s->num_frames;k++){
                        if( (k==0) || (k==k_nyquist) )
                            pscale = 0.5;
                        else
                            pscale = 2.0;
                        level_in += pscale*abs2(value(s,k,ch_idx));
                    }
                }
            for(ch=0;ch<naudiochannels;ch++){
                ch_idx = kfb + nbands*ch;
                if( !powersum ){
                    level_in = 0;
                    for(k=0;k<s->num_frames;k++){
                        if( (k==0) || (k==k_nyquist) )
                            pscale = 0.5;
                        else
                            pscale = 2.0;
                        level_in += pscale*abs2(value(s,k,ch_idx));
                    }
                }
                level_in /= 4e-10;
                if( level_in < 1e-10 )
                    level_in = 1e-10;
                level_in_db.value(kfb,ch) =
                    decay(ch_idx,attack(ch_idx,10.0*log10(level_in)));
            }
        }
        level_in_db_adjusted.copy(level_in_db);
        if( nbands > 1 ){
            // apply max level differences between adjacent bands
            for(ch=0;ch<naudiochannels;ch++){
                // adjust level in higher bands
                for(kfb=0; kfb < nbands-1; kfb++ ){
                    level_in_db_adjusted.value(kfb+1,ch) =
                        std::max(level_in_db.value(kfb+1,ch),
                                 level_in_db_adjusted.value(kfb,ch) -
                                 max_level_difference.value(kfb,ch));
                }
                // adjust level in lower bands
                for(kfb=nbands-1; kfb > 0; kfb--){
                    level_in_db_adjusted.value(kfb-1,ch) =
                        std::max(level_in_db_adjusted.value(kfb-1,ch),
                                 level_in_db_adjusted.value(kfb,ch) -
                                 max_level_difference.value(kfb-1,ch));
                }
            }
        }
        if( wbinhib ){
            int kmin = -(int)(wbinhib->weights.size())/2;
            int kmax = (int)(wbinhib->weights.size())+kmin-1;
            for(ch=0;ch<naudiochannels;ch++){
                for(kfb=0;kfb<nbands;kfb++){
                    float bbratio;
                    float lbb(0.0f);
                    float w;
                    float wtotal = 0;
                    for( int kdelta=std::max(kmin,-(int)kfb);kdelta<=std::min(kmax,(int)(nbands-kfb-1));kdelta++){
                        w = wbinhib->weights[kdelta-kmin];
                        lbb += w*level_in_db_adjusted.value(kfb+kdelta,ch);
                        wtotal += w;
                    }
                    bbratio = level_in_db_adjusted.value(kfb,ch) - lbb/wtotal;
                    if( bbratio < wbinhib->dl_map_min )
                        bbratio = wbinhib->dl_map_min;
                    if( bbratio > wbinhib->dl_map_max )
                        bbratio = wbinhib->dl_map_max;
                    bbratio = (bbratio-wbinhib->dl_map_min)/wbinhib->dl_diff;
                    bbratio = (1-bbratio)*wbinhib->g_scale[ch][kfb]*std::max(0.0f,level_in_db_adjusted.value(kfb,ch)-wbinhib->l_min);
                    inhib_gain(kfb,ch) = powf(10.0,-0.05*bbratio);
                }
            }
        }
        // apply gains:
        if (bypass) return s;
        for(ch=0;ch<naudiochannels;ch++)
            for(kfb=0;kfb<nbands;kfb++){
                ch_idx = kfb + nbands*ch;
                gain = std::min(gt[ch_idx].interp(level_in_db.value(kfb,ch)),
                                gt[ch_idx].interp(level_in_db_adjusted.value(kfb,ch)));
                if( gain < 0 )
                    gain = 0;
                if( wbinhib && (gain > 1) ){
                    gain *= inhib_gain(kfb,ch);
                    if( gain < 1.0f )
                        gain = 1.0f;
                }
                for(k=0;k<s->num_frames;k++){
                    value(s,k,ch_idx) *= gain;
                }
            }
        return s;
    }

    dc_vars_t::dc_vars_t(MHAParser::parser_t& p)
        : powersum("Input level is summed accross channels","no"),
          gtdata("gaintable data in dB gains","[[]]"),
          gtmin("input level for first gain entry in dB SPL","[]"),
          gtstep("level step size in dB","[]"),
          taurmslevel("RMS level averaging time constant in s","[]"),
          tauattack("attack time constant in s","[]"),
          taudecay("decay time constant in s","[]"),
          filterbank("Name of fftfilterbank plugin."
                     "  Used to extract frequency information.",
                     "fftfilterbank"),
          chname("name of audio channel number variable (empty: broadband)",""),
          bypass("bypass dynamic compression", "no"),
          clientid("Client ID of last fit",""),
          gainrule("Gain rule of last fit",""),
          preset("Preset name of last fit",""),
          modified("Flag if configuration has been modified"),
          max_level_difference("maximum level difference in dB between adjacent bands","[]","[0,["),
          input_level("input level of last block / dB SPL"),
          filtered_level("input level after time-constant filters / dB SPL"),
          center_frequencies("nominal center frequencies of filterbank bands"),
          edge_frequencies("edge frequencies of filterbank bands"),
          band_weights("Weights of the individual frequency bands.\n"
                       "Computed as (sum of squared fft-bin-weigths) / num_frames."),
          use_wbinhib("Use wideband inhibition?","no")
    {
        p.set_node_id("dc");
        p.insert_member(powersum);
        p.insert_member(gtdata);
        p.insert_member(gtmin);
        p.insert_member(gtstep);
        p.insert_item("tau_rmslev",&taurmslevel);
        p.insert_item("tau_attack",&tauattack);
        p.insert_item("tau_decay",&taudecay);
        p.insert_item("fb", &filterbank);
        p.insert_member(chname);
        p.insert_member(bypass);
        p.insert_member(clientid);
        p.insert_member(gainrule);
        p.insert_member(preset);
        p.insert_member(modified);
        p.insert_member(max_level_difference);
        p.insert_item("level_in", &input_level);
        p.insert_item("level_in_filtered", &filtered_level);
        p.insert_item("cf", &center_frequencies);
        p.insert_item("ef", &edge_frequencies);
        p.insert_member(band_weights);
        p.insert_member(use_wbinhib);
    }

    wideband_inhib_vars_t::wideband_inhib_vars_t()
        : weights("weighting of neighbour frequencies","[1 2 4 4 4 2 1]"),
          dl_map_min("mapping of level/broadband level ratio (in dB) into inhibition ratio, lower value means full inhibition","-8"),
          dl_map_max("mapping of level/broadband level ratio (in dB) into inhibition ratio, upper value means no inhibition","2"),
          l_min("level threshold, inhibition starts above this level","40"),
          g_scale("scaling factor of inhibition gain","[[1]]"),
          channels(0),
          bands(0)
    {
        insert_member(weights);
        insert_member(dl_map_min);
        insert_member(dl_map_max);
        insert_member(l_min);
        insert_member(g_scale);
        patchbay.connect(&writeaccess,this,&wideband_inhib_vars_t::update);
        update();
    }

    void wideband_inhib_vars_t::update()
    {
        if( channels > 0 )
            push_config(new wb_inhib_cfg_t(*this));
    }


    wb_inhib_cfg_t::wb_inhib_cfg_t(const wideband_inhib_vars_t& vars)
        : weights(vars.weights.data),
          dl_map_min(vars.dl_map_min.data),
          dl_map_max(vars.dl_map_max.data),
          dl_diff(dl_map_max-dl_map_min),
          l_min(vars.l_min.data),
          g_scale(vars.g_scale.data)
    {
        if( g_scale.size() == 1 ){
            for(unsigned int k=1;k<vars.channels;k++)
                g_scale.push_back(g_scale[0]);
        }
        for( unsigned int ch=0;ch<g_scale.size();ch++){
            if( g_scale[ch].size() == 1){
                for( unsigned int k=1;k<vars.bands;k++)
                    g_scale[ch].push_back(g_scale[ch][0]);
            }
        }
        if( g_scale.size() != vars.channels )
            throw MHA_Error(__FILE__,__LINE__,"Invalid number of channels in g_scale variable (wideband inhibition [const]): got %d, expected %d.",g_scale.size(),vars.channels);
        for( unsigned int ch=0;ch<g_scale.size();ch++){
            if( g_scale[ch].size() != vars.bands )
                throw MHA_Error(__FILE__,__LINE__,"Invalid number of bands in g_scale[%d] (wideband inhibition [const]): got %d, expected %d.",ch,g_scale[ch].size(),vars.bands);
        }
    }

}

MHAPLUGIN_CALLBACKS(dc,dc::dc_if_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(dc,dc::dc_if_t,spec,spec)
MHAPLUGIN_DOCUMENTATION(dc,"level compression",
                        "\\MHAfigure{Input-output function of one channel in the dc dynamic compression algorithm.}{dc_in_out}"
                        "The plugin {\\em dc} is a multiband dynamic range compression plugin.\n"
                        "One compression function (input-output function) is applied to\n"
                        "each audio channel. Frequency-dependent compression can be achieved by using the\n"
                        "{\\em fftfilterbank} plugin in conjunction with this plugin. \n"
                        "The input-level dependent gain function is determined by a gain table containing the gain values applied in different \n"
                        " channels and frequency bands. Between the points of the gain table linear interpolation is used."
                        " For gains outside of the range of the gaintable a linear extrapolation based on the two nearest points is used."
                        "\n"
                        "If spectral processing is used, the input level (abscissa of the input-output function, cf. \\figref{dc_in_out})\n"
                        "is determined by an attack- and release-filter of the short time RMS level $L_{st}$, given in dB (SPL)."
                        " The attack filter $L_a$ is a first order low pass filter. The release filter $L_{in}$ is a maximum tracker, i.e.\n"
                        "\\begin{eqnarray}\n"
                        "L_{a} &=& \\left\\langle 20\\log_{10}(L_{st}) \\right\\rangle_{\\tau_{attack}}\\\\\n"
                        "L_{in} &=& {\\textrm{max}}(L_{a},\\left\\langle L_{a} \\right\\rangle_{\\tau_{release}})\n"
                        "\\end{eqnarray}\n"
                        "The gain table is given as a matrix with $n_{f} \\cdot n_{ch}$ rows, where $n_{f}$ is\n"
                        "the number of frequency bands and $n_{ch}$ is the number of channels. The order of row indices is \n"
                        "$0...n_{f}...n_{f} \\cdot n_{ch}$. The x-values for the n-th column of the gain table are given as $x_n=gtmin+gtstep\\cdot n$."
                        " See also \\figref{dc_in_out}.\n A configuration fragment reproducing \\figref{dc_in_out} could be:\n"
                        "\n"
                        "\\begin{verbatim}\n"
                        "algos = [fftfilterbank dc combinechannels]\n"
                        "fftfilterbank.ftype = center\n"
                        "fftfilterbank.f = [200 2000]\n"
                        "fftfilterbank.ovltype = rect\n"
                        "fftfilterbank.fscale = bark\n"
                        "dc.gtmin=[20 20]\n"
                        "dc.gtstep=[40 40]\n"
                        "dc.gtdata = [[40 30 -10];[40 30 -10];]\n"
                        "dc.tau_attack = [0.005 0.005]\n"
                        "dc.tau_rmslev = [0.005 0.005]\n"
                        "dc.tau_decay = [0.015 0.015]\n"
                        "dc.combinechannels.name = fftfilterbank_channels\n"
                        "\\end{verbatim}\n"
                        "\n"
                        "In this configuration it is assumed that one audio channel is\n"
                        "configured. All variables of {\\em dc} have two entries, one for"
                        " each frequency band.\n"
                        )
// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
