// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2013 2014 2015 2016 2017 HörTech gGmbH
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

#ifdef _WIN32
#define PLUG __declspec(dllexport)
#else
#define PLUG
#endif

#include "mha.h"

#ifdef MHA_STATIC_PLUGINS
#define MHAGetVersion        MHA_STATIC_identity_MHAGetVersion
#define MHAProc_wave2wave    MHA_STATIC_identity_MHAProc_wave2wave
#define MHAProc_spec2spec    MHA_STATIC_identity_MHAProc_spec2spec
#define MHAPluginCategory    MHA_STATIC_identity_MHAPluginCategory
#define dummy_interface_test MHA_STATIC_identity_dummy_interface_test
#endif

PLUG unsigned MHAGetVersion(void)
{
    return MHA_VERSION;
}

PLUG int MHAProc_wave2wave(void* handle,mha_wave_t* sig, mha_wave_t** out)
{
    *out = sig;
    return 0;
}

PLUG int MHAProc_spec2spec(void* handle,mha_spec_t* sig, mha_spec_t** out)
{
    *out = sig;
    return 0;
}

PLUG const char* MHAPluginCategory(void)
{
    return "signalflow";
}

PLUG void dummy_interface_test(void)
{
#ifdef MHA_STATIC_PLUGINS
    MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_identity_,MHAProc_wave2wave);
    MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_identity_,MHAProc_spec2spec);
    MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_identity_,MHAGetVersion);
    MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_identity_,MHAPluginCategory);
#else
    MHA_CALLBACK_TEST(MHAProc_wave2wave);
    MHA_CALLBACK_TEST(MHAProc_spec2spec);
    MHA_CALLBACK_TEST(MHAGetVersion);
    MHA_CALLBACK_TEST(MHAPluginCategory);
#endif
}

/*
  Local Variables:
  compile-command: "make"
  c-basic-offset: 4
  End:
*/
