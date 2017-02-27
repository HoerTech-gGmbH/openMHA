// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2009 2010 2012 2013 2016 HörTech gGmbH
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

#ifndef COMPLEX_FILTER_H
#define COMPLEX_FILTER_H

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include "mha_signal.hh"
#include "mha_error.hh"
#include "mha_parser.hh"

namespace MHAFilter {

    /**
       \brief Complex bandpass filter
    */
    class complex_bandpass_t {
    public:
	static std::vector<mha_complex_t> creator_A(std::vector<mha_real_t> cf,std::vector<mha_real_t> bw,mha_real_t srate,unsigned int order);
	static std::vector<mha_complex_t> creator_B(std::vector<mha_complex_t> A,unsigned int order);
	/**
	   \brief Constructor with filter coefficients (one per channel)
	   \param A complex filter coefficients, one per band
	   \param B complex weights
	*/
	complex_bandpass_t(std::vector<mha_complex_t> A,std::vector<mha_complex_t> B);
	void set_state(mha_real_t val);
	void set_state(std::vector<mha_real_t> val);
	void set_state(mha_complex_t val);
	/**
	   \brief Allow to modify the input weights at a later stage.
	*/
	void set_weights(std::vector<mha_complex_t> new_B);
	std::vector<mha_complex_t> get_weights() const { return B_;};
	/**
	   \brief Filter method for real value input.
	*/
	inline void filter(const mha_wave_t& X,mha_spec_t& Y) {
	    MHA_assert_equal(X.num_channels,A_.size());
	    MHA_assert_equal(X.num_channels,Y.num_channels);
	    MHA_assert_equal(X.num_frames,Y.num_frames);
	    for( unsigned int k=0;k<X.num_frames;k++)
		for( unsigned int ch=0;ch<X.num_channels;ch++){
		    Yn[ch] *= A_[ch];
		    Yn[ch].re += B_[ch].re*value(X,k,ch);
		    Yn[ch].im += B_[ch].im*value(X,k,ch);
		    value(Y,k,ch) = Yn[ch];
		}
	};
	/**
	   \brief Filter method for real value input.
	*/
	inline void filter(const mha_wave_t& X,mha_wave_t& Yre, mha_wave_t& Yim) {
	    MHA_assert_equal(X.num_channels,A_.size());
	    MHA_assert_equal(X.num_channels,Yre.num_channels);
	    MHA_assert_equal(X.num_frames,Yre.num_frames);
	    MHA_assert_equal(X.num_channels,Yim.num_channels);
	    MHA_assert_equal(X.num_frames,Yim.num_frames);
	    for( unsigned int k=0;k<X.num_frames;k++)
		for( unsigned int ch=0;ch<X.num_channels;ch++){
		    Yn[ch] *= A_[ch];
		    Yn[ch].re += B_[ch].re*value(X,k,ch);
		    Yn[ch].im += B_[ch].im*value(X,k,ch);
		    value(Yre,k,ch) = Yn[ch].re;
		    value(Yim,k,ch) = Yn[ch].im;
		}
	};
	/**
	   \brief Filter method for complex value input.
	*/
	inline void filter(const mha_spec_t& X,mha_spec_t& Y) {
	    MHA_assert_equal(X.num_channels,A_.size());
	    MHA_assert_equal(X.num_channels,Y.num_channels);
	    MHA_assert_equal(X.num_frames,Y.num_frames);
	    mha_complex_t val;
	    for( unsigned int k=0;k<X.num_frames;k++)
		for( unsigned int ch=0;ch<X.num_channels;ch++){
		    val = value(X,k,ch);
		    val *= B_[ch];
		    Yn[ch] *= A_[ch];
		    Yn[ch] += val;
		    value(Y,k,ch) = Yn[ch];
		}
	};
	/**
	   \brief Filter method for complex value input.
	*/
	inline void filter(const mha_wave_t& Xre,const mha_wave_t& Xim,mha_wave_t& Yre,mha_wave_t& Yim) {
	    MHA_assert_equal(Xre.num_channels,A_.size());
	    MHA_assert_equal(Xre.num_channels,Yre.num_channels);
	    MHA_assert_equal(Xre.num_frames,Yre.num_frames);
	    MHA_assert_equal(Xre.num_channels,Yim.num_channels);
	    MHA_assert_equal(Xre.num_frames,Yim.num_frames);
	    MHA_assert_equal(Xre.num_channels,Xim.num_channels);
	    MHA_assert_equal(Xre.num_frames,Xim.num_frames);
	    mha_complex_t val;
	    for( unsigned int k=0;k<Xre.num_frames;k++)
		for( unsigned int ch=0;ch<Xre.num_channels;ch++){
		    val.re = value(Xre,k,ch);
		    val.im = value(Xim,k,ch);
		    val *= B_[ch];
		    Yn[ch] *= A_[ch];
		    Yn[ch] += val;
		    value(Yre,k,ch) = Yn[ch].re;
		    value(Yim,k,ch) = Yn[ch].im;
		}
	};
	std::string inspect() const {
		std::ostringstream o;
		o << "<complex_bandpass_t:" << ((void*)this)
		  << " A_=" << MHAParser::StrCnv::val2str(A_)
		  << " B_=" << MHAParser::StrCnv::val2str(B_)
		  << " Yn=" << MHAParser::StrCnv::val2str(Yn) << ">";
		return o.str();
	}
    private:
	std::vector<mha_complex_t> A_;
	std::vector<mha_complex_t> B_;
	std::vector<mha_complex_t> Yn;
    };

    /**
       \brief Class for gammatone filter
    */
    class gamma_flt_t {
    public:
	/**
	   \brief Constructor

	   \param cf Center frequency in Hz.
	   \param bw Bandwidth in Hz (same number of entries as in cf).
	   \param srate Sampling frequency in Hz.
	   \param order Filter order.
	*/
	gamma_flt_t(std::vector<mha_real_t> cf,std::vector<mha_real_t> bw,mha_real_t srate,unsigned int order);
	~gamma_flt_t();
	/**
	   \brief Filter method.
	*/
	inline void operator()(mha_wave_t& X,mha_spec_t& Y) {
	    mha_wave_t* Xin = &X;
	    if( delay )
		Xin = delay->process(Xin);
	    GF[0].filter(*Xin,Y);
	    for(unsigned int ord=1;ord<GF.size();ord++)
		GF[ord].filter(Y,Y);
	};
	/**
	   \brief Filter method.
	*/
	inline void operator()(mha_wave_t& X,mha_wave_t& Yre,mha_wave_t& Yim) {
	    mha_wave_t* Xin = &X;
	    if( delay )
		Xin = delay->process(Xin);
	    GF[0].filter(*Xin,Yre,Yim);
	    for(unsigned int ord=1;ord<GF.size();ord++)
		GF[ord].filter(Yre,Yim,Yre,Yim);
	};
	/**
	   \brief Filter method for specific stage.
	*/
	inline void operator()(mha_wave_t& Yre,mha_wave_t& Yim, unsigned int stage) {
	    if( stage >= GF.size() )
		return;
	    if( stage == 0 ){
		mha_wave_t* Xin = &Yre;
		if( delay )
		    Xin = delay->process(Xin);
		GF[0].filter(*Xin,Yre,Yim);
	    }else{
		GF[stage].filter(Yre,Yim,Yre,Yim);
	    }
	};
	void phase_correction(unsigned int desired_delay,unsigned int inchannels);
	void set_weights(std::vector<mha_complex_t> new_B);
	void set_weights(unsigned int stage,std::vector<mha_complex_t> new_B);
	std::vector<mha_complex_t> get_weights() const { return GF[0].get_weights(); };
	std::vector<mha_complex_t> get_weights(unsigned int stage) const { return GF[stage].get_weights(); };
	std::vector<mha_real_t> get_resynthesis_gain() const { return resynthesis_gain; };
	void reset_state();
	const std::vector<mha_complex_t> & get_A() {return A;}
	std::string inspect() const {
		std::ostringstream o;
		o << "<gamma_flt_t:" << ((void*)this)
		  << "A=" << MHAParser::StrCnv::val2str(A)
		  << "GF=[";
		for (unsigned i = 0; i < GF.size(); ++i) {
			o << GF[i].inspect();
			if ((i+1) < GF.size()) {
				o << ",";
			}
		}
		o << "] delay=" << (delay?(delay->inspect()):0)
		  << " envelope_delay=" << MHAParser::StrCnv::val2str(envelope_delay)
		  << " resynthesis_gain=" << MHAParser::StrCnv::val2str(resynthesis_gain)
		  << " cf=" << MHAParser::StrCnv::val2str(cf_)
		  << " bw=" << MHAParser::StrCnv::val2str(bw_)
		  << " srate_=" << MHAParser::StrCnv::val2str(srate_) << ">";
		return o.str();
	}
    private:
	std::vector<mha_complex_t> A;
	std::vector<complex_bandpass_t> GF;
	MHASignal::delay_t* delay;
	std::vector<int> envelope_delay;
	std::vector<mha_real_t> resynthesis_gain;
	std::vector<mha_real_t> cf_;
	std::vector<mha_real_t> bw_;
	mha_real_t srate_;
    };

    class thirdoctave_analyzer_t {
    public:
	thirdoctave_analyzer_t(mhaconfig_t cfg);
	mha_wave_t* process(mha_wave_t*);
	unsigned int nbands();
	unsigned int nchannels();
	std::vector<mha_real_t> get_cf_hz();
	static std::vector<mha_real_t> cf_generator(mhaconfig_t cfg);
	static std::vector<mha_real_t> bw_generator(mhaconfig_t cfg);
	static std::vector<mha_real_t> dup(std::vector<mha_real_t>,mhaconfig_t cfg);
    private:
	mhaconfig_t cfg_;
	std::vector<mha_real_t> cf;
	MHAFilter::gamma_flt_t fb;
	MHASignal::waveform_t out_chunk;
	MHASignal::waveform_t out_chunk_im;
    };

}

#endif

// Local Variables:
// mode: c++
// c-basic-offset: 4
// compile-command: "make -C .."
// coding: utf-8-unix
// End:
