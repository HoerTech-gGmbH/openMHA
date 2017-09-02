// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2013 2016 2017 HörTech gGmbH
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

#ifndef MHA_IO_IFC_H
#define MHA_IO_IFC_H

#include "mha.h"

/**

\brief Event handler for signal stream

This event handler needs to be realtime compatible. All signal path
processing will be performed in this callback.

*/
typedef int (*IOProcessEvent_t)(void* handle,mha_wave_t* sIn,mha_wave_t** sOut);

/**

\brief Event handler for stop event

This event handler needs to be realtime compatible. The function must
return immediatly.

*/
typedef void (*IOStoppedEvent_t)(void* handle, int proc_err, int io_err);

/**

\brief Event handler for start event

This event handler needs to be realtime compatible. The function must
return immediatly.

*/
typedef void (*IOStartedEvent_t)(void* handle);


typedef int (*IOInit_t)(int fragsize, 
                        float samplerate,
                        IOProcessEvent_t proc_event,
                        void* proc_handle,
                        IOStartedEvent_t start_event,
                        void* start_handle,
                        IOStoppedEvent_t stop_event,
                        void* stop_handle,
                        void** handle);

typedef int (*IOPrepare_t)(void* handle,
                           int num_inchannels,
                           int num_outchannels);

typedef int (*IOStart_t)(void* handle);

typedef int (*IOStop_t)(void* handle);

typedef int (*IORelease_t)(void* handle);

typedef int (*IOSetVar_t)(void* handle, const char* cmd, char* retval, unsigned int len);

typedef const char* (*IOStrError_t)(void* handle, int err);

typedef void (*IODestroy_t)(void* handle);

#endif

/*
 * Local Variables:
 * compile-command: "make -C .."
 * coding: utf-8-unix
 * End:
 */
