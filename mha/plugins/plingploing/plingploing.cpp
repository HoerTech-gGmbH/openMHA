// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2009 2010 2013 2014 2015 2016 2018 HörTech gGmbH
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
#include "mha_events.h"
#include "mha_windowparser.cpp"
/** All classes for the plingploing music generator live in this namespace. */
namespace plingploing {
    /** Run-time configuration of the plingploing music generator. */
    class plingploing_t {
    public:
        plingploing_t(mhaconfig_t,mha_real_t level,mha_real_t pitch, mha_real_t k1, mha_real_t k2, mha_real_t i1, mha_real_t i2, mha_real_t bpm, mha_real_t minlen,mha_real_t maxlen,mha_real_t bassmod,mha_real_t bassperiod);
        void process(mha_wave_t*);
    private:
        mhaconfig_t cf;
        mha_real_t pitch_;
        unsigned int bt, t, len;
        mha_real_t dur_;
        mha_real_t minlen_;
        mha_real_t maxlen_;
        mha_real_t bass;
        mha_real_t freq;
        mha_real_t fun1_key;
        mha_real_t fun1_range;
        mha_real_t fun1;
        mha_real_t fun2;
        mha_real_t fun2_key;
        mha_real_t fun2_range;
        mha_real_t dist;
        mha_real_t dist1;
        mha_real_t alph;
        mha_real_t rms;
        mha_real_t bassmod_;
        mha_real_t bassperiod_;
        MHAWindow::hanning_t hann1, hann2;
        mha_real_t level;
    };

    plingploing_t::plingploing_t(mhaconfig_t c,mha_real_t level_,mha_real_t pitch, mha_real_t k1, mha_real_t k2, mha_real_t i1, mha_real_t i2, mha_real_t bpm, mha_real_t minlen,mha_real_t maxlen,mha_real_t bassmod,mha_real_t bassperiod)
        : cf(c),
          pitch_(pitch),
          bt(0),
          t(0),
          len(0),
          dur_(60.0/bpm),
          minlen_(minlen),
          maxlen_(maxlen),
          fun1_key(k1),
          fun1_range(i1),
          fun2_key(k2),
          fun2_range(i2),
          bassmod_(bassmod),
          bassperiod_(bassperiod*cf.srate),
          hann1(200),
          hann2(1000),
          level(level_)
    {
    }

    double drand(double a, double b){
        return a + (b-a)*rand()/(double)RAND_MAX;
    }

    void plingploing_t::process(mha_wave_t* s)
    {
        unsigned int k;
        if( !s )
            return;
        if( !s->num_channels )
            return;
        for(k=0;k<s->num_frames;k++){
            if( bt == len ){
                // create a new chord
                // len is the new duration of the chord, in samples
                len = (unsigned int)(dur_*cf.srate*(int)drand( minlen_, maxlen_ ));
                // bt is a counter used for windowing, reset:
                bt = 0;
                // note of fundamental tone:
                bass = floor( (bassmod_ * sin( M_PI*t/bassperiod_)*sin( M_PI*t/bassperiod_) * rand()/RAND_MAX) ) / 12;
                freq = pitch_ * exp( log(2) * bass );
                // intervals of second and third tone relative to bass:
                fun1 = fun1_key + fun1_range * (int)(2.0*rand()/RAND_MAX);
                fun2 = fun2_key + fun2_range * (int)(2.0*rand()/RAND_MAX);
                fun1 = exp( log(2) * fun1/12.0 );
                fun2 = exp( log(2) * fun2/12.0 );
                // level/amplitude randomization:
                alph = drand(0.3, 4.3 );
                dist = drand(0.8, 1.2 );
                dist1 = drand(0.8, 1.3 );
                rms = drand( 2*alph, 10+2*alph );
                rms = 2e-5*pow(10,0.05*(level + rms - 15));
            }
            // generate next audio sample:
            value(s,k,0) = rms * exp(-alph*(double)bt/len)
                * ( sin(freq * bt * M_PI/cf.srate ) +
                    dist * sin(fun1 * freq * bt * M_PI/cf.srate ) +
                    dist1 * sin(fun2 * freq * bt * M_PI/cf.srate ) );
            if( bt < 100 )
                value(s,k,0) *= hann1[bt];
            if( bt > std::max(len,500u)-500 )
                value( s,k,0) *= hann2[std::max(len,500u)-bt];
            bt++;
            t++;
        }
        // copy signal to all channels:
        for(unsigned int ch=1;ch<s->num_channels;ch++)
            for(k=0;k<s->num_frames;k++)
                value(s,k,ch) = value(s,k,0);
    }
    /** Plugin class of the plingploing music generator */
    class if_t : public MHAPlugin::plugin_t<plingploing_t> {
    public:
        if_t(algo_comm_t,const char*,const char*);
        mha_wave_t* process(mha_wave_t*);
        void prepare(mhaconfig_t& cf);
    private:
        MHAEvents::patchbay_t<if_t> patchbay;
        void update();
        /** Output level in dB SPL */
        MHAParser::float_t level;
        /** Bass pitch in Hz */
        MHAParser::float_t pitch;
        /** Key1 */
        MHAParser::float_t fun1_key;
        /** Range1 */
        MHAParser::float_t fun1_range;
        /** Key 2 */
        MHAParser::float_t fun2_key;
        /** Range 2 */
        MHAParser::float_t fun2_range;
        /** Speed in beats per minute (bpm) */
        MHAParser::float_t bpm;
        /** Minimum note length in beats */
        MHAParser::float_t minlen;
        /** Maximum note length in beats */
        MHAParser::float_t maxlen;
        /** Bass key modulation depth */
        MHAParser::float_t bassmod;
        /** Bass key modulation period */
        MHAParser::float_t bassperiod;
    };

    if_t::if_t(algo_comm_t iac,const char*,const char*)
        : MHAPlugin::plugin_t<plingploing_t>("plingploing algorithm.",iac),
        level("Output level in dB SPL","70"),
        pitch("Bass pitch in Hz","415","[1,]"),
        fun1_key("minimum interval of second tone relative to bass, in semitones","3"),
        fun1_range("randomized interval of second tone, added to fun1_key, in semitones","2","[0,]"),
        fun2_key("minimum interval of third tone relative to bass, in semitones","5"),
        fun2_range("randomized interval of third tone, added to fun2_key, in semitones","2","[0,]"),
        bpm("beats per minute","200","[1,]"),
        minlen("minimum note length / beats","1","[1,]"),
        maxlen("maximum note length / beats","5","[1,]"),
        bassmod("bass key modulation depth","5"),
        bassperiod("bass key modulation period","28")
    {
        insert_member(level);
        insert_member(pitch);
        insert_member(fun1_key);
        insert_member(fun1_range);
        insert_member(fun2_key);
        insert_member(fun2_range);
        insert_member(bpm);
        insert_member(minlen);
        insert_member(maxlen);
        insert_member(bassmod);
        insert_member(bassperiod);
        patchbay.connect(&writeaccess,this,&if_t::update);
    }

    void if_t::update()
    {
        if( is_prepared() )
            push_config(new plingploing_t(tftype,level.data,pitch.data,fun1_key.data,fun2_key.data,fun1_range.data,fun2_range.data,bpm.data,minlen.data,maxlen.data,bassmod.data,bassperiod.data));
    }

    void if_t::prepare(mhaconfig_t& tf)
    {
        if( tf.domain != MHA_WAVEFORM )
            throw MHA_ErrorMsg("plingploing: Only waveform processing is supported.");
        tftype = tf;
        update();
    }

    mha_wave_t* if_t::process(mha_wave_t* s)
    {
        poll_config();
        cfg->process(s);
        return s;
    }

}
MHAPLUGIN_CALLBACKS(plingploing,plingploing::if_t,wave,wave)
MHAPLUGIN_DOCUMENTATION(plingploing,"generator","This plugin creates music (jazz-inspired chord sequence).")


// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
