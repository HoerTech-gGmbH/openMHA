// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2009 2010 2013 2014 2015 2017 2018 HörTech gGmbH
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

class iirfilter_t : public MHAFilter::iir_filter_t {
public:
    iirfilter_t(const algo_comm_t&,const std::string&,const std::string&);
    void prepare_(mhaconfig_t&);
    void release_(){};
    mha_wave_t* process(mha_wave_t*);
private:
};

iirfilter_t::iirfilter_t(const algo_comm_t& ac,
                         const std::string& th,
                         const std::string& al)
    : MHAFilter::iir_filter_t("IIR filter")
{
}

void iirfilter_t::prepare_(mhaconfig_t& tf)
{
    if( tf.domain != MHA_WAVEFORM )
        throw MHA_ErrorMsg("iirfilter: Only waveform processing is supported.");
    resize(tf.channels);
}

mha_wave_t* iirfilter_t::process(mha_wave_t* s)
{
    filter(s,s);
    return s;
}

MHAPLUGIN_CALLBACKS(iirfilter,iirfilter_t,wave,wave)
    MHAPLUGIN_DOCUMENTATION(iirfilter,
        "filter lti",
"The 'iirfilter' plugin implements a generic IIR filter (direct form\n"
"II). The coefficients have the same names as in \\Matlab{}. Due to\n"
"different internal implementations and numeric resolutions, filters\n"
"may be instable with coeffients which are stable in \\Matlab{}.\n"
""      )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
