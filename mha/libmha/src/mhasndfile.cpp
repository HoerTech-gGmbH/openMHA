// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2007 2012 2013 2016 2017 2018 2020 HörTech gGmbH
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

#include "mhasndfile.h"
#include "mha_error.hh"
#include <string.h>

void write_wave(const mha_wave_t& sig,
                const char* fname,
                const float& srate,
                const int& format)
{
    SF_INFO sf_info;
    SNDFILE* sf;
    memset(&sf_info,0,sizeof(sf_info));
    sf_info.samplerate = (int)srate;
    sf_info.channels = sig.num_channels;
    sf_info.format = format;
    sf = sf_open(fname, SFM_WRITE, &sf_info );
    if( !sf )
        throw MHA_Error(__FILE__,__LINE__,
                        "Failed to create sound file \"%s\": %s",
                        fname, sf_strerror(NULL));
    sf_writef_float( sf, sig.buf, sig.num_frames );
    sf_close( sf );
}

MHASndFile::sf_t::sf_t(const std::string& fname)
{
    frames = 0;
    samplerate = 0;
    channels = 0;
    format = 0;
    sections = 0;
    seekable = 0;
    sf = sf_open(fname.c_str(),SFM_READ,this);
    if( !sf )
        throw MHA_Error(__FILE__,__LINE__,
                        "Unable to open sound file \"%s\" for reading: %s",
                        fname.c_str(),sf_strerror(sf));
}

MHASndFile::sf_t::~sf_t()
{
    if( sf )
        sf_close( sf );
    sf = NULL;
}

unsigned int validator_channels(std::vector<int> channel_map, unsigned int channels)
{
    if( channel_map.size() == 0 )
        return channels;
    for( unsigned int k=0;k<channel_map.size();k++)
        if( unsigned(channel_map[k]) >= channels )
            throw MHA_Error(__FILE__,__LINE__,
                            "Invalid channel number in channel map (entry %u/%zu, value %d, channels %u)",
                            k+1,channel_map.size(),channel_map[k],channels);
    return channel_map.size();
}

unsigned int validator_length(unsigned int maxlen, unsigned int frames, unsigned int startpos)
{
    if( startpos >= frames )
        throw MHA_Error(__FILE__,__LINE__,
                        "The starting position is larger then the sound file length (startpos %u, length %u).",
                        startpos,frames);
    return std::min(frames-startpos,maxlen);
}

MHASndFile::sf_wave_t::sf_wave_t(const std::string& fname,
                                 mha_real_t peaklevel_db,
                                 unsigned int maxlen,
                                 unsigned int startpos,
                                 std::vector<int> channel_map)
    : MHASndFile::sf_t(fname),
      MHASignal::waveform_t(validator_length(maxlen,frames,startpos),
                            validator_channels(channel_map,channels))
{
    if( startpos > 0 )
        sf_seek( sf, startpos, SEEK_SET );
    if( channel_map.size() == 0 ){
        sf_readf_float(sf,buf,num_frames);
    }else{
        MHASignal::waveform_t tmpbuf(num_frames,channels);
        sf_readf_float(sf,tmpbuf.buf,num_frames);
        for( unsigned int k=0;k<channel_map.size();k++){
            copy_channel(tmpbuf,channel_map[k],k);
        }
    }
    *this *= 2.0e-5 * pow(10.0f,0.05*peaklevel_db);
    sf_close(sf);
    sf = NULL;
}

/*
 * Local Variables:
 * compile-command: "make -C .."
 * c-basic-offset: 4
 * coding: utf-8-unix
 * indent-tabs-mode: nil
 * End:
 */
