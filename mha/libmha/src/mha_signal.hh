// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2005 2006 2007 2008 2009 2010 2011 2012 HörTech gGmbH
// Copyright © 2013 2014 2016 2017 2018 2019 2020 HörTech gGmbH
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

#ifndef __MHA_SIGNAL_H__
#define __MHA_SIGNAL_H__

#include "mha.hh"
#include "mha_error.hh"
#include <limits>
#include <string>
#include <cmath>
#include <string>
#include <iostream>
#include <sstream>
#include <complex>
#include <cstring>
#include "mha_os.h"
#include <vector>
#include <numeric>
#include <algorithm>
#include <type_traits>
#include "mha_parser.hh"

// some platforms do not define M_PI in <cmath>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define mha_round(x) (int)((float)x+0.5)

namespace MHASignal {

    /**
       \brief Apply a function to each element of a mha_wave_t.
       \param s Pointer to a mha_wave_t structure
       \param fun Function to be applied (one argument)
    */
    inline void for_each(mha_wave_t* s,mha_real_t (*fun)(mha_real_t))
    {
        for(unsigned int k=0;k<s->num_channels*s->num_frames;k++)
            s->buf[k] = fun(s->buf[k]);
    }

    /**
       \brief Conversion from linear scale to dB (no SPL reference)
       \param x Linear input
       \param eps minimum linear value (if x < eps --> convert eps instead), eps < 0 not allowed
       \return NaN if x < 0 (log not defined for negative)
       \throw MHA_Error if eps < 0
    */
    inline mha_real_t lin2db(mha_real_t x, mha_real_t eps)
    {
        if (eps < 0)
            throw MHA_Error(__FILE__,__LINE__,
                            "Minimum linear value eps < 0 is not allowed!");
        if (x < 0)
            return std::numeric_limits<float>::quiet_NaN(); //log not defined for negative
        else
            return 20.0f * log10f(std::max(x,eps));
    }

    /**
       \brief Conversion from linear scale to dB (no SPL reference)
       \param x Linear input.
       \return NaN if x < 0 (log not defined for negative)
    */
    inline mha_real_t lin2db(mha_real_t x)
    {
        return lin2db(x, 0);
    }

    /**
       \brief Conversion from dB scale to linear (no SPL reference)
       \param x dB input.
    */
    inline mha_real_t db2lin(mha_real_t x)
    {
        return powf(10.0f,0.05f*x);
    }

    /**
       \brief conversion from squared values to dB (no SPL reference)
       \param x squared value input
       \param eps minimum squared value (if x < eps --> convert eps instead), eps < 0 not allowed
       \return NaN if x < 0 (log not defined for negative)
       \throw MHA_Error if eps < 0
    */
    inline mha_real_t sq2db(mha_real_t x, mha_real_t eps = 0.0f)
    {
        if (eps < 0)
            throw MHA_Error(__FILE__,__LINE__,
                            "Minimum squared value eps < 0 is not allowed!");
        if (x < 0)
            return std::numeric_limits<float>::quiet_NaN(); //log not defined for negative
        else
            return 10.0f * log10f(std::max(x,eps));
    }

    /**
       \brief conversion from dB to squared values (no SPL reference)
       \param x dB input
    */
    inline mha_real_t db2sq(mha_real_t x)
    {
        return powf(10.0f, 0.1f * x);
    }


    /**
       \brief Conversion from linear Pascal scale to dB SPL
       \param x Linear input
       \param eps minimum pascal value (if x < eps --> convert eps instead), eps < 0 not allowed
       \return NaN if x < 0 (log not defined for negative)
       \throw MHA_Error if eps < 0
    */
    inline mha_real_t pa2dbspl(mha_real_t x, mha_real_t eps = 0.0f)
    {
        if (eps < 0)
            throw MHA_Error(__FILE__,__LINE__,
                            "Minimum pascal value eps < 0 is not allowed!");
        if (x < 0)
            return std::numeric_limits<float>::quiet_NaN(); //log not defined for negative
        else
            return 20.0f * log10f(5e+4f * std::max(x,eps));
    }

    /**
       \brief Conversion from dB SPL to linear Pascal scale
       \param x Linear input.
    */
    inline mha_real_t dbspl2pa(mha_real_t x)
    {
        return db2lin(x)*2e-5f;
    }

    /**
       \brief Conversion from squared Pascal scale to dB SPL
       \param x squared pascal input
       \param eps minimum squared-pascal value (if x < eps --> convert eps instead), eps < 0 not allowed
       \return NaN if x < 0 (log not defined for negative)
       \throw MHA_Error if eps < 0
    */
    inline mha_real_t pa22dbspl(mha_real_t x, mha_real_t eps = 0.0f)
    {
        if (eps < 0)
            throw MHA_Error(__FILE__,__LINE__,
                            "Minimum squared pascal value eps < 0 is not allowed!");
        if (x < 0)
            return std::numeric_limits<float>::quiet_NaN(); //log not defined for negative
        else
            return 10.0f * log10f(25e+8f * std::max(x,eps));
    }

    /**
       \brief conversion from dB SPL to squared Pascal scale
       \param x dB SPL input
    */
     inline mha_real_t dbspl2pa2(mha_real_t x)
    {
        return db2sq(x) * 400e-12f;
    }

    /**
       \brief conversion from samples to seconds
       \param n     number of samples
       \param srate sampling rate / Hz
    */
    inline mha_real_t smp2sec(mha_real_t n, mha_real_t srate)
    {
        return n / srate;
    }

    /**
       \brief conversion from seconds to samples
       \param sec   time in seconds
       \param srate sampling rate / Hz
       \return number of samples, generally has non-zero fractional part
    */
    inline mha_real_t sec2smp(mha_real_t sec, mha_real_t srate)
    {
        return sec * srate;
    }

    /**
       \ingroup mhasignal
       \brief conversion from fft bin index to frequency
       \param bin    index of fft bin, index 0 has dc
       \param fftlen FFT length
       \param srate  sampling frequency / Hz
       \return frequency of fft bin / Hz
    */
    inline mha_real_t bin2freq(mha_real_t bin,
                               unsigned   fftlen,
                               mha_real_t srate)
    {
        if (bin < 0) {
            throw MHA_Error(__FILE__,__LINE__,
                            "frequency for negative bin %f requested", bin);
        }
        if (bin*2 > fftlen) {
            throw MHA_Error(__FILE__,__LINE__,
                            "frequency for trans-nyquist bin %f "
                            "(fftlen=%u) requested", bin, fftlen);
        }
        return bin / fftlen * srate;
    }

    /**
       \ingroup mhasignal
       \brief conversion from frequency to fft bin index
       \param freq   frequency / Hz
       \param fftlen FFT length
       \param srate  sampling frequency / Hz
       \return 0-based index of fft bin, generally has non-zero fractional part
    */
     inline mha_real_t freq2bin(mha_real_t freq,
                                unsigned   fftlen,
                                mha_real_t srate)
    {
        if (freq < 0) {
            throw MHA_Error(__FILE__,__LINE__,
                            "bin index for negative frequency %f requested",
                            freq);
        }
        if (freq > srate / 2) {
            throw MHA_Error(__FILE__,__LINE__,
                            "bin index for trans-nyquist frequency %f "
                            "(samplint rate=%f) requested", freq, srate);
        }
        return freq * fftlen / srate;
    }
    
    /**
       \ingroup mhasignal
       \brief conversion from delay in samples to phase shift

       Compute phase shift that needs to be applied to fft spectrum to
       achieve the desired delay.

       \param samples delay in samples. 
                      Positive delay: shift current signal to future.
       \param bin     index of fft bin, index 0 has dc
                      (index 0 and nyqvist bin cannot be delayed)
       \param fftlen FFT length
       \return The phase shift in radiant that needs to be applied to fft bin
               to achieve the desired delay.  A positive delay requires a
               negative phase shift.
               If required phase shift is >pi or <-pi, then the
               desired delay cannot be applied in the fft domain with given
               parameters. Required phase shifts close to pi should not be used.
               If bin is 0 or nyqvist, returns 0 phase shift.
    */
    inline mha_real_t smp2rad(mha_real_t samples, 
                              unsigned   bin,
                              unsigned   fftlen)
    {
        if (bin == 0U || 
            (((fftlen & 1U) == 0U) && (bin >= (fftlen+1U) / 2U)))
            return 0.0f;
        return -2.0 * M_PI * bin / fftlen * samples;
    }

    /**
       \ingroup mhasignal
       \brief conversion from phase shift to delay in samples

       Compute delay in samples that is achieved by a phase shift.

       \param phase_shift phase shift in radiant 
       \param bin     index of fft bin, index 0 has dc
                      (index 0 and nyqvist bin cannot be delayed)
       \param fftlen FFT length
       \return The delay in samples achieved by applying the phase shift.
               A negative phase shift causes a positive delay:
               shifts current signal to future.
    */
    inline mha_real_t rad2smp(mha_real_t phase_shift, 
                              unsigned   bin,
                              unsigned   fftlen)
    {
        double toomany2pis = floor((phase_shift + M_PI) / 2 / M_PI);
        if (bin == 0 && phase_shift == 0) return 0;
        return -0.5 / M_PI / bin * fftlen * 
            (phase_shift - toomany2pis * 2.0 * M_PI);
    }

    /**
       \ingroup mhasignal
       \brief Duplicate last vector element to match desired size

       \param vec Input vector.
       \param n Target number of elements.
       \retval Resized vector.
    */
    template <class elem_type>
    std::vector<elem_type> dupvec(std::vector<elem_type> vec, unsigned n)
    {
        if( vec.size() )
            vec.resize(n,vec[vec.size()-1]);
        else
            vec.resize(n);
        return vec;
    }
    
    /**
       \ingroup mhasignal
       \brief Duplicate last vector element to match desired size, check for dimension

       The input dimension can be either 1 or the target length.

       \param vec Input vector.
       \param n Target number of elements.
       \retval Resized vector.
    */
    template <class elem_type>
    std::vector<elem_type> dupvec_chk(std::vector<elem_type> vec, unsigned n)
    {
        if( vec.size() == 1)
            vec.resize(n,vec[vec.size()-1]);
        if( vec.size() != n )
            throw MHA_Error(__FILE__,__LINE__,"Invalid vector length (expected 1 or %u, got %zu).",
                            n,vec.size());
        return vec;
    }
    
}

/** 
    \ingroup mhasignal
    \brief Test for equal dimension of waveform structures
*/
inline bool
equal_dim(const mha_wave_t& a,const mha_wave_t& b)
{
    return (a.num_channels==b.num_channels)&&(a.num_frames==b.num_frames);
}

/** 
    \ingroup mhasignal
    \brief Test for match of waveform dimension with mhaconfig structure.
*/
inline bool
equal_dim(const mha_wave_t& a,const mhaconfig_t& b)
{
    return (a.num_channels==b.channels)&&(a.num_frames==b.fragsize);
}

/** \ingroup mhasignal
    \brief Test for equal dimension of spectrum structures.
*/
inline bool
equal_dim(const mha_spec_t& a,const mha_spec_t& b)
{
    return (a.num_channels==b.num_channels)&&(a.num_frames==b.num_frames);
}

/** \ingroup mhasignal
    \brief Test for match of spectrum dimension with mhaconfig structure.
*/
inline bool
equal_dim(const mha_spec_t& a,const mhaconfig_t& b)
{
    return (a.num_channels==b.channels)&&(a.num_frames==b.fftlen/2+1);
}

/** \ingroup mhasignal
    \brief Test for equal dimension of waveform/spectrum structures

    \warning Waveform structures mha_wave_t use interleaved data order, while
    spectrum structures mha_spec_t use non-interleaved.
*/
inline bool
equal_dim(const mha_wave_t& a,const mha_spec_t& b)
{
    return (a.num_channels==b.num_channels)&&(a.num_frames==b.num_frames);
}

/** \ingroup mhasignal
    \brief Test for equal dimension of waveform/spectrum structures

    \warning Waveform structures mha_wave_t use interleaved data order, while
    spectrum structures mha_spec_t use non-interleaved.
*/
inline bool
equal_dim(const mha_spec_t& a,const mha_wave_t& b)
{
    return (a.num_channels==b.num_channels)&&(a.num_frames==b.num_frames);
}

/**
   \ingroup mhasignal
   \brief Numeric integration of a signal vector (real values)

   \param s Input signal vector
*/
void integrate(mha_wave_t& s);

/**
   \ingroup mhasignal
   \brief Numeric integration of a signal vector (complex values)

   \param s Input signal vector
*/
void integrate(mha_spec_t& s);

inline unsigned int mha_min_1(unsigned int a)
{
    if( a )
        return a;
    return 1;
}

/** \ingroup mhasignal
    \brief Return size of a waveform structure */
inline unsigned int size(const mha_wave_t& s)
{
    return s.num_channels * s.num_frames;
}

/** \ingroup mhasignal
    \brief Return size of a spectrum structure */
inline unsigned int size(const mha_spec_t& s)
{
    return s.num_channels * s.num_frames;
}

/** \ingroup mhasignal
    \brief Return size of a waveform structure */
inline unsigned int size(const mha_wave_t* s)
{
    return s->num_channels * s->num_frames;
}

/** \ingroup mhasignal
    \brief Return size of a spectrum structure */
inline unsigned int size(const mha_spec_t* s)
{
    return s->num_channels * s->num_frames;
}

/** \ingroup mhasignal
    \brief Set all values of waveform to zero */
inline void clear(mha_wave_t& s)
{
    memset( s.buf, 0, size(s)*sizeof(s.buf[0]) );
}

/** \ingroup mhasignal
    \brief Set all values of waveform to zero */
inline void clear(mha_wave_t* s)
{
    memset( s->buf, 0, size(s)*sizeof(s->buf[0]) );
}

/** \ingroup mhasignal
    \brief Set all values of spectrum to zero */
inline void clear(mha_spec_t& s)
{
    memset( s.buf, 0, size(s)*sizeof(s.buf[0]) );
}

/** \ingroup mhasignal
    \brief Set all values of spectrum to zero */
inline void clear(mha_spec_t* s)
{
    memset( s->buf, 0, size(s)*sizeof(s->buf[0]) );
}

/** \ingroup mhasignal
    \brief Set all values of waveform 'self' to 'val'.
    \param self Waveform to be modified.
    \param val Value to be assigned to all entries of waveform.
*/
inline void assign(mha_wave_t self,mha_real_t val)
{
    unsigned int k_max = size(self);
    for(unsigned int k=0;k<k_max;k++)
        self.buf[k] = val;
}

/** \ingroup mhasignal
    \brief Set all values of waveform 'self' to 'val'.
    \param self Waveform to be modified.
    \param val Source waveform structure.
*/
void assign(mha_wave_t self,const mha_wave_t& val);


/** \ingroup mhasignal
    \brief Set all values of spectrum 'self' to 'val'.
    \param self Spectrum to be modified.
    \param val Source spectrum.
*/
void assign(mha_spec_t self,const mha_spec_t& val);

/** \ingroup mhasignal
    \brief Time shift of waveform chunk

    Shifted areas are filled with zeros.
    
    \param self Waveform chunk to be shifted
    \param shift Shift amount, positive values shift to later times
*/
void timeshift(mha_wave_t&self,int shift);

mha_wave_t range(mha_wave_t s,unsigned int k0,unsigned int len);
mha_spec_t channels(mha_spec_t s,unsigned int ch_start,unsigned int nch);

/** \ingroup mhasignal
    \brief Access an element of a waveform structure
  
    \param s    Waveform structure
    \param fr   Frame number
    \param ch   Channel number
    \return Reference to element
*/
inline mha_real_t& value(mha_wave_t* s,unsigned int fr,unsigned int ch)
{
    return s->buf[fr*s->num_channels+ch];
}

/** \ingroup mhasignal
    \brief Constant access to an element of a waveform structure
  
    \param s    Waveform structure
    \param fr   Frame number
    \param ch   Channel number
    \return Reference to element
*/
inline const mha_real_t& value(const mha_wave_t* s,unsigned int fr,unsigned int ch)
{
    return s->buf[fr*s->num_channels+ch];
}

inline mha_real_t& value(mha_wave_t* s,unsigned int k)
{
    return s->buf[k];
}

inline mha_complex_t& value(mha_spec_t* s,unsigned int k)
{
    return s->buf[k];
}

/** \ingroup mhasignal
    \brief Access to an element of a spectrum
  
    \param s    Spectrum structure
    \param fr   Bin number
    \param ch   Channel number
    \return Reference to element
*/
inline mha_complex_t& value(mha_spec_t* s,unsigned int fr,unsigned int ch)
{
    return s->buf[fr + ch*s->num_frames];
}

/** \ingroup mhasignal
    \brief Constant access to an element of a spectrum
  
    \param s    Spectrum structure
    \param fr   Bin number
    \param ch   Channel number
    \return Reference to element
*/
inline const mha_complex_t& value(const mha_spec_t* s,unsigned int fr,unsigned int ch)
{
    return s->buf[fr + ch*s->num_frames];
}

/** \ingroup mhasignal
    \brief Access to an element of a waveform structure
  
    \param s    Waveform structure
    \param fr   Frame number
    \param ch   Channel number
    \return Reference to element
*/
inline mha_real_t& value(mha_wave_t& s,unsigned int fr,unsigned int ch)
{
    return s.buf[fr*s.num_channels+ch];
}

/** \ingroup mhasignal
    \brief Constant access to an element of a waveform structure
  
    \param s    Waveform structure
    \param fr   Frame number
    \param ch   Channel number
    \return Reference to element
*/
inline const mha_real_t& value(const mha_wave_t& s,unsigned int fr,unsigned int ch)
{
    return s.buf[fr*s.num_channels+ch];
}

/** \ingroup mhasignal
    \brief Access to an element of a spectrum
  
    \param s    Spectrum structure
    \param fr   Bin number
    \param ch   Channel number
    \return Reference to element
*/
inline mha_complex_t& value(mha_spec_t& s,unsigned int fr,unsigned int ch)
{
    return s.buf[fr + ch*s.num_frames];
}

/** \ingroup mhasignal
    \brief Constant access to an element of a spectrum
  
    \param s    Spectrum structure
    \param fr   Bin number
    \param ch   Channel number
    \return Reference to element
*/
inline const mha_complex_t& value(const mha_spec_t& s,unsigned int fr,unsigned int ch)
{
    return s.buf[fr + ch*s.num_frames];
}

/**
   \ingroup mhasignal
   \brief Converts a mha_wave_t structure into a std::vector<float> (interleaved order).
   \warning This function is not real-time safe. Do not use in signal processing thread.
*/
std::vector<float> std_vector_float(const mha_wave_t&);

/**
   \ingroup mhasignal
   \brief Converts a mha_wave_t structure into a std::vector< std::vector<float> > (outer vector represents channels).
   \warning This function is not real-time safe. Do not use in signal processing thread.
*/
std::vector<std::vector<float> > std_vector_vector_float(const mha_wave_t&);

/**
   \ingroup mhasignal
   \brief Converts a mha_spec_t structure into a std::vector< std::vector<mha_complex_t> > (outer vector represents channels).
   \warning This function is not real-time safe. Do not use in signal processing thread.
*/
std::vector<std::vector<mha_complex_t> > std_vector_vector_complex(const mha_spec_t&);

/** \ingroup mhasignal
    \brief Addition operator */
mha_wave_t& operator+=(mha_wave_t&,const mha_real_t&);
/** \ingroup mhasignal
    \brief Addition operator */
mha_wave_t& operator+=(mha_wave_t&,const mha_wave_t&);
/** \ingroup mhasignal
    \brief Subtraction operator */
mha_wave_t& operator-=(mha_wave_t&,const mha_wave_t&);
/** \ingroup mhasignal
    \brief Subtraction operator */
mha_spec_t& operator-=(mha_spec_t&,const mha_spec_t&);
/** \ingroup mhasignal
    \brief Element-wise multiplication operator */
mha_wave_t& operator*=(mha_wave_t&,const mha_real_t&);
/** \ingroup mhasignal
    \brief Element-wise multiplication operator */
mha_wave_t& operator*=(mha_wave_t&,const mha_wave_t&);
/** \ingroup mhasignal
    \brief Element-wise multiplication operator */
mha_spec_t& operator*=(mha_spec_t&,const mha_real_t&);
/** \ingroup mhasignal
    \brief Element-wise multiplication operator */
mha_spec_t& operator*=(mha_spec_t&,const mha_wave_t&);
/** \ingroup mhasignal
    \brief Element-wise multiplication operator */
mha_spec_t& operator*=(mha_spec_t&,const mha_spec_t&);
/** \ingroup mhasignal
    \brief Element-wise division operator */
mha_spec_t& operator/=(mha_spec_t&,const mha_spec_t&);
/** \ingroup mhasignal
    \brief Element-wise division operator */
mha_wave_t& operator/=(mha_wave_t&,const mha_wave_t&);
/** \ingroup mhasignal
    \brief Addition operator */
mha_spec_t& operator+=(mha_spec_t&,const mha_spec_t&);
/** \ingroup mhasignal
    \brief Addition operator */
mha_spec_t& operator+=(mha_spec_t&,const mha_real_t&);
void set_minabs(mha_spec_t&,const mha_real_t&);
/// In-Place division with lower limit on divisor.
mha_spec_t & safe_div( mha_spec_t & self, const mha_spec_t & v,
                       mha_real_t eps );
/** \ingroup mhasignal
    \brief Exponent operator
    \warning This overwrites the xor operator!
*/
mha_wave_t& operator^=(mha_wave_t& self,const mha_real_t& arg);

/** 
    \ingroup mhatoolbox
    \ingroup mhasignal
    \brief Namespace for audio signal handling and processing classes
*/
namespace MHASignal {

    /**
       \ingroup mhasignal
       \brief Copy one channel of a source signal
       \param self Destination.
       \param src Source
       \param sch Source channel number
       \param dch Destination channel number
    */
    void copy_channel(mha_spec_t& self, const mha_spec_t& src, unsigned sch, unsigned dch);
    /**
       \ingroup mhasignal
       \brief Copy one channel of a source signal
       \param self Destination.
       \param src Source
       \param src_channel Source channel number
       \param dest_channel Destination channel number
    */
    void copy_channel(mha_wave_t& self, const mha_wave_t& src, 
                      unsigned src_channel, unsigned dest_channel);


    /** 
        
        \brief Signal counter to produce signal ID strings
    */
    extern unsigned long int signal_counter;

    /**
       \ingroup mhasignal
       \brief Return RMS level of a spectrum channel

       Computes the RMS level of the signal in Pascal in the given channel.

       Takes into account the the negative frequency bins that are not stored
       (\ref clb).
       \param s Input spectrum
       \param channel Channel number to be tested
       \param fftlen FFT length (to correctly count the level of the Nyquist bin)
       \return RMS level in Pa
    */
    mha_real_t rmslevel(const mha_spec_t& s,unsigned int channel,unsigned int fftlen);
    /**
       \ingroup mhasignal
       \brief Colored spectrum intensity
       
       computes the squared sum of the spectrum after filtering with the
       frequency response. Takes into account the negative frequency bins
       that are not stored (\ref clb).
       \param s Input spectrum
       \param channel Channel number to be tested
       \param fftlen FFT length (to correctly count the level of the Nyquist bin)
       \param sqfreq_response An array with one squared weighting factor for every
                              fft bin. Array length must be equal to s->num_frames.
                              nullptr can be given for equal weighting of all
                              frequencies.
       \return sum of squares. Root of this is the colored level in Pa
    */
    mha_real_t colored_intensity(const mha_spec_t& s,
                                 unsigned int channel,
                                 unsigned int fftlen,
                                 mha_real_t * sqfreq_response = nullptr);
    /**
       \ingroup mhasignal
       \brief Find maximal absolute value
       \param s Input signal
       \param channel Channel to be tested
       \return maximum absolute value
    */
    mha_real_t maxabs(const mha_spec_t& s,unsigned int channel);
    /**
       \ingroup mhasignal
       \brief Return RMS level of a waveform channel
       \param s Input waveform signal
       \param channel Channel number to be tested
       \return RMS level in Pa
    */
    mha_real_t rmslevel(const mha_wave_t& s,unsigned int channel);
    /**
       \ingroup mhasignal
       \brief Find maximal absolute value
       \param s Input signal
       \param channel Channel to be tested
       \return maximum absolute value
    */
    mha_real_t maxabs(const mha_wave_t& s,unsigned int channel);
    /**
       \ingroup mhasignal
       \brief Find maximal absolute value
       \param s Input signal
       \return maximum absolute value
    */
    mha_real_t maxabs(const mha_wave_t& s);
    /**
       \ingroup mhasignal
       \brief Find maximal value
       \param s Input signal
       \return maximum absolute value
    */
    mha_real_t max(const mha_wave_t& s);
    /**
       \ingroup mhasignal
       \brief Find minimal value
       \param s Input signal
       \return maximum absolute value
    */
    mha_real_t min(const mha_wave_t& s);
    /**
       \ingroup mhasignal
       \brief Calculate sum of squared values in one channel
       \param s Input signal
       \param channel Channel
       \return \f$\sum x^2\f$
    */
    mha_real_t sumsqr_channel(const mha_wave_t& s,unsigned int channel);
    /**
       \ingroup mhasignal
       \brief Calculate sum over all channels of squared values
       \param s Input signal
       \param frame Frame number
       \return \f$\sum x^2\f$
    */
    mha_real_t sumsqr_frame(const mha_wave_t& s,unsigned int frame);


    /** 

        \ingroup mhasignal
        \brief a signal processing class for spectral data (based on mha_spec_t) 
    */
    class spectrum_t : public mha_spec_t {
    public:
        spectrum_t(const unsigned int& frames,const unsigned int& channels);
        explicit spectrum_t(const mha_spec_t&);
        spectrum_t(const MHASignal::spectrum_t&);
        spectrum_t(const std::vector<mha_complex_t>&);
        virtual ~spectrum_t(void);
        /**
           \brief Access to element
           \param f Bin number
           \param ch Channel number
           \return Reference to element
        */
        inline mha_complex_t& operator()(unsigned int f,unsigned int ch){return buf[num_frames*ch+f];};
        /**
           \brief Access to a single element, direct index into data buffer
           \param k Buffer index
           \return Reference to element
        */
        inline mha_complex_t& operator[](unsigned int k) {return buf[k];};
        /**
           \brief Access to element
           \param f Bin number
           \param ch Channel number
           \return Reference to element
        */
        inline mha_complex_t& value(unsigned int f,unsigned int ch){return buf[num_frames*ch+f];};
        void copy(const mha_spec_t&);
        void copy_channel(const mha_spec_t & s, unsigned sch, unsigned dch);
        void export_to(mha_spec_t&);
        void scale(const unsigned int&,const unsigned int&,const unsigned int&,const mha_real_t&);
        void scale_channel(const unsigned int&,const mha_real_t&);
    };

    /** 

        \ingroup mhasignal
        \brief signal processing class for waveform data (based on mha_wave_t) 
    */
    class waveform_t : public mha_wave_t {
    public:
        waveform_t(const unsigned int& frames,const unsigned int& channels);
        explicit waveform_t(const mhaconfig_t& cf);
        explicit waveform_t(const mha_wave_t& src);
        waveform_t(const MHASignal::waveform_t& src);
        waveform_t(const std::vector<mha_real_t>& src);
        virtual ~waveform_t(void);
        std::vector<mha_real_t> flatten() const;
        explicit operator std::vector<mha_real_t>() const;
        inline void operator=(const mha_real_t& v){assign(v);};
        inline mha_real_t& operator[](unsigned int k) {return buf[k];};
        inline const mha_real_t& operator[](unsigned int k) const {return buf[k];};
        /**
           \brief Element accessor
           \param t Frame number
           \param ch Channel number
           \return Reference to element
        */
        inline mha_real_t& value(unsigned int t,unsigned int ch)
            { return buf[num_channels*t+ch]; }
        /**
           \brief Element accessor
           \param t Frame number
           \param ch Channel number
           \return Reference to element
        */
        inline mha_real_t& operator()(unsigned int t,unsigned int ch)
            { return buf[num_channels*t+ch]; }
        /**
           \brief Constant element accessor
           \param t Frame number
           \param ch Channel number
           \return Reference to element
        */
        inline const mha_real_t& value(unsigned int t,unsigned int ch) const
            { return buf[num_channels*t+ch]; }
        /**
           \brief Constant element accessor
           \param t Frame number
           \param ch Channel number
           \return Reference to element
        */
        inline const mha_real_t& operator()(unsigned int t,unsigned int ch) const
            { return buf[num_channels*t+ch]; }
        mha_real_t sum(const unsigned int &a, const unsigned int &b);
        mha_real_t sum(const unsigned int &a, const unsigned int &b,
                       const unsigned int &ch);
        mha_real_t sum();
        mha_real_t sumsqr();
        mha_real_t sum_channel(const unsigned int&);
        void assign(const unsigned int &k, const unsigned int &ch,
                    const mha_real_t &val);
        void assign(const mha_real_t&);
        void assign_frame(const unsigned int &k, const mha_real_t &val);
        void assign_channel(const unsigned int &c, const mha_real_t &val);
        void copy(const std::vector<mha_real_t>& v);
        void copy(const mha_wave_t&);
        void copy(const mha_wave_t*);
        void copy_channel(const mha_wave_t&,unsigned int,unsigned int);
        void copy_from_at(unsigned int,unsigned int,const mha_wave_t&,unsigned int);
        void export_to(mha_wave_t&);
        void limit(const mha_real_t& min,const mha_real_t& max);
        void power(const waveform_t&);
        void powspec(const mha_spec_t&);
        void scale(const unsigned int &a, const unsigned int &b,
                   const unsigned int &ch, const mha_real_t &val);
        void scale(const unsigned int &k, const unsigned int &ch,
                   const mha_real_t &val);
        void scale_channel(const unsigned int&,const mha_real_t&);
        void scale_frame(const unsigned int&,const mha_real_t&);
        unsigned int get_size() const {return size(*this);};
    };

    void scale(mha_spec_t* dest, const mha_wave_t* src);
    void limit(mha_wave_t& s,const mha_real_t& min,const mha_real_t& max);

    /**
       \ingroup mhasignal
       \brief Double-buffering class.

       This class has two layers: The outer layer, with an outer
       fragment size, and an inner layer, with its own fragment
       size. Data is passed into the inner layer through the
       doublebuffer_t::outr_process() callback. The pure virtual
       method doublebuffer_t::inner_process() is called whenever
       enough data is available.
    */
    class doublebuffer_t {
    public:
        /** \brief Constructor of double buffer.
            \param nchannels_in Number of channels at the input (both layers).
            \param nchannels_out Number of channels at the output (both layers).
            \param outer_fragsize Fragment size of the outer layer (e.g., hardware fragment size)
            \param inner_fragsize Fragment size of the inner layer (e.g., software fragment size)
        */
        doublebuffer_t(unsigned int nchannels_in,
                       unsigned int nchannels_out,
                       unsigned int outer_fragsize,
                       unsigned int inner_fragsize);
        virtual ~doublebuffer_t();
        /**
           \brief Method to pass audio fragments into the inner layer.
           \param s Pointer to input waveform fragment.
           \return Pointer to output waveform fragment.
        */
        mha_wave_t* outer_process(mha_wave_t* s);
    protected:
        /**
           \brief Method to realize inner processing callback.

           To be overwritten by derived classes.

           \param s Pointer to input waveform fragment.
           \return Pointer to output waveform fragment.
        */
        virtual mha_wave_t* inner_process(mha_wave_t* s) = 0;
    private:
        inline unsigned int min(unsigned int a,unsigned int b){if(a<b)return a;return b;};
        waveform_t outer_out;
        mha_wave_t this_outer_out;
        waveform_t inner_in;
        waveform_t inner_out;
        unsigned int k_inner;
        unsigned int k_outer;
        unsigned int ch;
    };

    /** A ringbuffer class for time domain audio signal, which makes no
        assumptions with respect to fragment size.
        
        Blocks of audio signal can be placed into the ringbuffer using
        the #write method.  Individual audio samples can be accessed
        and altered using the #value method.  Blocks of audio data can
        be deleted from the ringbuffer using the #discard method.
    */
    class ringbuffer_t : private waveform_t {

        /// Index of oldest frame in underlying storage for the ringbuffer.
        /// This value is added to the frame parameter of the #value method,
        /// and this value is altered when #discard is called.
        unsigned next_read_frame_index;

        /// Index of first free frame in underlying storage.  Next
        /// frame to be stored will be placed here.
        unsigned next_write_frame_index;

    public:
        /** Creates new ringbuffer for time domain signal.
            Constructor allocates enough storage so that \a frames
            audio samples can be stored in the ringbuffer.

            @param frames Size of ringbuffer in samples per channel.
                          Maximum number of frames that can be stored
                          in the ringbuffer at one time.  This number
                          cannot be changed after instance creation.

            @param channels Number of audio channels.
   
            @param prefilled_frames Number of frames to be prefilled
                          with zero values.  Many applications of a
                          ringbuffer require the introduction of a
                          delay.  In practice, this delay is achieved
                          by inserting silence audio samples (zeros)
                          into the ringbuffer before the start of the
                          actual signal is inserted for the first
                          time.

            @throw MHA_Error if prefilled_frames > frames
        */
        ringbuffer_t(unsigned frames,
                     unsigned channels,
                     unsigned prefilled_frames);

        /// number of currently contained frames
        unsigned contained_frames() const {
            unsigned result = 
                (next_write_frame_index + num_frames - next_read_frame_index)
                % num_frames;
            return result;
        }

        /** Access to value stored in ringbuffer.
         *
         * \a frame index is relative to the oldest frame stored in
         * the ringbuffer, therefore, the meaning of the \a frame
         * changes when the #discard method is called.
         *
         * @param frame frame index, 0 corresponds to oldest frame stored.
         * @param channel audio channel
         * @return reference to contained sample value
         * @throw MHA_Error if channel or frame out of bounds.
         */
        mha_real_t & value(unsigned frame, unsigned channel) {
            if (channel >= num_channels)
                throw MHA_Error(__FILE__,__LINE__,
                                "ringbuffer_t::value: channel %u out of bounds",
                                channel);
            if (frame >= contained_frames())
                throw MHA_Error(__FILE__,__LINE__,
                                "ringbuffer_t::value: frame %u out of bounds",
                                frame);
            return waveform_t::value((frame + next_read_frame_index)
                                     % num_frames,
                                     channel);
        }
        /** Discards the oldest frames. 
         * Makes room for new #write, alters base frame index for #value
         * @param frames how many frames to discard.
         * @throw MHA_Error if frames > #contained_frames*/
        void discard(unsigned frames) {
            if (frames > contained_frames())
                throw MHA_Error(__FILE__, __LINE__, 
                                "ringbuffer_t::discard: parameter %u exceeds"
                                " value %u of contained frames",
                                frames, contained_frames());
            next_read_frame_index += frames;
            next_read_frame_index %= num_frames;
        }
        /** Copies the contents of the signal into the ringbuffer if there is
            enough space.
            @param signal New signal to be appended to the signal already
                present in the ringbuffer
 
            @throw MHA_Error if there is not enough space or if the
                channel count mismatches.  Nothing is copied if the
                space is insufficient.
        */
        void write(mha_wave_t & signal) {
            if (signal.num_channels != num_channels) 
                throw MHA_Error(__FILE__,__LINE__,
                                "ringbuffer_t::write: number %u of channels in"
                                " signal does not match the number %u of"
                                " channels of this ringbuffer",
                                signal.num_channels, num_channels);
            if (signal.num_frames > (num_frames - contained_frames() - 1U))
                throw MHA_Error(__FILE__,__LINE__,
                                "ringbuffer_t::write: number %u of frames"
                                " contained in input signal exceeds available"
                                " space of %u frames in this ringbuffer",
                                signal.num_frames,
                                num_frames - contained_frames() - 1U);
            unsigned first_part_frames =
                std::min(num_frames - next_write_frame_index,
                         signal.num_frames);
            unsigned second_part_frames = signal.num_frames - first_part_frames;
            copy_from_at(next_write_frame_index,
                         first_part_frames,
                         signal,
                         0U);
            next_write_frame_index += first_part_frames;
            if (second_part_frames) {
                copy_from_at(0U,
                             second_part_frames,
                             signal,
                             first_part_frames);
                next_write_frame_index = second_part_frames;
            }
        }
    };
}

/**
   \ingroup mhacomplex
   \brief Assign real and imaginary parts to a mha_complex_t variable.
   \param self  The mha_complex_t variable whose value is about to change.
   \param real  The new real part.
   \param imag  The new imaginary part.
   \return      A reference to the changed variable.
*/
inline mha_complex_t & 
set(mha_complex_t & self, mha_real_t real, mha_real_t imag = 0)
{
    self.re = real;
    self.im = imag;
    return self;
}

/**
   \ingroup mhacomplex
   \brief Create a new mha_complex_t with specified real and imaginary parts
   \param real  The real part.
   \param imag  The imaginary part.
   \return      The new value.
*/
inline mha_complex_t 
mha_complex(mha_real_t real, mha_real_t imag = 0)
{
    mha_complex_t tmp;
    set(tmp,real,imag);
    return tmp;
}

/**
   \ingroup mhacomplex
   \brief Assign a mha_complex_t variable from a std::complex.
   \param self       The mha_complex_t variable whose value is about to change.
   \param stdcomplex The new complex value.
   \return           A reference to the changed variable.
*/
inline mha_complex_t &
set(mha_complex_t & self, const std::complex<mha_real_t> & stdcomplex)
{
    self.re = static_cast<mha_real_t>(stdcomplex.real());
    self.im = static_cast<mha_real_t>(stdcomplex.imag());
    return self;
}

/** 
    \ingroup mhacomplex
    \brief Create a std::complex from mha_complex_t.
*/
inline std::complex<mha_real_t>
stdcomplex(const mha_complex_t & self)
{ return std::complex<mha_real_t>(self.re, self.im); }

/** 
    \ingroup mhacomplex
    \brief replaces the value of the given mha_complex_t with exp(i*b).
    \param self    The mha_complex_t variable whose value is about to change.
    \param angle   The angle in the complex plane [rad].
    \return        A reference to the changed variable.
*/
inline mha_complex_t & 
expi(mha_complex_t & self, mha_real_t angle)
{
    return set(self, cos(angle), sin(angle));
}

/** 
    \ingroup mhacomplex
    \brief Computes the angle of a complex number in the complex plane.
    \param self  The complex number whose angle is needed.
    \return      The angle of a complex number in the complex plane.
*/
inline double
angle(const mha_complex_t & self) 
{ return atan2(self.im, self.re); }

/** 
    \ingroup mhacomplex
    \brief Addition of two complex numbers, overwriting the first.
*/
inline mha_complex_t &
operator+=(mha_complex_t & self, const mha_complex_t & other)
{
    self.re += other.re;
    self.im += other.im;
    return self;
}

/** 
    \ingroup mhacomplex
    \brief Addition of two complex numbers, result is a temporary object.
*/
inline mha_complex_t
operator+(const mha_complex_t & self, const mha_complex_t & other)
{
    mha_complex_t tmp(self);
    return tmp += other;
}

/** 
    \ingroup mhacomplex
    \brief Addition of a complex and a real number, overwriting the complex.
*/
inline mha_complex_t &
operator+=(mha_complex_t & self, mha_real_t other_real)
{
    self.re += other_real;
    return self;
}

/** 
    \ingroup mhacomplex
    \brief Addition of a complex and a real number, result is a temporary object.
*/
inline mha_complex_t
operator+(const mha_complex_t & self, mha_real_t other_real)
{
    mha_complex_t tmp(self);
    return tmp += other_real;
}

/** 
    \ingroup mhacomplex
    \brief Subtraction of two complex numbers, overwriting the first.
*/
inline mha_complex_t &
operator-=(mha_complex_t & self, const mha_complex_t & other)
{
    self.re -= other.re;
    self.im -= other.im;
    return self;
}       

/** 
    \ingroup mhacomplex
    \brief Subtraction of two complex numbers, result is a temporary object.
*/
inline mha_complex_t
operator-(const mha_complex_t & self, const mha_complex_t & other)
{
    mha_complex_t tmp(self);
    return tmp -= other;
}       

/** 
    \ingroup mhacomplex
    \brief Subtraction of a complex and a real number, overwriting the complex.
*/
inline mha_complex_t &
operator-=(mha_complex_t & self, mha_real_t other_real)
{
    self.re -= other_real;
    return self;
}

/** 
    \ingroup mhacomplex
    \brief Subtraction of a complex and a real number, result is a temporary object.
*/
inline mha_complex_t
operator-(const mha_complex_t & self, mha_real_t other_real)
{
    mha_complex_t tmp(self);
    return tmp -= other_real;
}

/** 
    \ingroup mhacomplex
    \brief Multiplication of two complex numbers, overwriting the first.
*/
inline mha_complex_t &
operator*=(mha_complex_t & self, const mha_complex_t & other)
{
    mha_real_t tmp_real = self.re * other.re - self.im * other.im;
    self.im = other.re * self.im + self.re * other.im;
    self.re = tmp_real;
    return self;
}

/** 
    \ingroup mhacomplex
    \brief Multiplication of two complex numbers, result is a temporary object.
*/
inline mha_complex_t
operator*(const mha_complex_t & self, const mha_complex_t & other)
{
    mha_complex_t tmp(self);
    return tmp *= other;
}

/** 
    \ingroup mhacomplex
    \brief Multiplication of a complex and a real number, overwriting the complex.
*/
inline mha_complex_t &
operator*=(mha_complex_t & self, mha_real_t other_real)
{
    self.re *= other_real;
    self.im *= other_real;
    return self;
}

/** 
    \ingroup mhacomplex
    \brief replaces (!) the value of the given mha_complex_t with a * exp(i*b)
    \param self    The mha_complex_t variable whose value is about to change.
    \param angle   The imaginary exponent.
    \param factor  The absolute value of the result.
    \return        A reference to the changed variable.
*/
inline mha_complex_t & 
expi(mha_complex_t & self, mha_real_t angle, mha_real_t factor)
{
    expi(self, angle);
    return self *= factor;
}
        
/** 
    \ingroup mhacomplex
    \brief Multiplication of a complex and a real number, result is a temporary object.
*/
inline mha_complex_t
operator*(const mha_complex_t & self, mha_real_t other_real)
{
    mha_complex_t tmp(self);
    return tmp *= other_real;
}

/** 
    \ingroup mhacomplex
    \brief Compute the square of the absolute value of a complex value.
    \return The square of the absolute value of self.
*/
inline
mha_real_t abs2(const mha_complex_t & self)
{
    return self.re * self.re + self.im * self.im;
}

/** 
    \ingroup mhacomplex
    \brief Compute the absolute value of a complex value.
    \return The absolute value of self.
*/
inline
mha_real_t abs(const mha_complex_t & self)
{
    return static_cast<mha_real_t>(sqrt(static_cast<double>(abs2(self))));
}

/** 
    \ingroup mhacomplex
    \brief Division of a complex and a real number, overwriting the complex.
*/
inline mha_complex_t &
operator/=(mha_complex_t & self, mha_real_t other_real)
{
    self.re /= other_real;
    self.im /= other_real;
    return self;
}

/** 
    \ingroup mhacomplex
    \brief Division of a complex and a real number, result is a temporary object.
*/
inline mha_complex_t
operator/(const mha_complex_t & self, mha_real_t other_real)
{
    mha_complex_t tmp(self);
    return tmp /= other_real;
}

/**
   
   \ingroup mhacomplex
   Safe division of two complex numbers, overwriting the first.
   If abs(divisor) < eps, then divisor is replaced by eps. eps2 = eps*eps.
*/
inline mha_complex_t &
safe_div( mha_complex_t & self, const mha_complex_t & other,
          mha_real_t eps, mha_real_t eps2 )
{
    const mha_real_t tmp_abs2 = abs2(other);
    if ( tmp_abs2 < eps2 )
        return self /= eps;
    const mha_real_t tmp_real = self.re * other.re - self.im * -other.im;
    self.im = other.re * self.im + self.re * -other.im;
    self.im /= tmp_abs2;
    self.re = tmp_real / tmp_abs2;
    return self;
}

/** 
    \ingroup mhacomplex
    \brief Division of two complex numbers, overwriting the first.
*/
inline mha_complex_t &
operator/=(mha_complex_t & self, const mha_complex_t & other)
{
    return safe_div(self, other, 0, 0);
}

/** 
    \ingroup mhacomplex
    \brief Division of two complex numbers, result is a temporary object.
*/
inline mha_complex_t
operator/(const mha_complex_t & self, const mha_complex_t & other)
{
    mha_complex_t tmp(self);
    return tmp /= other;
}

/** 
    \ingroup mhacomplex
    \brief Unary minus on a complex results in a negative temporary object.
*/
inline mha_complex_t
operator-(const mha_complex_t & self)
{
    mha_complex_t tmp;
    set(tmp, -self.re, -self.im);
    return tmp;
}

/** 
    \ingroup mhacomplex
    \brief Compare two complex numbers for equality
*/
inline bool
operator==(const mha_complex_t & x, const mha_complex_t & y) 
{
    return (x.re == y.re) && (x.im == y.im);
}

/** 
    \ingroup mhacomplex
    \brief Compare two complex numbers for inequality
*/
inline
bool operator!=(const mha_complex_t & x, const mha_complex_t & y)
{ return !(x == y); }

/** 
    \ingroup mhacomplex
    \brief Replace (!) the value of this mha_complex_t with its conjugate.
*/
inline void
conjugate(mha_complex_t & self)
{
    self.im = -self.im;
}

/** 
    \ingroup mhasignal
    \brief Replace (!) the value of this mha_spec_t with its conjugate.
*/
inline void
conjugate(mha_spec_t & self)
{
    for(unsigned int k=0;k<size(self);k++)
        conjugate(self.buf[k]);
}

/** 
    \ingroup mhacomplex
    \brief Compute the cojugate of this complex value.
    \return A temporary object holding the conjugate value.
*/
inline mha_complex_t
_conjugate(const mha_complex_t & self)
{
    mha_complex_t tmp(self); 
    conjugate(tmp);
    return tmp;
}

/** 
    \ingroup mhacomplex
    \brief Replace the value of this complex with its reciprocal.
*/
inline void
reciprocal(mha_complex_t & self)
{
    conjugate(self);
    self /= abs2(self);
}

/** 
    \ingroup mhacomplex
    \brief compute the reciprocal of this complex value.
    \return A temporary object holding the reciprocal value.
*/
inline mha_complex_t
_reciprocal(const mha_complex_t & self)
{
    mha_complex_t tmp(self); 
    reciprocal(tmp);
    return tmp;
}

/** 
    \ingroup mhacomplex
    \brief Divide a complex by its absolute value, thereby normalizing it
    * (projecting onto the unit circle).
    */
inline void
normalize(mha_complex_t & self)
{
    const mha_real_t tmp = abs(self);
    self.re /= tmp;
    self.im /= tmp;
}

/** 
    \ingroup mhacomplex
    \brief Divide a complex by its absolute value, thereby normalizing it
    (projecting onto the unit circle), with a safety margin.
*/
inline void
normalize(mha_complex_t & self, mha_real_t margin)
{
    const mha_real_t tmp = std::max(abs(self),margin);
    self.re /= tmp;
    self.im /= tmp;
}

/** 
    \ingroup mhacomplex
    \brief Compare two complex numbers for equality except for a small relative
    error.
    \param self
    The first complex number.
    \param other
    The second complex number.
    \param times_epsilon
    Permitted relative error is this number multiplied with the
    machine accuracy for this Floating point format
    (std::numeric_limits<mha_real_t>::epsilon)
    \return true if the relative difference is below
    times_epsilon * std::numeric_limits<mha_real_t>::epsilon
*/
inline bool
almost(const mha_complex_t & self, const mha_complex_t & other,
       mha_real_t times_epsilon = 1e2)
{
    return abs(self - other) / abs(self)
        < times_epsilon * std::numeric_limits<mha_real_t>::epsilon();
}

/** 
    \ingroup mhacomplex
    \brief Compares the absolute values of two complex numbers.
*/
inline
bool operator<(const mha_complex_t & x, const mha_complex_t & y)
{ return abs2(x) < abs2(y); }

    
/**
 * ostream operator for mha_complex_t
 */
inline
std::ostream & operator<<(std::ostream & o, const mha_complex_t & c) {
    return o << "(" << c.re << ((c.im >= 0) ? "+" : "") << c.im << "i)";
}

/**
 * preliminary istream operator for mha_complex_t without error checking
 */
inline
std::istream & operator>>(std::istream & i, mha_complex_t & c) {
    char paren = ' ';
    while (paren == ' ' || paren == '\n' || paren == '\t' || paren == '\r')
        i >> paren;
    if (paren == '(') {
        i >> c.re >> c.im;
        while (paren != ')') i >> paren;
    }
    return i;
}

/**
   \ingroup mhafft
   \brief Create a new FFT handle.
   \param n FFT length.
*/
mha_fft_t mha_fft_new(unsigned int n);
/**
   \ingroup mhafft
   \brief Destroy an FFT handle.
   \param h Handle to be destroyed.
*/
void mha_fft_free(mha_fft_t h);
/**
   \ingroup mhafft
   \brief Tranform waveform segment into spectrum.
   \param h FFT handle.
   \param in Input waveform segment.
   \param out Output spectrum.
*/
void mha_fft_wave2spec(mha_fft_t h,const mha_wave_t* in, mha_spec_t* out);
/**
   \ingroup mhafft
   \brief Tranform waveform segment into spectrum.
   
   Like normal wave2spec, but swaps wave buffer halves before transforming if
   the swaps parameter is true.

   Warning: These \mha FFTs adopt a nonstandard scaling scheme in which
   the forward transform scales by 1/N and the backward does not scale.
   We would recommend using the '_scale' methods instead.
   \param h FFT handle.
   \param in Input waveform segment.
   \param out Output spectrum.
   \param swaps Function swaps the first and second half of the waveform buffer
   before the FFT transform when this parameter is set to true.
*/
void mha_fft_wave2spec(mha_fft_t h,const mha_wave_t* in, mha_spec_t* out,
                       bool swaps);
/**
   \ingroup mhafft
   \brief Tranform spectrum into waveform segment.

   Warning: These \mha FFTs adopt a nonstandard scaling scheme in which
   the forward transform scales by 1/N and the backward does not scale.
   We would recommend using the '_scale' methods instead.
   \param h FFT handle.
   \param in Input spectrum.
   \param out Output waveform segment.
*/
void mha_fft_spec2wave(mha_fft_t h,const mha_spec_t* in, mha_wave_t* out);

/**
   \ingroup mhafft
   \brief Tranform spectrum into waveform segment.
   out may have fewer number of frames than needed for a complete iFFT.
   Only as many frames are written into out as fit, starting with offset
   offset of the complete iFFT.

   Warning: These \mha FFTs adopt a nonstandard scaling scheme in which
   the forward transform scales by 1/N and the backward does not scale.
   We would recommend using the '_scale' methods instead.
   \param h FFT handle.
   \param in Input spectrum.
   \param out Output waveform segment.
   @param offset Offset into iFFT wave buffer
*/
void mha_fft_spec2wave(mha_fft_t h,const mha_spec_t* in, mha_wave_t* out,
                       unsigned int offset);

/**
   \ingroup mhafft
   \brief Complex to complex FFT (forward).

   sIn and sOut need to have nfft bins (please note that mha_spec_t
   typically has nfft/2+1 bins for half-complex representation).

   Warning: These \mha FFTs adopt a nonstandard scaling scheme in which
   the forward transform scales by 1/N and the backward does not scale.
   We would recommend using the '_scale' methods instead.
   \param h FFT handle.
   \param sIn Input spectrum.
   \param sOut Output spectrum.
*/
void mha_fft_forward(mha_fft_t h, mha_spec_t* sIn, mha_spec_t* sOut);


/**
   \ingroup mhafft
   \brief Complex to complex FFT (backward).

   sIn and sOut need to have nfft bins (please note that mha_spec_t
   typically has nfft/2+1 bins for half-complex representation).

   Warning: These \mha FFTs adopt a nonstandard scaling scheme in which
   the forward transform scales by 1/N and the backward does not scale.
   We would recommend using the '_scale' methods instead.
   \param h FFT handle.
   \param sIn Input spectrum.
   \param sOut Output spectrum.
*/
void mha_fft_backward(mha_fft_t h, mha_spec_t* sIn, mha_spec_t* sOut);

/* gkc: scale-correct versions of DFT transforms */
/**
   \ingroup mhafft
   \brief Complex to complex FFT (forward).

   sIn and sOut need to have nfft bins (please note that mha_spec_t
   typically has nfft/2+1 bins for half-complex representation).

   The _scale methods use standard DFT scaling:
   There is no scaling in the forward transformation, and 1/N scaling for the backward.

   \param h FFT handle.
   \param sIn Input spectrum.
   \param sOut Output spectrum.
*/
void mha_fft_forward_scale(mha_fft_t h, mha_spec_t* sIn, mha_spec_t* sOut);

/**
   \ingroup mhafft
   \brief Complex to complex FFT (backward).

   sIn and sOut need to have nfft bins (please note that mha_spec_t
   typically has nfft/2+1 bins for half-complex representation).

   The _scale methods use standard DFT scaling:
   There is no scaling in the forward transformation, and 1/N scaling for the backward.

   \param h FFT handle.
   \param sIn Input spectrum.
   \param sOut Output spectrum.
*/
void mha_fft_backward_scale(mha_fft_t h, mha_spec_t* sIn, mha_spec_t* sOut);

/**
   \ingroup mhafft
   \brief Tranform waveform segment into spectrum.

   The _scale methods use standard DFT scaling:
   There is no scaling in the forward transformation, and 1/N scaling for the backward.

   \param h FFT handle.
   \param in Input waveform segment.
   \param out Output spectrum.
*/
void mha_fft_wave2spec_scale(mha_fft_t h,const mha_wave_t* in, mha_spec_t* out);

/**
   \ingroup mhafft
   \brief Tranform spectrum into waveform segment.

   The _scale methods use standard DFT scaling:
   There is no scaling in the forward transformation, and 1/N scaling for the backward.

   \param h FFT handle.
   \param in Input spectrum.
   \param out Output waveform segment.
*/
void mha_fft_spec2wave_scale(mha_fft_t h,const mha_spec_t* in, mha_wave_t* out);

namespace MHASignal {
    
    /**
       \ingroup mhasignal
       \brief Hilbert transformation of a waveform segment

       Returns the imaginary part of the inverse Fourier
       transformation of the Fourier transformed input signal with
       negative frequencies set to zero.
    */
    class hilbert_t {
    public:
        /** \param len Length of waveform segment */
        hilbert_t(unsigned int len);
        ~hilbert_t();
        /** \brief Apply Hilbert transformation on a waveform segment */
        void operator()(const mha_wave_t*,mha_wave_t*);
    private:
        void* h;
    };

    /**
       \ingroup mhasignal
       \brief Minimal phase function

       The output spectrum \f$Y(f)\f$ is
       \f[
       Y(f) = |X(f)| e^{i \mathcal{H}\left\{\log|X(f)|\right\}},
       \f]
       with the input spectrum \f$X(f)\f$ and the Hilbert
       transformation \f$\mathcal{H}\{\cdots\}\f$.
    */
    class minphase_t : private MHASignal::hilbert_t {
    public:
        /**
           \brief Constructor.
           \param fftlen FFT length
           \param ch Number of channels
        */
        minphase_t(unsigned int fftlen,unsigned int ch);
        /**
           \brief Transform input spectrum to a minimal-phase spectrum, discarding the original phase.
           \param s Spectrum to operate on.
        */
        void operator()(mha_spec_t* s);
    private:
        MHASignal::waveform_t phase;
    };
    
    class stat_t {
    public:
        stat_t(const unsigned int& frames, const unsigned int& channels);
        void mean(mha_wave_t& m);
        void mean_std(mha_wave_t& m,mha_wave_t& s);
        void push(const mha_wave_t&);
        void push(const mha_real_t& x,
                  const unsigned int& k,
                  const unsigned int& ch);
    private:
        MHASignal::waveform_t n;
        MHASignal::waveform_t sum;
        MHASignal::waveform_t sum2;
    };

    class delay_wave_t 
    {
    public:
        delay_wave_t(unsigned int delay,
                     unsigned int frames,
                     unsigned int channels);
        ~delay_wave_t();
        mha_wave_t* process(mha_wave_t*);
    private:
        unsigned int delay;
        MHASignal::waveform_t** buffer;
        unsigned int pos;
    };

    class delay_spec_t 
    {
    public:
        delay_spec_t(unsigned int delay,
                     unsigned int frames,
                     unsigned int channels);
        ~delay_spec_t();
        mha_spec_t* process(mha_spec_t*);
    private:
        unsigned int delay;
        MHASignal::spectrum_t** buffer;
        unsigned int pos;
    };

    class async_rmslevel_t : private MHASignal::waveform_t
    {
    public:
        async_rmslevel_t(unsigned int frames,unsigned int channels);
        std::vector<float> rmslevel() const;
        std::vector<float> peaklevel() const;
        void process(mha_wave_t* s);
    private:
        unsigned int pos;
        unsigned int filled;
    };


    /**
       \ingroup mhasignal
       \brief Vector of unsigned values, used for size and index description of n-dimensional matrixes.
    */
    class uint_vector_t
    {
    public:
        /**
           \brief Constructor, initializes all elements to zero.
           
           \param len Length of vector.
        */
        uint_vector_t(unsigned int len);
        uint_vector_t(const uint_vector_t&);
        /** \brief Construct from memory area 
            \warning This constructor is not real time safe
        */
        uint_vector_t(const uint8_t* buf,unsigned int len);
        ~uint_vector_t();
        /**
           \brief Check for equality
        */
        bool operator==(const uint_vector_t&) const;
        /**
           \brief Assign from other uint_vector_t
           \warning This assignment will fail if the lengths mismatch.
        */
        uint_vector_t& operator=(const uint_vector_t&);
        /**
           \brief Return the length of the vector.
        */
        unsigned int get_length() const {return length;};
        /** \brief Read-only access to elements */
        const uint32_t & operator[](unsigned int k) const {return data[k];};
        /** \brief Access to elements */
        uint32_t & operator[](unsigned int k) {return data[k];};
        /** \brief Return number of bytes needed to store into memory. */
        unsigned int numbytes() const;
        /** \brief Copy to memory area. */
        unsigned int write(uint8_t* buf,unsigned int len) const;
        /** \brief Return pointer to the data field. */
        const uint32_t* getdata() const {return data;};
    protected:
        uint32_t length;
        uint32_t* data;
    };

    /**
       \ingroup mhasignal
       \brief n-dimensional matrix with real or complex floating point values.
       
       \warning The member functions imag() and operator() should only be
       called if the matrix is defined to hold complex values.
    */
    class matrix_t : private MHASignal::uint_vector_t
    {
    public:
        /**
           \brief Create a two-dimensional matrix
           \param nrows Number of rows
           \param ncols Number of columns
           \param b_is_complex Add space for complex values
        */
        matrix_t(unsigned int nrows, unsigned int ncols,bool b_is_complex = true);
        /**
           \brief Create a two-dimensional matrix from a spectrum, copy values
           \param spec Source spectrum structure
        */
        matrix_t(const mha_spec_t& spec);
        /**
           \brief Create n-dimensional matrix, descriped by size argument.
           \param size Size vector
           \param b_is_complex Add space for complex values
        */
        matrix_t(const MHASignal::uint_vector_t& size,bool b_is_complex = true);
        matrix_t(const MHASignal::matrix_t&);
        /** \brief Construct from memory area 
            \warning This constructor is not real time safe
        */
        matrix_t(const uint8_t* buf,unsigned int len);
        ~matrix_t();
        MHASignal::matrix_t& operator=(const MHASignal::matrix_t&);
        /**
           \brief Fill matrix with data of an AC variable object.
           \param v Source AC variable (comm_var_t)
           \note The type and dimension of the AC variable must match the type and dimension of the matrix.
           
        */
        MHASignal::matrix_t& operator=(const comm_var_t& v);
        /**
           \brief Return a AC communication variable pointing to the data of the current matrix
           \return AC variable object (comm_var_t), valid for the life time of the matrix.
           
        */
        comm_var_t get_comm_var();
        /**
           \brief Return the dimension of the matrix
           \return Dimension of the matrix
        */
        unsigned int dimension() const {return MHASignal::uint_vector_t::get_length();};
        /**
           \brief Return the size of the matrix
           \param k Dimension
           \return Size of the matrix in dimension k
        */
        unsigned int size(unsigned int k) const {return MHASignal::uint_vector_t::operator[](k);};
        /**
           \brief Return total number of elements
        */
        unsigned int get_nelements() const;
        /** \brief Test if matrix has same size as other */
        bool is_same_size(const MHASignal::matrix_t&);
        /** \brief Return information about complexity */
        bool iscomplex() const {return complex_ofs==2;};
        /**
           \brief Access real part of an element in a n-dimensional matrix
           \param index Index vector
        */
        mha_real_t& real(const MHASignal::uint_vector_t& index) {return rdata[complex_ofs*get_index(index)];};
        /**
           \brief Access imaginary part of an element in a n-dimensional matrix
           \param index Index vector
        */
        mha_real_t& imag(const MHASignal::uint_vector_t& index) {return rdata[complex_ofs*get_index(index)+1];};
        /**
           \brief Access complex value of an element in a n-dimensional matrix
           \param index Index vector
        */
        mha_complex_t& operator()(const MHASignal::uint_vector_t& index) {return cdata[get_index(index)];};
        /**
           \brief Access real part of an element in a n-dimensional matrix
           \param index Index vector
        */
        const mha_real_t& real(const MHASignal::uint_vector_t& index) const {return rdata[complex_ofs*get_index(index)];};
        /**
           \brief Access imaginary part of an element in a n-dimensional matrix
           \param index Index vector
        */
        const mha_real_t& imag(const MHASignal::uint_vector_t& index) const {return rdata[complex_ofs*get_index(index)+1];};
        /**
           \brief Access complex value of an element in a n-dimensional matrix
           \param index Index vector
        */
        const mha_complex_t& operator()(const MHASignal::uint_vector_t& index) const {return cdata[get_index(index)];};
        /**
           \brief Access real part of an element in a two-dimensional matrix
           \param row Row number of element
           \param col Column number of element
        */
        mha_real_t& real(unsigned int row,unsigned int col) {return rdata[complex_ofs*get_index(row,col)];};
        /**
           \brief Access imaginary part of an element in a two-dimensional matrix
           \param row Row number of element
           \param col Column number of element
        */
        mha_real_t& imag(unsigned int row,unsigned int col) {return rdata[complex_ofs*get_index(row,col)+1];};
        /**
           \brief Access complex value of an element in a two-dimensional matrix
           \param row Row number of element
           \param col Column number of element
        */
        mha_complex_t& operator()(unsigned int row,unsigned int col) {return cdata[get_index(row,col)];};
        /**
           \brief Access real part of an element in a two-dimensional matrix
           \param row Row number of element
           \param col Column number of element
        */
        const mha_real_t& real(unsigned int row,unsigned int col) const {return rdata[complex_ofs*get_index(row,col)];};
        /**
           \brief Access imaginary part of an element in a two-dimensional matrix
           \param row Row number of element
           \param col Column number of element
        */
        const mha_real_t& imag(unsigned int row,unsigned int col) const {return rdata[complex_ofs*get_index(row,col)+1];};
        /**
           \brief Access complex value of an element in a two-dimensional matrix
           \param row Row number of element
           \param col Column number of element
        */
        const mha_complex_t& operator()(unsigned int row,unsigned int col) const {return cdata[get_index(row,col)];};
        unsigned int get_nreals() const {return get_nelements()*complex_ofs;};
        unsigned int get_index(unsigned int row, unsigned int col) const;
        unsigned int get_index(const MHASignal::uint_vector_t& index) const;
        /** \brief Return number of bytes needed to store into memory */
        unsigned int numbytes() const;
        /** \brief Copy to memory area */
        unsigned int write(uint8_t* buf,unsigned int len) const;
        /** \brief Return pointer of real data */
        const mha_real_t* get_rdata() const {return rdata;};
        /** \brief Return pointer of complex data */
        const mha_complex_t* get_cdata() const {return cdata;};
    private:
        uint32_t complex_ofs;
        uint32_t nelements;
        union {
            mha_real_t* rdata;
            mha_complex_t* cdata;
        };
    };

    class schroeder_t : public MHASignal::waveform_t
    {
    public:
        typedef enum { up, down } sign_t;
        /**
           \brief Function type for group delay definition

           \param f Frequency relative to Nyquist frequency.
           \param fmin Minimum frequency relative to Nyquist frequency.
           \param fmax Maximum frequency relative to Nyquist frequency.
        */
        typedef float (*groupdelay_t)(float f,float fmin,float fmax);
        static float identity(float x,float,float){return x;};
        static float log_up(float x,float fmin,float fmax){
            if( x < fmin )
                return 0;
            if( x > fmax )
                return 1;
            return logf(x/fmin)/logf(fmax/fmin);
        };
        static float log_down(float x,float fmin,float fmax){
            if( x < fmin )
                return 0;
            if( x > fmax )
                return -1;
            return -logf(x/fmin)/logf(fmax/fmin);
        };
        schroeder_t(unsigned int len,unsigned int channels = 1,
                    schroeder_t::sign_t sign = up,
                    mha_real_t speed = 1);
        schroeder_t(unsigned int len,
                    unsigned int channels = 1,
                    schroeder_t::groupdelay_t freqfun = MHASignal::schroeder_t::identity,
                    float fmin = 0, float fmax = 1,float eps=1e-10);
    };

    /**
       \brief Simple simulation of fixpoint quantization

    */
    class quantizer_t 
    {
    public:
        /**
           \brief Constructor
           \param num_bits Number of bits to simulate, or zero for limiting to [-1,1] only.
        */
        quantizer_t(unsigned int num_bits);
        /**
           \brief Quantization of a waveform fragment.
           \param s Waveform fragment to be quantized.
        */
        void operator()(mha_wave_t& s);
    private:
        bool limit;
        mha_real_t upscale;
        mha_real_t downscale;
        mha_real_t up_limit;
    };

    class loop_wavefragment_t : public MHASignal::waveform_t
    {
    public:
        typedef enum { relative, peak, rms, rms_limit40 } level_mode_t;
        typedef enum { add, replace, input, mute } playback_mode_t;
        loop_wavefragment_t(const mha_wave_t& src, bool loop, level_mode_t level_mode, std::vector<int> channels, unsigned int startpos = 0);
        std::vector<int> get_mapping(unsigned int channels);
        void playback(mha_wave_t* s, playback_mode_t pmode, mha_wave_t* level_pa, const std::vector<int>& channels);
        void playback(mha_wave_t* s, playback_mode_t pmode, mha_wave_t* level_pa);
        void playback(mha_wave_t* s, playback_mode_t pmode);
        void set_level_lin(mha_real_t l);
        void set_level_db(mha_real_t l);
        void rewind() { pos = 0; };
        void locate_end() { pos = num_frames; };
        inline bool is_playback_active() const {
            return (pos < num_frames) || b_loop;
        }
    private:
        std::vector<int> playback_channels;
        bool b_loop;
        unsigned int pos;
        MHASignal::waveform_t intern_level;
    };

    /**
       \brief Class to realize a simple delay of waveform streams.
    */
    class delay_t {
    public:
        /**
           \brief Constructor
           \param delays Vector of delays, one entry for each channel.
           \param channels Number of channels expected.
        */
        delay_t(std::vector<int> delays,unsigned int channels);
        /**
           \brief Processing method.
           \param s Input waveform fragment, with number of channels provided in constructor.
           \return Output waveform fragment.
        */
        mha_wave_t* process(mha_wave_t* s);
        ~delay_t();
        std::string inspect() const {
                std::ostringstream o;
                o << "<delay_t:" << ((void*)this)
                  << " channels=" << channels
                  << " delays=" << MHAParser::StrCnv::val2str(std::vector<int>(delays,delays+channels))
                  << " pos=" << MHAParser::StrCnv::val2str(std::vector<int>(pos,pos+channels))
                  << " buffer=###>";
                return o.str();
        }
    private:
        unsigned int channels;
        unsigned int* delays;
        unsigned int* pos;
        mha_real_t** buffer;
    };
    
    /** implements subsample delay in spectral domain. When transformed back to
        the time domain, the signal is delayed by the configured fraction of a
        sample. This operation must not be used in a smoothgains bracket. */
    class subsample_delay_t {
    public:
        /** Constructor computes complex phase factors to apply to achieve
            subsample delay 
            @param subsample_delay The subsample delay to apply.
                   -0.5 <= subsample_delay <= 0.5
            @param fftlen FFT length
            @throw MHA_Error if the parameters are out of range
        */
        subsample_delay_t(const std::vector<float>& subsample_delay, unsigned fftlen);

        /** Apply the phase_gains to s to achieve the subsample delay */
        void process(mha_spec_t * s);

        /** Apply the pase gains to channel idx in s to achieve the subsample
            delay in channel idx
            @param s signal
            @param idx channel index, 0-based 
            @throw MHA_Error if idx >= s->num_channels */
        void process (mha_spec_t * s, unsigned idx);

        /** The complex factors to apply to achieve the necessary phase shift */
        spectrum_t phase_gains;

    private:
        /** index of the last complex fft bin for the used fft length. */
        unsigned last_complex_bin;
    };

    /** Fast search for the kth smallest element of an array. The order of
     * elements is altered, but not completely sorted. Using the algorithm
     * from N. Wirth, published in "Algorithms + data structures = programs",
     * Prentice-Hall, 1976 
     * @param array
     *        Element array
     * \post  The order of elements in the array is altered.
     *        array[k] then holds the result.
     * @param n
     *        number of elements in array
     * \pre n >= 1
     * @param k
     *        The k'th smalles element is returned: k = 0 returns the minimum,
     *        k = (n-1)/2 returns the median, k=(n-1) returns the maximum
     * \pre k < n
     * @return The kth smallest array element */
    template <class elem_type>
    elem_type kth_smallest(elem_type array[], unsigned n, unsigned k)
    {
        if (n < 1 || array == NULL || k >= n)
            throw MHA_Error(__FILE__,__LINE__,
                            "kth_smallest(%p,%u,%u): invalid parameters",
                            array, n, k);
        int k_signed = k;
        int i,j,l,m;
        elem_type x;
        elem_type tmp;

        l=0; m=n-1;
        while (l<m) {
            x=array[k_signed] ;
            i=l;
            j=m;
            do {
                while (array[i]<x) ++i;
                while (x<array[j]) --j;
                if (i<=j) {
                    tmp=array[i]; array[i]=array[j]; array[j]=tmp;
                    ++i;
                    --j;
                }
            } while (i<=j) ;
            if (j<k_signed) l=i;
            if (k_signed<i) m=j;
        }
        return array[k_signed];
    }


    /** Fast median search.
     * The order of elements is altered, but not completely sorted.
     * @param array
     *       Element array
     * \post The order of elements in the array is altered.
     *       array[(n-1)/2] then holds the median.
     * @param n
     *       number of elements in array
     * \pre n >= 1
     * @return The median of the array elements */
    template <class elem_type>
    inline
    elem_type median(elem_type array[], unsigned n)
    { return kth_smallest(array, n, (n-1)/2); }

    /**
       \brief Calculate average of elements in a vector
       \param data Input vector
       \param start_val Value for initialization of the return value before sum.
       \return The average of the vector elements
     */
    template <class elem_type>
    inline
    elem_type mean(const std::vector<elem_type>& data,elem_type start_val)
    {
        elem_type retv(start_val);
        for(unsigned int k=0;k<data.size();k++)
            retv += data[k];
        if( data.size() )
            retv /= data.size();
        return retv;
    }

    /**
       \brief Calculate quantile of elements in a vector
       \param data Input vector
       \param p Vector of probability values.
       \return Vector of quantiles of input data, one entry for each probability value.
     */
    template <class elem_type>
    inline
    std::vector<elem_type> quantile(std::vector<elem_type> data,const std::vector<elem_type>& p)
    {
        std::vector<elem_type> retv;
        retv.resize(p.size());
        std::sort(data.begin(),data.end());
        for(unsigned int k=0;k<p.size();k++){
            retv[k] = data[std::min((size_t)(p[k] * (data.size()-1)),(data.size()-1))];
        }
        return retv;
    }

    /**
       \brief Save a \mha spectrum as a variable in a Matlab4 file.

       \param data \mha spectrum to be saved.
       \param varname Matlab variable name (Matlab4 limitations on maximal length are not checked).
       \param fh File handle to Matlab4 file.
     */
    void saveas_mat4(const mha_spec_t& data,const std::string& varname,FILE* fh);

    /**
       \brief Save a \mha waveform as a variable in a Matlab4 file.

       \param data \mha waveform to be saved.
       \param varname Matlab variable name (Matlab4 limitations on maximal length are not checked).
       \param fh File handle to Matlab4 file.
     */
    void saveas_mat4(const mha_wave_t& data,const std::string& varname,FILE* fh);

    /**
       \brief Save a float vector as a variable in a Matlab4 file.

       \param data Float vector to be saved.
       \param varname Matlab variable name (Matlab4 limitations on maximal length are not checked).
       \param fh File handle to Matlab4 file.
     */
    void saveas_mat4(const std::vector<mha_real_t>& data,const std::string& varname,FILE* fh);

    /**
       \brief Copy contents of a waveform to a permuted waveform.

       \param dest Destination waveform
       \param src Source waveform

       The total size of src and dest must be the same, num_frames and
       num_channels must be exchanged in dest.
     */
    void copy_permuted(mha_wave_t* dest,const mha_wave_t* src);
}
#endif

// Local Variables:
// compile-command: "make -C .."
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
