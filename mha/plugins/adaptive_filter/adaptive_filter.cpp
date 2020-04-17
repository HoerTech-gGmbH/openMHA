// mhainfo: plugin
/*
 * Configuration interface for Shynk FIR-filter adaptation.
 */
#include "adaptive_filter.h"

/*
 * Implements the binaural beamforming algorithm described in
 * Development and Objective Perceptual Quality
 * Assessment of Monaural and Binaural Noise
 * Reduction Schemes for Hearing Aids,
 * PhD thesis from Thomas Rohdenburg
 */

#define LPSCALE (5.2429e+007) //large power scaling
#define POWSPEC_FACTOR  0.0025 //MHA default powspec scaling
#define OVERLAP_FACTOR  2   //redefine if needed, or make into a parameter

#define CHANLOOP for ( unsigned int c=0; c<nchan; ++c )
#define FREQLOOP for ( unsigned int f=0; f<nfreq; ++f )

#define MU_CONST 0.001
#define ALP 0.5
#define DELT (1e-12)

/** Constructs the beamforming plugin. */
adaptive_filter_if_t::adaptive_filter_if_t(algo_comm_t & ac,
                                           const std::string & chain_name,
                                           const std::string & algo_name)
  : MHAPlugin::plugin_t<adaptive_filter_t>("Shynk FIR-filter adaptation",ac),
    lenOldSamps("how many old samples to buffer", "1024", "[0,5000]"),
    doCircularComp("whether to compensate for circular convolution", "no"),
    mu("scalar value in computing gradient", "0.2", "[0,2]"),
    alp("autoregressive coefficient for estimating PSD", "0.5", "[0,1]"),
    useVAD("whether to use the VAD given in AC-variable", "yes"),
    prepared(false)
{
  insert_item("lenOldSamps", &lenOldSamps);
  insert_item("doCircularComp", &doCircularComp);
  insert_item("mu", &mu);
  insert_item("alp", &alp);
  insert_item("useVAD", &useVAD);

  patchbay.connect(&lenOldSamps.valuechanged, this,
                   &adaptive_filter_if_t::on_model_param_valuechanged);
  patchbay.connect(&doCircularComp.valuechanged, this,
                   &adaptive_filter_if_t::on_model_param_valuechanged);
  patchbay.connect(&mu.valuechanged, this,
                   &adaptive_filter_if_t::on_model_param_valuechanged);
  patchbay.connect(&alp.valuechanged, this,
                   &adaptive_filter_if_t::on_model_param_valuechanged);
  patchbay.connect(&useVAD.valuechanged, this,
                   &adaptive_filter_if_t::on_model_param_valuechanged);
}

/** Plugin preparation. This plugin checks that the input signal has the
 * spectral domain and contains at least one channel
 * @param signal_info
 *   Structure containing a description of the form of the signal (domain,
 *   number of channels, frames per block, sampling rate.
 */
void adaptive_filter_if_t::prepare(mhaconfig_t & signal_info)
{
  if (signal_info.domain != MHA_WAVEFORM)
    throw MHA_Error(__FILE__, __LINE__,
                    "This plugin can only process waveform signals.");

  //there is only one output channel, the error signal
  signal_info.channels = 1;

  //tell the plugin that it's ok to prepare configurations
  prepared = true;

  /* remember the transform configuration (i.e. channel numbers): */
  tftype = signal_info;
  /* make sure that a valid runtime configuration exists: */
  update_cfg();
}

/* when one of the angles or radii changes, recompute the head model */
void adaptive_filter_if_t::on_model_param_valuechanged()
{
  //only push configurations if prepare has already been called
  if ( prepared ) update_cfg();
}

void adaptive_filter_if_t::update_cfg()
{
  adaptive_filter_t *config = new adaptive_filter_t( ac, input_cfg(),
                                                     lenOldSamps.data, doCircularComp.data,
                                                     mu.data, alp.data, useVAD.data );
  push_config( config );
}

/** This plugin implements noise reduction using spectral
 * subtraction: by nonnegative subtraction from the output magnitude
 * of the estimated noise magnitude spectrum.
 * @param signal
 *   Pointer to the input signal structure.
 * @return
 *   Returns a pointer to the input signal structure,
 *   with a the signal modified by this plugin.
 */
mha_wave_t * adaptive_filter_if_t::process(mha_wave_t * signal)
{
  poll_config();
  return cfg->process(signal);
}

adaptive_filter_t::adaptive_filter_t(algo_comm_t & ac, const mhaconfig_t in_cfg,
                                     int lenOldSamps, bool doCircularComp, float mu,
                                     float alp, bool useVAD) :

  ac( ac ), in_cfg( in_cfg ),
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
  Psum( ac, "P", nfreq, 1, true ),
  ref( ac, "ref", bufSize, nchan, true ),
  Xinit( ac, "Xinit", nfreq, nchan, true )
{
  //assume P starts out flat. this is to avoid blowing up at first
  for ( unsigned int f=0; f<nfreq; ++f )  {
    for ( unsigned int c=0; c<nchan; ++c ) {
      P(f,c) = 0.01; //initializing PSD to constant
    }
  }
}

/* This constructor copies relevant buffer and filter state for the filter */
adaptive_filter_t::adaptive_filter_t(adaptive_filter_t &old,
                                     bool doCircularComp, float mu,
                                     float alp, bool useVAD) :

  ac( old.ac ), in_cfg( old.in_cfg ),
  lenOldSamps( old.lenOldSamps ), lenNewSamps( old.lenNewSamps ),
  bufSize( lenOldSamps+lenNewSamps ),
  frac_old( old.frac_old ),
  mha_fft( mha_fft_new(bufSize) ),
  nfreq( bufSize/2+1 ),
  //convention: last channel is desired signal; VAD comes in via AC
  nchan( old.nchan ), //extra input channels
  desired_chan( nchan ), //vad_chan( nchan+1 ),
  doCircularComp( doCircularComp ),
  mu( mu ), alp( alp ), useVAD( useVAD ),
  x( old.x ), //copy x to maintain buffering scheme
  X( ac, "X", nfreq, nchan, true  ), //X is volatile
  W( old.W ), //copy W to maintain filter state
  Y( ac, "Y", nfreq, 1, true  ),
  y( ac, "y", bufSize, 1, true  ),
  d( ac, "d", bufSize, 1, true  ),
  e( ac, "e", bufSize, 1, true  ),
  E( ac, "E", nfreq, 1, true  ),
  E2( ac, "E2", nfreq, nchan, true  ),
  grad( ac, "grad", bufSize, nchan, true  ),
  Grad( ac, "Grad", nfreq, nchan, true  ),
  e_out( ac, "e_out", lenNewSamps, 1, true ),
  P( old.P ), //copy P to maintain PSD estimation for learning rule
  Psum( old.Psum ), //copy P to maintain PSD estimation for learning rule
  ref( ac, "ref", bufSize, nchan, true ),
  Xinit( ac, "Xinit", nfreq, nchan, true )
{
  //all initialization happens above
}

mha_wave_t *adaptive_filter_t::process(mha_wave_t *wavin)
{
  insert();

  //assume that blocksize==lenNewSamps

  //add new samples to the end of x buffer
  timeshift(x, -lenNewSamps);
  for (unsigned int i=0; i<lenNewSamps; ++i)
    {
      for ( unsigned int c=0; c<nchan; ++c ) {
        x(i+lenOldSamps,c) = value(wavin,i,c);
      }}

  //OUTPUT CHAIN
  mha_fft_wave2spec_scale( mha_fft, &x, &X );

  //initialize Y
  for ( unsigned int f=0; f<nfreq; ++f ) { Y(f,0).re = 0; Y(f,0).im = 0; }

  // Y = X * W
  for ( unsigned int f=0; f<nfreq; ++f ) { for ( unsigned int c=0; c<nchan; ++c ) {
      Y(f,0) += X(f,c) * W(f,c);
    }}

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
  for ( unsigned int f=0; f<nfreq; ++f ) { for ( unsigned int c=0; c<nchan; ++c ) {
      E2(f,c) = _conjugate( X(f,c) ) * E(f,0);
    }}

  //update P
  for ( unsigned int f=0; f<nfreq; ++f ) {
    float sum_chan; sum_chan = 0.0;
    for ( unsigned int c=0; c<nchan; ++c ) {
      P(f,c) = alp * P(f,c);
      P(f,c) += (1-alp) * abs2( X(f,c) ) * frac_old;
      sum_chan += P(f,c);
    }
    Psum(f,0) = sum_chan;
    //for ( unsigned int c=0; c<nchan; ++c ) { P(f,c) = sum_chan; } //P = sum(P,2);
  }

  //Grad(:,idx2) = mu.*(1./(P(:,idx2)+delt)).*E2(:,idx2);
  for ( unsigned int f=0; f<nfreq; ++f ) { for ( unsigned int c=0; c<nchan; ++c ) {
      Grad(f,c) = E2(f,c) * mu / (Psum(f,0)+DELT);
    }}

  //this part attempts to linearize the convolution
  if ( doCircularComp ) { //this step may be unnecessary

    mha_fft_spec2wave_scale( mha_fft, &Grad, &grad );

    //set lenNewSamps at end to be zero
    for (unsigned int i=0; i<lenNewSamps; ++i)
      {
        for ( unsigned int c=0; c<nchan; ++c ) { grad(lenOldSamps+i,c) = 0; }
      }

    mha_fft_wave2spec_scale( mha_fft, &grad, &Grad );
  }

  //TODO: compute Mu here. for the moment Mu is a constant
  //for ( unsigned int f=0; f<nfreq; ++f ) { for ( unsigned int c=0; c<nchan; ++c ) { Mu(f,c) = MU_CONST; }}

  //Grad = Grad .* Mu
  //for ( unsigned int f=0; f<nfreq; ++f ) { for ( unsigned int c=0; c<nchan; ++c ) { Grad(f,c) *= Mu(f,c); }}

  //TODO: make the VAD name configurable
  if (useVAD) {
    mha_wave_t VAD = MHA_AC::get_var_waveform(ac, "VAD");

    //choose to adapt or not for each channel
    for (unsigned int c=0; c<VAD.num_channels; c++)
      {

        //sum the current block of the VAD and decide whether to adapt
        //TODO: maybe you have to buffer the vad like with x, not sure
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
    for ( unsigned int c=0; c<nchan; ++c ) { for ( unsigned int f=0; f<nfreq; ++f ) { W(f,c) += Grad(f,c); }}
  }

  //return e_out
  return &e_out;
}

void adaptive_filter_t::insert()
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

  ref.insert();
  Xinit.insert();
}

/*
 * This macro connects the adaptive_filtering_if_t class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(adaptive_filter,adaptive_filter_if_t,wave,wave)

/*
 * This macro creates code classification of the plugin and for
 * automatic documentation.
 *
 * The first argument to the macro is a space separated list of
 * categories, starting with the most relevant category. The second
 * argument is a LaTeX-compatible character array with some detailed
 * documentation of the plugin.
 */
MHAPLUGIN_DOCUMENTATION(adaptive_filter,
                        "adaptive filtering",
                        "Implements the FIR-filter adaptation scheme found in "
                        "John J. Shynk, "
                        "Frequency-Domain and Multirate Adaptive Filtering, "
                        "IEEE Signal Processing Magazine, 1992.\n\n"
                        )
