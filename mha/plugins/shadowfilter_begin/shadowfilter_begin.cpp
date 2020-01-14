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

namespace shadowfilter_begin {

class cfg_t {
public:
    cfg_t(int nfft, int inch, int outch, algo_comm_t ac, std::string name);
    mha_spec_t* process(mha_spec_t*);
private:
    MHA_AC::spectrum_t in_spec_copy;
    MHASignal::spectrum_t out_spec;
    MHA_AC::int_t nch;
    MHA_AC::int_t ntracks;
};

cfg_t::cfg_t(int nfft, int inch, int outch, algo_comm_t ac, std::string name)
    : in_spec_copy(ac,name,nfft/2+1,inch,true),
      out_spec(nfft/2+1,outch),
      nch(ac,name+"_nch",outch),
      ntracks(ac,name+"_ntracks",inch/outch)
{
    if( inch < outch )
        throw MHA_ErrorMsg("Number of input channels is less than number of output channels.");
}

mha_spec_t* cfg_t::process(mha_spec_t* s)
{
    if( s->num_channels != in_spec_copy.num_channels )
        throw MHA_Error(__FILE__,__LINE__,
                        "shadowfilter_begin: Mismatching channel count (got %u, expected %u).",
                        s->num_channels, in_spec_copy.num_channels );
    if( s->num_frames != in_spec_copy.num_frames )
        throw MHA_Error(__FILE__,__LINE__,
                        "shadowfilter_begin: Mismatching frame count (got %u, expected %u).",
                        s->num_frames, in_spec_copy.num_frames );
    unsigned int kfr, kch;
    in_spec_copy.copy(*s);
    for(kch=0;kch<out_spec.num_channels;kch++)
        for(kfr=0;kfr<out_spec.num_frames;kfr++)
            out_spec.buf[kfr + kch*out_spec.num_frames] = 
                in_spec_copy.buf[kfr + kch*in_spec_copy.num_frames];
    return &out_spec;
}

class shadowfilter_begin_t : public MHAPlugin::plugin_t<cfg_t> {
public:
    shadowfilter_begin_t(const algo_comm_t&,const std::string&,const std::string&);
    mha_spec_t* process(mha_spec_t*);
    void prepare(mhaconfig_t&);
private:
    std::string basename;
    MHAParser::int_t nch;
    MHAParser::int_t ntracks;
};

shadowfilter_begin_t::shadowfilter_begin_t(
                                       const algo_comm_t& iac,
                                       const std::string& ith,
                                       const std::string& ial)
    : MHAPlugin::plugin_t<cfg_t>("Save signal spectrum to AC variable",iac),
      basename(ial),
      nch("number of processing channels","1","[1,["),
      ntracks("number of input sources, each with nch audio channels","1","[1,[")
{
    insert_item("nch",&nch);
    insert_item("ntracks",&ntracks);
}

mha_spec_t* shadowfilter_begin_t::process(mha_spec_t* s)
{
    poll_config();
    return cfg->process(s);
}

void shadowfilter_begin_t::prepare(mhaconfig_t& tf)
{
    try{
        nch.setlock(true);
        ntracks.setlock(true);
        if( tf.domain != MHA_SPECTRUM )
            throw MHA_ErrorMsg("shadowfilter_begin: Only spectral processing is suported.");
        if( (int)tf.channels != nch.data * ntracks.data )
            throw MHA_Error(__FILE__,__LINE__,
                            "shadowfilter_begin: %d input channels are configured, but %u input channels received.",
                            nch.data * ntracks.data, tf.channels);
        tf.channels = nch.data;
        tftype = tf;
        push_config(new cfg_t(tftype.fftlen,nch.data * ntracks.data,tf.channels,ac,basename));
    }
    catch(...){
        nch.setlock(false);
        ntracks.setlock(false);
        throw;
    }
}

}

MHAPLUGIN_CALLBACKS(shadowfilter_begin,shadowfilter_begin::shadowfilter_begin_t,spec,spec)
MHAPLUGIN_DOCUMENTATION\
(shadowfilter_begin,
 "data-flow feature-extraction filter",
 "The plugins 'shadowfilter\\_begin' and 'shadowfilter\\_end' (section\n"
 "\\ref{plug:shadowfilter_end}) are designed to measure the gains\n"
 "produced by any spectral plugins and apply those gains to audio\n"
 "channels not passed to the algorithm. This method can be used to\n"
 "process a mixed signal, but apply the same gains to the unmixed signal\n"
 "parts seperately. For a stereo mixed signal, this can be done by\n"
 "reading the mixed signal from channels 1 and 2, the desired signal\n"
 "from channels 3 and 4, and the competing signal from channels 5 and\n"
 "6. The 'shadowfilter\\_begin' plugin hides channels 3 to 6 from the\n"
 "plugin, and remembers the input spectrae for all channels. The\n"
 "'shadowfilter\\_end' plugin compares the processed output signal\n"
 "(channels 1 and 2) with its input spectrum and derives complex gains\n"
 "produced by the algorithm (without any knowledge of the algorithm).\n"
 "The same gains are applied to channels 3 to 6."
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// End:
