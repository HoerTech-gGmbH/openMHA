// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2007 2009 2010 2011 2013 2016 HörTech gGmbH
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

#include "speechnoise.h"
#include "mha_tablelookup.hh"

#define NUM_ENTR_MHAORIG 76
#define NUM_ENTR_LTASS 25
#define NUM_ENTR_OLNOISE 49

float vMHAOrigSpec[NUM_ENTR_MHAORIG] = {-1.473, 0, -4.939, -10.14, -13.94, -14.83, -14.27, -15.66, -16.16, -18.22, -20.5, -21.23, -22.13, -22.58, -23.98, -26.58, -26.4, -25.15, -23.89, -25.54, -27, -30.15, -31.68, -30.14, -27.55, -25.79, -25.89, -26.11, -27.48, -30.37, -33.13, -36.23, -36.64, -36.35, -35.03, -35.48, -36.35, -37.95, -40.53, -42.37, -41.29, -38.49, -36.32, -34.85, -34.05, -33.81, -33.48, -34.1, -35.19, -36.29, -36.94, -37.53, -38.71, -38.7, -38.92, -40.36, -41.26, -42.19, -43.65, -44.37, -43.95, -43.15, -42.57, -41.57, -41.86, -42.34, -42.87, -42.35, -42.71, -42.85, -43.47, -47.43, -67.54, -76.3, -77.43, -77.43};

float vMHAOrigFreq[NUM_ENTR_MHAORIG] = {172.266,344.532,516.797,689.063,861.329,1033.59,1205.86,1378.13,1550.39,1722.66,1894.92,2067.19,2239.46,2411.72,2583.99,2756.25,2928.52,3100.78,3273.05,3445.32,3617.58,3789.85,3962.11,4134.38,4306.64,4478.91,4651.18,4823.44,4995.71,5167.97,5340.24,5512.51,5684.77,5857.04,6029.3,6201.57,6373.83,6546.1,6718.37,6890.63,7062.9,7235.16,7407.43,7579.69,7751.96,7924.23,8096.49,8268.76,8441.02,8613.29,8785.56,8957.82,9130.09,9302.35,9474.62,9646.88,9819.15,9991.42,10163.7,10335.9,10508.2,10680.5,10852.7,11025,11197.3,11369.5,11541.8,11714.1,11886.3,12058.6,12230.9,12403.1,12575.4,12747.7,12919.9,13092.2};

float vLTASS_freq[NUM_ENTR_LTASS] = {63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300, 8000, 10000, 12500, 16000};

float vLTASS_combined_lev[NUM_ENTR_LTASS] = {38.6, 43.5, 54.4, 57.7, 56.8, 60.2, 60.3, 59.0, 62.1, 62.1, 60.5, 56.8, 53.7, 53.0, 52.0, 48.7, 48.1, 46.8, 45.6, 44.5, 44.3, 43.7, 43.4, 41.3, 40.7};

float vLTASS_female_lev[NUM_ENTR_LTASS] = {37.0,36.0,37.5,40.1,53.4,62.2,60.9,58.1,61.7,61.7,60.4,58,54.3,52.3,51.7,48.8,47.3,46.7,45.3,44.6,45.2,44.9,45.0,42.8,41.1};

float vLTASS_male_lev[NUM_ENTR_LTASS] = {38.6,43.5,54.4,57.7,56.8,58.2,59.7,60.0,62.4,62.6,60.6,55.7,53.1,53.7,52.3,48.7,48.9,47.0,46.0,44.4,43.3,42.4,41.9,39.8,40.4};

float vOlnoiseFreq[NUM_ENTR_OLNOISE] = {62.5,70.1539,78.7451,88.3884,99.2126,111.362,125,140.308,157.49,176.777,198.425,222.725,250,280.616,314.98,353.553,396.85,445.449,500,561.231,629.961,707.107,793.701,890.899,1000,1122.46,1259.92,1414.21,1587.4,1781.8,2000,2244.92,2519.84,2828.43,3174.8,3563.59,4000,4489.85,5039.68,5656.85,6349.6,7127.19,8000,8979.7,10079.4,11313.7,12699.2,14254.4,16000};

float vOlnoiseLev[NUM_ENTR_OLNOISE] = {45.9042,38.044,48.9444,61.3697,67.6953,69.7451,71.6201,71.2431,65.2754,63.2547,70.2264,72.1434,73.4433,73.2659,69.8424,71.0132,70.9577,70.3492,68.691,64.8436,64.0435,64.2879,60.5889,60.6596,60.3727,61.2003,61.8477,61.1478,61.2312,58.6584,57.2892,56.8299,56.0191,53.3018,56.0525,54.3592,50.8823,55.992,54.6768,47.2616,46.9914,45.209,50.413,47.5848,43.3215,43.754,38.5773,-0.39427,5.74224};

float fhz2bandno( float x )
{
    return log2(0.001*x);
}

float erb_hz_f_hz( float f_hz )
{
    return 0.108*f_hz + 24.7;
}

float hz2hz( float x )
{
    return x;
}

float bandw_correction( float f, float ldb )
{
    return 10.0*log10( 1.0f/f*pow(10.0,0.1*ldb) );
}

speechnoise_t::speechnoise_t(float duration, float srate, unsigned int channels, speechnoise_t::noise_type_t noise_type)
    : MHASignal::waveform_t((unsigned int)(duration*srate),channels)
{
    creator(noise_type,srate);
}

speechnoise_t::speechnoise_t(unsigned int length_samples, float srate, unsigned int channels, speechnoise_t::noise_type_t noise_type)
    : MHASignal::waveform_t(length_samples,channels)
{
    creator(noise_type,srate);
}

void speechnoise_t::creator(speechnoise_t::noise_type_t noise_type, float srate)
{
    MHASignal::spectrum_t temp_spec(num_frames/2+1,num_channels);
    MHATableLookup::xy_table_t interp;
    interp.set_xfun(fhz2bandno);
    float (*yfun)(float) = &MHASignal::db2lin;
    unsigned int k, ch;
    float freq;
    float extrap_slope(12.0f); // slope for extrapolation, dB/octave
    unsigned int kfbin(0);
    switch( noise_type ){
    case speechnoise_t::mha :
        yfun = NULL;
        interp.set_xfun(NULL);
        interp.set_yfun(MHASignal::db2lin);
        interp.add_entry(vMHAOrigFreq,vMHAOrigSpec,NUM_ENTR_MHAORIG);
        interp.add_entry(0.0f,-100.0f);
        break;
    case speechnoise_t::olnoise :
        interp.set_xyfun(bandw_correction);
        interp.add_entry(vOlnoiseFreq,vOlnoiseLev,NUM_ENTR_OLNOISE);
        interp.add_entry( 0.5*vOlnoiseFreq[0], vOlnoiseLev[0]-extrap_slope );
        interp.add_entry( 2.0*vOlnoiseFreq[NUM_ENTR_OLNOISE-1], vOlnoiseLev[NUM_ENTR_OLNOISE-1]-extrap_slope );
        break;
    case speechnoise_t::LTASS_combined :
        interp.set_xyfun(bandw_correction);
        interp.add_entry( vLTASS_freq, vLTASS_combined_lev, NUM_ENTR_LTASS );
        interp.add_entry( 0.5*vLTASS_freq[0], vLTASS_combined_lev[0]-extrap_slope );
        interp.add_entry( 2.0*vLTASS_freq[NUM_ENTR_LTASS-1], vLTASS_combined_lev[NUM_ENTR_LTASS-1]-extrap_slope );
        break;
    case speechnoise_t::LTASS_female :
        interp.set_xyfun(bandw_correction);
        interp.add_entry( vLTASS_freq, vLTASS_female_lev, NUM_ENTR_LTASS );
        interp.add_entry( 0.5*vLTASS_freq[0], vLTASS_female_lev[0]-extrap_slope );
        interp.add_entry( 2.0*vLTASS_freq[NUM_ENTR_LTASS-1], vLTASS_female_lev[NUM_ENTR_LTASS-1]-extrap_slope );
        break;
    case speechnoise_t::LTASS_male :
        interp.set_xyfun(bandw_correction);
        interp.add_entry( vLTASS_freq, vLTASS_male_lev, NUM_ENTR_LTASS );
        interp.add_entry( 0.5*vLTASS_freq[0], vLTASS_male_lev[0]-extrap_slope );
        interp.add_entry( 2.0*vLTASS_freq[NUM_ENTR_LTASS-1], vLTASS_male_lev[NUM_ENTR_LTASS-1]-extrap_slope );
        break;
    case speechnoise_t::white :
        interp.add_entry( 100.0f, 0.0f );
        interp.add_entry( 1000.0f, 0.0f );
        break;
    case speechnoise_t::pink :
        interp.add_entry(25.0f, 0.0f);
        interp.add_entry(50.0f, 0.0f);
        interp.add_entry(100.0f,-3.0f);
        break;
    case speechnoise_t::brown :
        interp.add_entry(25.0f, 0.0f);
        interp.add_entry(50.0f, 0.0f);
        interp.add_entry(100.0f,-6.0f);
        break;
    case speechnoise_t::TEN_SPL :
        interp.set_xfun(NULL);
        yfun = NULL;
        for(float f=20.0f;f<20000.0f;f *= 1.059463094f)
            interp.add_entry(f,1.0f/sqrt(erb_hz_f_hz(f)));
        break;
    case speechnoise_t::TEN_SPL_250_8k :
        interp.set_xfun(NULL);
        yfun = NULL;
        for(float f=250.0f;f<8000.0f;f *= 1.059463094f)
            interp.add_entry(f,1.0f/sqrt(erb_hz_f_hz(f)));
        for(float f=250.0f;f>10.0f;f /= 1.059463094f)
            interp.add_entry(f,powf(f/250.0f,4.0f)/sqrt(erb_hz_f_hz(250.0f)));
        for(float f=8000.0f;f<64000.0f;f *= 1.059463094f)
            interp.add_entry(f,powf(8000.0f/f,4.0f)/sqrt(erb_hz_f_hz(8000.0f)));
        break;
    case speechnoise_t::TEN_SPL_50_16k :
        interp.set_xfun(NULL);
        yfun = NULL;
        for(float f=50.0f;f<16000.0f;f *= 1.059463094f)
            interp.add_entry(f,1.0f/sqrt(erb_hz_f_hz(f)));
        for(float f=50.0f;f>10.0f;f /= 1.059463094f)
            interp.add_entry(f,powf(f/50.0f,4.0f)/sqrt(erb_hz_f_hz(50.0f)));
        for(float f=16000.0f;f<64000.0f;f *= 1.059463094f)
            interp.add_entry(f,powf(16000.0f/f,4.0f)/sqrt(erb_hz_f_hz(16000.0f)));
        break;
    case speechnoise_t::sin125 :
    case speechnoise_t::sin250 :
    case speechnoise_t::sin500 :
    case speechnoise_t::sin1k :
    case speechnoise_t::sin2k :
    case speechnoise_t::sin4k :
    case speechnoise_t::sin8k :
        interp.set_xfun(NULL);
        yfun = NULL;
        interp.add_entry(1.0f,0.0f);
        interp.add_entry(100000.0f,0.0f);
        break;
    }
    switch( noise_type ){
    case speechnoise_t::sin125 :
        kfbin = static_cast<unsigned>(125.f*num_frames/srate);
        break;
    case speechnoise_t::sin250 :
        kfbin = static_cast<unsigned>(250.f*num_frames/srate);
        break;
    case speechnoise_t::sin500 :
        kfbin = static_cast<unsigned>(500.f*num_frames/srate);
        break;
    case speechnoise_t::sin1k :
        kfbin = static_cast<unsigned>(1000.f*num_frames/srate);
        break;
    case speechnoise_t::sin2k :
        kfbin = static_cast<unsigned>(2000.f*num_frames/srate);
        break;
    case speechnoise_t::sin4k :
        kfbin = static_cast<unsigned>(4000.f*num_frames/srate);
        break;
    case speechnoise_t::sin8k :
        kfbin = static_cast<unsigned>(8000.f*num_frames/srate);
        break;
    default:
        break;
    }
    for(ch=0;ch<num_channels;ch++)
        temp_spec.value(0,ch) = mha_complex(0,0);
    mha_real_t amplitude(0);
    for(k=1;k<temp_spec.num_frames;k++){
        freq = k*srate/num_frames;
        amplitude = interp.interp(freq);
        if( yfun )
            amplitude = yfun(amplitude);
        expi(temp_spec.value(k,0),2.0*M_PI*rand()/RAND_MAX,amplitude);
        if( k==kfbin )
            temp_spec.value(k,0) = mha_complex(1,0);
        for(ch=1;ch<num_channels;ch++)
            temp_spec.value(k,ch) = temp_spec.value(k,0);
    }
    mha_fft_t fft = mha_fft_new(num_frames);
    mha_fft_spec2wave(fft, &temp_spec, this );
    mha_fft_free(fft);
}

// Local Variables:
// compile-command: "make -C .."
// coding: utf-8-unix
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
