// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2007 2008 2013 2016 2017 2018 HörTech gGmbH
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

#include <alsa/asoundlib.h>

int main(int argc,char** argv)
{
    snd_pcm_t* pcm;
    int err;
    if( argc < 2 ){
        fprintf(stderr,"Usage: testalsadevice <card>\n");
        exit(2);
    }
    if( (err = snd_pcm_open(&pcm,argv[1],SND_PCM_STREAM_PLAYBACK,0)) < 0 )
        exit(1);
    snd_pcm_close(pcm);
    return 0;
}

// Local Variables:
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
