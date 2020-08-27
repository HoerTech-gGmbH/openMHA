// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2008 2009 2010 2011 2012 HörTech gGmbH
// Copyright © 2013 2014 2015 2016 2017 2018 2019 HörTech gGmbH
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


#ifndef MHA_TYPES_H
#define MHA_TYPES_H

#include <stdlib.h>
#include <cstddef>
#include <string>

/** Test macro to compare function type definition and declaration */
#define MHA_CALLBACK_TEST(x) {\
    x ## _t var_ ## x = x;    \
    var_ ## x = NULL;         \
    (void) var_ ## x;}
#define MHA_CALLBACK_TEST_PREFIX(prefix,x) {\
    x ## _t var_ ## x = prefix ## x;    \
    var_ ## x = NULL;         \
    (void) var_ ## x;}
#define MHA_XSTRF( x ) MHA_STRF( x )
#define MHA_STRF( x ) #x

/*****************************************************************************/
/*                                                                           */
/*   version control                                                         */
/*                                                                           */
/*****************************************************************************/

/** Major version number of MHA */
#define MHA_VERSION_MAJOR 4

/** Minor version number of MHA */
#define MHA_VERSION_MINOR 12

/** Release number of MHA */
#define MHA_VERSION_RELEASE 0

/** Build number of MHA (currently unused) */
#define MHA_VERSION_BUILD 0

/** Test number for structure sizes */
#define MHA_STRUCT_SIZEMATCH (unsigned int)((sizeof(mha_real_t)==4)+2*(sizeof(mha_complex_t)==8)+4*(sizeof(mha_wave_t)==8+2*sizeof(void*))+8*(sizeof(mha_spec_t)==8+2*sizeof(void*))+16*(sizeof(mhaconfig_t)==24))

/** Full version number of MHA kernel */
#define MHA_VERSION (unsigned int)((MHA_STRUCT_SIZEMATCH | (MHA_VERSION_RELEASE << 8) | (MHA_VERSION_MINOR << 16) | (MHA_VERSION_MAJOR << 24)))


/** Version string of MHA kernel (major.minor) */
#define MHA_VERSION_STRING MHA_XSTRF(MHA_VERSION_MAJOR) "." MHA_XSTRF(MHA_VERSION_MINOR)

/** Version string of MHA kernel (major.minor.release) */
#define MHA_RELEASE_VERSION_STRING MHA_XSTRF(MHA_VERSION_MAJOR) "." MHA_XSTRF(MHA_VERSION_MINOR) "." MHA_XSTRF(MHA_VERSION_RELEASE)

/*****************************************************************************/
/*                                                                           */
/*   domain types                                                            */
/*                                                                           */
/*****************************************************************************/

typedef unsigned int mha_domain_t;
#define MHA_WAVEFORM 0
#define MHA_SPECTRUM 1
#define MHA_DOMAIN_MAX 2
#define MHA_DOMAIN_UNKNOWN MHA_DOMAIN_MAX

/*****************************************************************************/
/*                                                                           */
/*   signal processing types                                                 */
/*                                                                           */
/*****************************************************************************/

/** 
    \ingroup mhasignal
    \brief \mha type for real numbers 
    
    This type is expected to be allways the C-type 'float' (IEEE 754
    single).
*/
typedef float mha_real_t;

/**
   \ingroup mhacomplex
   \brief Type for complex floating point values.
 */
typedef struct {
    mha_real_t re;/**< \brief Real part. */
    mha_real_t im;/**< \brief Imaginary part. */
} mha_complex_t;

/** Several places in MHA rely on the fact that you can cast an array of
  mha_complex_t c[] to an array of mha_real_t r[] with
  r[0] == c[0].re
  r[1] == c[0].im
  r[2] == c[1].re
  ...
  Check these expectations in static asserts.
*/

static_assert(offsetof(mha_complex_t, re) == 0,
               "re is expected to be packed to the start of mha_complex_t");
static_assert(offsetof(mha_complex_t, im) == sizeof(mha_real_t),
               "re is expected to be packed to the start of mha_complex_t");
struct mha_complex_test_array_t { mha_complex_t c[2]; };
struct mha_real_test_array_t    { mha_real_t    r[4]; };
static_assert(offsetof(mha_complex_test_array_t, c[0].re)
               == offsetof( mha_real_test_array_t, r[0]),
               "expected c[0].re to be at the same offset as r[0]");
static_assert(offsetof(mha_complex_test_array_t, c[0].im)
               == offsetof(mha_real_test_array_t, r[1]),
               "expected c[0].im to be at the same offset as r[1]");
static_assert(offsetof(mha_complex_test_array_t, c[1].re)
               == offsetof(mha_real_test_array_t, r[2]),
               "expected c[1].re to be at the same offset as r[2]");
static_assert(offsetof(mha_complex_test_array_t, c[1].im)
               == offsetof(mha_real_test_array_t, r[3]),
               "expected c[1].im to be at the same offset as r[3]");
/** 
    Channel source direction structure 
*/
typedef struct {
    mha_real_t azimuth; /**< azimuth in radiants */
    mha_real_t elevation; /**< elevation in radiants */
    mha_real_t distance; /**< distance in meters */
} mha_direction_t;

/** 
    Channel information structure 
*/
typedef struct {
    int id; /**< channel id */
    char idstr[32]; /**< channel id */
    unsigned int side; /**< side (left/right) */
    mha_direction_t dir; /**< source direction */
    mha_real_t peaklevel; /**< Peak level corresponds to this SPL (dB) level */
} mha_channel_info_t;

/** 
    \ingroup mhasignal
    \brief Waveform signal structure

    This structure contains one fragment of a waveform signal. The
    member num_frames describes the number of audio samples in each
    audio channel.

    In a calibrated \mha, audio samples are stored in unit Pascal, see \ref clb.

    The field channel_info must be an array of num_channels entries or
    NULL.
*/
typedef struct {
    mha_real_t* buf; /**< signal buffer */
    unsigned int num_channels; /**< number of channels */
    unsigned int num_frames; /**< number of frames in each channel */
    mha_channel_info_t* channel_info; /**< detailed channel description */
} mha_wave_t;

/** 
    \ingroup mhasignal
    \brief Spectrum signal structure 

This structure contains the short time fourier transform output of the
windowed input signal. The member \c num_frames describes the number
of frequency bins in each channel. For an even FFT length \f$N\f$,
this is \f$N/2+1\f$. With odd FFT lengths, it is \f$(N+1)/2\f$. The
imaginary part of the first bin is zero. For even FFT lengths, also
the imaginary part at the Nyquist frequency is zero.

\image html spec_order.png "Data order of FFT spectrum."
\image latex spec_order.pdf "Data order of FFT spectrum." width=0.5\linewidth

Only the FFT bins for the positive frequencies, 0, and the Nyquist frequency
are stored in this structure. The negative frequencies are not stored, because
for a real-valued time signal they are the complex conjugates of the positive
frequencies.

The negative frequencies still contribute to the signal's level. Refer to
\ref clb for a description of the scaling and how the level would be computed
from the spectrum.  It is recommended to use the library function
MHASignal::rmslevel to compute the unweighted level correctly in Pascal, or
MHASignal::colored_intensity to compute a possibly weighted intensity.
*/
typedef struct {
    mha_complex_t* buf; /**< signal buffer */
    unsigned int num_channels; /**< number of channels */
    unsigned int num_frames; /**< number of frames in each channel */
    mha_channel_info_t* channel_info; /**< detailed channel description */
} mha_spec_t;

/**
   \ingroup mhasignal
   \brief Description of an audio fragment (planned as a replacement of mhaconfig_t).

 */
typedef struct {
    unsigned int n_samples;  /**< Number of samples */
    unsigned int n_channels; /**< Number of audio channels */
    unsigned int n_freqs;    /**< Number of frequency bands */
    unsigned int is_complex; /**< Flag about sample type */
    mha_real_t dt;           /**< Time distance between samples (only equidistant samples allowed) */
    mha_real_t* cf;          /**< Center frequencies of frequency bands */
    mha_real_t* chdir;       /**< Hint on source direction of channel, values below zero is left, values above zero is right, zero means unknown */
} mha_audio_descriptor_t;

/**
   \ingroup mhasignal
   \brief An audio fragment in the \mha (planned as a replacement of mha_wave_t and mha_spec_t).

   The data alignment is
   \f$(t_0,c_0,f_0),(t_0,c_0,f_1),\dots,(t_0,c_0,f_{freqs}),(t_0,c_1,f_0),\dots\f$. This
   allows a direct cast of the current mha_wave_t and mha_spec_t data
   pointers into corresponding mha_audio_t objects.
 */
typedef struct {
    mha_audio_descriptor_t descriptor; /**< Dimension and description of the data. */
    mha_real_t* rdata;                 /**< Data pointer if flag mha_audio_descriptor_t::is_complex is unset. */
    mha_complex_t* cdata;              /**< Data pointer if flag mha_audio_descriptor_t::is_complex is set. */
} mha_audio_t;

/** 
    \ingroup kernel

    \brief MHA prepare configuration structure 

    This structure contains information about channel number and
    domain for input and output signals of a \mha Plugin. Each plugin
    can change any of these parameters, e.g. by resampling of the
    signal. The only limitation is that the callback frequency is
    fixed (except for the plugins \c db and \c dbasync).

    \todo Add information on number of bands and on center frequencies, or replace by mha_audio_descriptor_t.
*/
typedef struct {
    unsigned int channels; /**< \brief Number of audio channels */
    unsigned int domain; /**< \brief Signal domain (MHA_WAVEFORM or MHA_SPECTRUM) */
    unsigned int fragsize; /**< \brief Fragment size of waveform data */
    unsigned int wndlen; /**< \brief Window length of spectral data */
    unsigned int fftlen; /**< \brief FFT length of spectral data */
    mha_real_t srate;    /**< \brief Sampling rate in Hz */
} mhaconfig_t;

/** 
 * \ingroup mhafft
 * \brief Handle for an FFT object
 *
 * This FFT object is used by the functions mha_fft_wave2spec and
 * mha_fft_spec2wave. The FFT back-end is the FFTW library. The
 * back-end is completely hidden, including external header files or
 * linking external libraries is not required.
 */
typedef void* mha_fft_t;

#define MHA_AC_UNKNOWN 0
#define MHA_AC_CHAR 1
#define MHA_AC_INT 2
#define MHA_AC_MHAREAL 3
#define MHA_AC_FLOAT 4
#define MHA_AC_DOUBLE 5
#define MHA_AC_MHACOMPLEX 6
#define MHA_AC_VEC_FLOAT 51
#define MHA_AC_USER 1000

typedef struct {
    unsigned int data_type; 
    unsigned int num_entries;
    unsigned int stride;
    void* data;         
} comm_var_t;

typedef struct algo_comm_t {
    void* handle;
    int (*insert_var)(void*,const char*,comm_var_t);
    int (*insert_var_int)(void*,const char*,int*);
    int (*insert_var_float)(void*,const char*,float*);
    int (*remove_var)(void*,const char*);
    int (*remove_ref)(void*,void*);
    int (*is_var)(void*,const char*);
    int (*get_var)(void*,const char*,comm_var_t*);
    int (*get_var_int)(void*,const char*,int*);
    int (*get_var_float)(void*,const char*,float*);
    int (*get_entries)(void*,char*,unsigned int);
    const char* (*get_error)(int);
} algo_comm_t;

typedef unsigned int (*MHAGetVersion_t)(void);

typedef int (*MHAInit_t)(algo_comm_t algo_comm,
                         const char* chain,
                         const char* algo,
                         void** h);

typedef int (*MHAPrepare_t)(void* h,
                            mhaconfig_t* cfg);

typedef int (*MHARelease_t)(void* h);

typedef void (*MHADestroy_t)(void* h);
typedef int (*MHASet_t)(void* h,
                        const char *cmd,
                        char *retval,
                        unsigned int len);
typedef std::string (*MHASetcpp_t)(void* h,
                                   const std::string & command);

typedef const char* (*MHAStrError_t)(void* h, 
                                     int err);

typedef int (*MHAProc_wave2wave_t)(void* h,
                                   mha_wave_t* sIn,
                                   mha_wave_t** sOut);

typedef int (*MHAProc_wave2spec_t)(void* h,
                                   mha_wave_t* sIn,
                                   mha_spec_t** sOut);

typedef int (*MHAProc_spec2wave_t)(void* h,
                                   mha_spec_t* sIn,
                                   mha_wave_t** sOut);

typedef int (*MHAProc_spec2spec_t)(void* h,
                                   mha_spec_t* sIn,
                                   mha_spec_t** sOut);

typedef const char* (*MHAPluginDocumentation_t)(void);

typedef const char* (*MHAPluginCategory_t)(void);

#endif

// Local Variables:
// coding: utf-8-unix
// c-basic-offset: 4
// indent-tabs-mode: nil
// compile-command: "make -C .."
// End:
