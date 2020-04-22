// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2013 2014 2016 2018 2020 HörTech gGmbH
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

#include "mha_plugin.hh"

#include "gsc_adaptive_stage.hh"
#include "gsc_adaptive_stage_if.hh"

/** Ctor of the rt processing class
    @param ac Handle to AC space
    @param in_cfg Input signal configuration
    @param lenOldSamps How many old samples to buffer
    @param doCircularComp Compensate for circular convolution?
    @param mu Scalar coefficient for gradient
    @param alp Autoregressive coefficient for estimating PSD
    @param useVAD Use voice activity detection?
    @param vadName_ Name of the VAD AC variable
 */
gsc_adaptive_stage::gsc_adaptive_stage::gsc_adaptive_stage(algo_comm_t & ac, const mhaconfig_t in_cfg,
                                     int lenOldSamps, bool doCircularComp, float mu,
                                     float alp, bool useVAD, const std::string& vadName_) :

  ac( ac ),
  lenOldSamps( lenOldSamps ),
  lenNewSamps( in_cfg.fragsize ), //use the fragsize for lenNewSamps
  bufSize( lenOldSamps+lenNewSamps ),
  frac_old( (float) lenOldSamps / (float) (lenNewSamps + lenOldSamps) ),
  mha_fft( mha_fft_new(bufSize) ),
  nfreq( bufSize/2+1 ),
  //convention: last channel is desired signal; VAD comes in via AC
  nchan( in_cfg.channels-1 ), //extra input channels
  desired_chan( nchan ), //vad_chan( nchan+1 ),
  doCircularComp( doCircularComp ),
  mu( mu ), alp( alp ), useVAD( useVAD ),
  vadName(vadName_),
  x( ac, "x", bufSize, nchan, true  ),
  X( ac, "X", nfreq, nchan, true  ),
  W( ac, "W", nfreq, nchan, true  ),
  Y( ac, "Y", nfreq, 1, true  ),
  y( ac, "y", bufSize, 1, true  ),
  d( ac, "d", bufSize, 1, true  ),
  e( ac, "e", bufSize, 1, true  ),
  E( ac, "E", nfreq, 1, true  ),
  E2( ac, "E2", nfreq, nchan, true  ),
  grad( ac, "grad", bufSize, nchan, true  ),
  Grad( ac, "Grad", nfreq, nchan, true  ),
  e_out( ac, "e_out", lenNewSamps, 1, true ),
  P( ac, "P", nfreq, nchan, true ),
  Psum( ac, "P", nfreq, 1, true )
{
  //assume P starts out flat. this is to avoid blowing up at first
  for ( unsigned int f=0; f<nfreq; ++f )  {
    for ( unsigned int c=0; c<nchan; ++c ) {
      P(f,c) = 0.01; //initializing PSD to constant
    }
  }
}
/** Processing callback
 * @param wavin input signal
 * @return Returns a pointer to the output signal
 */
mha_wave_t *gsc_adaptive_stage::gsc_adaptive_stage::process(mha_wave_t *wavin)
{
  insert();

  //assume that blocksize==lenNewSamps

  //add new samples to the end of x buffer
  timeshift(x, -lenNewSamps);
  for (unsigned int i=0; i<lenNewSamps; ++i) {
    for ( unsigned int c=0; c<nchan; ++c ) {
      x(i+lenOldSamps,c) = value(wavin,i,c);
    }
  }

  //OUTPUT CHAIN
  mha_fft_wave2spec_scale( mha_fft, &x, &X );

  //initialize Y
  for ( unsigned int f=0; f<nfreq; ++f ) { Y(f,0).re = 0; Y(f,0).im = 0; }

  // Y = X * W
  for ( unsigned int f=0; f<nfreq; ++f ) {
    for ( unsigned int c=0; c<nchan; ++c ) {
      Y(f,0) += X(f,c) * W(f,c);
    }
  }

  mha_fft_spec2wave_scale( mha_fft, &Y, &y );

  //discard earliest samples from y, discard lenOldSamps
  for (unsigned int i=0; i<lenOldSamps; ++i)
    {
      y(i,0) = 0;
    }

  //copy new samples to end of d, with leading zeros
  for (unsigned int i=0; i<lenNewSamps; ++i)
    {
      d(lenOldSamps+i,0) = value(wavin,i,desired_chan);
    }

  //FILTER ADAPTATION

  //e = d - y (zeros already present in d,y)
  for (unsigned int i=0; i<lenNewSamps; ++i)
    {
      e(lenOldSamps+i,0) = d(lenOldSamps+i,0) - y(lenOldSamps+i,0);
    }

  //copy error signal to output
  for (unsigned int i=0; i<lenNewSamps; ++i)
    {
      e_out(i,0) = e(lenOldSamps+i,0);
    }

  mha_fft_wave2spec_scale( mha_fft, &e, &E );

  //E2 = X^T * E
  for ( unsigned int f=0; f<nfreq; ++f ) {
    for ( unsigned int c=0; c<nchan; ++c ) {
      E2(f,c) = _conjugate( X(f,c) ) * E(f,0);
    }
  }

  //update P
  for ( unsigned int f=0; f<nfreq; ++f ) {
    float sum_chan = 0.0;
    for ( unsigned int c=0; c<nchan; ++c ) {
      P(f,c) = alp * P(f,c);
      P(f,c) += (1-alp) * abs2( X(f,c) ) * frac_old;
      sum_chan += P(f,c);
    }
    Psum(f,0) = sum_chan;
  }

  //Grad(:,idx2) = mu.*(1./(P(:,idx2)+delt)).*E2(:,idx2);
  for ( unsigned int f=0; f<nfreq; ++f ) {
      for ( unsigned int c=0; c<nchan; ++c ) {
          Grad(f,c) = E2(f,c) * mu / (Psum(f,0)+DELT);
      }
  }

  //this part attempts to linearize the convolution
  if ( doCircularComp ) { //this step may be unnecessary

    mha_fft_spec2wave_scale( mha_fft, &Grad, &grad );

    //set lenNewSamps at end to be zero
    for (unsigned int i=0; i<lenNewSamps; ++i) {
        for ( unsigned int c=0; c<nchan; ++c ) {
            grad(lenOldSamps+i,c) = 0;
        }
    }
    mha_fft_wave2spec_scale( mha_fft, &grad, &Grad );
  }

  if (useVAD) {
    mha_wave_t VAD = MHA_AC::get_var_waveform(ac, vadName.c_str());

    //choose to adapt or not for each channel
    for (unsigned int c=0; c<VAD.num_channels; c++)
      {

        //sum the current block of the VAD and decide whether to adapt
        float vad_sum = 0;
        for (unsigned int i=0; i<VAD.num_frames; ++i)
          {
            vad_sum += value(VAD,i,c);
          }

        //if VAD is low (there is no voice and only noise), adapt the filter
        if ( vad_sum < (lenNewSamps / 2) )
          {
            //W += Grad
            for ( unsigned int f=0; f<nfreq; ++f ) { W(f,c) += Grad(f,c); }
          }

      } //end of all channels
  } else {
      for ( unsigned int c=0; c<nchan; ++c ) {
          for ( unsigned int f=0; f<nfreq; ++f ) {
              W(f,c) += Grad(f,c);
          }
      }
  }
  return &e_out;
}
/** Re-insert all AC variables into the AC space */
void gsc_adaptive_stage::gsc_adaptive_stage::insert()
{
  x.insert();
  X.insert();
  W.insert();
  Y.insert();
  y.insert();
  d.insert();
  e.insert();
  E.insert();
  E2.insert();
  grad.insert();
  Grad.insert();
}

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
