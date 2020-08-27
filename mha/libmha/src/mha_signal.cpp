// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2005 2006 2007 2008 2009 2010 2011 2012 HörTech gGmbH
// Copyright © 2013 2016 2017 2018 2019 2020 HörTech gGmbH
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

#include "mha_signal.hh"
#include "mha_error.hh"
#include "mha_defs.h"
#include <limits>
#include <string.h>
#include <float.h>
#include "mha_signal_fft.h"

/**
   \defgroup mhatoolbox The \mha Toolbox library

   The \mha toolbox is a static C++ library which makes it more
   comfortable to develop \mha plugins. It contains the \mha script
   language classes.

*/

/**
   
   \ingroup mhatoolbox
   \file   mha_signal.hh
   \brief  Header file for audio signal handling and processing classes

   The classes for waveform, spectrum and filterbank signals defined in
   this file are "intelligent" versions of the basic waveform, spectrum
   and filterbank structures used in the C function calls.

*/

/**
   \ingroup mhatoolbox
   \defgroup mhasignal Vector and matrix processing toolbox

   The vector and matrix processing toolbox consists of a number of
   classes defined in the namespace MHASignal, and many functions and
   operators for use with the structures mha_wave_t and mha_spec_t.

*/

/**
   \ingroup mhatoolbox
   \defgroup mhacomplex Complex arithmetics in the \mha

*/

/**
   \ingroup mhatoolbox
   \defgroup mhafft Fast Fourier Transform functions
*/

#define MHA_ID_UINT_VECTOR "MHASignal::uint_vector_t"
#define MHA_ID_MATRIX "MHASignal::matrix_t"

using namespace MHASignal;

#define ASSERT_EQUAL_DIM(a,b) {                                         \
        if( !equal_dim( a, b ) )                                        \
            throw MHA_Error(__FILE__,__LINE__,                          \
                            "Dimension check failed: '%s' is %u x %u (frames x channels), but '%s' is %u x %u.", \
                            #a,(a).num_frames,(a).num_channels,         \
                            #b,(b).num_frames,(b).num_channels);        \
    }

#define ASSERT_EQUAL_DIM_PTR(a,b) {                                     \
        if( !equal_dim( *a, *b ) )                                      \
            throw MHA_Error(__FILE__,__LINE__,                          \
                            "Dimension check failed: '%s' is %u x %u (frames x channels), but '%s' is %u x %u.", \
                            #a,a->num_frames,a->num_channels,           \
                            #b,b->num_frames,b->num_channels);          \
    }

unsigned long int MHASignal::signal_counter = 0;

/**********************************************************************
 **                                                                  **
 **    WAVEFORM                                                      **
 **                                                                  **
 **********************************************************************/


//! constructor of waveform_t
/*!
  Allocates buffer memory and initializes values to zero.
  \param frames number of frames in each channel
  \param channels       number of channels
*/
waveform_t::waveform_t( const unsigned int &frames,
                        const unsigned int &channels )
{
    num_channels = channels;
    num_frames = frames;
    unsigned int alloc_size = 1;
    if( num_channels )
        alloc_size *= num_channels;
    if( num_frames )
        alloc_size *= num_frames;
    buf = new mha_real_t[alloc_size];
    memset( buf, 0, alloc_size * sizeof ( mha_real_t ) );
    channel_info = NULL;
}

/**
   \brief Constructor to create a waveform from plugin configuration
   \param cf Plugin configuration
*/
waveform_t::waveform_t( const mhaconfig_t& cf )
{
    num_channels = cf.channels;
    num_frames = cf.fragsize;
    unsigned int alloc_size = 1;
    if( num_channels )
        alloc_size *= num_channels;
    if( num_frames )
        alloc_size *= num_frames;
    buf = new mha_real_t[alloc_size];
    memset( buf, 0, alloc_size * sizeof ( mha_real_t ) );
    channel_info = NULL;
}

/**
   \brief Copy contructor for mha_wave_t source
*/
waveform_t::waveform_t(const mha_wave_t& src )
{
    num_channels = src.num_channels;
    num_frames = src.num_frames;
    unsigned int alloc_size = 1;
    if( num_channels )
        alloc_size *= num_channels;
    if( num_frames )
        alloc_size *= num_frames;
    buf = new mha_real_t[alloc_size];

    channel_info = NULL;
    copy(src);
    // \todo allocate channel descriptor if src has a valid channel descriptor!
}

/**
   \brief Copy contructor for std::vector<mha_real_t> source

   A waveform structure with a single channel is created, the length
   is equal to the number of elements in the source vector.
*/
waveform_t::waveform_t(const std::vector<mha_real_t>& src )
{
    num_channels = 1;
    num_frames = src.size();
    unsigned int alloc_size = 1;
    if( num_channels )
        alloc_size *= num_channels;
    if( num_frames )
        alloc_size *= num_frames;
    buf = new mha_real_t[alloc_size];

    channel_info = NULL;
    for(unsigned int k=0;k<num_frames;k++)
        buf[k] = src[k];
}

/**
   \brief Copy contructor
*/
waveform_t::waveform_t(const MHASignal::waveform_t& src )
{
    num_channels = src.num_channels;
    num_frames = src.num_frames;
    unsigned int alloc_size = 1;
    if( num_channels )
        alloc_size *= num_channels;
    if( num_frames )
        alloc_size *= num_frames;
    buf = new mha_real_t[alloc_size];

    channel_info = NULL;
    copy(src);
    // \todo allocate channel descriptor if src has a valid channel descriptor!
}

waveform_t::~waveform_t( void )
{
    try {
        if( channel_info )
            delete [] channel_info;
        if( buf )
            delete [] buf;
    }
    catch( MHA_Error & e ) {
        mha_debug( "%s\n", Getmsg(e) );
    }
}

std::vector<mha_real_t> waveform_t::flatten() const {
    return std::vector<mha_real_t>(buf,buf+size(*this));
}
void waveform_t::copy(const std::vector<mha_real_t>& v)
{
    if( (std::min(num_channels,num_frames)==1) && (size(*this) == v.size()) )
        for(unsigned int k=0;k<v.size();k++)
            buf[k] = v[k];
    else
        throw MHA_Error(__FILE__,__LINE__,"Mismatching size in copy: src has %zu entries, target is %ux%u.",
                        v.size(),num_frames,num_channels);
}


//! copy data from source into current waveform
/*!
  \param src    input data (need to be same size as target)
*/
void waveform_t::copy( const mha_wave_t & src )
{
    ASSERT_EQUAL_DIM_PTR( this, (&src) );
    if( buf != src.buf )
        memmove( buf, src.buf, get_size(  ) * sizeof ( buf[0] ) );
    if( channel_info && src.channel_info &&
        ( src.channel_info != channel_info ) ) {
        memmove( channel_info,
                 src.channel_info,
                 num_channels * sizeof ( mha_channel_info_t ) );
    }
}

void waveform_t::copy( const mha_wave_t * src )
{
    ASSERT_EQUAL_DIM_PTR( this, src );
    if( buf != src->buf )
        memmove( buf, src->buf, get_size(  ) * sizeof ( buf[0] ) );
    if( channel_info && src->channel_info &&
        ( src->channel_info != channel_info ) ) {
        memmove( channel_info,
                 src->channel_info,
                 num_channels * sizeof ( mha_channel_info_t ) );
    }
}

/** 
    
\brief Copy one channel of a given waveform signal to a target channel
  
\param src Input waveform signal
\param src_channel Channel in source signal
\param dest_channel Channel number in destination signal
*/
void waveform_t::copy_channel( const mha_wave_t & src,
                               unsigned int src_channel,
                               unsigned int dest_channel )
{
    if( src.num_frames != num_frames )
        throw
            MHA_ErrorMsg
            ( "Cannot copy channel because source and destination frame numbers differ." );
    if( src_channel >= src.num_channels )
        throw
            MHA_ErrorMsg
            ( "Cannot copy channel because the desired source channel is out of range." );
    if( dest_channel >= num_channels )
        throw
            MHA_ErrorMsg
            ( "Cannot copy channels because the desired destination channel is out of range." );
    for( unsigned int k = 0; k < num_frames; k++ )
        buf[k * num_channels + dest_channel] =
            src.buf[k * src.num_channels + src_channel];
    if( src.channel_info && channel_info )
        channel_info[dest_channel] = src.channel_info[src_channel];
}

void MHASignal::copy_channel( mha_wave_t& self,
                              const mha_wave_t& src,
                              unsigned src_channel,
                              unsigned dest_channel )
{
    if( src.num_frames != self.num_frames )
        throw MHA_ErrorMsg( "Cannot copy channel because source and destination frame numbers differ." );
    if( src_channel >= src.num_channels )
        throw MHA_ErrorMsg( "Cannot copy channel because the desired source channel is out of range." );
    if( dest_channel >= self.num_channels )
        throw MHA_ErrorMsg( "Cannot copy channels because the desired destination channel is out of range." );
    for( unsigned int k = 0; k < self.num_frames; k++ )
        self.buf[k * self.num_channels + dest_channel] =
            src.buf[k * src.num_channels + src_channel];
    if( src.channel_info && self.channel_info )
        self.channel_info[dest_channel] = src.channel_info[src_channel];
}


//! transform waveform signal (in Pa) to squared signal (in W/m^2)
/*!
  \param src    linear waveform signal (in Pa)
*/
void waveform_t::power( const waveform_t & src )
{
    unsigned int ch, k;
    mha_real_t val;
    if( src.num_channels != num_channels )
        throw MHA_ErrorMsg( "different number of channels" );
    if( src.num_frames != num_frames )
        throw MHA_ErrorMsg( "different number of frames" );
    for( ch = 0; ch < num_channels; ch++ )
        for( k = 0; k < num_frames; k++ ) {
            val = src.buf[num_channels * k + ch];
            // return the intensity in W/m^2:
            buf[num_channels * k + ch] = 0.0025 * val * val;
        }

}


//! get the power spectrum (in W/m^2) from a complex spectrum
/*!
  \param src    complex spectrum (normalized to Pa)
*/
void waveform_t::powspec( const mha_spec_t & src )
{
    unsigned int ch, k;
    mha_complex_t val;
    if( src.num_channels != num_channels )
        throw MHA_ErrorMsg( "different number of channels" );
    if( src.num_frames != num_frames )
        throw MHA_ErrorMsg( "different number of frames" );
    for( ch = 0; ch < num_channels; ch++ )
        for( k = 0; k < num_frames; k++ ) {
            val = src.buf[num_frames * ch + k];
            // return the intensity in W/m^2:
            buf[num_channels * k + ch] = 0.0025 *
                ( val.re * val.re + val.im * val.im );
        }
}

/**
   \brief scale one channel of target with a scalar

   \param ch     channel number
   \param src    factor
*/
void waveform_t::scale_channel( const unsigned int &ch,
                                const mha_real_t & src )
{
    CHECK_EXPR( ch < num_channels );
    unsigned int k;
    for( k = 0; k < num_frames; k++ )
        buf[num_channels * k + ch] *= src;
}


//! limit target to range [min,max]
/*!
  \param min    lower limit
  \param max    upper limit
*/
void waveform_t::limit( const mha_real_t & min, const mha_real_t & max )
{
    MHASignal::limit(*this,min,max);
}

/** Limit the singal in the waveform buffer to the range [min, max]
 * @param s The signal to limit. The signal in this wave buffer is modified.
 * @param min lower limit
 * @param max upper limit
 */
void MHASignal::limit( mha_wave_t& s, const mha_real_t & min, const mha_real_t & max )
{
    for( unsigned int k=0; k<s.num_frames*s.num_channels; k++ ) {
        if( s.buf[k] < min )
            s.buf[k] = min;
        if( s.buf[k] > max )
            s.buf[k] = max;
    }
}

//! copy data into allocated mha_wave_t structure
/*!
  \param dest   destination structure
*/
void waveform_t::export_to( mha_wave_t & dest )
{
    ASSERT_EQUAL_DIM_PTR( this, (&dest) );
    if( buf != dest.buf )
        memmove( dest.buf, buf, get_size(  ) * sizeof ( buf[0] ) );
    if( channel_info && dest.channel_info &&
        ( dest.channel_info != channel_info ) ) {
        memmove( dest.channel_info,
                 channel_info, num_channels * sizeof ( mha_channel_info_t ) );
    }
}

/**
   \brief sum of all elements
   \return sum of all elements
*/
mha_real_t waveform_t::sum(  )
{
    mha_real_t ret = 0;
    for( unsigned int k = 0; k < get_size(  ); k++ )
        ret += buf[k];
    return ret;
}

/**
   \brief sum of square of all elements
   \return sum of square of all elements
*/
mha_real_t waveform_t::sumsqr(  )
{
    mha_real_t ret = 0;
    for( unsigned int k = 0; k < get_size(  ); k++ )
        ret += buf[k] * buf[k];
    return ret;
}

/**
   \brief sum of all elements between [a,b) in all channels
   \param a      starting frame
   \param b      end frame (excluded)
   \return sum
*/
mha_real_t waveform_t::sum( const unsigned int &a, const unsigned int &b )
{
    CHECK_EXPR( a <= b );
    CHECK_EXPR( b <= num_frames );
    mha_real_t ret = 0;
    unsigned int k, ch;
    for( ch = 0; ch < num_channels; ch++ )
        for( k = a; k < b; k++ )
            ret += buf[num_channels * k + ch];
    return ret;
}


//! sum of all elements between [a,b) in channel ch
/*!
  \param a      starting frame 
  \param b      end frame (exluded)
  \param ch     channel number
  \return sum
*/
mha_real_t waveform_t::sum( const unsigned int &a, const unsigned int &b,
                            const unsigned int &ch )
{
    CHECK_EXPR( a < b );
    CHECK_EXPR( b <= num_frames );
    CHECK_EXPR( ch < num_channels );
    mha_real_t ret = 0;
    unsigned int k;
    for( k = a; k < b; k++ )
        ret += buf[num_channels * k + ch];
    return ret;
}


//! return sum of all elements in one channel
/*!
  \param ch     channel number
  \return sum
*/
mha_real_t waveform_t::sum_channel( const unsigned int &ch )
{
    CHECK_EXPR( ch < num_channels );
    mha_real_t ret = 0;
    unsigned int k;
    for( k = 0; k < num_frames; k++ )
        ret += buf[num_channels * k + ch];
    return ret;
}


//! assign value "val" to frame k in all channels
/*!
  \param k      frame number
  \param val    new value
*/
void waveform_t::assign_frame( const unsigned int &k, const mha_real_t & val )
{
    unsigned int ch;
    CHECK_EXPR( k < num_frames );
    for( ch = 0; ch < num_channels; ch++ )
        buf[num_channels * k + ch] = val;
}

//! assign value "val" to channel ch in all frames
/*!
  \param ch      channel number
  \param val    new value
*/
void waveform_t::assign_channel( const unsigned int &ch, const mha_real_t & val )
{
    unsigned int k;
    CHECK_EXPR( ch < num_channels );
    for( k = 0; k < num_frames; k++ )
        buf[num_channels * k + ch] = val;
}


//! set frame "k" in channel "ch" to value "val"
/*!
  \param k      frame number
  \param ch     channel number
  \param val    new value
*/
void waveform_t::assign( const unsigned int &k, const unsigned int &ch,
                         const mha_real_t & val )
{
    CHECK_EXPR( k < num_frames );
    CHECK_EXPR( ch < num_channels );
    buf[num_channels * k + ch] = val;
}


//! set all elements to value
/*!
  \param val    new value
*/
void waveform_t::assign( const mha_real_t & val )
{
    for( unsigned int k = 0; k < get_size(  ); k++ )
        buf[k] = val;
}

//! scale section [a,b) in channel "ch" by "val"
/*!
  \param a      starting frame
  \param b      end frame (excluded)
  \param ch     channel number
  \param val    scale factor
*/
void waveform_t::scale( const unsigned int &a, const unsigned int &b,
                        const unsigned int &ch, const mha_real_t & val )
{
    CHECK_EXPR( a < b );
    CHECK_EXPR( b <= num_frames );
    CHECK_EXPR( ch < num_channels );
    unsigned int k;
    for( k = a; k < b; k++ )
        buf[num_channels * k + ch] *= val;
}


//! scale one element
/*!
  \param k      frame number 
  \param ch     channel number
  \param val    scale factor
*/
void waveform_t::scale( const unsigned int &k,
                        const unsigned int &ch, const mha_real_t & val )
{
    CHECK_EXPR( k < num_frames );
    CHECK_EXPR( ch < num_channels );
    buf[num_channels * k + ch] *= val;
}


//! Copy part of the source signal into part of this waveform object
/*!
  Source and target have to have the same number of channels.
  @param to_pos Offset in target
  @param len Number of frames copied
  @param src Source
  @param from_pos Offset in source
 */
void waveform_t::copy_from_at( unsigned int to_pos, unsigned int len,
                               const mha_wave_t & src, unsigned int from_pos )
{
    if( to_pos + len > num_frames )
        throw MHA_Error( __FILE__, __LINE__,
                         "destination too small "
                         "(to_pos:%u, len:%u, num_frames:%u)",
                         to_pos, len, num_frames );
    if( from_pos + len > src.num_frames )
        throw MHA_Error( __FILE__, __LINE__,
                         "source too small "
                         "(from_pos:%u, len:%u, num_frames:%u, src.num_frames:%u)",
                         from_pos, len, num_frames, src.num_frames );
    if( num_channels != src.num_channels )
        throw MHA_Error( __FILE__, __LINE__,
                         "channel number mismatch (dest:%u src:%u)",
                         num_channels, src.num_channels );
    memmove( &(buf[to_pos*num_channels]), 
             &(src.buf[from_pos*num_channels]), 
             len * num_channels * sizeof ( buf[0] ) );
}


void waveform_t::scale_frame( const unsigned int &frame,
                              const mha_real_t & val )
{
    CHECK_VAR( buf );
    if( frame > num_frames )
        throw MHA_Error( __FILE__, __LINE__, "frame (%u) > num_frames (%u)",
                         frame, num_frames );
    for( unsigned int ch = 0; ch < num_channels; ch++ )
        buf[num_channels * frame + ch] *= val;
}

/**********************************************************************
 **                                                                  **
 **    SPECTRUM                                                      **
 **                                                                  **
 **********************************************************************/

//! constructor of spectrum class
/*!
  Allocates buffers and initializes memory to zeros.
  \param frames number of frames (fft bins) in one channel. 
                Number of Frames is usually fftlen / 2 + 1
  \param channels       number of channels
*/
spectrum_t::spectrum_t( const unsigned int& frames,
                        const unsigned int& channels )
{
    num_channels = channels;
    num_frames = frames;
    unsigned int alloc_size = 1;
    if( num_channels )
        alloc_size *= num_channels;
    if( num_frames )
        alloc_size *= num_frames;
    buf = new mha_complex_t[alloc_size];
    memset( buf, 0, alloc_size * sizeof ( mha_complex_t ) );
    channel_info = NULL;
}

/**
   \brief Copy constructor
*/
spectrum_t::spectrum_t(const mha_spec_t& src)
{
    num_channels = src.num_channels;
    num_frames = src.num_frames;
    unsigned int alloc_size = 1;
    if( num_channels )
        alloc_size *= num_channels;
    if( num_frames )
        alloc_size *= num_frames;
    buf = new mha_complex_t[alloc_size];
    memset( buf, 0, alloc_size * sizeof ( mha_complex_t ) );
    channel_info = NULL;
    copy(src);
}

/**
   \brief Copy constructor
*/
spectrum_t::spectrum_t(const MHASignal::spectrum_t& src)
{
    num_channels = src.num_channels;
    num_frames = src.num_frames;
    unsigned int alloc_size = 1;
    if( num_channels )
        alloc_size *= num_channels;
    if( num_frames )
        alloc_size *= num_frames;
    buf = new mha_complex_t[alloc_size];
    memset( buf, 0, alloc_size * sizeof ( mha_complex_t ) );
    channel_info = NULL;
    copy(src);
}

spectrum_t::spectrum_t(const std::vector<mha_complex_t>& src)
{
    num_channels = 1;
    num_frames = src.size();
    unsigned int alloc_size = 1;
    if( num_channels )
        alloc_size *= num_channels;
    if( num_frames )
        alloc_size *= num_frames;
    buf = new mha_complex_t[alloc_size];
    memset( buf, 0, alloc_size * sizeof ( mha_complex_t ) );
    channel_info = NULL;
    for(unsigned int k=0;k<src.size();k++)
        buf[k] = src[k];
}

spectrum_t::~spectrum_t( void )
{
    try {
        if( channel_info )
            delete [] channel_info;
        if( buf )
            delete [] buf;
    }
    catch( MHA_Error & e ) {
        mha_debug( "%s\n", Getmsg(e) );
    }
}

//! copy all elements from a spectrum
/*!
  \param src    input spectrum
*/
void spectrum_t::copy( const mha_spec_t & src )
{
    ASSERT_EQUAL_DIM_PTR( this, (&src) );
    if( buf != src.buf )
        memmove( buf, src.buf, size(this) * sizeof ( buf[0] ) );
    if( channel_info && src.channel_info &&
        ( src.channel_info != channel_info ) ) {
        memmove( channel_info,
                 src.channel_info,
                 num_channels * sizeof ( mha_channel_info_t ) );
    }
}

/** 
\brief Copy one channel of a given spectrum signal to a target channel
  
\param s Input spectrum signal
\param sch Channel index in source signal
\param dch Channel index in destination (this) signal
*/
void spectrum_t::copy_channel(const mha_spec_t & s,
                              unsigned sch,
                              unsigned dch)
{
    if( s.num_frames != num_frames )
        throw MHA_ErrorMsg("Cannot copy channel because number of fft bins in"
                         " source and destination signals differ.");
    if( sch >= s.num_channels )
        throw MHA_ErrorMsg("Cannot copy channel because the desired"
                         " source channel is out of range.");
    if( dch >= num_channels )
        throw MHA_ErrorMsg("Cannot copy channels because the desired"
                         " destination channel is out of range.");
    for( unsigned int k = 0; k < num_frames; k++ )
        value(k,dch) = ::value(s,k,sch);
    if( s.channel_info && channel_info )
        channel_info[dch] = s.channel_info[sch];
}

void MHASignal::copy_channel(mha_spec_t& self,
                             const mha_spec_t & s,
                             unsigned sch,
                             unsigned dch)
{
    if( s.num_frames != self.num_frames )
        throw MHA_ErrorMsg("Cannot copy channel because number of fft bins in"
                         " source and destination signals differ.");
    if( sch >= s.num_channels )
        throw MHA_ErrorMsg("Cannot copy channel because the desired"
                         " source channel is out of range.");
    if( dch >= self.num_channels )
        throw MHA_ErrorMsg("Cannot copy channels because the desired"
                         " destination channel is out of range.");
    for( unsigned int k = 0; k < self.num_frames; k++ )
        value(self,k,dch) = ::value(s,k,sch);
    if( s.channel_info && self.channel_info )
        self.channel_info[dch] = s.channel_info[sch];
}

void set_minabs(mha_spec_t& self,const mha_real_t& m)
{
    mha_real_t a;
    for(unsigned int k=0;k<size(self);k++){
        a = abs(self.buf[k]);
        if( a < m )
            expi(self.buf[k],angle(self.buf[k]),m);
    }
}

void MHASignal::scale( mha_spec_t * dest, const mha_wave_t * src )
{
    unsigned int ch, k;
    if( src->num_channels != dest->num_channels )
        throw MHA_ErrorMsg( "different number of channels" );
    if( src->num_frames != dest->num_frames )
        throw MHA_ErrorMsg( "different number of frames" );
    for( ch = 0; ch < dest->num_channels; ch++ )
        for( k = 0; k < dest->num_frames; k++ ) {
            dest->buf[dest->num_frames * ch + k].re *=
                src->buf[dest->num_channels * k + ch];
            dest->buf[dest->num_frames * ch + k].im *=
                src->buf[dest->num_channels * k + ch];
        }
}


//! scale all elements in one channel
/*!
  \param ch     channel number
  \param src    scale factor
*/
void spectrum_t::scale_channel( const unsigned int &ch,
                                const mha_real_t & src )
{
    unsigned int k;
    for( k = 0; k < num_frames; k++ ) {
        buf[num_frames * ch + k].re *= src;
        buf[num_frames * ch + k].im *= src;
    }
}


//! copy elements to spectrum structure
/*!
  \param dest   destination spectrum structure
*/
void spectrum_t::export_to( mha_spec_t & dest )
{
    ASSERT_EQUAL_DIM_PTR( this, (&dest) );
    if( buf != dest.buf )
        memmove( dest.buf, buf, size(this) * sizeof ( buf[0] ) );
    if( channel_info && dest.channel_info &&
        ( dest.channel_info != channel_info ) ) {
        memmove( dest.channel_info,
                 channel_info, num_channels * sizeof ( mha_channel_info_t ) );
    }
}

//! scale section [a,b) in channel "ch" by "val"
/*!
  \param a      starting frame
  \param b      end frame (excluded)
  \param ch     channel number
  \param val    scale factor
*/
void spectrum_t::scale( const unsigned int &a, const unsigned int &b,
                        const unsigned int &ch, const mha_real_t & val )
{
    CHECK_EXPR( a <= b );
    CHECK_EXPR( b <= num_frames );
    CHECK_EXPR( ch < num_channels );
    unsigned int k;
    for( k = a; k < b; k++ ) {
        buf[num_frames * ch + k].re *= val;
        buf[num_frames * ch + k].im *= val;
    }
}

mha_wave_t & operator+=( mha_wave_t & self, const mha_real_t & v )
{
    for( unsigned int k = 0; k < size( self ); k++ )
        self.buf[k] += v;
    return self;
}

mha_wave_t & operator*=( mha_wave_t & self, const mha_real_t & v )
{
    for( unsigned int k = 0; k < size( self ); k++ )
        self.buf[k] *= v;
    return self;
}

mha_spec_t & operator*=( mha_spec_t & self, const mha_real_t & v )
{
    for( unsigned int k = 0; k < size( self ); k++ ) {
        self.buf[k].re *= v;
        self.buf[k].im *= v;
    }
    return self;
}

mha_wave_t & operator*=( mha_wave_t & self, const mha_wave_t & v )
{
    ASSERT_EQUAL_DIM(self,v);
    unsigned int ch, k;
    for(k = 0; k < self.num_frames; k++ )
        for( ch=0;ch<self.num_channels; ch++)
            value(self,k,ch) *= value(v,k,ch);
    return self;
}

mha_spec_t & operator*=( mha_spec_t & self, const mha_wave_t & v )
{
    ASSERT_EQUAL_DIM(self,v);
    unsigned int ch, k;
    for(k = 0; k < self.num_frames; k++ )
        for( ch=0;ch<self.num_channels; ch++)
            value(self,k,ch) *= value(v,k,ch);
    return self;
}

mha_spec_t & operator*=( mha_spec_t & self, const mha_spec_t & v )
{
    ASSERT_EQUAL_DIM(self,v);
    unsigned int ch, k;
    for(k = 0; k < self.num_frames; k++ )
        for( ch=0;ch<self.num_channels; ch++)
            value(self,k,ch) *= value(v,k,ch);
    return self;
}

mha_spec_t & safe_div( mha_spec_t & self, const mha_spec_t & v,
                       mha_real_t eps )
{
    if( size( self ) != size( v ) )
        throw MHA_Error( __FILE__, __LINE__,
                         "Mismatching dimension in operator *=" );
    mha_real_t eps2 = eps * eps;
    for( unsigned int k = 0; k < size( self ); k++ )
        safe_div(self.buf[k], v.buf[k], eps, eps2);
    return self;
}

mha_spec_t & operator/=( mha_spec_t & self, const mha_spec_t & v )
{
    return safe_div(self, v, 0);
}

mha_wave_t & operator/=( mha_wave_t & self, const mha_wave_t & v )
{
    if( size( self ) != size( v ) )
        throw MHA_Error( __FILE__, __LINE__,
                         "Mismatching dimension in operator /= (mha_wave_t)" );
    for(unsigned int k=0;k<size(self);k++)
        self.buf[k] /= v.buf[k];
    return self;
}

mha_spec_t & operator+=( mha_spec_t & self, const mha_spec_t & v )
{
    ASSERT_EQUAL_DIM(self,v);
    unsigned int ch, k;
    for(k = 0; k < self.num_frames; k++ )
        for( ch=0;ch<self.num_channels; ch++)
            value(self,k,ch) += value(v,k,ch);
    return self;
}

mha_spec_t & operator+=( mha_spec_t & self, const mha_real_t & v )
{
    for( unsigned int k = 0; k < size( self ); k++ )
        self.buf[k] += v;
    return self;
}

mha_wave_t & operator+=( mha_wave_t & self, const mha_wave_t & v )
{
    ASSERT_EQUAL_DIM(self,v);
    unsigned int ch, k;
    for(k = 0; k < self.num_frames; k++ )
        for( ch=0;ch<self.num_channels; ch++)
            value(self,k,ch) += value(v,k,ch);
    return self;
}

mha_wave_t & operator-=( mha_wave_t & self, const mha_wave_t & v )
{
    ASSERT_EQUAL_DIM(self,v);
    unsigned int ch, k;
    for(k = 0; k < self.num_frames; k++ )
        for( ch=0;ch<self.num_channels; ch++)
            value(self,k,ch) -= value(v,k,ch);
    return self;
}

mha_spec_t & operator-=( mha_spec_t & self, const mha_spec_t & v )
{
    ASSERT_EQUAL_DIM(self,v);
    unsigned int ch, k;
    for(k = 0; k < self.num_frames; k++ )
        for( ch=0;ch<self.num_channels; ch++)
            value(self,k,ch) -= value(v,k,ch);
    return self;
}

MHASignal::doublebuffer_t::doublebuffer_t(unsigned int nchannels_in,
                                          unsigned int nchannels_out,
                                          unsigned int outer_fragsize,
                                          unsigned int inner_fragsize)
    : outer_out(outer_fragsize,nchannels_out),
      inner_in(inner_fragsize,nchannels_in),
      inner_out(inner_fragsize,nchannels_out),
      k_inner(0),
      k_outer(0),
      ch(0)
{
    this_outer_out = outer_out;
}

MHASignal::doublebuffer_t::~doublebuffer_t()
{
}

mha_wave_t* MHASignal::doublebuffer_t::outer_process(mha_wave_t* outer_in)
{
    if( outer_in->num_frames > outer_out.num_frames )
        throw MHA_Error(__FILE__,__LINE__,
                        "Doublebuffer: Input size (%u frames) exceeded the maximum input size (%u).",
                        outer_in->num_frames, outer_out.num_frames);
    if( outer_in->num_channels != inner_in.num_channels )
        throw MHA_Error(__FILE__,__LINE__,
                        "Doublebuffer: Input has %u channels, but expected %u.",
                        outer_in->num_channels, inner_in.num_channels);
    this_outer_out.num_frames = outer_in->num_frames;
    for(k_outer=0;k_outer<outer_in->num_frames;k_outer++){
        for(ch=0;ch<inner_in.num_channels;ch++)
            inner_in(k_inner,ch) = value(outer_in,k_outer,ch);
        for(ch=0;ch<inner_out.num_channels;ch++)
            value(this_outer_out,k_outer,ch) = inner_out(k_inner,ch);
        k_inner++;
        if( k_inner == inner_in.num_frames ){
            k_inner = 0;
            inner_out.copy( inner_process( &inner_in ) );
        }
    }
    return &this_outer_out;
}

ringbuffer_t::ringbuffer_t(unsigned frames,
                           unsigned channels,
                           unsigned prefilled_frames)
    : waveform_t(frames + 1U, channels),
      next_read_frame_index(0U),
      next_write_frame_index(prefilled_frames)
{
    if (prefilled_frames > frames) 
        throw MHA_Error(__FILE__,__LINE__,
                        "ringbuffer_t: %u prefilled_frames > %u frames capacity",
                        prefilled_frames, frames);
}

/********************************************************************/
/********************************************************************/
/**************                 FFT                     *************/
/********************************************************************/
/********************************************************************/

MHASignal::fft_t::fft_t( const unsigned int &n )
    : nfft( n ), 
      n_re( 1 + n / 2 ), 
      n_im( ( 1 + n ) / 2 - 1 ), 
      scale( 1.0 / (mha_real_t)n ), 
      buf_in( NULL ), 
      buf_out( NULL )
{
    if( n < 2 )
        throw MHA_Error( __FILE__, __LINE__, "fft length is too small (%u < 2)", n );
    fftw_plan_wave2spec = rfftw_create_plan( nfft, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE );
    fftw_plan_spec2wave = rfftw_create_plan( nfft, FFTW_COMPLEX_TO_REAL, FFTW_ESTIMATE );
    fftw_plan_fft = fftw_create_plan( nfft, FFTW_FORWARD, FFTW_ESTIMATE );
    fftw_plan_ifft = fftw_create_plan( nfft, FFTW_BACKWARD, FFTW_ESTIMATE );
    // this is 2 times as much as needed:
    buf_in = new mha_real_t[2 * nfft];
    buf_out = new mha_real_t[2 * nfft];
}

MHASignal::fft_t::~fft_t(  )
{
    rfftw_destroy_plan( fftw_plan_wave2spec );
    rfftw_destroy_plan( fftw_plan_spec2wave );
    fftw_destroy_plan( fftw_plan_fft );
    fftw_destroy_plan( fftw_plan_ifft );
    if( buf_in )
        delete [] buf_in;
    if( buf_out )
        delete [] buf_out;
}

/** Arrange the order of an fftw spectrum to the internal order 
 *
 * The fftw spectrum is arranged [r0 r1 r2 ... rn-1 in in-1 ... i1],
 * while the interal order is [r0 -- r1 i1 r2 i2 ... rn-1 in-1 rn --].
 */
void MHASignal::fft_t::sort_fftw2spec( fftw_real * s_fftw, mha_spec_t * s_spec, unsigned int ch )
{
    if( sizeof ( mha_real_t ) != sizeof ( fftw_real ) )
        throw MHA_ErrorMsg( "MHA and FFTW use different precision" );
    if( s_spec->num_frames < n_re ){
        throw MHA_Error( __FILE__, __LINE__,
                         "Input spectrum contains only %u bins, but %u real parts are available.",
                         s_spec->num_frames, n_re );
    }
    if( s_spec->num_frames < n_im + 1 ){
        throw MHA_Error( __FILE__, __LINE__,
                         "Input spectrum contains only %u bins, but %u imaginary parts are available.",
                         s_spec->num_frames, n_im );
    }
    unsigned int k;
    for( k = 0; k < n_re; k++ )
        value( s_spec, k, ch ).re = s_fftw[k];
    for( k = n_re; k < s_spec->num_frames; k++ )
        value( s_spec, k, ch ).re = 0;
    for( k = 1; k < n_im + 1; k++ )
        value( s_spec, k, ch ).im = s_fftw[nfft - k];
    for( k = n_im + 1; k < s_spec->num_frames; k++ )
        value( s_spec, k, ch ).im = 0;
    value( s_spec, 0, ch ).im = 0;
}

/** Arrange the order of an internal spectrum to the fftw order */
void MHASignal::fft_t::sort_spec2fftw( fftw_real * s_fftw, const mha_spec_t * s_spec, unsigned int ch )
{
    if( sizeof ( mha_real_t ) != sizeof ( fftw_real ) )
        throw MHA_ErrorMsg( "MHA and FFTW use different precision" );
    if( s_spec->num_frames < n_re )
        throw MHA_Error( __FILE__, __LINE__,
                         "Input spectrum contains only %u bins, but %u real parts are available.",
                         s_spec->num_frames, n_re );
    if( s_spec->num_frames < n_im + 1 )
        throw MHA_Error( __FILE__, __LINE__,
                         "Input spectrum contains only %u bins, but %u imaginary parts are available.",
                         s_spec->num_frames, n_im );
    unsigned int k;
    for( k = 0; k < n_re; k++ )
        s_fftw[k] = value( s_spec, k, ch ).re;
    for( k = 1; k < n_im + 1; k++ )
        s_fftw[nfft - k] = value( s_spec, k, ch ).im;
}

void MHASignal::fft_t::forward( mha_spec_t* sIn, mha_spec_t* sOut )
{
    CHECK_VAR( sIn );
    CHECK_VAR( sOut );
    if( sIn->num_frames != nfft )
        throw MHA_Error( __FILE__,__LINE__,
                         "fft: The input spectrum does not contain fftlen bins (sIn: %u, nFFT: %u).",
                         sIn->num_frames, nfft );
    if( sOut->num_frames != nfft )
        throw MHA_Error( __FILE__,__LINE__,
                         "fft: The output spectrum does not contain fftlen bins (sOut: %u, nFFT: %u).",
                         sOut->num_frames, nfft );
    if( sIn->num_channels != sOut->num_channels )
        throw MHA_Error( __FILE__, __LINE__, 
                         "fft: Mismatching number of channels in input and output (input: %u, output: %u).",
                         sIn->num_channels, sOut->num_channels );
    fftw(fftw_plan_fft,sIn->num_channels,
         (fftw_complex*)(sIn->buf),1,sIn->num_frames,
         (fftw_complex*)(sOut->buf),1,sOut->num_frames);
    *sOut *= 1.0/nfft;
}

void MHASignal::fft_t::backward( mha_spec_t* sIn, mha_spec_t* sOut )
{
    CHECK_VAR( sIn );
    CHECK_VAR( sOut );
    if( sIn->num_frames != nfft )
        throw MHA_Error( __FILE__,__LINE__,
                         "fft: The input spectrum does not contain fftlen bins (sIn: %u, nFFT: %u).",
                         sIn->num_frames, nfft );
    if( sOut->num_frames != nfft )
        throw MHA_Error( __FILE__,__LINE__,
                         "fft: The output spectrum does not contain fftlen bins (sOut: %u, nFFT: %u).",
                         sOut->num_frames, nfft );
    if( sIn->num_channels != sOut->num_channels )
        throw MHA_Error( __FILE__, __LINE__, 
                         "fft: Mismatching number of channels in input and output (input: %u, output: %u).",
                         sIn->num_channels, sOut->num_channels );
    fftw(fftw_plan_ifft,sIn->num_channels,
         (fftw_complex*)(sIn->buf),1,sIn->num_frames,
         (fftw_complex*)(sOut->buf),1,sOut->num_frames);
}

void MHASignal::fft_t::wave2spec( const mha_wave_t * wave, mha_spec_t * spec,
                                  bool swap)
{
    CHECK_VAR( wave );
    CHECK_VAR( spec );
    if( wave->num_channels != spec->num_channels )
        throw MHA_Error(__FILE__,__LINE__,
                        "fft: Mismatching channel number: waveform has %u, spectrum has %u.",
                        wave->num_channels, spec->num_channels );

    unsigned int k, ch, max_frames(std::min(nfft,wave->num_frames));
    for( ch = 0; ch < wave->num_channels; ch++ ) {
        for( k = 0; k < max_frames; ++k )
            buf_in[swap ? ((k + wave->num_frames / 2) % wave->num_frames) : k] =
                scale * wave->buf[k * wave->num_channels + ch];
        for( k = max_frames; k < nfft; ++k )
            buf_in[swap ? ((k + wave->num_frames / 2) % wave->num_frames) : k] = 0;
        rfftw_one( fftw_plan_wave2spec, buf_in, buf_out );
        sort_fftw2spec( buf_out, spec, ch );
    }
}

void MHASignal::fft_t::spec2wave( const mha_spec_t * spec, mha_wave_t * wave )
{
    CHECK_VAR( wave );
    CHECK_VAR( spec );
    if( wave->num_channels != spec->num_channels )
        throw MHA_Error( __FILE__, __LINE__,
                         "channel number mismatch in spec2wave: spec has %u"
                         " channels, wave has %u channels",
                         spec->num_channels, wave->num_channels);
    if( wave->num_frames != nfft )
        throw MHA_Error( __FILE__, __LINE__, "waveform has invalid length (%u, nfft:%u)", wave->num_frames, nfft );
    unsigned int ch, k;
    for( ch = 0; ch < wave->num_channels; ch++ ) {
        sort_spec2fftw( buf_in, spec, ch );
        rfftw_one( fftw_plan_spec2wave, buf_in, buf_out );
        for( k = 0; k < wave->num_frames; k++ )
            wave->buf[wave->num_channels * k + ch] = buf_out[k];
    }
}

/** 
 * wave may have fewer number of frames than needed for a complete iFFT.
 * Only as many frames are written into wave as fit, starting with offset
 * offset of the complete iFFT. */
void MHASignal::fft_t::spec2wave( const mha_spec_t * spec, mha_wave_t * wave,
                                  unsigned int offset )
{
    CHECK_VAR( wave );
    CHECK_VAR( spec );
    if( wave->num_channels != spec->num_channels )
        throw MHA_Error(__FILE__,__LINE__,
                        "spec2wave(0x%p,0x%p,%u): channel number mismatch"
                        " (spec has %u channels, wave has %u channels)",
                        spec, wave, offset,
                        spec->num_channels, wave->num_channels);
    if( offset > nfft )
        throw MHA_ErrorMsg( "offset has to be smaller than or equal to the fft length" );
    if( wave->num_frames > nfft - offset )
        throw MHA_Error( __FILE__, __LINE__, "waveform has invalid length (%u, nfft:%u, offset:%u)",
                         wave->num_frames, nfft, offset );
    unsigned int ch, k;
    for( ch = 0; ch < wave->num_channels; ch++ ) {
        sort_spec2fftw( buf_in, spec, ch );
        rfftw_one( fftw_plan_spec2wave, buf_in, buf_out );
        for( k = 0; k < wave->num_frames; k++ )
            wave->buf[wave->num_channels * k + ch] = buf_out[k+offset];
    }
}

/* gkc: scale correct versions */
void MHASignal::fft_t::wave2spec_scale( const mha_wave_t * wave, mha_spec_t * spec,
                                  bool swap)
{
    CHECK_VAR( wave );
    CHECK_VAR( spec );
    if( wave->num_channels != spec->num_channels )
        throw MHA_Error(__FILE__,__LINE__,
            "fft: Mismatching channel number: waveform has %u, spectrum has %u.",wave->num_channels, spec->num_channels );
    unsigned int k, ch, max_frames(std::min(nfft,wave->num_frames));
    for( ch = 0; ch < wave->num_channels; ch++ ) {
        for( k = 0; k < max_frames; ++k )
            buf_in[swap ? ((k + wave->num_frames / 2) % wave->num_frames) : k] =
                wave->buf[k * wave->num_channels + ch]; //removed scale ehre
        for( k = max_frames; k < nfft; ++k )
            buf_in[swap ? ((k + wave->num_frames / 2) % wave->num_frames) : k] = 0;
        rfftw_one( fftw_plan_wave2spec, buf_in, buf_out );
        sort_fftw2spec( buf_out, spec, ch );
    }
}

void MHASignal::fft_t::spec2wave_scale( const mha_spec_t * spec, mha_wave_t * wave )
{
    CHECK_VAR( wave );
    CHECK_VAR( spec );
    if( wave->num_channels != spec->num_channels )
        throw MHA_Error( __FILE__, __LINE__,
                         "channel number mismatch in spec2wave: spec has %u"
                         " channels, wave has %u channels",
                         spec->num_channels, wave->num_channels);
    if( wave->num_frames != nfft )
        throw MHA_Error( __FILE__, __LINE__, "waveform has invalid length (%u, nfft:%u)", wave->num_frames, nfft );
    unsigned int ch, k;
    for( ch = 0; ch < wave->num_channels; ch++ ) {
        sort_spec2fftw( buf_in, spec, ch );
        rfftw_one( fftw_plan_spec2wave, buf_in, buf_out );
        for( k = 0; k < wave->num_frames; k++ )
            wave->buf[wave->num_channels * k + ch] = scale * buf_out[k]; //added scale
    }
}

void MHASignal::fft_t::forward_scale( mha_spec_t* sIn, mha_spec_t* sOut )
{
    CHECK_VAR( sIn );
    CHECK_VAR( sOut );
    if( sIn->num_frames != nfft )
        throw MHA_Error( __FILE__,__LINE__,
                         "fft: The input spectrum does not contain fftlen bins (sIn: %u, nFFT: %u).",
                         sIn->num_frames, nfft );
    if( sOut->num_frames != nfft )
        throw MHA_Error( __FILE__,__LINE__,
                         "fft: The output spectrum does not contain fftlen bins (sOut: %u, nFFT: %u).",
                         sOut->num_frames, nfft );
    if( sIn->num_channels != sOut->num_channels )
        throw MHA_Error( __FILE__, __LINE__,
                         "fft: Mismatching number of channels in input and output (input: %u, output: %u).",
                         sIn->num_channels, sOut->num_channels );
    fftw(fftw_plan_fft,sIn->num_channels,
         (fftw_complex*)(sIn->buf),1,sIn->num_frames,
         (fftw_complex*)(sOut->buf),1,sOut->num_frames);
    *sOut *= 1.0; // removed scaling
}

void MHASignal::fft_t::backward_scale( mha_spec_t* sIn, mha_spec_t* sOut )
{
    CHECK_VAR( sIn );
    CHECK_VAR( sOut );
    if( sIn->num_frames != nfft )
        throw MHA_Error( __FILE__,__LINE__,
                         "fft: The input spectrum does not contain fftlen bins (sIn: %u, nFFT: %u).",
                         sIn->num_frames, nfft );
    if( sOut->num_frames != nfft )
        throw MHA_Error( __FILE__,__LINE__,
                         "fft: The output spectrum does not contain fftlen bins (sOut: %u, nFFT: %u).",
                         sOut->num_frames, nfft );
    if( sIn->num_channels != sOut->num_channels )
        throw MHA_Error( __FILE__, __LINE__,
                         "fft: Mismatching number of channels in input and output (input: %u, output: %u).",
                         sIn->num_channels, sOut->num_channels );
    fftw(fftw_plan_ifft,sIn->num_channels,
         (fftw_complex*)(sIn->buf),1,sIn->num_frames,
         (fftw_complex*)(sOut->buf),1,sOut->num_frames);
    *sOut *= 1.0/nfft;
}


/** \brief Create a new instance of an FFT object

\param n FFT length
\retval FFT object
*/
mha_fft_t mha_fft_new(unsigned int n)
{
    return new MHASignal::fft_t(n);
}

/** \brief Remove an FFT object
 
\param h FFT object to be removed
*/
void mha_fft_free(mha_fft_t h)
{
    delete (MHASignal::fft_t*)h;
}

/** \brief Perform an FFT on each channel of input waveform signal
 
\param h FFT object handle
\param in pointer to input waveform signal
\param out pointer to output spectrum signal (has to be allocated)
*/
void mha_fft_wave2spec(mha_fft_t h,const mha_wave_t* in, mha_spec_t* out)
{
    ((MHASignal::fft_t*)h)->wave2spec(in,out, false);
}

void mha_fft_wave2spec(mha_fft_t h,const mha_wave_t* in, mha_spec_t* out, 
                       bool swap)
{
    ((MHASignal::fft_t*)h)->wave2spec(in,out,swap);
}

/** \brief Perform an inverse FFT on each channel of input spectrum

\param h FFT object handle
\param in pointer to input spectrum
\param out pointer to output waveform signal (has to be allocated)
*/
void mha_fft_spec2wave(mha_fft_t h,const mha_spec_t* in, mha_wave_t* out)
{
    ((MHASignal::fft_t*)h)->spec2wave(in,out);
}

/** \brief Perform an inverse FFT on each channel of input spectrum.
Only part of the iFFT is tranferred into the out buffer.

Out may have fewer number of freames than needed for a complete iFFT.
Only as many frames are written into out as fit, starting with offset
offset of the complete iFFT.

\param h FFT object handle
\param in pointer to input spectrum
\param out pointer to output waveform signal (has to be allocated)
@param offset Offset into complete iFFT buffer.
*/
void mha_fft_spec2wave(mha_fft_t h,const mha_spec_t* in, mha_wave_t* out,
                       unsigned int offset)
{
    ((MHASignal::fft_t*)h)->spec2wave(in,out,offset);
}

void mha_fft_forward(mha_fft_t h, mha_spec_t* sIn, mha_spec_t* sOut)
{
    ((MHASignal::fft_t*)h)->forward(sIn,sOut);
}

void mha_fft_backward(mha_fft_t h, mha_spec_t* sIn, mha_spec_t* sOut)
{
    ((MHASignal::fft_t*)h)->backward(sIn,sOut);
}


/* gkc: scale-correct versions of DFT transforms */
void mha_fft_forward_scale(mha_fft_t h, mha_spec_t* sIn, mha_spec_t* sOut)
{
    ((MHASignal::fft_t*)h)->forward_scale(sIn,sOut);
}

void mha_fft_backward_scale(mha_fft_t h, mha_spec_t* sIn, mha_spec_t* sOut)
{
    ((MHASignal::fft_t*)h)->backward_scale(sIn,sOut);
}

void mha_fft_wave2spec_scale(mha_fft_t h,const mha_wave_t* in, mha_spec_t* out)
{
    ((MHASignal::fft_t*)h)->wave2spec_scale(in,out, false);
}

void mha_fft_spec2wave_scale(mha_fft_t h,const mha_spec_t* in, mha_wave_t* out)
{
    ((MHASignal::fft_t*)h)->spec2wave_scale(in,out);
}

/*
 * HILBERT
 */

namespace MHASignal {
    class hilbert_fftw_t {
    public:
        /** C'tor of hilbert_fftw_t
         * @param len fft length
         **/
        hilbert_fftw_t(unsigned int len);
        /** D'tor of hilbert_fftw_t
         **/
        ~hilbert_fftw_t();
        void hilbert(const mha_wave_t*,mha_wave_t*);
    private:
        unsigned int n;
        rfftw_plan p1;
        fftw_plan p2;
        fftw_real* buf_r_in;
        fftw_real* buf_r_out;
        fftw_complex* buf_c_in;
        fftw_complex* buf_c_out;
        mha_real_t sc;
    };
}

MHASignal::hilbert_fftw_t::hilbert_fftw_t(unsigned int len)
    : n(len),
      buf_r_in(new fftw_real[n]),
      buf_r_out(new fftw_real[n]),
      buf_c_in(new fftw_complex[n]),
      buf_c_out(new fftw_complex[n])
{
    p1 = rfftw_create_plan( n, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE );
    p2 = fftw_create_plan( n, FFTW_BACKWARD, FFTW_ESTIMATE );
    sc = 2.0/(mha_real_t)n;
}

MHASignal::hilbert_fftw_t::~hilbert_fftw_t()
{
    delete [] buf_r_in;
    delete [] buf_r_out;
    delete [] buf_c_in;
    delete [] buf_c_out;
}

void MHASignal::hilbert_fftw_t::hilbert(const mha_wave_t* s_in,mha_wave_t* s_out)
{
    if( !s_in )
        throw MHA_ErrorMsg("hilbert: Invalid input signal pointer (NULL).");
    if( !s_out )
        throw MHA_ErrorMsg("hilbert: Invalid output signal pointer (NULL).");
    if( s_in->num_frames != n )
        throw MHA_Error(__FILE__,__LINE__,
                        "hilbert: Invalid input signal dimension (s_in->num_frames: %u, n: %u).",
                        s_in->num_frames, n);
    if( s_out->num_frames != n )
        throw MHA_Error(__FILE__,__LINE__,
                        "hilbert: Invalid output signal dimension (s_out->num_frames: %u, n: %u).",
                        s_out->num_frames, n);
    if( s_in->num_channels != s_out->num_channels )
        throw MHA_Error(__FILE__,__LINE__,
                        "hilbert: Input and output signal need same number of channels (in: %u, out: %u).",
                        s_in->num_channels, s_out->num_channels);
    unsigned int ch, k;
    for( ch=0;ch<s_in->num_channels;ch++){
        for(k=0;k<n;k++)
            buf_r_in[k] = value(s_in,k,ch);
        rfftw_one(p1,buf_r_in,buf_r_out);
        memset(buf_c_in,0,n*sizeof(buf_c_in[0]));
        for(k=0;k<n/2+1;k++)
            buf_c_in[k].re = buf_r_out[k];
        for(k=n/2+1;k<n;k++)
            buf_c_in[n-k].im = buf_r_out[k];
        fftw_one(p2,buf_c_in,buf_c_out);
        for(k=0;k<n;k++)
            value(s_out,k,ch) = sc * buf_c_out[k].im;
    }
}
    

MHASignal::hilbert_t::hilbert_t(unsigned int len)
    : h(new hilbert_fftw_t(len))
{
}

MHASignal::hilbert_t::~hilbert_t()
{
    delete (hilbert_fftw_t*)h;
}

void MHASignal::hilbert_t::operator()(const mha_wave_t* s_in,mha_wave_t* s_out)
{
    ((hilbert_fftw_t*)h)->hilbert(s_in,s_out);
}

MHASignal::minphase_t::minphase_t(unsigned int nfft,unsigned int ch)
    : MHASignal::hilbert_t(nfft),
      phase(nfft,ch)
{
}

void MHASignal::minphase_t::operator()(mha_spec_t* h)
{
    if( !h )
        throw MHA_ErrorMsg("minphase: Invalid signal pointer.");
    if( h->num_frames != phase.num_frames/2+1 )
        throw MHA_ErrorMsg("minphase: Invalid number of frames.");
    if( h->num_channels != phase.num_channels )
        throw MHA_ErrorMsg("minphase: Invalid number of channels.");
    unsigned int k, ch;
    for(ch=0;ch<phase.num_channels;ch++){
        for(k=0;k<h->num_frames;k++)
            phase.value(k,ch) = log(std::max(1e-10f,abs(value(h,k,ch))));
        for(k=h->num_frames;k<phase.num_frames;k++)
            phase.value(k,ch) = phase.value(phase.num_frames-k,ch);
        MHASignal::hilbert_t::operator()(&phase,&phase);
        for(k=0;k<h->num_frames;k++)
            expi(value(h,k,ch),-phase.value(k,ch),abs(value(h,k,ch)));
    }   
}

MHASignal::stat_t::stat_t(const unsigned int& frames, const unsigned int& channels)
    : n(frames,channels),sum(frames,channels),sum2(frames,channels)
{
}

void MHASignal::stat_t::mean_std(mha_wave_t& m,mha_wave_t& s)
{
    ASSERT_EQUAL_DIM(s,sum);
    for(unsigned int k=0;k<size(sum);k++){
        m.buf[k] = sum.buf[k]/std::max(1.0f,n.buf[k]);
        s.buf[k] = sqrt(sum2.buf[k]/std::max(1.0f,n.buf[k]) -
                        m.buf[k]*m.buf[k]);
    }
}

void MHASignal::stat_t::mean(mha_wave_t& m)
{
    ASSERT_EQUAL_DIM(m,sum);
    for(unsigned int k=0;k<size(sum);k++){
        m.buf[k] = sum.buf[k]/std::max(1.0f,n.buf[k]);
    }
}

void MHASignal::stat_t::push(const mha_wave_t& x)
{
    ASSERT_EQUAL_DIM(x,sum);
    for(unsigned int k=0;k<size(sum);k++){
        sum.buf[k] += x.buf[k];
        sum2.buf[k] += x.buf[k]*x.buf[k];
        n.buf[k] += 1.0f;
    }
}

void MHASignal::stat_t::push(const mha_real_t& x,
                             const unsigned int& k,
                             const unsigned int& ch)
{
    CHECK_EXPR(k<sum.num_frames);
    CHECK_EXPR(ch<sum.num_channels);
    sum.value(k,ch) += x;
    sum2.value(k,ch) += x*x;
    n.value(k,ch) += 1.0f;
}

/**

   \class MHASignal::delay_wave_t
   \ingroup mhasignal

   \brief Delayline containing wave fragments.

   The delayline contains waveform fragments. The delay can be
   configured in integer fragments (sample delay or sub-sample delay
   is not possible).

 */

MHASignal::delay_wave_t::delay_wave_t(unsigned int delay_,
                                      unsigned int frames,
                                      unsigned int channels)
    : delay(delay_),
      pos(0)
{
    buffer = new MHASignal::waveform_t*[delay+1];
    for(unsigned int k=0;k<delay+1;k++)
        buffer[k] = new MHASignal::waveform_t(frames,channels);
}

MHASignal::delay_wave_t::~delay_wave_t()
{
    for(unsigned int k=0;k<delay+1;k++)
        delete buffer[k];
    delete [] buffer;
}

mha_wave_t* MHASignal::delay_wave_t::process(mha_wave_t* s)
{
    buffer[pos]->copy(*s);
    pos = (pos+1) % (delay+1);
    return buffer[pos];
}

MHASignal::delay_spec_t::delay_spec_t(unsigned int delay_,
                                      unsigned int frames,
                                      unsigned int channels)
    : delay(delay_),
      pos(0)
{
    buffer = new MHASignal::spectrum_t*[delay+1];
    for(unsigned int k=0;k<delay+1;k++)
        buffer[k] = new MHASignal::spectrum_t(frames,channels);
}

MHASignal::delay_spec_t::~delay_spec_t()
{
    for(unsigned int k=0;k<delay+1;k++)
        delete buffer[k];
    delete [] buffer;
}

mha_spec_t* MHASignal::delay_spec_t::process(mha_spec_t* s)
{
    buffer[pos]->copy(*s);
    pos = (pos+1) % (delay+1);
    return buffer[pos];
}

MHASignal::uint_vector_t::uint_vector_t(unsigned int len)
    : length(len),data(new unsigned int[mha_min_1(length)])
{
    memset(data,0,length*sizeof(unsigned int));
}

MHASignal::uint_vector_t::uint_vector_t(const uint_vector_t& src)
    : length(src.length),data(new unsigned int[mha_min_1(length)])
{
    memmove(data,src.data,length*sizeof(unsigned int));
}

MHASignal::uint_vector_t::~uint_vector_t()
{
    delete [] data;
}

bool MHASignal::uint_vector_t::operator==(const uint_vector_t& src) const
{
    if( length != src.get_length() )
        return false;
    for( unsigned int k=0;k<length;k++ )
        if( data[k] != src[k] )
            return false;
    return true;
}

uint_vector_t& MHASignal::uint_vector_t::operator=(const uint_vector_t& src)
{
    if( src.get_length() != length )
        throw MHA_Error(__FILE__,__LINE__,"Size mismatch (expeczed %u, got %u).",
                        length, src.get_length());
    for( unsigned int k=0;k<length;k++ )
        data[k] = src[k];
    return *this;
}

unsigned int MHASignal::matrix_t::get_nelements() const
{
    unsigned int n = 1;
    for(unsigned k=0;k<length;k++)
        n *= data[k];
    return n;
}

unsigned int MHASignal::matrix_t::get_index(unsigned int row, unsigned int col) const
{
    // assuming 2 dimensions:
    return row + col * operator[](0);
}

unsigned int MHASignal::matrix_t::get_index(const MHASignal::uint_vector_t& index) const
{
    if( index.get_length() != get_length() )
        throw MHA_Error(__FILE__,__LINE__,
                        "Dimension mismatch: size has %u, index has %u dimensions.",
                        get_length(), index.get_length() );
    unsigned int idx = index[index.get_length()-1];
    for( unsigned int k=index.get_length()-1; k>0; k-- )
        idx = idx * operator[](k-1) + index[k-1];
    return idx;
}

MHASignal::matrix_t::matrix_t(unsigned int nrows, unsigned int ncols,bool b_is_complex)
    : MHASignal::uint_vector_t(2),complex_ofs(b_is_complex+1)
{
    data[0] = nrows;
    data[1] = ncols;
    nelements = get_nelements();
    rdata = new mha_real_t[mha_min_1(get_nreals())];
    memset(rdata,0,get_nreals()*sizeof(mha_real_t));
}

MHASignal::matrix_t::matrix_t(const mha_spec_t& spec)
    : MHASignal::uint_vector_t(2),complex_ofs(2)
{
    data[0] = spec.num_channels;
    data[1] = spec.num_frames;
    nelements = get_nelements();
    rdata = new mha_real_t[mha_min_1(get_nreals())];
    memset(rdata,0,get_nreals()*sizeof(mha_real_t));
    unsigned int k, ch;
    for( ch = 0;ch < spec.num_channels; ch++)
        for( k=0;k<spec.num_frames;k++)
            operator()(ch,k) = value(spec,k,ch);
}

MHASignal::matrix_t::matrix_t(const MHASignal::uint_vector_t& size,bool b_is_complex)
    : MHASignal::uint_vector_t(size),complex_ofs(b_is_complex+1),nelements(get_nelements()),
      rdata(new mha_real_t[mha_min_1(get_nreals())])
{
    memset(rdata,0,get_nreals()*sizeof(mha_real_t));
}

MHASignal::matrix_t::matrix_t(const MHASignal::matrix_t& src)
    : MHASignal::uint_vector_t(src),complex_ofs(src.iscomplex()+1),nelements(get_nelements()),
      rdata(new mha_real_t[mha_min_1(get_nreals())])
{
    memmove(rdata,src.rdata,get_nreals()*sizeof(mha_real_t));
}

MHASignal::matrix_t::~matrix_t()
{
    delete [] rdata;
}

matrix_t& MHASignal::matrix_t::operator=(const MHASignal::matrix_t& src)
{
    if( !is_same_size(src) )
        throw MHA_Error(__FILE__,__LINE__,"Source matrix has different size.");
    memmove(rdata,src.rdata,get_nreals()*sizeof(mha_real_t));
    return *this;
}

bool MHASignal::matrix_t::is_same_size(const MHASignal::matrix_t& src)
{
    if( iscomplex() != src.iscomplex() )
        return false;
    if( ! uint_vector_t::operator==(src) )
        return false;
    return true;
}

MHASignal::matrix_t& MHASignal::matrix_t::operator=(const comm_var_t& src)
{
    if( dimension() != 2 )
        throw MHA_Error(__FILE__,__LINE__,"Expected dimension of 2, got %u.",dimension());
    if( size(0) != src.stride )
        throw MHA_Error(__FILE__,__LINE__,
                        "Size of first dimension mismatches: expected %u, found %u.",
                        size(0),src.stride);
    if( size(1) != src.num_entries / src.stride )
        throw MHA_Error(__FILE__,__LINE__,
                        "Size of second dimension mismatches: expected %u, found %u.",
                        size(1), src.num_entries / src.stride );
    unsigned int k0, k1;
    switch( src.data_type ){
    case MHA_AC_MHACOMPLEX :
        if( !iscomplex() )
            throw MHA_Error(__FILE__,__LINE__,"Source is complex, target is pure real.");
        for(k0=0;k0<size(0);k0++)
            for(k1=0;k1<size(1);k1++)
                operator()(k0,k1) = ((const mha_complex_t*)src.data)[k0+src.stride*k1];
        break;
    case MHA_AC_MHAREAL :
        for(k0=0;k0<size(0);k0++)
            for(k1=0;k1<size(1);k1++)
                real(k0,k1) = ((const mha_real_t*)src.data)[k0+src.stride*k1];
        break;
    case MHA_AC_FLOAT :
        for(k0=0;k0<size(0);k0++)
            for(k1=0;k1<size(1);k1++)
                real(k0,k1) = ((const float*)src.data)[k0+src.stride*k1];
        break;
    case MHA_AC_DOUBLE :
        for(k0=0;k0<size(0);k0++)
            for(k1=0;k1<size(1);k1++)
                real(k0,k1) = ((const double*)src.data)[k0+src.stride*k1];
        break;
    case MHA_AC_INT :
        for(k0=0;k0<size(0);k0++)
            for(k1=0;k1<size(1);k1++)
                real(k0,k1) = ((const int*)src.data)[k0+src.stride*k1];
        break;
    default:
        throw MHA_Error(__FILE__,__LINE__,
                        "Unsupported AC data format (%u).",
                        src.data_type);
    }
    return *this;
}

comm_var_t MHASignal::matrix_t::get_comm_var()
{
    comm_var_t retv;
    memset(&retv,0,sizeof(retv));
    if( dimension() != 2 )
        throw MHA_Error(__FILE__,__LINE__,"Expected dimension of 2, got %u.",dimension());
    retv.stride = size(0);
    retv.num_entries = size(0)*size(1);
    if( iscomplex() ){
        retv.data_type = MHA_AC_MHACOMPLEX;
        retv.data = cdata;
    }else{
        retv.data_type = MHA_AC_MHAREAL;
        retv.data = rdata;
    }
    return retv;
}

MHASignal::uint_vector_t::uint_vector_t(const uint8_t* buf,unsigned int len)
    : length(0),data(NULL)
{
    if( len < strlen(MHA_ID_UINT_VECTOR)+sizeof(uint32_t) )
        throw MHA_Error(__FILE__,__LINE__,"The provided data length is to short.");
    if( strncmp((const char*)buf,MHA_ID_UINT_VECTOR,strlen(MHA_ID_UINT_VECTOR)) != 0 )
        throw MHA_Error(__FILE__,__LINE__,"Invalid identifier (expected %s).",MHA_ID_UINT_VECTOR);
    buf += strlen(MHA_ID_UINT_VECTOR);
    len -= strlen(MHA_ID_UINT_VECTOR);
    length = *(uint32_t*)buf;
    buf += sizeof(uint32_t);
    len += sizeof(uint32_t);
    if( len < sizeof(uint32_t)*length )
        throw MHA_Error(__FILE__,__LINE__,"The provided data does not hold %u values.",length);
    data = new unsigned int[mha_min_1(length)];
    for(unsigned int k=0;k<length;k++)
        data[k] = ((uint32_t*)buf)[k];
}

unsigned int MHASignal::uint_vector_t::numbytes() const
{
    return strlen(MHA_ID_UINT_VECTOR)+sizeof(uint32_t)+sizeof(uint32_t)*length;
}

unsigned int MHASignal::uint_vector_t::write(uint8_t* buf,unsigned int len) const
{
    if( len < numbytes() )
        throw MHA_Error(__FILE__,__LINE__,"The provided buffer is to small to hold data.");
    memmove(buf,MHA_ID_UINT_VECTOR,strlen(MHA_ID_UINT_VECTOR));
    buf += strlen(MHA_ID_UINT_VECTOR);
    memmove(buf,&length,sizeof(length));
    buf += sizeof(length);
    memmove(buf,data,length*sizeof(uint32_t));
    return numbytes();
}


unsigned int MHASignal::matrix_t::numbytes() const
{
    return strlen(MHA_ID_MATRIX)+MHASignal::uint_vector_t::numbytes()+
        sizeof(complex_ofs)+sizeof(nelements)+sizeof(mha_real_t)*get_nreals();
}

unsigned int MHASignal::matrix_t::write(uint8_t* buf,unsigned int len) const
{
    if( len < numbytes() )
        throw MHA_Error(__FILE__,__LINE__,"The provided buffer is to small to hold data.");
    memmove(buf,MHA_ID_MATRIX,strlen(MHA_ID_MATRIX));
    buf += strlen(MHA_ID_MATRIX);
    len -= strlen(MHA_ID_MATRIX);
    buf += MHASignal::uint_vector_t::write(buf,len);
    memmove(buf,&complex_ofs,sizeof(complex_ofs));
    buf += sizeof(complex_ofs);
    memmove(buf,&nelements,sizeof(nelements));
    buf += sizeof(nelements);
    memmove(buf,rdata,get_nreals()*sizeof(mha_real_t));
    return numbytes();
}

MHASignal::matrix_t::matrix_t(const uint8_t* buf,unsigned int len)
    : MHASignal::uint_vector_t(buf+strlen(MHA_ID_MATRIX),
                               std::max(len,(unsigned int)strlen(MHA_ID_MATRIX))-strlen(MHA_ID_MATRIX))
{
    if( len < strlen(MHA_ID_MATRIX)+MHASignal::uint_vector_t::numbytes()+sizeof(complex_ofs)+sizeof(nelements))
        throw MHA_Error(__FILE__,__LINE__,"The provided data length is to short.");
    if( strncmp((const char*)buf,MHA_ID_MATRIX,strlen(MHA_ID_MATRIX)) != 0 )
        throw MHA_Error(__FILE__,__LINE__,"Invalid identifier (expected %s).",MHA_ID_MATRIX);
    buf += strlen(MHA_ID_MATRIX)+MHASignal::uint_vector_t::numbytes();
    len -= strlen(MHA_ID_MATRIX)+MHASignal::uint_vector_t::numbytes();
    complex_ofs = *(uint32_t*)buf;
    buf += sizeof(complex_ofs);
    len += sizeof(complex_ofs);
    nelements = *(uint32_t*)buf;
    buf += sizeof(nelements);
    len += sizeof(nelements);
    if( len < sizeof(uint32_t)*get_nreals() )
        throw MHA_Error(__FILE__,__LINE__,"The provided data does not hold %u values.",get_nreals());
    rdata = new mha_real_t[mha_min_1(get_nreals())];
    for(unsigned int k=0;k<get_nreals();k++)
        rdata[k] = ((mha_real_t*)buf)[k];
}

std::vector<float> std_vector_float(const mha_wave_t& w)
{
    std::vector<float> retv(size(w),0.0f);
    for(unsigned int k=0;k<retv.size();k++)
        retv[k] = w.buf[k];
    return retv;
}

std::vector<std::vector<float> > std_vector_vector_float(const mha_wave_t& w)
{
    std::vector<std::vector<float> > retv;
    retv.resize(w.num_channels);
    for(unsigned int ch=0;ch<w.num_channels;ch++){
        retv[ch].resize(w.num_frames);
        for(unsigned int k=0;k<w.num_frames;k++){
            retv[ch][k] = value(w,k,ch);
        }
    }
    return retv;
}


std::vector<std::vector<mha_complex_t> > std_vector_vector_complex(const mha_spec_t& w)
{
    std::vector<std::vector<mha_complex_t> > retv;
    retv.resize(w.num_channels);
    for(unsigned int ch=0;ch<w.num_channels;ch++){
        retv[ch].resize(w.num_frames);
        for(unsigned int k=0;k<w.num_frames;k++){
            retv[ch][k] = value(w,k,ch);
        }
    }
    return retv;
}


static mha_real_t intensity(const mha_spec_t& s,
                            unsigned int channel,
                            unsigned int fftlen,
                            mha_real_t * sqfreq_response = 0)
{
    unsigned int k;
    unsigned int k_nyquist = fftlen/2;
    mha_real_t val;
    if( fftlen & 1 )
        k_nyquist = s.num_frames;
    if (sqfreq_response)
        val = 0.5*abs2(value(s,0,channel)) * sqfreq_response[0];
    else
        val = 0.5*abs2(value(s,0,channel));
    for( k=1;k<k_nyquist;k++)
        if (sqfreq_response)
            val += abs2(value(s,k,channel)) * sqfreq_response[k];
        else
            val += abs2(value(s,k,channel));
    for( k=k_nyquist;k<s.num_frames;k++)
        if (sqfreq_response)
            val += 0.5*abs2(value(s,k,channel)) * sqfreq_response[k];
        else
            val += 0.5*abs2(value(s,k,channel));
    // level is sqrt of sum of abs(x)^2:
    // including negative frequencies!! Thus factor of 2:
    // no sqrt invocation here because intensity is wanted.
    return 2*val;
}

mha_real_t MHASignal::rmslevel(const mha_spec_t& s,unsigned int channel,unsigned int fftlen)
{
    return sqrt(intensity(s,channel,fftlen));
}

mha_real_t MHASignal::colored_intensity(const mha_spec_t& s,
                                        unsigned int channel,
                                        unsigned int fftlen,
                                        mha_real_t * sqfreq_response)
{
    return intensity(s,channel,fftlen,sqfreq_response);
}

mha_real_t MHASignal::maxabs(const mha_spec_t& s,unsigned int channel)
{
    mha_real_t val = 0;
    for(unsigned int k=0;k<s.num_frames;k++)
        val = std::max(val,abs(value(s,k,channel)));
    return val;
}

mha_real_t MHASignal::rmslevel(const mha_wave_t& s,unsigned int channel)
{
    mha_real_t val = 0;
    for(unsigned int k=0;k<s.num_frames;k++)
        val += value(s,k,channel)*value(s,k,channel);
    return sqrt(val/s.num_frames);
}

mha_real_t MHASignal::maxabs(const mha_wave_t& s,unsigned int channel)
{
    mha_real_t val = 0;
    for(unsigned int k=0;k<s.num_frames;k++)
        val = std::max(val,(float)fabs(value(s,k,channel)));
    return val;
}

mha_real_t MHASignal::maxabs(const mha_wave_t& s)
{
    mha_real_t val = 0;
    for(unsigned int k=0;k<s.num_channels;k++)
        val = std::max(val,MHASignal::maxabs(s,k));
    return val;
}

mha_real_t MHASignal::max(const mha_wave_t& s)
{
    mha_real_t val = s.buf[0];
    for(unsigned int k=1;k<size(s);k++)
        val = std::max(val,s.buf[k]);
    return val;
}

mha_real_t MHASignal::min(const mha_wave_t& s)
{
    mha_real_t val = s.buf[0];
    for(unsigned int k=1;k<size(s);k++)
        val = std::min(val,s.buf[k]);
    return val;
}

mha_real_t MHASignal::sumsqr_channel(const mha_wave_t& s,unsigned int channel)
{
    mha_real_t val = 0;
    for(unsigned int k=0;k<s.num_frames;k++)
        val += value(s,k,channel)*value(s,k,channel);
    return val;
}

mha_real_t MHASignal::sumsqr_frame(const mha_wave_t& s,unsigned int frame)
{
    mha_real_t val = 0;
    for(unsigned int ch=0;ch<s.num_channels;ch++)
        val += value(s,frame,ch)*value(s,frame,ch);
    return val;
}


/**

\enum MHASignal::schroeder_t::sign_t
\brief Enumerator for sign of Schroeder tone complex sweep direction

 */

/**
\var MHASignal::schroeder_t::sign_t MHASignal::schroeder_t::up

Sweep from zero to Nyquist frequency (\f$\sigma = -1\f$)
*/ 
    
/**
\var MHASignal::schroeder_t::sign_t MHASignal::schroeder_t::down

Sweep from Nyquist frequency to zero (\f$\sigma = +1\f$)
*/ 

/**  
\class MHASignal::schroeder_t
\brief Schroeder tone complex class

The Schroeder tone complex is a sweep defined in the sampled spectrum:

\f[
\Phi(f) = \sigma 2\pi \tau (2f/f_s)^{2\alpha},\quad S(f) = e^{i\Phi(f)}
\f]

\f$f\f$ is the sampled frequency in Hz, \f$\sigma\f$ is the sign of
the sweep (-1 for up sweep, +1 for down sweep), \f$\tau\f$ is the
sweep duration in samples, \f$f_s\f$ is the sampling rate in Hz and
\f$\alpha\f$ is the relative sweep speed.
  
*/


/** 
    
\brief Constructor
  
Parameters of the Schroeder tone complex are configured in the constructor.

\param len  Length \f$\tau\f$ of the Schroeder tone complex in samples
\param channels     Number of channels
\param sign Sign \f$\sigma\f$ of Schroeder sweep
\param speed    Relative speed \f$\alpha\f$ (curvature of phase function)
*/
MHASignal::schroeder_t::schroeder_t(unsigned int len,unsigned int channels,
                                    schroeder_t::sign_t sign,
                                    mha_real_t speed)
    : MHASignal::waveform_t(len,channels)
{
    mha_fft_t fft = mha_fft_new(len);
    MHASignal::spectrum_t spec(len/2+1,channels);
    unsigned int k, ch;
    mha_real_t rsign = 1;
    switch( sign ){
    case up : rsign = -1; break;
    case down : rsign = 1; break;
    }
    for(k=0; k < spec.num_frames; k++){
        expi(spec(k,0), 
             rsign*len*2.0f*M_PI*pow((mha_real_t)k/(mha_real_t)(spec.num_frames),2.0f*speed), 
             1.0f/sqrt((mha_real_t)len) );
        for(ch=1;ch<spec.num_channels;ch++)
            spec(k,ch) = spec(k,0);
    }
    mha_fft_spec2wave(fft,&spec,this);
    mha_fft_free( fft );
}

/** 
    \brief Construct create Schroeder tone complex from a given frequency function.
  
    The frequency function \f$g(f)\f$ defines the sweep speed and sign
    (based on the group delay). It must be defined in the interval
    [0,1) and should return values in the interval [0,1].

    \f[
    \Phi(f) = -4\pi\tau\int\limits_0^\tau g(f)\, {\textrm{d}}f,\quad S(f) = e^{i\Phi(f)}
    
    \f]

    \param len    Length \f$\tau\f$ of the Schroeder tone complex in samples.
    \param channels          Number of channels.
    \param freqfun Frequency function \f$g(f)\f$.
    \param fmin Start frequency (relative to Nyquist frequency).
    \param fmax End frequency (relative to Nyquist frequency).
    \param eps Stability constant for frequency ranges not covered by Schroeder tone complex.
*/
MHASignal::schroeder_t::schroeder_t(unsigned int len,
                                    unsigned int channels,
                                    schroeder_t::groupdelay_t freqfun,
                                    float fmin, float fmax,
                                    float eps)
    : MHASignal::waveform_t(len,channels)
{
    unsigned int nbins = len/2+1;
    unsigned int kmin = std::min(nbins,(unsigned int)(fmin*nbins));
    unsigned int kmax = std::min(nbins,(unsigned int)(fmax*nbins));
    if( kmax <= kmin )
        throw MHA_Error(__FILE__,__LINE__,
                        "Invalid frequency range."
                        " fmin (%g, bin %u) must be below fmax (%g, bin %u). Try to increase resolution.",
                        fmin,kmin,fmax,kmax);
    MHASignal::waveform_t phase(nbins,1);
    mha_fft_t fft = mha_fft_new(len);
    MHASignal::spectrum_t spec(nbins,channels);
    unsigned int k, ch;
    for(k=0;k<phase.num_frames;k++)
        phase[k] = freqfun((float)k/(float)nbins,
                           (float)kmin/(float)nbins,
                           (float)kmax/(float)nbins);
    integrate(phase);
    phase *= -M_PI*(float)len;
    for(k=0; k < spec.num_frames; k++){
        if( (k >= kmin) && (k < kmax) ){
            expi(spec(k,0), phase[k], 1.0f/sqrt((mha_real_t)len) );
        }else{
            expi(spec(k,0), 2.0f*M_PI*(float)rand()/RAND_MAX, eps/sqrt((mha_real_t)len) );
        }
        for(ch=1;ch<spec.num_channels;ch++)
            spec(k,ch) = spec(k,0);
    }
    mha_fft_spec2wave(fft,&spec,this);
    mha_fft_free( fft );
}



void integrate(mha_wave_t& s)
{
    unsigned int k, ch;
    for(ch=0;ch<s.num_channels;ch++)
        for(k=1;k<s.num_frames;k++)
            value(s,k,ch) += value(s,k-1,ch);
    s *= 1.0f/(float)s.num_frames;
}

void integrate(mha_spec_t& s)
{
    unsigned int k, ch;
    for(ch=0;ch<s.num_channels;ch++)
        for(k=1;k<s.num_frames;k++)
            value(s,k,ch) += value(s,k-1,ch);
    s *= 1.0f/(float)s.num_frames;
}

MHASignal::quantizer_t::quantizer_t(unsigned int num_bits)
    : limit(num_bits==0),
      upscale(pow(2.0,(double)(num_bits-1))),
      downscale(1.0/upscale),
      up_limit(1.0-downscale)
{
    if( limit )
        up_limit = 1.0f;
}

void MHASignal::quantizer_t::operator()(mha_wave_t& s)
{
    unsigned int k;
    if( !limit )
        for(k=0;k<s.num_channels*s.num_frames;k++)
            s.buf[k] = floor(s.buf[k]*upscale)*downscale;
    for(k=0;k<s.num_channels*s.num_frames;k++)
        s.buf[k] = std::min(up_limit,std::max(-1.0f,s.buf[k]));
}

mha_wave_t& operator^=(mha_wave_t& self,const mha_real_t& arg)
{
    for(unsigned int k=0;k<size(self);k++)
        self.buf[k] = pow(self.buf[k],arg);
    return self;
}

/** \ingroup mhasignal
    \brief Return a time interval from a waveform chunk.

    A waveform chunk containing a time intervall of a larger waveform
    chunk is returned. The number of channels remains constant. The
    data of the output waveform structure points to the data of the
    input structure, i.e., write access to the output waveform chunk
    modifies the corresponding entries in the input chunk.

    \param s Waveform structure
    \param k0 Index of first value in output
    \param len Number of frames in output
    \return Waveform structure representing the sub-interval.
 */
mha_wave_t range(mha_wave_t s,unsigned int k0,unsigned int len)
{
    if( len+k0 > s.num_frames )
        throw MHA_Error(__FILE__,__LINE__,
                        "The input waveform is not long enough (length: %u, required %u output samples at position %u).",
                        s.num_frames,len,k0);
    mha_wave_t retv = s;
    retv.num_frames = len;
    retv.buf = &(s.buf[k0*s.num_channels]);
    return retv;
}

/** \ingroup mhasignal
    \brief Return a channel interval from a spectrum.

    \param s Input spectrum
    \param ch_start Index of first channel in output
    \param nch Number of channels in output
    \return Spectrum structure representing the sub-interval.
 */
mha_spec_t channels(mha_spec_t s,unsigned int ch_start,unsigned int nch)
{
    if( s.num_channels < ch_start+nch )
        throw MHA_Error(__FILE__,__LINE__,"Not enough channels (source has %u, requested %u channels starting at %u).",
                        s.num_channels,nch,ch_start);
    mha_spec_t retv;
    retv.num_frames = s.num_frames;
    retv.num_channels = nch;
    retv.buf = &(s.buf[ch_start*s.num_frames]);
    return retv;
}

void assign(mha_wave_t self,const mha_wave_t& val)
{
    if( !equal_dim( self, val ) )
        throw MHA_Error(__FILE__,__LINE__,"Mismatching size for assignment (self: %ux%u, val: %ux%u)",
                        self.num_frames,self.num_channels,val.num_frames,val.num_channels);
    unsigned int k_max = size(self);
    for(unsigned int k=0;k<k_max;k++)
        self.buf[k] = val.buf[k];
}

void assign(mha_spec_t self,const mha_spec_t& val)
{
    if( !equal_dim( self, val ) )
        throw MHA_Error(__FILE__,__LINE__,"Mismatching size for assignment (self: %ux%u, val: %ux%u)",
                        self.num_frames,self.num_channels,val.num_frames,val.num_channels);
    unsigned int k_max = size(self);
    for(unsigned int k=0;k<k_max;k++)
        self.buf[k] = val.buf[k];
}

void timeshift(mha_wave_t& self,int shift)
{
    if( shift == 0 )
        return;
    int k;
    unsigned int ch;
    if( shift > 0 ){
        for(ch=0;ch<self.num_channels;ch++){
            for(k=self.num_frames-1;k>=shift;k--)
                value(self,k,ch) = value(self,k-shift,ch);
        }
        assign(range(self,0,shift),0.0f);
    }else{
        for(ch=0;ch<self.num_channels;ch++){
            for(k=0;k<(int)self.num_frames+shift;k++)
                value(self,k,ch) = value(self,k-shift,ch);
            for(k=(int)self.num_frames+shift;k<(int)self.num_frames;k++)
                value(self,k,ch) = 0.0f;
        }
    }
}

/**
   \class MHASignal::async_rmslevel_t
   \ingroup mhasignal
   \brief Class for asynchronous level metering

*/

/**
   \brief Constructor for level metering class.
   
   Allocate memory for metering. The RMS integration time corresponds
   to the number of frames in the buffer.

   \param frames Number of frames to integrate.
   \param channels Number of channels used for level-metering.
 */
MHASignal::async_rmslevel_t::async_rmslevel_t(unsigned int frames,unsigned int channels)
    : MHASignal::waveform_t(frames,channels),
      pos(0),
      filled(1)
{
}

/**
   \brief Read-only function for querying the current RMS level.
   \return Vector of floats, one value for each channel, containing
   the RMS level in dB (SPL if calibrated properly).
 */
std::vector<float> MHASignal::async_rmslevel_t::rmslevel() const
{
    std::vector<float> retv;
    retv.resize(num_channels);
    unsigned range_frames = std::min(filled, num_frames);
    const mha_wave_t * parent_ptr = this; // Needed for borland compiler
    mha_wave_t range_wave = range(*parent_ptr, 0, range_frames);
    for (unsigned int ch=0; ch<num_channels; ch++) {
        mha_real_t range_rms = std::max(MHASignal::rmslevel(range_wave, ch),
                                        2e-15f);
        retv[ch] = 20*log10(range_rms * 5e4);
    }
    return retv;
}

/**
   \brief Read-only function for querying the current peak level.
   \return Vector of floats, one value for each channel, containing
   the peak level in dB (SPL if calibrated properly).
 */
std::vector<float> MHASignal::async_rmslevel_t::peaklevel() const
{
    std::vector<float> retv;
    retv.resize(num_channels);
    unsigned range_frames = std::min(filled, num_frames);
    const mha_wave_t * parent_ptr = this; // Needed for borland compiler
    mha_wave_t range_wave = range(*parent_ptr, 0, range_frames);
    for (unsigned int ch=0; ch<num_channels; ch++) {
        mha_real_t range_peak = std::max(MHASignal::maxabs(range_wave, ch),
                                        2e-15f);
        retv[ch] = 20*log10(range_peak * 5e4);
    }
    return retv;
}

/**
   \brief Function to store a chunk of audio in the level meter.
   \param s Audio chunk (same number of channels required as given in
   the constructor).
 */
void MHASignal::async_rmslevel_t::process(mha_wave_t* s)
{
    if( s->num_channels != num_channels )
        throw MHA_Error(__FILE__,__LINE__,
                        "Invalid channel count (level meter has %u channels, input has %u).",
                        num_channels,s->num_channels);
    unsigned int k, ch;
    if( num_frames > s->num_frames ){
        for(k=0;k<s->num_frames;k++){
            pos++;
            filled ++;
            if( pos >= num_frames )
                pos = 0;
            for(ch=0;ch<num_channels;ch++)
                value(pos,ch) = ::value(s,k,ch);
        }
    }else{
        filled = num_frames;
        copy(range(*s,0,num_frames));
    }
    if( filled > num_frames )
        filled = num_frames;
}

/**
   \class MHASignal::loop_wavefragment_t
   \brief Copy a fixed waveform fragment to a series of waveform fragments of other size.

   This class is designed to continously play back a waveform to an
   output stream, with variable output block size.
*/

/**
   \enum MHASignal::loop_wavefragment_t::level_mode_t
   \brief Switch for playback level mode.
*/
/**
   \var MHASignal::loop_wavefragment_t::level_mode_t MHASignal::loop_wavefragment_t::relative
   \brief The nominal level is applied as a gain to the source signal.

*/
/**
   \var MHASignal::loop_wavefragment_t::level_mode_t MHASignal::loop_wavefragment_t::peak
   \brief The nominal level is the peak level of source signal in Pascal.
*/
/**
   \var MHASignal::loop_wavefragment_t::level_mode_t::rms MHASignal::loop_wavefragment_t::rms
   \brief The nominal level is the RMS level of the source signal in Pascal.
*/

/**
   \enum MHASignal::loop_wavefragment_t::playback_mode_t
   \brief Switch for playback mode.
*/
/**
   \var MHASignal::loop_wavefragment_t::playback_mode_t MHASignal::loop_wavefragment_t::add
   \brief Add source signal to output stream.
*/
/**
   \var MHASignal::loop_wavefragment_t::playback_mode_t MHASignal::loop_wavefragment_t::replace
   \brief Replace output stream by source signal.
*/
/**
   \var MHASignal::loop_wavefragment_t::playback_mode_t MHASignal::loop_wavefragment_t::input
   \brief Do nothing, keep output stream (source position is unchanged).
*/
/**
   \var MHASignal::loop_wavefragment_t::playback_mode_t MHASignal::loop_wavefragment_t::mute
   \brief Mute output stream (source position is unchanged).
 */

/**
   \brief Constructor to create an instance of loop_wavefragment_t based on an existing waveform block.

   \param src Waveform block to copy data from.
   \param loop Flag whether the block should be looped or played once.
   \param level_mode Configuration of playback level (see MHASignal::loop_wavefragment_t::level_mode_t for details)
   \param channels Mapping of input to output channels.
   \param startpos Starting position
 */
MHASignal::loop_wavefragment_t::loop_wavefragment_t(const mha_wave_t& src, bool loop, level_mode_t level_mode, std::vector<int> channels, unsigned int startpos)
    : MHASignal::waveform_t(src),
      playback_channels(channels),
      b_loop(loop),
      pos(std::min(startpos,std::max(num_frames,1u)-1u)),
      intern_level(1,1)
{
    mha_real_t file_level(0);
    switch( level_mode ){
    case relative : 
        break;
    case peak : 
        file_level = MHASignal::maxabs(*this);
        if( file_level > 0 )
            *this *= 1.0f/file_level;
        break;
    case rms :
        file_level = sumsqr();
        if( file_level > 0 )
            *this *= 1.0f*sqrt(size(*this)/file_level);
        break;
    case rms_limit40 :
        // use maximum of RMS and peak-40dB
        file_level = std::max(sqrtf(sumsqr()/std::max(1u,size(*this))),0.01f*MHASignal::maxabs(*this));
        if( file_level > 0 )
            *this *= 1.0f/file_level;
        break;
    }
}

std::vector<int> MHASignal::loop_wavefragment_t::get_mapping(unsigned int nchannels)
{
    std::vector<int> mapping(nchannels,-1);
    for(unsigned int ch=0;ch<playback_channels.size();ch++){
        if( (playback_channels[ch] < (int)nchannels) && (playback_channels[ch] >= 0) )
            mapping[playback_channels[ch]] = ch % num_channels;
    }
    return mapping;
}

/**
   \brief Add source waveform block to an output block.

   \param s Output block (streamed signal).
   \param pmode Playback mode (add, replace, input, mute).
   \param level_pa Linear output level/gain (depending on level_mode parameter in constructor); one value for each sample in output block.
   \param channels Output channels
 */
void MHASignal::loop_wavefragment_t::playback(mha_wave_t* s, playback_mode_t pmode, mha_wave_t* level_pa,const std::vector<int>& channels)
{
    if( pmode == input ) // input
        return;
    if( pmode == replace ) // replace
        clear(s);
    if( pmode == mute ){ // mute
        clear(s);
        return;
    }
    unsigned int k, kch;
    for(k=0;k<s->num_frames;k++){
        if( pos < num_frames ){
            for(kch=0;kch<channels.size();kch++)
                if( (channels[kch] < (int)(s->num_channels)) && (channels[kch] >= 0) ){
                    ::value(s,k,channels[kch]) += 
                        ::value( level_pa, k % level_pa->num_frames, kch % level_pa->num_channels) *
                        value( pos, kch % num_channels );
                }
            pos++;
            if( (pos == num_frames) && b_loop )
                pos = 0;
        }
    }
}

/**
   \brief Add source waveform block to an output block.

   \param s Output block (streamed signal).
   \param pmode Playback mode (add, replace, input, mute).
   \param level_pa Linear output level/gain (depending on level_mode parameter in constructor); one value for each sample in output block.
 */
void MHASignal::loop_wavefragment_t::playback(mha_wave_t* s, playback_mode_t pmode, mha_wave_t* level_pa)
{
    playback(s,pmode,level_pa,playback_channels);
}

/**
   \brief Add source waveform block to an output block.

   \param s Output block (streamed signal).
   \param pmode Playback mode (add, replace, input, mute).
*/
void MHASignal::loop_wavefragment_t::playback(mha_wave_t* s, playback_mode_t pmode)
{
    playback(s,pmode,&intern_level);
}

void MHASignal::loop_wavefragment_t::set_level_lin(mha_real_t l)
{
    intern_level = l;
}

void MHASignal::loop_wavefragment_t::set_level_db(mha_real_t l)
{
    set_level_lin(2e-5*pow(10.0,0.05*l));
}

MHASignal::delay_t::delay_t(std::vector<int> delays_,unsigned int nch)
    : channels(nch)
{
    if( delays_.size() != channels )
        throw MHA_Error(__FILE__,__LINE__,"The number of entries in the delay vector does not match the channel count.");
    delays = new unsigned int[channels];
    pos = new unsigned int[channels];
    buffer = new mha_real_t*[channels];
    unsigned int k,ch;
    for(ch=0;ch<channels;ch++){
        if(delays_[ch] < 0 )
            throw MHA_Error(__FILE__,__LINE__,"Invalid (negative) delay.");
    }
    for(ch=0;ch<channels;ch++){
        delays[ch] = delays_[ch];
        pos[ch] = 0;
        buffer[ch] = new mha_real_t[delays[ch]+1];
        for(k=0;k<delays[ch];k++)
            buffer[ch][k] = 0;
    }
}

MHASignal::delay_t::~delay_t()
{
    for(unsigned int ch=0;ch<channels;ch++)
        delete [] buffer[ch];
    delete [] buffer;
    delete [] pos;
    delete [] delays;
}

mha_wave_t* MHASignal::delay_t::process(mha_wave_t* s)
{
    unsigned int k, ch, idx;
    mha_real_t tmp;
    if( channels != s->num_channels )
        throw MHA_Error(__FILE__,__LINE__,"Invalid number of channels.");
    for( ch=0;ch<channels;ch++){
        if( delays[ch] ){
            for( k=0;k<s->num_frames;k++){
                idx = ch + channels * k;
                tmp = buffer[ch][pos[ch]];
                buffer[ch][pos[ch]] = s->buf[idx];
                s->buf[idx] = tmp;
                pos[ch]++;
                if( pos[ch] >= delays[ch] )
                    pos[ch] = 0;
            }
        }
    }
    return s;
}

MHASignal::subsample_delay_t::subsample_delay_t(const std::vector<float>& subsample_delay,
                                                unsigned fftlen)
    : phase_gains(fftlen/2+1,subsample_delay.size())
{
    if (fftlen <= 1)
        throw MHA_Error(__FILE__,__LINE__,"counterproductive fft length"
                        " specified: %u", fftlen);

    if (fftlen & 1)
        last_complex_bin = phase_gains.num_frames - 1;
    else {
        last_complex_bin = phase_gains.num_frames - 2;
        for( unsigned int ch = 0; ch < phase_gains.num_channels; ch++ )
            set(phase_gains(phase_gains.num_frames-1,0), 1, ch );
    }
    for( unsigned int ch = 0; ch < phase_gains.num_channels; ch++ )
        for (unsigned frame = 0; frame <= last_complex_bin; ++frame){
            mha_real_t phase = -2 * M_PI * frame / fftlen * subsample_delay[ch];
            expi(phase_gains(frame,ch), phase);
        }
}

void MHASignal::subsample_delay_t::process(mha_spec_t * s)
{
    MHA_assert_equal(s->num_channels, phase_gains.num_channels);
    for (unsigned channel = 0; channel < s->num_channels; ++channel) {
        for (unsigned frame = 1; frame <= last_complex_bin; ++frame) {
            value(s,frame,channel) *= phase_gains( frame, channel );
        }
    }
}

void MHASignal::subsample_delay_t::process (mha_spec_t * s, unsigned idx)
{
    if (idx >= s->num_channels)
        throw MHA_Error(__FILE__,__LINE__,
                        "MHASignal::subsample_delay_t::process: channel index"
                        " %u out of range (signal has %u channels)",
                        idx, s->num_channels);
    for (unsigned frame = 1; frame <= last_complex_bin; ++frame) {
        value(s,frame,idx) *= phase_gains(frame,0);
    }
}

void MHASignal::saveas_mat4(const mha_wave_t& data,const std::string& varname,FILE* fh)
{
    struct {
        int32_t t;
        int32_t rows;
        int32_t cols;
        int32_t imag;
        int32_t namelen;
    } m4h;
    m4h.t = 0000;
    m4h.rows = data.num_frames;
    m4h.cols = data.num_channels;
    m4h.imag = 0;
    m4h.namelen = varname.size()+1;
    fwrite(&m4h,sizeof(m4h),1,fh);
    fwrite(varname.c_str(),1,m4h.namelen,fh);
    // copy the matrix and write to disk:
    double* newdata = new double[size(data)];
    for(unsigned int ch=0;ch<data.num_channels;ch++)
        for(unsigned int k=0;k<data.num_frames;k++)
            newdata[k+ch*data.num_frames] = (double)value(data,k,ch);
    fwrite(newdata,sizeof(double),size(data),fh);
    delete [] newdata;
    // ok, saved.
}

void MHASignal::saveas_mat4(const mha_spec_t& data,const std::string& varname,FILE* fh)
{
    struct {
        int32_t t;
        int32_t rows;
        int32_t cols;
        int32_t imag;
        int32_t namelen;
    } m4h;
    m4h.t = 0000;
    m4h.rows = data.num_frames;
    m4h.cols = data.num_channels;
    m4h.imag = 1;
    m4h.namelen = varname.size()+1;
    fwrite(&m4h,sizeof(m4h),1,fh);
    fwrite(varname.c_str(),1,m4h.namelen,fh);
    // copy the matrix and write to disk:
    double* newdata = new double[size(data)];
    for(unsigned int ch=0;ch<data.num_channels;ch++)
        for(unsigned int k=0;k<data.num_frames;k++)
            newdata[k+ch*data.num_frames] = (double)value(data,k,ch).re;
    fwrite(newdata,sizeof(double),size(data),fh);
    for(unsigned int ch=0;ch<data.num_channels;ch++)
        for(unsigned int k=0;k<data.num_frames;k++)
            newdata[k+ch*data.num_frames] = (double)value(data,k,ch).im;
    fwrite(newdata,sizeof(double),size(data),fh);
    delete [] newdata;
    // ok, saved.
}

void MHASignal::saveas_mat4(const std::vector<mha_real_t>& data,const std::string& varname,FILE* fh)
{
    struct {
        int32_t t;
        int32_t rows;
        int32_t cols;
        int32_t imag;
        int32_t namelen;
    } m4h;
    m4h.t = 0000;
    m4h.rows = 1;
    m4h.cols = data.size();
    m4h.imag = 0;
    m4h.namelen = varname.size()+1;
    fwrite(&m4h,sizeof(m4h),1,fh);
    fwrite(varname.c_str(),1,m4h.namelen,fh);
    // copy the matrix and write to disk:
    double* newdata = new double[data.size()];
    for(unsigned int k=0;k<data.size();k++)
        newdata[k] = (double)data[k];
    fwrite(newdata,sizeof(double),data.size(),fh);
    delete [] newdata;
    // ok, saved.
}

void MHASignal::copy_permuted(mha_wave_t* dest,const mha_wave_t* src)
{
    MHA_assert_equal(src->num_channels,dest->num_frames);
    MHA_assert_equal(src->num_frames,dest->num_channels);
    unsigned int kf;
    unsigned int kc;
    for( unsigned int k=0;k<size(dest); k++){
        kf = k/dest->num_channels;
        kc = k % dest->num_channels;
        dest->buf[k] = value(src,kf,kc);
    }
}

// Local Variables:
// compile-command: "make -C .."
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
