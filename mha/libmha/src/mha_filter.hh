// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2003 2004 2005 2006 2007 2008 2009 2010 HörTech gGmbH
// Copyright © 2011 2012 2013 2014 2016 2017 2018 2019 HörTech gGmbH
// Copyright © 2020
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

#ifndef __MHA_FILTER_H__
#define __MHA_FILTER_H__

#include "mha_toolbox.h"
#include "mha_plugin.hh"
#include "mha_windowparser.h"
#include <valarray>
#include <type_traits>
/**
    \ingroup mhatoolbox
    \file mha_filter.hh
    \brief Header file for IIR filter classes
*/

/** 
    \ingroup mhatoolbox
    \brief Namespace for IIR and FIR filter classes
*/
namespace MHAFilter {

    template<typename T,
             typename std::enable_if< std::is_floating_point< T >::value,
                                      T >::type* = nullptr >
    inline void make_friendly_number(T& x){
        if( (-std::numeric_limits<T>::max() <= x) && (x <= std::numeric_limits<T>::max() ) ){
            if( (0 < x) && (x < std::numeric_limits<T>::min()) )
                x = 0;
            if( (0 > x) && (x > -std::numeric_limits<T>::min()) )
                x = 0;
            return;
        }
        x = 0;
    }

    template<typename T,
             typename std::enable_if< std::is_same< T, mha_complex_t >::value,
                                      T >::type* = nullptr >
    inline void
    make_friendly_number(T& x)
    {
        make_friendly_number(x.re);
        make_friendly_number(x.im);
    }

    /** \brief Generic IIR filter class
        
        This class implements a generic multichannel IIR filter. It is
        realized as direct form II. It can work on any float array or
        on mha_wave_t structs. The filter coefficients can be directly
        accessed.

        \todo Implement a more robust filter form.
    */
    class filter_t {
    public:
        /** 
            \brief Constructor 
            \param ch Number of channels
            \param lena Number of recursive coefficients
            \param lenb Number of non-recursive coefficients
        */
        filter_t(unsigned int ch,  // channels
                 unsigned int lena,  // length A
                 unsigned int lenb); // length B
        /** 
            \brief Constructor with initialization of coefficients.
            \param ch Number of channels.
            \param vA Recursive coefficients.
            \param vB Non-recursive coefficients.
        */
        filter_t(unsigned int ch,const std::vector<mha_real_t>& vA, const std::vector<mha_real_t>& vB);

        filter_t(const MHAFilter::filter_t& src);

        ~filter_t();
        /** \brief Filter all channels in a waveform structure. 
            \param out Output signal
            \param in Input signal
        */
        void filter(mha_wave_t* out,const mha_wave_t* in);
        /** \brief Filter parts of a waveform structure
            \param dest Output signal.
            \param src Input signal.
            \param dframes Number of frames to be filtered.
            \param frame_dist Index distance between frames of one channel
            \param channel_dist Index distance between audio channels
            \param channel_begin Number of first channel to be processed
            \param channel_end Number of last channel to be processed
        */
        void filter(mha_real_t* dest,
                    const mha_real_t* src,
                    unsigned int dframes,
                    unsigned int frame_dist,
                    unsigned int channel_dist,
                    unsigned int channel_begin,
                    unsigned int channel_end);
        /**
           \brief Filter one sample 
           \param x Input value
           \param ch Channel number to use in filter state
        */
        mha_real_t filter(mha_real_t x,unsigned int ch);
        /** \brief Return length of recursive coefficients */
        unsigned int get_len_A() const {return len_A;};
        /** \brief Return length of non-recursive coefficients */
        unsigned int get_len_B() const {return len_B;};
        /** \brief Pointer to recursive coefficients */
        double* A;
        /** \brief Pointer to non-recursive coefficients */
        double* B;
    private:
        unsigned int len_A;
        unsigned int len_B;
        unsigned int len;
        unsigned int channels;
        double* state;
    };

    /**
       \brief Differentiator class (non-normalized)
     */
    class diff_t : public filter_t {
    public:
        diff_t(unsigned int ch);
    };

    /**
       \brief Set first order filter coefficients from time constant and sampling rate.

       \param tau Time constant
       \param fs Sampling rate
       \retval c1 Recursive filter coefficient
       \retval c2 Non-recursive filter coefficient
    */
    void o1_lp_coeffs(const mha_real_t tau,
                      const mha_real_t fs,
                      mha_real_t& c1,
                      mha_real_t& c2);
                      

    /**
       \brief First order attack-release lowpass filter

       This filter is the base of first order lowpass filter, maximum
       tracker and minimum tracker.
    */
    class o1_ar_filter_t : public MHASignal::waveform_t {
    public:
        /**
           \brief Constructor, setting all taus to zero.

           The filter state can be accessed through the member
           functions of MHASignal::waveform_t.
           
           \param channels Number of independent filters
           \param fs Sampling rate (optional, default = 1)
           \param tau_a Attack time constants (optional, default = 0)
           \param tau_r Release time constants (optional, default = 0)
        */
        o1_ar_filter_t(unsigned int channels,mha_real_t fs = 1.0f,std::vector<mha_real_t> tau_a = std::vector<float>(1,0.0f),std::vector<mha_real_t> tau_r = std::vector<float>(1,0.0f));
        /**
           \brief Set the attack time constant
           \param ch Channel number
           \param tau Time constant
        */
        void set_tau_attack(unsigned int ch,mha_real_t tau);
        /**
           \brief Set the release time constant
           \param ch Channel number
           \param tau Time constant
        */
        void set_tau_release(unsigned int ch,mha_real_t tau);
        /** 
            \brief Apply filter to value x, using state channel ch
            \param ch Cannel number
            \param x Input value
            \return Output value
        */
        inline mha_real_t operator()(unsigned int ch,mha_real_t x){
            if( ch >= num_channels )
                throw MHA_Error(__FILE__,__LINE__,"The filter channel is out of range (got %u, %u channels).",
                                ch,num_channels);
            if( x >= buf[ch] )
                buf[ch] = c1_a[ch] * buf[ch] + c2_a[ch] * x;
            else
                buf[ch] = c1_r[ch] * buf[ch] + c2_r[ch] * x;
            MHAFilter::make_friendly_number( buf[ch] );
            return buf[ch];
        };
        /** 
            \brief Apply filter to a mha_wave_t data.
            \param in Input signal
            \param out Output signal
            
            The number of channels must match the number of filter bands.
        */
        inline void operator()(const mha_wave_t& in,mha_wave_t& out){
            MHA_assert_equal(in.num_channels,num_channels);
            MHA_assert_equal(in.num_channels,out.num_channels);
            MHA_assert_equal(in.num_frames,out.num_frames);
            for(unsigned int k=0;k<in.num_frames;k++)
                for(unsigned int ch=0;ch<in.num_channels;ch++)
                    ::value(out,k,ch) = operator()(ch,::value(in,k,ch));
        };
    protected:
        MHASignal::waveform_t c1_a;
        MHASignal::waveform_t c2_a;
        MHASignal::waveform_t c1_r;
        MHASignal::waveform_t c2_r;
        mha_real_t fs;
    };

    /**
       \brief First order low pass filter 
    */
    class o1flt_lowpass_t : public o1_ar_filter_t {
    public:
        o1flt_lowpass_t(const std::vector<mha_real_t>&,mha_real_t,mha_real_t=0);
        o1flt_lowpass_t(const std::vector<mha_real_t>& tau, mha_real_t fs, const std::vector<mha_real_t>& startval);
        void set_tau(unsigned int ch,mha_real_t tau);//!< change the time constant in one channel
        void set_tau(mha_real_t tau);//!< set time constant in all channels to tau
        mha_real_t get_c1(unsigned int ch) const {return c1_a.buf[ch];};
        mha_real_t get_last_output(unsigned int ch) const {return buf[ch];}
    };

    /** \brief First order maximum tracker. */
    class o1flt_maxtrack_t : public o1flt_lowpass_t {
    public:
        o1flt_maxtrack_t(const std::vector<mha_real_t>&,mha_real_t,mha_real_t=0);
        o1flt_maxtrack_t(const std::vector<mha_real_t>& tau, mha_real_t fs, const std::vector<mha_real_t>& startval);
        void set_tau(unsigned int ch,mha_real_t tau);//!< change the time constant in one channel
        void set_tau(mha_real_t tau);//!< set time constant in all channels to tau
    };

    /** \brief First order minimum tracker. */
    class o1flt_mintrack_t : public o1flt_lowpass_t {
    public:
        o1flt_mintrack_t(const std::vector<mha_real_t>&,mha_real_t,mha_real_t=0);
        o1flt_mintrack_t(const std::vector<mha_real_t>&,mha_real_t,const std::vector<mha_real_t>&);
        void set_tau(unsigned int ch,mha_real_t tau);//!< change the time constant in one channel
        void set_tau(mha_real_t tau);//!< set time constant in all channels to tau
    };

    class iir_filter_state_t : public filter_t {
    public:
        iir_filter_state_t(unsigned int channels,std::vector<float> cf_A,std::vector<float> cf_B);
    };

    /** \brief IIR filter class wrapper for integration into parser structure.

        This class implements an infinite impulse response filter.
        Since it inherits from MHAParser::parser_t, it can easily be integrated
        in the \mha configuration tree.  It provides the configuration language
        variables "A" (vector of recursive filter coefficients) and "B" (vector
        of non-recursive filter coefficients). 

        The filter instance reacts to changes in filter coefficients through
        the \mha configuration language, and uses the updated coefficients in
        the next invocation of the filter method. 

        Update of the coefficients is thread-safe and non-blocking. 
        Simply add this subparser to your parser
        items and use the "filter" member function.
        Filter states are reset to all 0 on update.
    */
    class iir_filter_t : public MHAParser::parser_t, private MHAPlugin::config_t<iir_filter_state_t>
    {
    public:
        /** Constructor of the IIR filter.
         * Initialises the sub-parser structure and the memory for holding the
         * filter's state.
         * @param help
         *   The help string for the parser that groups the configuration
         *   variables of this filter.  Could be used to describe the purpose
         *   of this IIR filter.
         * @param def_A
         *   The initial value of the vector of the recursive filter
         *   coefficients, represented as string.
         * @param def_B
         *   The initial value of the vector of the non-recursive filter
         *   coefficients, represented as string.
         * @param channels
         *   The number of indipendent audio channels to process with this
         *   filter.  Needed to allocate a state vector for each audio channel.
         */
        iir_filter_t(std::string help="IIR filter structure",
                     std::string def_A="[1]",
                     std::string def_B="[1]",
                     unsigned int channels=1);
        /** The filter processes the audio signal.  All channels in the audio
         * signal are processed using the same filter coefficients.
         * Indipendent state is stored between calls for each audio channel.
         * @param y
         *   Pointer to output signal holder.
         *   The output signal is stored here.
         *   Has to have the same signal dimensions as the input signal x.
         *   In-place processing (y and x pointing to the same signal holder)
         *   is possible.
         * @param x
         *   Pointer to input signal holder.  Number of channels has to be the
         *   same as given to the constructor, or to the #resize method.
         */
        void filter(mha_wave_t * y, const mha_wave_t * x);
        /** Filter a single audio sample.
         * @param x The single audio sample
         * @param ch Zero-based channel index.  Use and change the state of
         *   channel ch.  ch has to be less than the number of channels given
         *   to the constructor or the #resize method.
         * @return the filtered result sample. */
        mha_real_t filter(mha_real_t x,unsigned int ch);
        /** Change the number of channels after object creation.
         * @param channels The new number of channels. 
         *   Old filter states are lost. */
        void resize(unsigned int channels);
    private:
        void update_filter();
        MHAParser::vfloat_t A;
        MHAParser::vfloat_t B;
        MHAEvents::patchbay_t<iir_filter_t> connector;
        unsigned int nchannels;
    };

    /** 
        
        \brief Setup a first order butterworth band stop filter.
      
        This function calculates the filter coefficients of a first order
        butterworth band stop filter.
    
        \retval A      recursive filter coefficients
        \retval B      non recursive filter coefficients
        \param f1     lower frequency
        \param f2     upper frequency
        \param fs     sample frequency
    */
    void butter_stop_ord1(double* A,double* B,double f1,double f2,double fs);

    /**

        \brief Setup a nth order fir low pass filter.

        This function calculates the filter coefficients of a nth order
        fir low pass filter filter. Frequency arguments above the nyquist frequency
        are accepted but the spectral response is truncated at the nyquist frequency
        \returns      vector containing filter coefficients
        \pre f_pass_ must be smaller or equal to f_stop_.
        \param f_pass_     Upper passband frequency
        \param f_stop-     Lower stopband frequency
        \param fs_     sample frequency
    */
    std::vector<float> fir_lp(float f_pass_, float f_stop_, float fs_, unsigned order_);

    class adapt_filter_state_t {
    public:
        adapt_filter_state_t(int ntaps,int nchannels);
        void filter(mha_wave_t y,mha_wave_t e,mha_wave_t x,mha_wave_t d,mha_real_t mu,bool err_in);
    private:
        int ntaps, nchannels;
        MHASignal::waveform_t W;
        MHASignal::waveform_t X;
        MHASignal::waveform_t od;
        MHASignal::waveform_t oy;
    };

    class adapt_filter_param_t {
    public:
        adapt_filter_param_t(mha_real_t imu,bool ierr_in);
        mha_real_t mu;
        bool err_in;
    };

    /**
       \brief Adaptive filter
    */
    class adapt_filter_t : public MHAParser::parser_t, 
                           private MHAPlugin::config_t<adapt_filter_state_t>,
                           private MHAPlugin::config_t<adapt_filter_param_t> {
    public:
        adapt_filter_t(std::string);
        void filter(mha_wave_t y,mha_wave_t e,mha_wave_t x,mha_wave_t d);
        void set_channelcnt(unsigned int);
    private:
        void update_mu();
        void update_ntaps();
        MHAParser::float_t mu;
        MHAParser::int_t ntaps;
        MHAParser::bool_t err_in;
        MHAEvents::patchbay_t<adapt_filter_t> connector;
        unsigned int nchannels;
    };

    /**
       \brief FFT based FIR filter implementation.

       The maximal number of coefficients can be FFT length - fragsize + 1.
    */
    class fftfilter_t 
    {
    public:
        /**
           \brief Constructor
           \param fragsize Number of frames expected in input signal (each cycle).
           \param channels Number of channels expected in input signal.
           \param fftlen FFT length of filter.
        */
        fftfilter_t(unsigned int fragsize,
                    unsigned int channels,
                    unsigned int fftlen);
        ~fftfilter_t();
        /**
           \brief Update the set of coefficients
           \param pwIRS Coefficients structure

           \note The number of channels in h must match the number of
           channels given in the constructor. The filter length is
           limited to fftlen-fragsize+1 (longer IRS will be
           shortened).
        */
        void update_coeffs(const mha_wave_t* pwIRS);
        /**
           \brief Apply filter with changing coefficients to a
           waveform fragment
           \param pwIn Input signal pointer.
           \retval ppwOut Pointer to output signal pointer, will be set to a valid signal.
           \param pwIRS Pointer to FIR coefficients structure.
        */
        void filter(const mha_wave_t* pwIn, mha_wave_t** ppwOut, const mha_wave_t* pwIRS);
        /**
           \brief Apply filter to waveform fragment, without changing the coefficients
           \param pwIn Input signal pointer.
           \retval ppwOut Pointer to output signal pointer, will be set to a valid signal
        */
        void filter(const mha_wave_t* pwIn, mha_wave_t** ppwOut);
        /**
           \brief Apply filter with changing coefficients to a
           waveform fragment
           \param pwIn Input signal pointer.
           \retval ppwOut Pointer to output signal pointer, will be set to a valid signal.
           \param psWeights Pointer to filter weights structure.
        */
        void filter(const mha_wave_t* pwIn, mha_wave_t** ppwOut, const mha_spec_t* psWeights);
    private:
        unsigned int fragsize;
        unsigned int channels;
        unsigned int fftlen;
        MHASignal::waveform_t wInput_fft;
        mha_wave_t wInput;
        MHASignal::waveform_t wOutput_fft;
        mha_wave_t wOutput;
        MHASignal::spectrum_t sInput;
        MHASignal::spectrum_t sWeights;
        MHASignal::waveform_t wIRS_fft;
        mha_fft_t fft;
    };

    /**
       \brief FFT based FIR filterbank implementation.

       This class convolves n input channels with m filter coefficient
       sets and returns n*m output channels.

       The maximal number of coefficients can be FFT length - fragsize + 1.
    */
    class fftfilterbank_t 
    {
    public:
        /**
           \brief Constructor.
           \param fragsize Number of frames expected in input signal (each cycle).
           \param inputchannels Number of channels expected in input signal.
           \param firchannels Number of channels expected in FIR filter coefficients (= number of bands).
           \param fftlen FFT length of filter.

           The number of output channels is inputchannels*firchannels.
        */
        fftfilterbank_t(unsigned int fragsize,
                        unsigned int inputchannels,
                        unsigned int firchannels,
                        unsigned int fftlen);
        ~fftfilterbank_t();
        /**
           \brief Update the set of coefficients
           \param h Coefficients structure

           \note The number of channels in h must match the number of
           channels given in the constructor, and the number of frames
           can not be more than fftlen-fragsize+1.
        */
        void update_coeffs(const mha_wave_t* h);
        /**
           \brief Apply filter with changing coefficients to a
           waveform fragment
           \param s_in Input signal pointer.
           \retval s_out Pointer to output signal pointer, will be set to a valid signal
           \param h FIR coefficients
        */
        void filter(const mha_wave_t* s_in, mha_wave_t** s_out, const mha_wave_t* h);
        /**
           \brief Apply filter to waveform fragment, without changing the coefficients
           \param s_in Input signal pointer.
           \retval s_out Pointer to output signal pointer, will be set to a valid signal
        */
        void filter(const mha_wave_t* s_in, mha_wave_t** s_out);
        /**
           \brief Return the current IRS.
         */
        const mha_wave_t* get_irs() const {return &hw;};
    private:
        unsigned int fragsize;
        unsigned int inputchannels;
        unsigned int firchannels;
        unsigned int outputchannels;
        unsigned int fftlen;
        MHASignal::waveform_t hw;
        MHASignal::spectrum_t Hs;
        MHASignal::waveform_t xw;
        MHASignal::spectrum_t Xs;
        MHASignal::waveform_t yw;
        MHASignal::spectrum_t Ys;
        MHASignal::waveform_t yw_temp;
        MHASignal::waveform_t tail;
        mha_fft_t fft;
    };

    /**
     * a structure containing a source channel number, a target channel number,
     * and an impulse response.
     */
    struct transfer_function_t {
        /** Source audio channel index for this transfer function */
        unsigned int source_channel_index;
        /** Target audio channel index for this transfer function */
        unsigned int target_channel_index;
        /** Impulse response of transfer from source to target channel */
        std::vector<float> impulse_response;

        /** Default constructor for STL conformity. Not used. */
        transfer_function_t()
            : source_channel_index(0),
              target_channel_index(0),
              impulse_response()
            {}

        /** Data constructor.
         * @param source_channel_index
         *   Source audio channel index for this transfer function
         * @param target_channel_index
         *   Target audio channel index for this transfer function
         * @param impulse_response
         *   Impulse response of transfer from source to target channel */
        transfer_function_t(unsigned int source_channel_index,
                            unsigned int target_channel_index,
                            const std::vector<float> & impulse_response);

        /** for the given partition size, return the number of partitions
         * of the impulse response.
         * @param fragsize partition size
         * @return number of partitions occupied by the impulse response */
        unsigned int partitions(unsigned int fragsize) const
            {
                if (fragsize == 0U) throw MHA_ErrorMsg("fragsize must be >0");
                return (impulse_response.size() + fragsize - 1) / fragsize; 
            }

        /** for the given partition size, return the number of non-empty
         * partitions of the impulse response.
         * @param fragsize partition size
         * @returns the number of non-empty partitions of the impulse
         *   response, i.e. partitions containing only zeros are not
         *   counted. */
        unsigned int non_empty_partitions(unsigned int fragsize) const
            {
                unsigned result = 0U;
                for (unsigned i = 0U; i < partitions(fragsize); ++i)
                    if (!isempty(fragsize, i))
                        ++result;
                return result;
            }

        /** checks if the partition contains only zeros
         * @param fragsize partition size
         * @param index partition index
         * @return true when this partition of the impulse response contains
         *         only zeros.
         */
        bool isempty(unsigned int fragsize, unsigned int index) const
            {
                for (unsigned i = 0; i < fragsize; ++i) {
                    if (index * fragsize + i >= impulse_response.size())
                        return true;
                    if (impulse_response[index * fragsize + i] != 0.0f)
                        return false;
                }
                return true;
            }
    };
    
    /**
     * A sparse matrix of transfer function partitionss.  Each matrix element
     * knows its position in the matrix, so they can be stored as a vector.
     */
    struct transfer_matrix_t : public std::vector<transfer_function_t>
    {
        /** Returns an array of the results of calling the partitions()
         * method on every matrix member */
        std::valarray<unsigned int> partitions(unsigned fragsize) const
            {
                std::valarray<unsigned int> result(0U, size());
                for (unsigned i = 0; i < size(); ++i)
                    result[i] = (*this)[i].partitions(fragsize);
                return result;
            }

        /** Returns an array of the results of calling the 
         * non_empty_partitions() method on every matrix member */
        std::valarray<unsigned int> non_empty_partitions(unsigned int fragsize)
            const
            {
                std::valarray<unsigned int> result(0U, size());
                for (unsigned i = 0; i < size(); ++i)
                    result[i] = (*this)[i].non_empty_partitions(fragsize);
                return result;
            }
    };

    /**
     * A filter class for partitioned convolution.
     * Impulse responses are partitioned into sections of fragment size.
     * Audio signal is convolved with every partition and delayed as needed.
     * Convolution is done according to overlap-save.
     * FFT length used is 2 times fragment size.
     */
    class partitioned_convolution_t {
    public:
        /**
         * Create a new partitioned convolver.
         * @param fragsize
         *    Audio fragment size, equal to partition size.
         * @param nchannels_in
         *    Number of input audio channels.
         * @param nchannels_out
         *    Number of output audio channels.
         * @param transfer
         *    A sparse matrix of impulse responses.
         */ 
        partitioned_convolution_t(unsigned int fragsize,
                                  unsigned int nchannels_in,
                                  unsigned int nchannels_out,
                                  const transfer_matrix_t & transfer);

        /** Free fftw resource allocated in constructor */
        ~partitioned_convolution_t();

        /** Audio fragment size, always equal to partition size. */
        unsigned int fragsize;
        /** Number of audio input channels. */
        unsigned int nchannels_in;
        /** Number of audio output channels. */
        unsigned int nchannels_out;

        /** The maximum number of partitions in any of the impulse responses.
         * Determines the size if the delay line. */
        unsigned int output_partitions;

        /** The total number of non-zero impulse response partitions. */
        unsigned int filter_partitions;

        /** Bookkeeping class.  For each impulse response partition,
         * keeps track of which input to filter, which output channel
         * to filter to, and the delay in blocks.  Objects of class
         * Index should be kept in an array with the same indices as
         * the corresponding inpulse response partitions.
         */
        struct index_t {
            /** The input channel index to apply the current partition to. */
            unsigned int source_channel_index;
            /** The index of the output channel to which the filter result
             * should go. */
            unsigned int target_channel_index;
            /** The delay (in blocks) of this partition */
            unsigned int delay;

            /** Data constructor
             * @param src
             *   The input channel index to apply the current partition to.
             * @param tgt
             *   The index of the output channel to which the filter result
             *   should go.
             * @param dly
             *   The delay (in blocks) of this partition
             */
            index_t(unsigned int src, unsigned int tgt, unsigned int dly)
                : source_channel_index(src),
                  target_channel_index(tgt),
                  delay(dly)
                {}
            /** Default constructor for STL compatibility */
            index_t(): source_channel_index(0),target_channel_index(0),delay(0)
                {}
        };

        /** Buffer for input signal. Has nchannels_in channels and fragsize*2
         * frames */
        MHASignal::waveform_t input_signal_wave;

        /** A counter modulo 2. Indicates the buffer half in input signal wave
         * into which to copy the current input signal. */
        unsigned int current_input_signal_buffer_half_index;

        /** Buffer for FFT transformed input signal. Has nchannels_in channels
         * and fragsize+1 frames (fft bins). */
        MHASignal::spectrum_t input_signal_spec;

        /** Buffers for frequency response spectra of impulse response
         * partitions.  Each "channel" contains another partition of
         * some impulse response.  The bookkeeping array is used to
         * keep track what to do with these frequency responses. 
         * This container has filter_partitions channels and fragsize+1
         * frames (fft bins).
         */
        MHASignal::spectrum_t frequency_response;
        
        /** Keeps track of input channels, output channels, impulse
         * response partition, and delay.  The index into this array
         * is the same as the "channel" index into the
         * frequency_response array.  Array has filter_partitions
         * entries.
         */
        std::vector<index_t> bookkeeping;
        
        /** Buffers for FFT transformed output signal.  For each array
         * member, Number of channels is equal to nchannels_out,
         * number of frames (fft bins) is equal to fragsize+1.
         * Array size is equal to output_partitions. */
        std::vector<MHASignal::spectrum_t> output_signal_spec;
        
        /** A counter modulo output_partitions, indexing the "current"
         * output partition. */
        unsigned int current_output_partition_index;

        /** Buffer for the wave output signal.
         *  Number of channels is equal to nchannels_out, number of frames
         *  is equal to fragsize */
        MHASignal::waveform_t output_signal_wave;

        /** The FFT transformer */
        mha_fft_t fft;

        /** processing */
        mha_wave_t * process(const mha_wave_t * s_in);
    };
    
    /**
       \brief Smooth spectral gains, create a windowed impulse response.

       Spectral gains are smoothed by multiplying the impulse
       response with a window function.
       
       If a minimal phase is used, then the original phase is
       discarded and replaced by the minimal phase function.  In this
       case, the window is applied to the beginning of the inverse
       Fourier transform of the input spectrum, and the remaining
       signal set to zero.  If the original phase is kept, the window
       is applied symmetrically arround zero, i.e. to the first and last
       samples of the inverse Fourier transform of the input
       spectrum.  The spec2fir() function creates a causal impulse
       response by circularly shifting the impulse response by half of
       the window length.

       The signal dimensions of the arguments of smoothspec() must
       correspond to the FFT length and number of channels provided in
       the constructor.  The function spec2fir() can fill signal
       structures with more than window length frames.
    */
    class smoothspec_t {
    public:
        /**
           \brief Constructor.
           \param fftlen FFT length of input spectrum (fftlen/2+1 bins)
           \param nchannels Number of channels in input spectrum
           \param window Window used for smoothing
           \param minphase Use minimal phase (true) or original phase (false)
           \param linphase_asym Keep phase, but apply full window at beginning of IRS
        */
        smoothspec_t(unsigned int fftlen,unsigned int nchannels,
                     const MHAWindow::base_t& window,bool minphase,bool linphase_asym = false);
        /** \brief Create a smoothed spectrum 
            \param s_in Input spectrum
            \retval s_out Output spectrum
        */
        void smoothspec(const mha_spec_t& s_in,mha_spec_t& s_out);
        /** \brief Create a smoothed spectrum (in place) 
            \param spec Spectrum to be smoothed.
        */
        void smoothspec(mha_spec_t& spec) {smoothspec(spec,spec);};
        /** \brief Return FIR coefficients 
            \param spec Input spectrum
            \retval fir FIR coefficients, minimum length is window length
        */
        void spec2fir(const mha_spec_t& spec,mha_wave_t& fir);
        ~smoothspec_t();
    private:
        void internal_fir(const mha_spec_t&);
        unsigned int fftlen;
        unsigned int nchannels;
        MHAWindow::base_t window;
        MHASignal::waveform_t tmp_wave;
        MHASignal::spectrum_t tmp_spec;
        MHASignal::minphase_t* minphase;
        bool _linphase_asym;
        mha_fft_t fft;
    };

    /**
       \brief Create a windowed impulse response/FIR filter coefficients from a spectrum.
       \param spec Input spectrum
       \param fftlen FFT length of spectrum
       \param window Window shape (with length, e.g. initialized with MHAWindow::hanning(54)).
       \param minphase Flag, true if original phase should be discarded and replaced by a minimal phase function.
    */
    MHASignal::waveform_t* spec2fir(const mha_spec_t* spec,const unsigned int fftlen,const MHAWindow::base_t& window,const bool minphase);

    /// greatest common divisor
    inline unsigned gcd(unsigned a, unsigned b) {return b ? gcd(b, a%b) : a;}

    /** sin(x)/x function, coping with x=0.
     * This is the historical sinc function, not the normalized sinc function.
     */
    double sinc(double x);

    /** Computes rational resampling factor from two sampling rates.
     * The function will fail if either sampling_rate * factor is not an integer
     * @param source_sampling_rate The original sampling rate
     * @param target_sampling_rate The desired sampling rate
     * @param factor A helper factor to use for non-integer sampling rates
     * @return a pair that contains first the upsampling factor and second the
     *         downsampling factor required for the specified resampling.
     * @throw MHA_Error if no rational resampling factor can be found. */
    std::pair<unsigned,unsigned> resampling_factors(float source_sampling_rate,
                                                    float target_sampling_rate,
                                                    float factor = 1.0f);

    /**
       \brief Hann shaped low pass filter for resampling.
       This class uses FFT filter at upsampled rate.
    */
    class resampling_filter_t : public MHAFilter::fftfilter_t {
    public:
        /**
           \brief Constructor.
           \param fftlen FFT length.
           \param irslen Length of filter.
           \param channels Number of channels to be filtered.
           \param Nup Upsampling ratio.
           \param Ndown Downsampling ratio.
           \param fCutOff Cut off frequency (relative to lower Nyquist Frequency)
        */
        resampling_filter_t(unsigned int fftlen, unsigned int irslen, unsigned int channels, unsigned int Nup, unsigned int Ndown, double fCutOff);
        static unsigned int fragsize_validator(unsigned int fftlen, unsigned int irslen);
    private:
        unsigned int fragsize;
    };
    
    /** A class that performs polyphase resampling. 
     *
     * Background information: When resampling from one sampling rate
     * to another, it helps when one sampling rate is a multiple of
     * the other sampling rate: In the case of upsampling, the samples
     * at the original rate are copied to the upsampled signal spread
     * out with a constant number of zero samples between the
     * originally adjacent samples.  The signal is then low-pass
     * filtered to avoid frequency aliasing and to fill the
     * zero-samples with interpolated values.  In the case of
     * down-sampling, the signal is first low-pass filtered for
     * anti-aliasing, and only every n<SUP>th</SUP> sample of the
     * filtered output is used for the signal at the new sample rate.
     * Of course, for finite-impulse-response (FIR) filters this means
     * that only every n<SUP>th</SUP> sample needs to be computed.
     *
     * When resampling from one sampling rate to another where neither
     * is a multiple of the other, the signal first needs to be
     * upsampled to a sampling rate that is a multiple of both (source
     * and target) sampling rates, and then downsampled again to the
     * target sampling rate.  Instead of applying two separate lowpass
     * filters directly after each other (one filter for upsampling
     * and another for downsampling), it is sufficient to apply only
     * one low-pass filter, when producing the output at the final
     * target rate, with a cut-off frequency equal to the lower
     * cut-off-frequency of the replaced two low-pass filters.  Not
     * filtering to produce a filtered signal already at the common
     * multiple sampling rate has the side effect that this
     * intermediate signal at the common multiple sampling rate keeps
     * its filler zero samples unaltered.  These zero samples can be
     * taken advantage of when filtering to produce the output at the
     * target rate: The zeros do not need to be multiplied with their
     * corresponding filter coefficients, because the result is known
     * to be zero again, and this zero product has no effect on the
     * summation operation to compute a target sample at the target
     * rate. 
     * To summarize, the following optimization techniques are available:
     *
     * * The signal does not need to be stored in memory at the
     *   interpolation rate. It is sufficient to have the signal
     *   available at the source rate and to know where the zeros
     *   would be.
     * * The signal needs to be low-pass-filtered only once.
     * * The FIR low-pass filtering can take advantage of
     *  * computing only filter outputs for the required samples at the target rate,
     *  * skipping over zero-samples at the interpolation rate.  
     *
     * The procedure that takes advantage of these optimization
     * possibilites is known as polyphase resampling.
     *
     * This class implements polyphase resampling in this way for a
     * source sampling rate and a target sampling rate that have common
     * multiple, the interpolation sampling rate.
     * Non-rational and drifting sample rates are outside the scope of
     * this resampler.
     */
    class polyphase_resampling_t {

        /** Integer upsampling factor.  Interpolation rate divided by source rate. */
        unsigned upsampling_factor;

        /** Integer downsampling factor.  Interpolation rate divided by target rate. */
        unsigned downsampling_factor;

        /** Index of "now" in the interpolated sampling rate.
         * @todo Index into what? What is the meaning of now? */
        unsigned now_index;

        /** Set to true when an underflow has occurred.  When this is
         * true, then the object can no longer be used.  Underflows
         * have to be avoided by clients, e.g. by checking that enough
         * #readable_frames are present before calling #read */
        bool underflow;

        /** Contains the impulse response of the lowpass filter needed
         * for anti-aliasing.  The impulse response is stored at the
         * interpolation sampling rate.  We use an instance of
         * MHAWindow::hanning_t here because we are limiting the sinc
         * impulse response with a Hanning window (otherwise the
         * impulse response would extend indefinitely into past and
         * future).  And the samples inside an MHAWindow::hanning_t
         * can be altered with *=, which our constructor does. */
        MHAWindow::hanning_t impulse_response;
        
        /** Storage of input signal.  Part of the polyphase resampling
         * optimization is that apart from the FIR impulse response,
         * nothing is stored at the interpolation rate, saving memory
         * and computation cycles. */
        MHASignal::ringbuffer_t ringbuffer;
    public:
        /** Construct a polyphase resampler instance.

            Allocates a ringbuffer with the given capacity \a
            n_ringbuffer.  Client that triggers the constructor must
            ensure that the capacity \a n_ringbuffer and the delay \a
            n_prefill are sufficient, i.e. enough old and new samples
            are always available to compute sufficient samples in
            using an impulse response of length \a n_irs.  Audio block
            sizes at both sides of the resampler have to be taken into
            account.  Class \c
            MHASignal::blockprocessing_polyphase_resampling_t takes
            care of this, and it is recommended to use this class for
            block-based processing.

            Based on \a n_up, \a n_down, \a n_irs and \a
            nyquist_ratio, a suitable sinc impulse response is
            computed and windowed with a hanning window to limit its
            extent.

            The actual source sampling rate, target sampling rate, and
            interpolation sampling rate are not parameters to this
            constructors, because only their ratios matter.

            @param n_up upsampling factor, ratio between interpolation
                   rate and source rate.
            @param n_down downsampling factor, ratio between interpolation
                   rate and target rate.
            @param nyquist_ratio low pass filter cutoff frequency relative to
                   the nyquist frequency of the smaller of the two sampling
                   rates.
                   Example values: E.g. 0.8, 0.9
            @param n_irs length of impulse response (in samples at
                                                     interpolation rate)
            @param n_ringbuffer length of ringbuffer,
                   in samples at source sampling rate
            @param n_channels audio channels count
            @param n_prefill Prefill the ringbuffer with this many zero frames
                             in samples at source sampling rate
        */
        polyphase_resampling_t(unsigned n_up, unsigned n_down, 
                               mha_real_t nyquist_ratio,
                               unsigned n_irs,
                               unsigned n_ringbuffer, unsigned n_channels,
                               unsigned n_prefill);
        /** Write signal to the ringbuffer.  Signal contained in
         * signal is appended to the audio frames already present in
         * the ringbuffer.
         * @param signal input signal in original sampling rate
         * @throw MHA_Error Raises exception if there is not enough room or
         *                  if the number of channels does not match. */
        void write(mha_wave_t & signal);
        /** Read resampled signal. Will perform the resampling and remove
         * no longer needed samples from the input buffer.
         * @param signal buffer to write the resampled signal to.
         * @throw MHA_Error Raises exception if there is not enough input signal
         *                  or if the number of channels is too high. */
        void read(mha_wave_t & signal);
        /** Number of frames at target sampling rate that can be
         * produced.  This method only checks for enough future
         * samples present, therefore, this number can be positive and
         * a read operation can still fail if there are not enough
         * past samples present to perform the filtering for the first
         * output sample.  This could only happen if the constructor
         * parameters \a n_ringbuffer or \a n_prefill have been chosen
         * too small, because otherwise the method #read ensures that
         * enough past samples are present to compute the next target
         * sample. */
        unsigned readable_frames() const {
            unsigned interpolation_frames =
                ringbuffer.contained_frames() * upsampling_factor;
            if (interpolation_frames < now_index)
                return 0U;
            return (interpolation_frames - now_index + downsampling_factor - 1U) / downsampling_factor;
        }
    };

    /** A class that does polyphase resampling and takes into account
     * block processing. */
    class blockprocessing_polyphase_resampling_t {
        polyphase_resampling_t * resampling;
        unsigned fragsize_in, fragsize_out, num_channels;
    public:
        /** Contructs a polyphase resampling filter that can be used for
            blockprocessing with the given parameters.
            @param source_srate
              Source sampling rate / Hz
            @param source_fragsize
              Fragment size of incoming audio blocks / frames at source_srate
            @param target_srate
              Target sampling rate / Hz
            @param target_fragsize
              Fragment size of produced audio blocks / frames at target_srate
            @param nyquist_ratio
              Low pass filter cutoff frequency relative to the nyquist frequency
              of the smaller of the two sampling rates.
              Example values: 0.8, 0.9
            @param irslen
              Impulse response length used for low pass filtering / s
            @param nchannels
              Number of audio channels
            @param add_delay
              To avoid underruns, a delay is generally necessary for round trip
              block size adaptations. It is only necessary to add this delay
              to one of the two resampling chains. 
              Set this parameter to true for the first resampling object of a
              round trip pair. It will add the necessary delay, and calculate
              the size of the ring buffer appropriately,
              When set to false, only the ringbuffer size will be set 
              sufficiently.
        */          
        blockprocessing_polyphase_resampling_t(float source_srate,
                                               unsigned source_fragsize,
                                               float target_srate,
                                               unsigned target_fragsize,
                                               float nyquist_ratio,
                                               float irslen,
                                               unsigned nchannels,
                                               bool add_delay);
        virtual ~blockprocessing_polyphase_resampling_t() {
            delete resampling; resampling = 0;
        }

        /** Write signal to the ringbuffer.
         * @param signal input signal in original sampling rate
         * @throw MHA_Error Raises exception if there is not enough room,
         *                  if the number of channels does not match,
         *                  or if the number of frames is not equal to the
         *                  number specified in the constructor */
        void write(mha_wave_t & signal);

        /** Read resampled signal. Will perform the resampling and remove
         * no longer needed samples from the input buffer.
         * @param signal buffer to write the resampled signal to.
         * @throw MHA_Error Raises exception if there is not enough input
         *                  signal, if the number of channels of frames does
         *                  not match. */
        void read(mha_wave_t & signal);
            
        /** Checks if the resampling ring buffer can produce another output
            signal block
         */
        bool can_read() const {
            return resampling->readable_frames() >= fragsize_out;
        }
            
    };

    /**
       \brief First order recursive filter
     */
    class iir_ord1_real_t {
    public:
        /**
           \brief Constructor with filter coefficients (one per channel)
         */
        iir_ord1_real_t(std::vector<mha_real_t> A,std::vector<mha_real_t> B);
        /**
           \brief Constructor for low pass filter (one time constant per channel)
         */
        iir_ord1_real_t(std::vector<mha_real_t> tau,mha_real_t srate);
        void set_state(mha_real_t val);
        void set_state(std::vector<mha_real_t> val);
        void set_state(mha_complex_t val);
        /**
           \brief Filter method for real value input, one element.
        */
        inline mha_real_t operator()(unsigned int ch,mha_real_t x) {
            Yn[ch].re *= A_[ch];
            Yn[ch].re += B_[ch]*x;
            return Yn[ch].re;
        };
        /**
           \brief Filter method for complex input, one element.
        */
        inline mha_complex_t operator()(unsigned int ch,mha_complex_t x) {
            Yn[ch].re *= A_[ch];
            Yn[ch].re += B_[ch]*x.re;
            Yn[ch].im *= A_[ch];
            Yn[ch].im += B_[ch]*x.im;
            return Yn[ch];
        };
        /**
           \brief Filter method for real value input.
        */
        inline void operator()(const mha_wave_t& X,mha_wave_t& Y) {
            MHA_assert_equal(X.num_channels,A_.size());
            MHA_assert_equal(X.num_channels,Y.num_channels);
            MHA_assert_equal(X.num_frames,Y.num_frames);
            for( unsigned int k=0;k<X.num_frames;k++)
                for( unsigned int ch=0;ch<X.num_channels;ch++)
                    value(Y,k,ch) = operator()(ch,value(X,k,ch));
        };
        /**
           \brief Filter method for complex value input.
        */
        inline void operator()(const mha_spec_t& X,mha_spec_t& Y) {
            MHA_assert_equal(X.num_channels,A_.size());
            MHA_assert_equal(X.num_channels,Y.num_channels);
            MHA_assert_equal(X.num_frames,Y.num_frames);
            for( unsigned int k=0;k<X.num_frames;k++)
                for( unsigned int ch=0;ch<X.num_channels;ch++)
                    value(Y,k,ch) = operator()(ch,value(X,k,ch));
        };
        /**
           \brief Filter method for complex value input.
        */
        inline void operator()(const mha_wave_t& Xre,const mha_wave_t& Xim,mha_wave_t& Yre,mha_wave_t& Yim) {
            MHA_assert_equal(Xre.num_channels,A_.size());
            MHA_assert_equal(Xre.num_channels,Yre.num_channels);
            MHA_assert_equal(Xre.num_frames,Yre.num_frames);
            MHA_assert_equal(Xre.num_channels,Yim.num_channels);
            MHA_assert_equal(Xre.num_frames,Yim.num_frames);
            MHA_assert_equal(Xre.num_channels,Xim.num_channels);
            MHA_assert_equal(Xre.num_frames,Xim.num_frames);
            for( unsigned int k=0;k<Xre.num_frames;k++)
                for( unsigned int ch=0;ch<Xre.num_channels;ch++){
                    Yn[ch].re *= A_[ch];
                    Yn[ch].im *= A_[ch];
                    Yn[ch].re += B_[ch]*value(Xre,k,ch);
                    Yn[ch].im += B_[ch]*value(Xim,k,ch);
                    value(Yre,k,ch) = Yn[ch].re;
                    value(Yim,k,ch) = Yn[ch].im;
                }
        };
    private:
        std::vector<mha_real_t> A_;
        std::vector<mha_real_t> B_;
        std::vector<mha_complex_t> Yn;
    };



}

#endif /* __MHA_FILTER_H__ */

// Local Variables:
// compile-command: "make -C .."
// c-basic-offset: 4
// coding: utf-8-unix
// indent-tabs-mode: nil
// End:
