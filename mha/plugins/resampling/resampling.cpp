// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2012 2013 2014 2015 2018 2019 2021 HörTech gGmbH
// Copyright © 2022 Hörzentrum Oldenburg gGmbH
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

#include "mha_filter.hh"
#include "mha_events.h"
#include "mha_os.h"
#include "mhapluginloader.h"

namespace MHAPlugin_Resampling {

    class resampling_t {
        unsigned outer_fragsize, inner_fragsize;
        float outer_srate, inner_srate;
        unsigned nchannels_in, nchannels_out;
        MHAFilter::blockprocessing_polyphase_resampling_t outer2inner_resampling;
        MHAFilter::blockprocessing_polyphase_resampling_t inner2outer_resampling;
        MHAParser::mhapluginloader_t& plugloader;
        MHASignal::waveform_t inner_signal, output_signal;
        
    public:
        resampling_t(unsigned int outer_fragsize,
                     float outer_srate,
                     unsigned int inner_fragsize,
                     float inner_scrate,
                     unsigned int nch_in,
                     float filter_length_in,
                     unsigned int nch_out,
                     float filter_length_out,
                     float nyquist_ratio,
                     MHAParser::mhapluginloader_t& plug);
        mha_wave_t * process(mha_wave_t *);
    };

    resampling_t::resampling_t(unsigned int _outer_fragsize,
                               float _outer_srate,
                               unsigned int _inner_fragsize,
                               float _inner_srate,
                               unsigned int nch_in,
                               float filter_length_in,
                               unsigned int nch_out,
                               float filter_length_out,
                               float nyquist_ratio,
                               MHAParser::mhapluginloader_t& plug_)
        : outer_fragsize(_outer_fragsize), inner_fragsize(_inner_fragsize),
          outer_srate(_outer_srate), inner_srate(_inner_srate),
          nchannels_in(nch_in), nchannels_out(nch_out),
          outer2inner_resampling(outer_srate, outer_fragsize,
                                 inner_srate, inner_fragsize,
                                 nyquist_ratio, filter_length_in,
                                 nchannels_in, true),
          inner2outer_resampling(inner_srate, inner_fragsize,
                                 outer_srate, outer_fragsize,
                                 nyquist_ratio, filter_length_out,
                                 nchannels_out, false),
          plugloader(plug_),
          inner_signal(inner_fragsize, nchannels_in),
          output_signal(outer_fragsize, nchannels_out)
    {
    }

    mha_wave_t * resampling_t::process(mha_wave_t * s)
    {
        mha_wave_t * inner_result = 0;
        outer2inner_resampling.write(*s);
        while (outer2inner_resampling.can_read()) {
            outer2inner_resampling.read(inner_signal);
            plugloader.process(&inner_signal, &inner_result);
            if (inner_result == 0)
                throw MHA_Error(__FILE__,__LINE__,
                                "resampling_t::process: inner plugin did not"
                                " produce output signal");
            inner2outer_resampling.write(*inner_result);
        }
        inner2outer_resampling.read(output_signal);
        return &output_signal;
    }

    class resampling_if_t : public MHAPlugin::plugin_t< resampling_t > {
    public:
        resampling_if_t(MHA_AC::algo_comm_t & iac, const std::string & configured_name);
        mha_wave_t* process(mha_wave_t*);
        void prepare(mhaconfig_t&);
        void release();
    private:
        MHAParser::float_t srate;
        MHAParser::int_t fragsize;
        MHAParser::float_t nyquist_ratio;
        MHAParser::float_t irslen_outer2inner, irslen_inner2outer;
        MHAParser::mhapluginloader_t plugloader;
        std::string algo;
    };

    resampling_if_t::resampling_if_t(MHA_AC::algo_comm_t & iac,
                                     const std::string & configured_name)
        : MHAPlugin::plugin_t<resampling_t>("Synchronous resampling plugin.",
                                            iac),
          srate("sampling rate of client plugin","44100","]0,]"),
          fragsize("fragment size of client plugin","200","]0,]"), 
          nyquist_ratio("lowpass filter cutoff frequency / lower nyquist frequency","0.85","]0,]"),
          irslen_outer2inner("filter lenth 1st resampling / sec","7e-4","]0,]"),
          irslen_inner2outer("filter lenth 2nd resampling / sec","7e-4","]0,]"),
          plugloader(*this, iac),
          algo(configured_name)
    {
        set_node_id("resampling");
        insert_item("srate",&srate);
        insert_item("fragsize",&fragsize);
        insert_member(nyquist_ratio);
        insert_member(irslen_outer2inner);
        insert_member(irslen_inner2outer);
    }

    void resampling_if_t::prepare(mhaconfig_t& conf)
    {
        if( conf.domain != MHA_WAVEFORM )
            throw MHA_ErrorMsg("resampling: Only waveform data can be processed.");
        // remember the outer fragsize:
        unsigned int outer_fragsize = conf.fragsize;
        float outer_srate = conf.srate;
        unsigned int inner_fragsize = fragsize.data;
        float inner_srate = srate.data;
        unsigned int input_channels = conf.channels;
        conf.fragsize = inner_fragsize;
        conf.srate = inner_srate;
        // sugest configuration to inner plugin, query requirements:
        plugloader.prepare(conf);
        if( conf.domain != MHA_WAVEFORM ) {
            plugloader.release();
            throw MHA_ErrorMsg("resampling: Only waveform data can be processed.");
        }
        // only fixed input/output fragsizes are allowed:
        if( inner_fragsize != conf.fragsize ) {
            plugloader.release();
            throw MHA_ErrorMsg("resampling: Plugin modified the fragment size.");
        }
        if( inner_srate != conf.srate ) {
            plugloader.release();
            throw MHA_ErrorMsg("resampling: Plugin modified the sampling rate.");
        }
        try {
            push_config(new resampling_t(outer_fragsize,
                                         outer_srate,
                                         inner_fragsize,
                                         inner_srate,
                                         input_channels,
                                         irslen_outer2inner.data,
                                         conf.channels,
                                         irslen_inner2outer.data,
                                         nyquist_ratio.data,
                                         plugloader));
        } catch (MHA_Error & e) {
            plugloader.release();
            throw;
        }
        conf.fragsize = outer_fragsize;
        conf.srate = outer_srate;
        fragsize.setlock(true);
        srate.setlock(true);
        nyquist_ratio.setlock(true);
        irslen_outer2inner.setlock(true);
        irslen_inner2outer.setlock(true);
    }

    void resampling_if_t::release()
    {
        fragsize.setlock(false);
        srate.setlock(false);
        nyquist_ratio.setlock(false);
        irslen_outer2inner.setlock(false);
        irslen_inner2outer.setlock(false);
        plugloader.release();
    }

    mha_wave_t* resampling_if_t::process(mha_wave_t* s)
    {
        poll_config();
        return cfg->process(s);
    }
}
MHAPLUGIN_CALLBACKS(resampling,MHAPlugin_Resampling::resampling_if_t,wave,wave)
MHAPLUGIN_DOCUMENTATION\
(resampling,
 "plugin-arrangement signal-transformation",
 "A bridge type resampling plugin.  The signal is converted to target\n"
 "sampling rate and fragment size. The converted signal is processed by the\n"
 "child plugin. The processed signal is then converted back to the original\n"
 "sampling rate and fragment size. The input data is\n"
 "buffered, and the data is processed when enough samples are available.\n"
 "\n"
 "Please note that double buffering adds an extra delay of the audio\n"
 "stream. If both fragment sizes are identical, the double buffering is\n"
 "bypassed.\n"
 "\n"
 "\\paragraph{Warning:}\n"
 "\n"
 "A synchronous resampling ringbuffer such as this causes varying\n"
 "computational loads in the outer processing buffer. It is therefore\n"
 "not real-time safe."
 )
// Local Variables:
// c-basic-offset: 4
// indent-tabs-mode: nil
// compile-command: "make"
// coding: utf-8-unix
// End:
