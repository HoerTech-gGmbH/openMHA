// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2012 2013 2016 2017 2018 HörTech gGmbH
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

#ifndef MHASNDFILE_H
#define MHASNDFILE_H

#include <sndfile.h>
#include "mha.hh"
#include "mha_signal.hh"

#ifdef __cplusplus

#include <limits>

void write_wave(const mha_wave_t& sig,
                const char* fname,
                const float& srate = 44100,
                const int& format = 
                SF_FORMAT_WAV | 
                SF_FORMAT_FLOAT | 
                SF_ENDIAN_FILE);

namespace MHASndFile {

  class sf_t : public SF_INFO {
  public:
    sf_t(const std::string& fname);
    ~sf_t();
    SNDFILE* sf;
  };

  class sf_wave_t : private sf_t, public MHASignal::waveform_t {
  public:
    sf_wave_t(const std::string& fname,
              mha_real_t peaklevel_db,
              unsigned int maxlen = std::numeric_limits<unsigned int>::max(),
              unsigned int startpos = 0,
              std::vector<int> channel_map = std::vector<int>());
    using SF_INFO::samplerate;
  };

}

#endif
#endif

/*
 * Local Variables:
 * compile-command: "make -C .."
 * mode: c++
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * coding: utf-8-unix
 * End:
 */
