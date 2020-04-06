// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2008 2009 2010 2013 2014 2015 2017 2018 2019 2020 HörTech gGmbH
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

#include <mha_plugin.hh>
#include <mha_signal.hh>
#include <math.h>
#include <mha_events.h>
#include <mhapluginloader.h>
#include <mha_filter.hh>
#include <mha_windowparser.h>

namespace smoothgains_bridge {

class smoothspec_wrap_t
{
public:
    smoothspec_wrap_t(mhaconfig_t spar_in,
                      mhaconfig_t spar_out,
                      const MHAParser::kw_t& mode,
                      const MHAParser::window_t& irswnd,
                      const MHAParser::float_t& epsilon);
    mha_spec_t* proc_1(mha_spec_t*);
    mha_spec_t* proc_2(mha_spec_t*);
private:
    MHASignal::spectrum_t spec_in_copy; ///< Copy of input spectrum for smoothspec
    MHAFilter::smoothspec_t smoothspec; ///< Smoothspec calculator
    bool use_smoothspec;
    float smoothspec_epsilon;
};

smoothspec_wrap_t::smoothspec_wrap_t(mhaconfig_t spar_in,
                                     mhaconfig_t spar_out,
                                     const MHAParser::kw_t& mode,
                                     const MHAParser::window_t& irswnd,
                                     const MHAParser::float_t& epsilon)
    : spec_in_copy(spar_in.fftlen/2+1,spar_in.channels),
      smoothspec(spar_in.fftlen,spar_in.channels,
                 irswnd.get_window(spar_in.fftlen-spar_in.wndlen+1,
                                   -1*(mode.data.get_index()==1),
                                   1,
                                   (mode.data.get_index()==2),
                                   false),
                 mode.data.get_index()==2),
      use_smoothspec(mode.data.get_index()>0),
      smoothspec_epsilon(epsilon.data)
{
    if( spar_in.channels != spar_out.channels )
        throw MHA_Error(__FILE__,__LINE__,
                        "Smoothing of gains can only be used if number of input channels matches the number of output"
                        " channels (currently %u input channels and %u output channels).",
                        spar_in.channels,spar_out.channels);
    if( spar_in.fragsize != spar_out.fragsize )
        throw MHA_Error(__FILE__,__LINE__,"overlap add sub-plugins are not allowed to change fragment size from %u to %u.",
                        spar_in.fragsize,spar_out.fragsize);
    if( spar_in.wndlen != spar_out.wndlen )
        throw MHA_Error(__FILE__,__LINE__,"overlap add sub-plugins are not allowed to change window lengt from %u to %u.",
                        spar_in.wndlen,spar_out.wndlen);
    if( spar_in.fftlen != spar_out.fftlen )
        throw MHA_Error(__FILE__,__LINE__,"overlap add sub-plugins are not allowed to change FFT length from %u to %u.",
                        spar_in.fftlen,spar_out.fftlen);
}

mha_spec_t* smoothspec_wrap_t::proc_1(mha_spec_t* s)
{
    spec_in_copy.copy( *s );
    return s;
}

mha_spec_t* smoothspec_wrap_t::proc_2(mha_spec_t* s)
{
    if( use_smoothspec ){
        // Compute Impulse response for original (coarse) gains
        //------------------------------------------------------
        // replace input spectrum with gains needed to apply to orig_spec
        // to get in_spec
        safe_div(*s, spec_in_copy, smoothspec_epsilon);
        smoothspec.smoothspec(*s);
        *s *= spec_in_copy;
    }
    return s;
}

class overlapadd_if_t : public MHAPlugin::plugin_t<smoothspec_wrap_t> {
public:
    overlapadd_if_t(const algo_comm_t&,const std::string&,const std::string&);
    ~overlapadd_if_t();
    void prepare(mhaconfig_t&);
    void release();
    mha_spec_t* process(mha_spec_t*);
private:
    void update();

    MHAEvents::patchbay_t<overlapadd_if_t> patchbay;
    MHAParser::kw_t mode;
    MHAParser::window_t irswnd;
    MHAParser::float_t epsilon;
    MHAParser::mhapluginloader_t plugloader;

    std::string algo;
    mhaconfig_t cf_in, cf_out;
};

overlapadd_if_t::overlapadd_if_t(const algo_comm_t& iac,const std::string&,const std::string& ialg)
    : MHAPlugin::plugin_t<smoothspec_wrap_t>("Gain smoothing for reduction of filter length",iac),
      mode("Gain smoothing mode\n\nNote: Appropriate settings of window position are required (linear_phase: 0.5, minimal_phase: 0)\n","linear_phase","[off linear_phase minimal_phase]"),
      irswnd("Impulse response window function"),
      epsilon("Epsilon for safe division by zero (avoid inf)","1e-18","[1.1e-19,]"),
      plugloader(*this,iac),
      algo(ialg),
      cf_in(tftype),cf_out(tftype)
{
    insert_item("mode",&mode);
    insert_item("irswnd",&irswnd);
    insert_item("epsilon",&epsilon);
    patchbay.connect(&mode.writeaccess,this,&overlapadd_if_t::update);
    patchbay.connect(&irswnd.writeaccess,this,&overlapadd_if_t::update);
    patchbay.connect(&epsilon.writeaccess,this,&overlapadd_if_t::update);
}

overlapadd_if_t::~overlapadd_if_t()
{}

void overlapadd_if_t::prepare(mhaconfig_t& t)
{
    if( t.domain != MHA_SPECTRUM )
        throw MHA_ErrorMsg("spectral input is required.");
    cf_in = tftype = t;
    plugloader.prepare(t);
    if( t.domain != MHA_SPECTRUM )
        throw MHA_Error(__FILE__, __LINE__, "The processing"
                        " plugin did not return spectral output.");
    /* prepare */
    cf_out = t;
    update();
    poll_config();
}

void overlapadd_if_t::release()
{
    plugloader.release();
}

void overlapadd_if_t::update()
{
    if( (cf_in.fftlen > 0) &&
        (cf_in.wndlen > 0) &&
        (cf_in.fragsize > 0) && 
        (cf_in.channels > 0) ){
        push_config(new smoothspec_wrap_t(cf_in,cf_out,
                                          mode,irswnd,epsilon));
    }
}

mha_spec_t* overlapadd_if_t::process(mha_spec_t* spec)
{
    poll_config();
    spec = cfg->proc_1(spec);
    plugloader.process(spec,&spec);
    return cfg->proc_2(spec);
}

}

MHAPLUGIN_CALLBACKS(smoothgains_bridge,smoothgains_bridge::overlapadd_if_t,spec,spec)
MHAPLUGIN_DOCUMENTATION\
(smoothgains_bridge,
 "level-modification filter data-flow overlap-add",
 "The overlap-add framework allows finite impulse response filter lengths up to the zero padding length.\n"
 "Longer filters will result in artifacts caused by circular aliasing.\n"
 "Artifacts can be reduced by either applying Hanning ramps to the zero-padded"
 " blocks after filtering,\n"
 "or by shortening the impulse response of the filter,"
 " thereby implicitely reducing the frequency resolution.\n"
 "This plugin reduces the filter length to match exactly the"
 " zero-padding length.\n"
 "It can either keep the phase (mode=linear\\_phase),"
 " and reduce causal and a-causal parts of the impulse response,\n"
 "or apply a minimum phase filter phase,"
 " and cut the causal part of the filter.\n"
 "The window position in the overlap-add framework has to be configured"
 " appropriately: \n"
 "For linear phase mode, a symmetric window position is required, i.e.,"
 " wnd.pos=0.5.\n"
 "To allow minimal phase filters,"
 " an asymmetric window position (wnd.pos=0) is needed.\n"
 "Using minimal phase filters will destroy the phase,"
 " but reduces the algorithmic delay.\n"
 "Using a minimal phase can lead to undesired interference between"
 " subsequent overlapping synthesized frames, also introducing"
 " unwanted sound artifacts.  It should only be used if the filter"
 " applied in the STFT domain does not change or only changes very"
 " slowly.\n"
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
