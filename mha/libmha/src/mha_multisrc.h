// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2013 2016 HörTech gGmbH
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

#ifndef MHA_MULTISRC_H
#define MHA_MULTISRC_H

#include "mha_plugin.hh"

namespace MHAMultiSrc {

    class channel_t {
    public:
        std::string name;
        int channel;
    };
        
    class channels_t : public std::vector<MHAMultiSrc::channel_t> {
    public:
        channels_t(const std::vector<std::string>& src,int in_channels);
    };

    class base_t : protected MHAPlugin::config_t<MHAMultiSrc::channels_t> {
    public:
        base_t(algo_comm_t iac);
        void select_source(const std::vector<std::string>& src,int in_channels);
    protected:
        algo_comm_t ac;
    };

    class waveform_t : public MHA_AC::waveform_t, public MHAMultiSrc::base_t {
    public:
        waveform_t(algo_comm_t iac,
                   std::string name,
                   unsigned int frames,
                   unsigned int channels);
        mha_wave_t* update(mha_wave_t* s);
    };

    class spectrum_t : public MHA_AC::spectrum_t, public MHAMultiSrc::base_t {
    public:
        spectrum_t(algo_comm_t iac,
                   std::string name,
                   unsigned int frames,
                   unsigned int channels);
        mha_spec_t* update(mha_spec_t* s);
    };
        
}

#endif

/*
 * Local Variables:
 * mode: c++
 * coding: utf-8-unix
 * End:
 */
