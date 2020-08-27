// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2007 2013 2016 2017 2020 HörTech gGmbH
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

#include "mha_multisrc.h"

/**

\namespace MHAMultiSrc
\brief Collection of classes for selecting audio chunks from multiple sources

 */

/**
\class MHAMultiSrc::base_t
\brief Base class for source selection 

\see MHAMultiSrc::channel_t
\see MHAMultiSrc::channels_t
 */

/**
\brief Separate a list of input sources into a parsable channel list

The number of input channels if verified, a list of
MHAMultiSrc::channel_t is filled.

@param route vector of source channel ids
@param in_channels number of channels in the processed input signal
 */
MHAMultiSrc::channels_t::channels_t(const std::vector<std::string>& route,int in_channels)
{
    clear();
    MHAMultiSrc::channel_t chn;
    for(unsigned int ch=0;ch<route.size();ch++){
        MHAParser::expression_t src(route[ch],":");
        chn.name = src.lval;
        MHAParser::StrCnv::str2val(src.rval,chn.channel);
        if( chn.channel < 0 )
            throw MHA_Error(__FILE__,__LINE__,
                            "Channel number must be zero ore more (%s).",
                            route[ch].c_str());
        if( (src.lval.size() == 0) && (chn.channel >= in_channels) )
            throw MHA_Error(__FILE__,__LINE__,
                            "Channel number is out of range (%s, %d channels).",
                            route[ch].c_str(), in_channels);
        push_back(chn);
    }
}

MHAMultiSrc::base_t::base_t(algo_comm_t iac)
    : ac(iac)
{
}

/**
\brief Change the selection of input sources

This function is real-time and thread safe.

\param src List of input sources
\param in_channels Number of input channels in direct input
                   (the processed signal)
 */
void MHAMultiSrc::base_t::select_source(const std::vector<std::string>& src, int in_channels)
{
    push_config(new MHAMultiSrc::channels_t(src,in_channels));
}

MHAMultiSrc::waveform_t::waveform_t(algo_comm_t ac,
                                    std::string name,
                                    unsigned int frames,
                                    unsigned int channels)
    : MHA_AC::waveform_t(ac,name,frames,channels,false),
      MHAMultiSrc::base_t(ac)
{
}

/**

\brief Update data of waveform to hold actual input data

\param s Input signal chunk
\return Return pointer to waveform structure
*/
mha_wave_t* MHAMultiSrc::waveform_t::update(mha_wave_t* s)
{
    poll_config();
    unsigned int k, ch;
    mha_wave_t win;
    if( num_channels != cfg->size() )
        throw MHA_Error(__FILE__,__LINE__,
                        "Mismatching dimension: route data has %zu entries, output signal %u.",
                        cfg->size(),num_channels);
    for(ch=0;ch<num_channels;ch++){
        win = *s;
        if( (*cfg)[ch].name.size() )
            win = MHA_AC::get_var_waveform( MHAMultiSrc::base_t::ac, (*cfg)[ch].name );
        if( win.num_frames != num_frames )
            throw MHA_Error(__FILE__,__LINE__,
                            "Invalid number of frames in source \"%s\" (got %u, expected %u)",
                            (*cfg)[ch].name.c_str(),win.num_frames,num_frames);
        if( ((*cfg)[ch].channel >= (int)win.num_channels) || ((*cfg)[ch].channel < 0) )
            throw MHA_Error(__FILE__,__LINE__,
                            "Source channel is out of range (%s, %u channels available).",
                            (*cfg)[ch].name.c_str(),win.num_channels);
        for(k=0;k<num_frames;k++)
            value(k,ch) = ::value(win,k,(*cfg)[ch].channel);
    }
    if( num_channels )
        insert();
    return this;
}

MHAMultiSrc::spectrum_t::spectrum_t(algo_comm_t ac,
                                    std::string name,
                                    unsigned int frames,
                                    unsigned int channels)
    : MHA_AC::spectrum_t(ac,name,frames,channels,false),
      MHAMultiSrc::base_t(ac)
{
}

/**

\brief Update data of spectrum to hold actual input data

\param s Input signal chunk
\return Return pointer to spectrum structure
*/
mha_spec_t* MHAMultiSrc::spectrum_t::update(mha_spec_t* s)
{
    poll_config();
    unsigned int k, ch;
    mha_spec_t win;
    if( num_channels != cfg->size() )
        throw MHA_Error(__FILE__,__LINE__,
                        "Mismatching dimension: route data has %zu entries, output signal %u.",
                        cfg->size(),num_channels);
    for(ch=0;ch<num_channels;ch++){
        win = *s;
        if( (*cfg)[ch].name.size() )
            win = MHA_AC::get_var_spectrum( MHAMultiSrc::base_t::ac, (*cfg)[ch].name );
        if( win.num_frames != num_frames )
            throw MHA_Error(__FILE__,__LINE__,
                            "Invalid number of frames in source \"%s\" (got %u, expected %u)",
                            (*cfg)[ch].name.c_str(),win.num_frames,num_frames);
        if( ((*cfg)[ch].channel >= (int)win.num_channels) || ((*cfg)[ch].channel < 0) )
            throw MHA_Error(__FILE__,__LINE__,
                            "Source channel is out of range (%s, %u channels available).",
                            (*cfg)[ch].name.c_str(),win.num_channels);
        for(k=0;k<num_frames;k++)
            value(k,ch) = ::value(win,k,(*cfg)[ch].channel);
    }
    if( num_channels )
        insert();
    return this;
}

// Local Variables:
// compile-command: "make -C .."
// c-basic-offset: 4
// coding: utf-8-unix
// indent-tabs-mode: nil
// End:
