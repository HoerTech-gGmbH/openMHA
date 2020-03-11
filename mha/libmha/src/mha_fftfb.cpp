// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2008 2009 2011 2013 2015 HörTech gGmbH
// Copyright © 2016 2017 2018 2019 2020 HörTech gGmbH
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

#include "mha_fftfb.hh"
#include "mha_error.hh"
#include <math.h>
#include <fstream>
#include <iostream>
#include "mha_defs.h"
#include "mha_tablelookup.hh"
#include <limits>
#include "speechnoise.h"


/*
  \class MHAOvlFilter::fftfb_t
  \brief Base class for FFT based filterbank plugins

  This class is base for plugins which use FFT based filterbank input,
  e.g. \ref adhocfilterbank "fftfilterbank".

  Configurable variables:

  \li <b>fscale</b>: frequency scale on which the filter crossings are symmetric [linear bark log]
  \li <b>ovltype</b>: filter overlap type [rectangular linear hanning]
  \li <b>plateau</b>: relative plateau width
  \li <b>cf</b>: center frequencies in Hz
  \li <b>snap_to_bin</b>: round center frequencies to FFT bin resolution
  \li <b>save_shapes</b>: save filter shapes to AC variable

  The filter bandwidth is computed from the
  distance to neighbour bands. The filter shapes can be rectangular,
  sawtooth or hanning windows, on different frequency scales (linear
  scale, bark scale, logarithmic scale). With rectangular shapes, the
  filter edges are on the mean frequency between two center frequencies,
  calculated on the given scale. To reach symmetric filters, the center
  frequencies must have equal distances on the given scale. See
  \ref filtershapefun and \ref freqscalefun for details.

  The band power \f$ P(k) \f$ in band \f$ k \f$ is

  \f[
  P(k) = \int\limits |W(f)\cdot X(f)|^2 {\rm d}f,
  \f]

  with a window (or filter shape) function \f$ W(f) \f$ and the
  complex input spectrum \f$ X(f) \f$.
    
  The function get_fbpower() calculates the filter band power and stores
  it to the protected member variable fbpow. If the funtion
  apply_gains() is called, the gains stored in the protected member
  variable gains are applied to the signal, according to the following
  equation (\f$ k \f$ is the filterbank channel index):

  \f[
  Y(f) = \sum\limits_{k} G(k)\cdot W(f) \cdot X(f)
  \f]


*/

#define BARKSCALE_ENTRIES 50

namespace MHAOvlFilter {

    namespace barkscale {

        extern mha_real_t vfreq[BARKSCALE_ENTRIES];

        extern mha_real_t vbark[BARKSCALE_ENTRIES];

        class hz2bark_t:public MHATableLookup::xy_table_t {
        public:
            hz2bark_t();
            ~hz2bark_t();
        };

        class bark2hz_t:public MHATableLookup::xy_table_t {
        public:
            bark2hz_t();
            ~bark2hz_t();
        };
    }

    /**
        \brief Transform functions from linear scale in Hz to new frequency scales.
    */ namespace FreqScaleFun {

        /**
            \brief Dummy scale transformation Hz to Hz.

            This function implements a dummy scale transformation (linear
            frequency scale).

            \param x      Input frequency in Hz
            \return Frequency in Hz
        */

        mha_real_t hz2hz(mha_real_t x);

        mha_real_t hz2khz(mha_real_t x);
        mha_real_t hz2octave(mha_real_t x);
        mha_real_t hz2third_octave(mha_real_t x);
        
        /**

        \brief Transformation to bark scale.

        This function implements a critical band rate (bark) scale.

        \param x      Input frequency in Hz
        \return Critical band rate in Bark
        */

        mha_real_t hz2bark(mha_real_t x);

        mha_real_t hz2bark_analytic(mha_real_t);

        mha_real_t hz2erb(mha_real_t);

        mha_real_t hz2erb_glasberg1990(mha_real_t);

        /**

        \brief Third octave frequency scale.

        This function implements a third octave scale. Frequencies
        below 16 Hz are mapped to 16 Hz.

        \param x      Frequency in Hz
        \return Third octaves relative to 1000 Hz
        */

        mha_real_t hz2log(mha_real_t x);

        mha_real_t inv_scale(mha_real_t, mha_real_t (*)(mha_real_t));


    }

    /**

    \brief Shape functions for overlapping filters.
    */ namespace ShapeFun {

        /**
            
        \brief Filter shape function for rectangular filters.
          
        This function creates rectangular filter shapes. The edge is
        exactly half way between two center frequencies (on a given
        scale).
        
        \param x      Input value in the range [-1,1].
        \return Weigth function in the range [0,1]
        */

        mha_real_t rect(mha_real_t x);

        /**
           
        \brief Filter shape function for sawtooth filters.

        This function creates sawtooth filter shapes. They rise
        linearly form 0 to 1 in the interval from the lower neighbor
        center frequency to the band center frequency and from 1 to 0
        in the interval from the band center frequency to the upper
        neighbour band center frequency. Linear means linear on a
        given frequency scale.

        \param x      Input value in the range [-1,1].
        \return Weigth function in the range [0,1]
        */

        mha_real_t linear(mha_real_t x);

        /**

        \brief Filter shape function for hanning shaped filters.

        This function creates hanning window shaped filters.

        \param x      Input value in the range [-1,1].
        \return Weigth function in the range [0,1]
        */

        mha_real_t hann(mha_real_t x);

        mha_real_t expflt(mha_real_t);
                
        mha_real_t gauss(mha_real_t);

        
    }

}

/** Transform the test frequency into the relative position on the filter
 * flank of the given frequency band.
 * @param f Test frequency in units corresponding to the chosen frequency scale
 * @param b Descriptor of a single filter bank band: E.g. contains center
 *          frequencies of this and the two adjacent bands, and the
 *          crossover ("edge") frequencies of this band.
 * @param plateau For non-rectangular filter shapes, specifies what
 *                frequency portion of the band around its center frequency
 *                should have no attenuation applied.
 * @pre 0 <= plateau <= 1
 * @return The position of frequency f on the filter flank as follows:
 *         A returned position of 0 means that f is equal to the band's
 *         center frequency, or should be treated the same as the center
 *         frequency (i.e. is within the band's plateau).  A returned
 *         position of -1 means that f is <= the lowest frequency
 *         of the filter flank (or is an even lower frequency). A returned
 *         value of -0.5 means that f is equal to the lower edge frequency.
 *         Positive returned values have equivalent meanings for the high
 *         half of the filter flank.
 */
mha_real_t filtershapefun(mha_real_t f,
                          MHAOvlFilter::band_descriptor_t b,
                          mha_real_t plateau)
{
    // lower cut-off frequency:
    // If we have a plateau width of 0, then the lower cut-off frequency
    // coincides with the center frequency of the lower frequency adjacent band,
    // i.e. the filter flank would extend from that band's center frequency to
    // this band's center frequency.
    // If we have a plateau of width 1, then the filter flank would
    // be narrowed to just the edge frequency, and we would effectively have
    // a rectangular filter shape regardless of the selected filter shape.
    // Plateau widths between 0 and 1 would see an increasingly narrower filter
    // flank.
    mha_real_t f_co_l = (1-plateau)*b.cf_l + plateau*b.ef_l;
    // upper cut-off frequency:
    mha_real_t f_co_h = (1-plateau)*b.cf_h + plateau*b.ef_h;
    // lower plateau frequency:
    mha_real_t f_pl_l = (1-plateau)*b.cf + plateau*b.ef_l;
    // upper plateau frequency:
    mha_real_t f_pl_h = (1-plateau)*b.cf + plateau*b.ef_h;
    // get section:
    // belongs to plateau:
    if( b.low_side_flat && (f <= b.cf) )
        return 0;
    if( b.high_side_flat && (f >= b.cf) )
        return 0;
    // belongs to previous band:
    if(f <= f_co_l)
        return -1;
    // belongs to next band:
    if(f >= f_co_h)
        return 1;
    // belongs to plateau:
    if((f_pl_l <= f) && (f <= f_pl_h))
        return 0;
    // all corner points (except edge frequencies) were included by
    // now, so the next comparisons do not include equality, except
    // for edge frequencies:
    if((f_co_l < f) && (f < b.ef_l))
        return 0.5*(f - f_co_l)/(b.ef_l - f_co_l) - 1.0;
    if((b.ef_l <= f) && (f < f_pl_l))
        return 0.5*(f - b.ef_l)/(f_pl_l-b.ef_l) - 0.5;
    if((f_pl_h < f) && (f <= b.ef_h))
        return 0.5*(f - f_pl_h)/(b.ef_h - f_pl_h);
    if((b.ef_h < f) && (f < f_co_h))
        return 0.5*(f - b.ef_h)/(f_co_h - b.ef_h) + 0.5;
    throw MHA_ErrorMsg("Fatal error in implementation!");
}

/*************************************************************
 *                                                           *
 * Skalentransformation und Fensterfunktionen:               *
 *                                                           *
 *************************************************************/

/*
 * Dokumentation von FreqScaleFun und ShapeFun im header
 * fft2f.hh.
 */

mha_real_t MHAOvlFilter::FreqScaleFun::hz2hz(mha_real_t x)
{
    return x;
}

mha_real_t MHAOvlFilter::FreqScaleFun::hz2khz(mha_real_t x)
{
    return 0.001*x;
}

mha_real_t MHAOvlFilter::FreqScaleFun::hz2octave(mha_real_t x)
{
    return log2f(x/1000.0);
}

mha_real_t MHAOvlFilter::FreqScaleFun::hz2third_octave(mha_real_t x)
{
    return 3*log2f(x/1000.0);
}


mha_real_t MHAOvlFilter::FreqScaleFun::hz2bark(mha_real_t x)
{
    barkscale::hz2bark_t h2b;
    return h2b.interp(x);
}

mha_real_t MHAOvlFilter::FreqScaleFun::hz2bark_analytic(mha_real_t x)
{
    return 13.0 * atan(0.00076 * x) + 3.5 * atan((x / 7500.0) * (x / 7500));
}

mha_real_t MHAOvlFilter::FreqScaleFun::hz2erb(mha_real_t x)
{
    return 21.36554*log10(0.004368*x+1.0);
    //return 9.265*log(0.0043698*x+1.0);
}

mha_real_t MHAOvlFilter::FreqScaleFun::hz2erb_glasberg1990(mha_real_t x)
{
    return 9.265*log(0.0043698*x+1.0);
}

mha_real_t MHAOvlFilter::FreqScaleFun::hz2log(mha_real_t x)
{
    if(x > 16.0)
        return 3.0 * log(x / 1000.0) / log(2.0);
    return 3.0 * log(16.0 / 1000.0) / log(2.0);
}

mha_real_t MHAOvlFilter::FreqScaleFun::inv_scale(mha_real_t y, mha_real_t (*fun)(mha_real_t))
{
    mha_real_t x = 1;
    mha_real_t dx = 100;
    unsigned int maxit = 100000;
    mha_real_t sig = 0;
    mha_real_t yn;
    mha_real_t eps = 1e-8;
    // identity check:
    if( y == fun(y) )
        return y;
    yn = fun(x);
    if(yn == y)
        return x;
    if( (y>0) && (x == yn) )
        x = y;
    for(unsigned int k=0;k<maxit;k++){
        yn = fun(x);
        if(yn == y)
            return x;
        if(yn < y){
            x += dx;
            if(sig > 0)
                dx /= 2;
            sig = -1;
        }else{
            x -= dx;
            if( x < eps ){
                x = eps;
                dx /= 2;
            }
            if(sig < 0)
                dx /= 2;
            sig = 1;
        }
    }
    return x;
}


mha_real_t MHAOvlFilter::ShapeFun::rect(mha_real_t x)
{
    if( (x >= -0.5) && (x < 0.5) )
        return 1;
    return 0;
}

mha_real_t MHAOvlFilter::ShapeFun::linear(mha_real_t x)
{
    if(fabs(x) > 1.0)
        return 0;
    return 1.0 - fabs(x);
}

mha_real_t MHAOvlFilter::ShapeFun::hann(mha_real_t x)
{
    if(fabs(x) > 1.0)
        return 0;
    return 0.5 + 0.5 * cos(M_PI * x);
}

mha_real_t MHAOvlFilter::ShapeFun::expflt(mha_real_t x)
{
    return exp(-fabs(2.0 * x) * log(2.0));
}

mha_real_t MHAOvlFilter::ShapeFun::gauss(mha_real_t x)
{
    return exp(-4.0 * x * x * log(2.0));
}

mha_real_t MHAOvlFilter::barkscale::vfreq[BARKSCALE_ENTRIES] = {
    0, 50, 100, 150, 200, 250, 300, 350, 400, 450, 510,
    570, 630, 700, 770, 840, 920, 1000, 1080, 1170, 1270, 1370, 1480,
    1600, 1720, 1850, 2000, 2150, 2320, 2500, 2700, 2900, 3150, 3400,
    3700, 4000, 4400, 4800, 5300, 5800, 6400, 7000, 7700, 8500, 9500,
    10500, 12000, 13500, 15500, 96000
};

mha_real_t MHAOvlFilter::barkscale::vbark[BARKSCALE_ENTRIES] = {
    0, .5, 1, 1.5, 2, 2.5, 3, 3.5, 4, 4.5, 5, 5.5, 6, 6.5,
    7, 7.5, 8, 8.5, 9, 9.5, 10, 10.5, 11, 11.5, 12, 12.5, 13, 13.5, 14,
    14.5, 15, 15.5, 16, 16.5, 17, 17.5, 18, 18.5, 19, 19.5, 20, 20.5, 21,
    21.5, 22, 22.5, 23, 23.5, 24, 30
};

MHAOvlFilter::barkscale::hz2bark_t::hz2bark_t()
    :  MHATableLookup::xy_table_t()
{
    add_entry(vfreq,vbark,BARKSCALE_ENTRIES);
}

MHAOvlFilter::barkscale::bark2hz_t::bark2hz_t()
    :MHATableLookup::xy_table_t()
{
    add_entry(vbark,vfreq,BARKSCALE_ENTRIES);
}

MHAOvlFilter::barkscale::bark2hz_t::~bark2hz_t()
{
}

MHAOvlFilter::barkscale::hz2bark_t::~hz2bark_t()
{
}

#include "mha_error.hh"

MHAOvlFilter::fftfb_t::~fftfb_t()
{
    delete[]vbin1;
    delete[]vbin2;
}

MHAOvlFilter::fftfb_t::fftfb_t(MHAOvlFilter::fftfb_vars_t& par, unsigned int nfft, mha_real_t fs)
    :MHAOvlFilter::fspacing_t(par, nfft, fs),
     MHASignal::waveform_t((unsigned int)(nfft / 2) + 1,nbands()),
     shape(par.ovltype.get_fun()),
     fftlen(nfft),
     samplingrate(fs)
{
    unsigned int ch, fr;
    if(num_channels){
        for(ch = 0; ch < num_channels; ch++) {// ch iterates over frequency-bands
            bool flag_allzero{true};
            for(fr = 0; fr < num_frames; fr++){// fr iterates over stft-bins
                mha_real_t f{symmetry_scale((mha_real_t) fr *fs /
                                            (mha_real_t) mha_min_1(nfft))};
                value(fr, ch) = shape(filtershapefun(f, bands[ch], par.plateau.data));
                if(value(fr,ch) != 0)
                    flag_allzero = false;
            }
            if(flag_allzero && !par.flag_allow_empty_bands.data){
                throw MHA_Error(__FILE__, __LINE__,
                          "The current fftfilterbank settings cause the STFT-bin-specific "
                          "gain factors that shape frequency band %u to be all zeros!\n"
                          "Set the variable 'flag_allow_empty_bands' to 'yes' if you want "
                          "to allow this behaviour.", ch);
            }
        }
        if( par.normalize.data ){
            *this *= (double)nfft/(2.0*sum());
        }
    }
    vbin1 = new unsigned int[nbands()];
    vbin2 = new unsigned int[nbands()];
    for(unsigned int kband = 0; kband < nbands(); kband++){
        vbin1[kband] = 0;
        vbin2[kband] = num_frames;
        if( num_frames > 1 ){
            while((vbin2[kband] > 0) && (value(vbin2[kband] - 1, kband) == 0))
                vbin2[kband]--;
            if( vbin2[kband] > 0 )
                while((vbin1[kband] < vbin2[kband] - 1) && (value(vbin1[kband], kband) == 0))
                    vbin1[kband]++;
        }
    }
    par.cf.data = get_cf_hz();
    par.ef.data = get_ef_hz();
    par.cLTASS.data = get_ltass_gain_db();
    par.shapes.data.clear();
    for(ch = 0; ch < num_channels; ch++){
        std::vector<mha_real_t> svw;
        for(fr = 0; fr < num_frames; fr++ )
            svw.push_back(value(fr,ch));
        par.shapes.data.push_back(svw);
    }
}
//**************************************************************
//
// Frequency spacing
//
//**************************************************************


void MHAOvlFilter::fspacing_t::ef2bands(std::vector<mha_real_t> vef)
{
    bands.clear();
    // edge frequency vector needs at least lower and upper frequency:
    if(vef.size() < 2)
        throw MHA_ErrorMsg("Invalid number of entries in edge frequency vector.");
    bands.resize(vef.size()-1);
    unsigned int k;
    for(k=0;k<bands.size();k++){
        bands[k].ef_l = symmetry_scale(vef[k]);
        if    (vef[k] == 0 &&
               bands[k].ef_l == -std::numeric_limits<mha_real_t>::infinity()) {
            throw MHA_ErrorMsg("In edge frequency mode, an edge frequency "
                               "of 0 Hz translates to -Inf on the "
                               "logarithmic scales, which prevents "
                               "reasonable filter shape computation.  "
                               "Choose either non-zero edge frequencies, "
                               "or a non-logarithmic frequency scale, or "
                               "center frequency mode for the filterbank.");
        }
        bands[k].ef_h = symmetry_scale(vef[k+1]);
        bands[k].cf = 0.5*(bands[k].ef_l + bands[k].ef_h);
        bands[k].low_side_flat = false;
        bands[k].high_side_flat = false;
    }
    // copy lower center frequencies
    for(k=1;k<bands.size();k++)
        bands[k].cf_l = bands[k-1].cf;
    // copy higher center frequencies
    for(k=0;k<bands.size()-1;k++)
        bands[k].cf_h = bands[k+1].cf;
    // special case in outmost bands:
    // virtual next band has same bandwith as this one:
    bands[0].cf_l = 2*bands[0].ef_l - bands[0].cf;
    bands[bands.size()-1].cf_h = 2*bands[bands.size()-1].ef_h - bands[bands.size()-1].cf;
}

void MHAOvlFilter::fspacing_t::cf2bands(std::vector<mha_real_t> vcf)
{
    bands.clear();
    // edge frequency vector needs at least lower and upper frequency:
    if(vcf.size() < 1)
        throw MHA_ErrorMsg("Invalid number of entries in center frequency vector.");
    bands.resize(vcf.size());
    unsigned int k;
    // copy center frequencies:
    for(k=0;k<bands.size();k++){
        bands[k].cf = symmetry_scale(vcf[k]);
        bands[k].low_side_flat = false;
        bands[k].high_side_flat = false;
    }
    // copy lower center frequencies and calculate edge freqs:
    for(k=1;k<bands.size();k++){
        bands[k].cf_l = bands[k-1].cf;
        bands[k].ef_l = 0.5*(bands[k].cf + bands[k].cf_l);
    }
    // copy higher center frequencies and calculate edge freqs:
    for(k=0;k<bands.size()-1;k++){
        bands[k].cf_h = bands[k+1].cf;
        bands[k].ef_h = 0.5*(bands[k].cf + bands[k].cf_h);
    }
    // special case in outmost bands:
    // (approximatly) flat response:
    const mha_real_t pm_inf = std::numeric_limits<mha_real_t>::max();
    bands[0].cf_l = bands[0].ef_l = -pm_inf;
    bands[bands.size()-1].cf_h = bands[bands.size()-1].ef_h = pm_inf;
    bands[0].low_side_flat = true;
    bands[bands.size()-1].high_side_flat = true;
}

void MHAOvlFilter::fspacing_t::fail_on_unique_fftbins()
{
    std::vector<unsigned int> fftbins = get_cf_fftbin();
    std::vector<mha_real_t> cf_hz = get_cf_hz();
    for(unsigned int k=1;k<fftbins.size();k++){
        if( fftbins[k] == fftbins[k-1] )
            throw MHA_Error(__FILE__,__LINE__,
                            "Band %u and %u share the same FFT bin %u (center frequencies: %g Hz and %g Hz).",
                            k-1,k,fftbins[k],cf_hz[k-1],cf_hz[k]);
    }
}

void MHAOvlFilter::fspacing_t::fail_on_nonmonotonic_cf()
{
    std::vector<mha_real_t> cfnative = get_cf_hz();
    for(unsigned int k=1;k<cfnative.size();k++){
        if( cfnative[k] <= cfnative[k-1] )
            throw MHA_Error(__FILE__,__LINE__,
                            "Non-monotonic center frequencies in band %u and %u: %g Hz and %g Hz.",
                            k-1,k,cfnative[k-1],cfnative[k]);
    }
}

std::vector<unsigned int> MHAOvlFilter::fspacing_t::get_cf_fftbin() const
{
    std::vector<mha_real_t> cfhz = get_cf_hz();
    std::vector<unsigned int> retv;
    for(unsigned int k=0;k<cfhz.size();k++ )
        retv.push_back(mha_round(cfhz[k]/fs_*nfft_));
    return retv;
}

std::vector<mha_real_t> MHAOvlFilter::fspacing_t::get_cf_hz() const
{
    std::vector<mha_real_t> retv;
    for(unsigned int k=0;k<bands.size();k++ )
        retv.push_back(MHAOvlFilter::FreqScaleFun::inv_scale(bands[k].cf,symmetry_scale));
    return retv;
}

std::vector<mha_real_t> MHAOvlFilter::fspacing_t::get_ef_hz() const
{
    std::vector<mha_real_t> retv;
    if( nbands() ){
        for(unsigned int k=0;k<bands.size();k++ )
            retv.push_back(MHAOvlFilter::FreqScaleFun::inv_scale(bands[k].ef_l,symmetry_scale));
        retv.push_back(MHAOvlFilter::FreqScaleFun::inv_scale(bands[bands.size()-1].ef_h,symmetry_scale));
    }
    return retv;
}

MHAOvlFilter::fspacing_t::fspacing_t(const MHAOvlFilter::fftfb_vars_t& par, unsigned int nfft, mha_real_t fs)
    : symmetry_scale(par.fscale.get_fun()),
      nfft_(nfft),
      fs_(fs)
{
    if(nfft == 0)
        throw MHA_Error(__FILE__,__LINE__,"Invalid FFT length (zero).");
    if(fs == 0)
        throw MHA_Error(__FILE__,__LINE__,"Invalid sampling rate (zero).");
    std::vector<mha_real_t> vF = par.f.get_f_hz();
    if( !par.fail_on_nonmonotonic.data ){
        sort(vF.begin(),vF.end());
        std::vector<mha_real_t>::iterator p = unique(vF.begin(),vF.end());
        vF.erase(p,vF.end());
    }
    // creator functions:
    if(par.ftype.isval("edge"))
        // edges:
        ef2bands(vF);
    else if(par.ftype.isval("center"))
        // center:
        cf2bands(vF);
    else{
        std::string model = par.ftype.data.get_value();
        throw MHA_Error(__FILE__,__LINE__,
                        "Programming error: Unsupported frequency spacing model \"%s\" (%zu).",
                        model.c_str(),par.ftype.data.get_index());
    }
    if( par.fail_on_nonmonotonic.data )
        fail_on_nonmonotonic_cf();
    if( par.fail_on_unique_bins.data )
        fail_on_unique_fftbins();
    if( !nbands() )
        throw MHA_Error(__FILE__,__LINE__,"Invalid number of bands (zero).");
}

MHAOvlFilter::fftfb_vars_t::fftfb_vars_t(MHAParser::parser_t & p)
    :  fscale("frequency scale of filter bank"),//, "linear", "[linear bark log erb]"),
       ovltype("filter overlap type"),//, "rect", "[rect linear hanning exp gauss]"),
       plateau("relative plateau width", "0", "[0,1["),
       ftype("frequency entry type", "center", "[center edge]"),
       f(p),
       normalize("normalize broadband output amplitude","no"),
       fail_on_nonmonotonic("Fail if frequency entries are non-monotonic (otherwise sort)","yes"),
       fail_on_unique_bins("Fail if center frequencies share the same FFT bin.","yes"),
       flag_allow_empty_bands("Set true to allow bands where all STFT-bin-gains equal zero.","no"),
       cf("final center frequencies in Hz"),
       ef("final edge frequencies in Hz"),
       cLTASS("Bandwidth level correction for LTASS noise in dB"),
       shapes("Frequency band shapes")
{
    fscale.add_fun("linear",MHAOvlFilter::FreqScaleFun::hz2hz);
    fscale.add_fun("bark",MHAOvlFilter::FreqScaleFun::hz2bark_analytic);
    fscale.add_fun("log",MHAOvlFilter::FreqScaleFun::hz2octave);
    fscale.add_fun("erb",MHAOvlFilter::FreqScaleFun::hz2erb);
    fscale.add_fun("ERB_Glasberg1990",MHAOvlFilter::FreqScaleFun::hz2erb_glasberg1990);
    ovltype.add_fun("rect",MHAOvlFilter::ShapeFun::rect);
    ovltype.add_fun("linear",MHAOvlFilter::ShapeFun::linear);
    ovltype.add_fun("hanning",MHAOvlFilter::ShapeFun::hann);
    ovltype.add_fun("exp",MHAOvlFilter::ShapeFun::expflt);
    ovltype.add_fun("gauss",MHAOvlFilter::ShapeFun::gauss);
    // variables:
    p.insert_member(fscale);
    p.insert_member(ovltype);
    p.insert_member(plateau);
    p.insert_member(ftype);
    p.insert_member(normalize);
    p.insert_member(fail_on_nonmonotonic);
    p.insert_member(fail_on_unique_bins);
    p.insert_member(flag_allow_empty_bands);
    // monitors:
    p.insert_member(cf);
    p.insert_member(ef);
    p.insert_member(cLTASS);
    p.insert_member(shapes);
}

void MHAOvlFilter::fftfb_t::apply_gains(mha_spec_t * s_out, const mha_spec_t * s_in, const mha_wave_t * gains)
{
    if(!s_out)
        throw MHA_ErrorMsg("Invalid output signal data.");
    if(!s_in)
        throw MHA_ErrorMsg("Invalid input signal data.");
    if(!gains)
        throw MHA_ErrorMsg("Invalid gain vector data.");
    if(s_in->num_frames != num_frames)
        throw MHA_Error(__FILE__, __LINE__, "Input signal has %u bins, weights have %u.", s_in->num_frames, num_frames);
    if(s_out->num_frames != num_frames)
        throw MHA_Error(__FILE__, __LINE__, "Output signal has %u bins, weights have %u.", s_out->num_frames, num_frames);
    if(s_in->num_channels != s_out->num_channels)
        throw MHA_Error(__FILE__, __LINE__,
                        "Input signal has %u channels, output signal has %u.", s_in->num_channels, s_out->num_channels);
    if(s_in->num_channels != gains->num_channels)
        throw MHA_Error(__FILE__, __LINE__,
                        "Input signal has %u channels, gain vector has %u.", s_in->num_channels, gains->num_channels);
    if(gains->num_frames != num_channels)
        throw MHA_Error(__FILE__, __LINE__, "Gain vector has %u bands, filterbank has %u.", gains->num_frames, num_channels);
    unsigned int fr, ch, fb;
    mha_complex_t vIn, vOut;
    mha_real_t gain;
    for(fr = 0; fr < num_frames; fr++){
        for(ch = 0; ch < s_in->num_channels; ch++){
            vIn =::value(s_in, fr, ch);
            vOut.re = 0.0f;
            vOut.im = 0.0f;
            for(fb = 0; fb < num_channels; fb++){
                gain = ::value(gains,fb,ch);
                gain *= value(fr, fb);
                vOut.re += gain * vIn.re;
                vOut.im += gain * vIn.im;
            }
            ::value(s_out, fr, ch) = vOut;
        }
    }
}


void MHAOvlFilter::fftfb_t::get_fbpower(mha_wave_t * fbpow, const mha_spec_t * s_in)
{
    if(!fbpow)
        throw MHA_ErrorMsg("Invalid filter bank power data.");

    if(!s_in)
        throw MHA_ErrorMsg("Invalid input signal data.");

    if(fbpow->num_frames != num_channels)
        throw MHA_Error(__FILE__, __LINE__,
                        "The filterbank power vector has %u entries (bands), but the filterbank has %u bands.",
                        fbpow->num_frames, num_channels);
    if(s_in->num_channels != fbpow->num_channels)
        throw MHA_Error(__FILE__, __LINE__,
                        "The input signal has %u channels, but the filterbank power vector has %u.",
                        s_in->num_channels, fbpow->num_channels);
    if(s_in->num_frames != num_frames)
        throw MHA_Error(__FILE__, __LINE__,
                        "The input signal has %u bins, but the weights data has %u.",
                        s_in->num_frames, num_frames);
    unsigned int fr, ch, fb;

    // No nyquist bin for odd fft lengths (one past last valid index)
    const unsigned nyquist_index = (fftlen/2U) + (fftlen & 1U);
    for(fb = 0; fb < num_channels; fb++){
        for(ch = 0; ch < s_in->num_channels; ch++){
            ::value(fbpow, fb, ch) = 0;
            for(fr = bin1(fb); fr < bin2(fb); fr++){
                float factor = 2; // account for negative frequencies
                if (fr == 0 || fr == (nyquist_index)) {
                    factor = 1; // no negative frequency for 0 and Nyquist
                }
                ::value(fbpow, fb, ch) += factor *
                    value(fr,fb) * value(fr,fb) * abs2(::value(s_in, fr, ch));
            }
        }
    }
}

void MHAOvlFilter::fftfb_t::get_fbpower_db(mha_wave_t * fbpow, const mha_spec_t * s_in)
{
    get_fbpower(fbpow, s_in);
    for(unsigned int k = 0; k < fbpow->num_channels * fbpow->num_frames; k++)
        fbpow->buf[k] = 10.0 * log10(fbpow->buf[k] / 4.0e-10);
}

std::vector<float> MHAOvlFilter::fftfb_t::get_ltass_gain_db() const
{
    std::vector<float> retv;
    retv.resize(nbands());
    if( nbands() == 0 )
        return retv;
    if( fftlen == 0 )
        return retv;
    speechnoise_t ltass(fftlen,samplingrate,nbands(),speechnoise_t::LTASS_combined);
    ltass *= 2e-5f/MHASignal::rmslevel(ltass,0);
    
    MHASignal::spectrum_t ltass_spec(fftlen/2+1,nbands());
    mha_fft_t fft = mha_fft_new( fftlen );
    mha_fft_wave2spec( fft, &ltass, &ltass_spec );
    mha_fft_free( fft );
    ltass_spec *= *this;
    for( unsigned int k=0; k<nbands(); k++ ){
        retv[k] = 20.0*log10(50000.0*std::max(1.0e-20f,MHASignal::rmslevel(ltass_spec,k,fftlen)));
    }
    return retv;
}

//**************************************************************************
//
// Overlap-Save filter bank
//
//**************************************************************************

MHAOvlFilter::overlap_save_filterbank_t::vars_t::vars_t(MHAParser::parser_t & p)
    : MHAOvlFilter::fftfb_vars_t(p),
      fftlen("FFT length of filterbank (affects time domain only)","128","[2,]"),
      phasemodel("Phase model (affects time domain only)","linear","[minimal linear]"),
      irswnd("IRS window function (affects time domain only)")
{
    p.insert_member(fftlen);
    p.insert_member(phasemodel);
    p.insert_member(irswnd);
}
      

MHAOvlFilter::overlap_save_filterbank_t::overlap_save_filterbank_t(MHAOvlFilter::overlap_save_filterbank_t::vars_t& fbpar,
                                                                   mhaconfig_t channelconfig_in)
    : MHAOvlFilter::fftfb_t(fbpar,fbpar.fftlen.data,channelconfig_in.srate),
      MHAFilter::fftfilterbank_t(channelconfig_in.fragsize,channelconfig_in.channels,nbands(),fbpar.fftlen.data),
      channelconfig_out_(channelconfig_in)
{
    channelconfig_out_.channels *= nbands();
    unsigned int fftlen = (unsigned int)fbpar.fftlen.data;
    if( fftlen < channelconfig_in.fragsize )
        throw MHA_Error(__FILE__,__LINE__,
                        "Invalid FFT length (%u); must be at least fragsize (%u).",
                        fftlen,channelconfig_in.fragsize);
    unsigned int FIRLength = fftlen-channelconfig_in.fragsize+1;
    MHASignal::spectrum_t FilterWeights(fftlen/2+1,nbands());
    MHASignal::waveform_t* FIRCoeffs;
    unsigned int kBin,kBand;
    for(kBin=0;kBin<fftlen/2+1;kBin++)
        for(kBand=0;kBand<nbands();kBand++)
            FilterWeights(kBin,kBand) = mha_complex(w(kBin,kBand),0);
    if( fbpar.phasemodel.isval("minimal") ){
        FIRCoeffs = MHAFilter::spec2fir(&FilterWeights,fftlen,fbpar.irswnd.get_window(FIRLength,0.0f,1.0f),true);
    }else{
        FIRCoeffs = MHAFilter::spec2fir(&FilterWeights,fftlen,fbpar.irswnd.get_window(FIRLength),false);
    }
    update_coeffs(FIRCoeffs);
    delete FIRCoeffs;
}


//**************************************************************************
//
// AC FFTFB info
//
//**************************************************************************

MHAOvlFilter::fftfb_ac_info_t::fftfb_ac_info_t(const MHAOvlFilter::fftfb_t& fb,algo_comm_t ac,const std::string& prefix)
: cfv(ac,prefix+"_cf",fb.nbands(),1,false),
  efv(ac,prefix+"_ef",fb.nbands()+1,1,false),
  bwv(ac,prefix+"_band_weights",fb.nbands(),1,false),
  cLTASS(ac,prefix+"_cLTASS",fb.nbands(),1,false)
{
    cfv.copy(fb.get_cf_hz());
    efv.copy(fb.get_ef_hz());
    cLTASS.copy(fb.get_ltass_gain_db());
    unsigned int kfb, kfr;
    for(kfb=0;kfb<fb.nbands();kfb++) {
        bwv[kfb] = 0.0f;
        for (kfr=fb.bin1(kfb); kfr < fb.bin2(kfb); kfr++) {
            bwv[kfb] += fb.w(kfr,kfb) * fb.w(kfr,kfb);
        }
        bwv[kfb] /= (fb.get_fftlen()/2+1);
    }
}
        
void MHAOvlFilter::fftfb_ac_info_t::insert()
{
    cfv.insert();
    efv.insert();
    bwv.insert();
    cLTASS.insert();
}

MHAOvlFilter::scale_var_t::scale_var_t(const std::string& help)
    : kw_t(help,"none","[none]")
{
}

void MHAOvlFilter::scale_var_t::add_fun(const std::string& name, scale_fun_t* fun)
{
    names.push_back(name);
    funs.push_back(fun);
    set_range(MHAParser::StrCnv::val2str(names));
    data.set_index(0);
}

MHAOvlFilter::overlap_save_filterbank_analytic_t::overlap_save_filterbank_analytic_t(MHAOvlFilter::overlap_save_filterbank_t::vars_t &fbpar, mhaconfig_t channelconfig_in)
    : MHAOvlFilter::overlap_save_filterbank_t(fbpar,channelconfig_in),
      imagfb(channelconfig_in.fragsize,channelconfig_in.channels,nbands(),fbpar.fftlen.data)
{
    MHASignal::hilbert_t hilbert(fbpar.fftlen.data);
    MHASignal::waveform_t IRS_new(fbpar.fftlen.data,nbands());
    hilbert(get_irs(),&IRS_new);
    mha_wave_t IRS_new_cut = range(IRS_new,0,fbpar.fftlen.data+1-channelconfig_in.fragsize);
    imagfb.update_coeffs(&IRS_new_cut);
}

void MHAOvlFilter::overlap_save_filterbank_analytic_t::filter_analytic(const mha_wave_t* sIn,mha_wave_t** fltRe,mha_wave_t** fltIm)
{
    filter(sIn,fltRe);
    imagfb.filter(sIn,fltIm);
}

MHAOvlFilter::fscale_t::fscale_t(MHAParser::parser_t& p)
    : unit("Frequency unit"),
      f("Frequencies","[]"),
      f_hz("Frequencies in Hz"),
      updater(&f.writeaccess,this,&fscale_t::update_hz)
{
    unit.add_fun("Hz",MHAOvlFilter::FreqScaleFun::hz2hz);
    unit.add_fun("kHz",MHAOvlFilter::FreqScaleFun::hz2khz);
    unit.add_fun("Oct",MHAOvlFilter::FreqScaleFun::hz2octave);
    unit.add_fun("Oct/3",MHAOvlFilter::FreqScaleFun::hz2third_octave);
    unit.add_fun("Bark",MHAOvlFilter::FreqScaleFun::hz2bark_analytic);
    unit.add_fun("Erb",MHAOvlFilter::FreqScaleFun::hz2erb);
    unit.add_fun("ERB_Glasberg1990",MHAOvlFilter::FreqScaleFun::hz2erb_glasberg1990);
    p.insert_member(unit);
    p.insert_member(f);
    p.insert_member(f_hz);
}

mha_real_t MHAOvlFilter::scale_var_t::unit2hz(mha_real_t x) const
{ 
    return MHAOvlFilter::FreqScaleFun::inv_scale(x,get_fun());
}

mha_real_t MHAOvlFilter::scale_var_t::hz2unit(mha_real_t x) const
{ 
    scale_fun_t* fun = get_fun(); 
    return (*fun)(x);
}

std::vector<mha_real_t> MHAOvlFilter::fscale_t::get_f_hz() const
{
    std::vector<mha_real_t> vF;
    for(unsigned int k=0;k<f.data.size();k++)
        vF.push_back(unit.unit2hz(f.data[k]));
    return vF;
}

void MHAOvlFilter::fscale_t::update_hz()
{
    f_hz.data = get_f_hz();
}

MHAOvlFilter::fscale_bw_t::fscale_bw_t(MHAParser::parser_t& parent)
    : fscale_t(parent),
      bw("Bandwidth","[]","]0,]"),
      bw_hz("Bandwidth in Hz"),
      updater(&bw.writeaccess,this,&fscale_bw_t::update_hz)
{
    parent.insert_member(bw);
    parent.insert_member(bw_hz);
}

std::vector<mha_real_t> MHAOvlFilter::fscale_bw_t::get_bw_hz() const
{
    std::vector<mha_real_t> bw_;
    if( bw.data.size() == f.data.size() ){
        bw_ = bw.data;
    }else if( bw.data.size() == 1 ){
        for(unsigned int k=0;k<f.data.size();k++)
            bw_.push_back(bw.data[0]);
    }else{
        throw MHA_Error(__FILE__,__LINE__,
                        "Mismatching number of entries in f and bw vector (f: %zu, bw: %zu entries).",
                        f.data.size(),bw.data.size());
    }
    for(unsigned int k=0;k<bw_.size();k++){
        mha_real_t f_l = unit.unit2hz(f.data[k]-0.5*bw_[k]);
        mha_real_t f_h = unit.unit2hz(f.data[k]+0.5*bw_[k]);
        bw_[k] = f_h - f_l;
    }
    return bw_;
}

void MHAOvlFilter::fscale_bw_t::update_hz()
{
    bw_hz.data = get_bw_hz();
}

// Local Variables:
// compile-command: "make -C .."
// c-basic-offset: 4
// coding: utf-8-unix
// indent-tabs-mode: nil
// End:
