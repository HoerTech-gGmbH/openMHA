// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2012 2013 2014 2015 2016 2017 2018 2019 2020 HörTech gGmbH
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

#include "rohBeam.hh"
#include "mha_utils.hh"
using namespace Eigen;

using MHAUtils::is_denormal;


  //In C++17, the spherical bessel function is in namespace std.
  // Before C++17, the spherical bessel function was in namespace std::tr1
  // libc++, clang's std implementation does not have the bessel fcts at all
#if __cplusplus >= 201703L and !defined(__clang__)// We are using C++17 are not using clang
#include <cmath>
double rohBeam::j0(double x){
    return std::cyl_bessel_j(0,x);
  }
#elif defined(__clang__)  // Clang has the POSIX implementation of the bessel function
#include <cmath>
double rohBeam::j0(double x){
    return ::j0(x);
  }
#elif __has_include(<tr1/cmath>) // We are on an older not-clang compiler and can use the tr1 headers
#include <tr1/cmath>
double rohBeam::j0(double x){
    return std::tr1::cyl_bessel_j(0,x);
  }
#elif __has_include(<boost/math/special_functions/bessel.hpp>) // Use boot as last resort
#include <boost/math/special_functions/bessel.hpp>
double rohBeam::j0(double x) {
    return boost::math::cyl_bessel_j(0,x);
  }
#else
#error "No implementation of cyl_bessel_j is available!"
#endif


namespace rohBeam {

  rohConfig::rohConfig(const mhaconfig_t in_cfg,const mhaconfig_t out_cfg,
                       std::unique_ptr<MatrixXcf> headModel_,
                       std::unique_ptr<MHASignal::matrix_t> beamW_,
                       std::unique_ptr<MHASignal::matrix_t> delayComp_,
                       const configOptions& options):
    nfreq( in_cfg.fftlen/2+1 ),
    nchan_block( in_cfg.channels-1 ),
    in_cfg( in_cfg ),
    out_cfg( out_cfg ),
    enable_adaptive_beam( options.enable_adaptive_beam ),
    binaural_type_index( options.binaural_type_index ),
    headModel( std::move(headModel_) ),
    beamW( std::move(beamW_) ),
    delayComp( std::move(delayComp_) ),
    alpha_postfilter( options.alpha_postfilter ),
    alpha_blocking_XkXi( options.alpha_blocking_XkXi ),
    alpha_blocking_XkY( options.alpha_blocking_XkY ),
    corrXpXp( nfreq, MatrixXcf::Identity(nchan_block,nchan_block)),
    corrXpYf( nfreq, VectorXcf::Constant(nchan_block,0) ),
    corrZZ( VectorXf::Constant(nfreq, 1/nfreq) ),
    corrLL( VectorXf::Constant(nfreq, 1/nfreq) ),
    corrRR( VectorXf::Constant(nfreq, 1/nfreq) ),
    hhCorrXpXp( nchan_block, nchan_block  ),
    nextXpYf( nchan_block ),
    blockXp( nchan_block ),
    freqResp( nchan_block ),
    magResp( nchan_block ),
    minLim( pow( 10.0f, -1 )),
    maxLim( pow( 10.0f, 1 ))
  {
    init_dynamic();
  }

  /* This is a constructor that copies most non-volatile/non-throwaway state.
     It is intended for smooth changes in configuration variables like speaker angle. */
  rohConfig::rohConfig(rohConfig *lastConfig, const mhaconfig_t in_cfg,const mhaconfig_t out_cfg,
                       std::unique_ptr<MatrixXcf> headModel_,
                       std::unique_ptr<MHASignal::matrix_t> beamW_,
                       std::unique_ptr<MHASignal::matrix_t> delayComp_,
                       const configOptions& options):
    nfreq( in_cfg.fftlen/2+1 ),
    nchan_block( in_cfg.channels-1 ),
    in_cfg( in_cfg ),
    out_cfg( out_cfg ),
    enable_adaptive_beam( options.enable_adaptive_beam ),
    binaural_type_index( options.binaural_type_index ),
    headModel( std::move(headModel_) ),
    beamW( std::move(beamW_) ),
    delayComp( std::move(delayComp_) ),
    alpha_postfilter( options.alpha_postfilter ),
    alpha_blocking_XkXi( options.alpha_blocking_XkXi ),
    alpha_blocking_XkY( options.alpha_blocking_XkY ),
    corrXpXp( lastConfig->corrXpXp ),
    corrXpYf( lastConfig->corrXpYf ),
    corrZZ( lastConfig->corrZZ ),
    corrLL( lastConfig->corrLL ),
    corrRR( lastConfig->corrRR ),
    hhCorrXpXp( nchan_block, nchan_block ),
    nextXpYf( nchan_block ),
    blockXp( nchan_block ), //state doesn't matter
    freqResp( nchan_block ),
    magResp( nchan_block ),
    minLim( lastConfig->minLim ), maxLim( lastConfig->maxLim )
  {
    init_dynamic();
  }

  //release the dynamically allocated resources
  rohConfig::~rohConfig() {

    delete beam1;
    delete beamA;
    delete blockSpec;
    delete outSpec;
  }

  void rohConfig::init_dynamic() {

    //allocate the dynamically allocated things
    this->beam1 = new MHASignal::spectrum_t(nfreq, 1);
    this->blockSpec = new MHASignal::spectrum_t(nfreq, nchan_block);
    this->outSpec = new MHASignal::spectrum_t(nfreq, out_cfg.channels);
    this->beamA = new MHASignal::spectrum_t(nfreq, 1);

    for (int f=0; f<nfreq; f++) {
      beam1->value(f,0).re = 0;
      beam1->value(f,0).im = 0;
    }

    for (int f=0; f<nfreq; f++) {
      for (int c=0; c<nchan_block; c++) {
        blockSpec->value(f,c).re = 0;
        blockSpec->value(f,c).im = 0;
      }
    }

    for (int f=0; f<nfreq; f++) {
      for (unsigned int c=0; c<out_cfg.channels; c++) {
        outSpec->value(f,c).re = 0;
        outSpec->value(f,c).im = 0;
      }
    }

    for (int f=0; f<nfreq; f++) {
      beamA->value(f,0).re = 0;
      beamA->value(f,0).im = 0;
    }
  }

  mha_spec_t * rohConfig::process(mha_spec_t *inSpec) {

    for (int f=0; f<nfreq; f++) {

      beam1->value(f,0).re = 0;
      beam1->value(f,0).im = 0;

      /* the output of this stage goes to mono beam1 */
      for (unsigned int ci=0; ci<in_cfg.channels; ci++) {
        beam1->value(f,0) += _conjugate((*beamW)(f,ci)) * value(inSpec,f,ci);
      }
    }

    //perform blocking here
    for (unsigned int f=0; f<inSpec->num_frames; f++) {

      //iterate over adjacent pairs of channels, hence minus one (-1)
      for (unsigned int c=0; c<in_cfg.channels-1; c++) {

        //subtract pairs of bins in adjacent channels modified by delayComp
        //NOTE: slightly inefficient because delayComp mult is happening twice
        //we could also multiply it in-place to the input
        blockSpec->value(f,c) =
          value(inSpec,f,c) * (*delayComp)(f,c) -
          value(inSpec,f,c+1) * (*delayComp)(f,c+1);
      }
    }

    //recursive estimation of noise matrices
    for (int f=0; f<nfreq; f++) {

      std::complex<float> valYf;

      //copy values to Eigen matrices for slices of blocked noise
      for (int c=0; c<nchan_block; c++) {
        blockXp(c) = stdcomplex( value( blockSpec, f, c ) );
      }

      //copy the most recent value from fixed bf output
      valYf = stdcomplex( value( beam1, f, 0 ) );

      //these operations are taking too long
      //nextXpXp = blockXp * blockXp.adjoint();
      nextXpYf = blockXp * conj( valYf );

      //exponential filters
      //corrXpXp->at(f) = alpha * corrXpXp->at(f) + (1 - alpha) * nextXpXp;
      corrXpYf[f] = alpha_blocking_XkY * corrXpYf[f] + (1 - alpha_blocking_XkY) * nextXpYf;
    }

    //alternate form of recursive estimation for X,X
    mha_complex_t val = mha_complex(0,0);
    std::complex<float> valC = std::complex<float>(0,0);
    for (int k=0; k<nchan_block; k++) {
      for (int i=0; i<nchan_block; i++) {
        for (int f=0; f<nfreq; f++) {
          val = blockSpec->value(f,k) * _conjugate( blockSpec->value(f,i) );
          valC = stdcomplex( val );

          //update value in matrix
          corrXpXp[f](k,i) = alpha_blocking_XkXi * corrXpXp[f](k,i) + (1 - alpha_blocking_XkXi) * valC;
        }
      }
    }

    //save work for the adaptive step if it is not needed
    if ( enable_adaptive_beam ) {

      mha_complex_t freqRespM = mha_complex(0,0);

      for (int f=0; f<nfreq; f++) {

        //Wiener filter design,
        //solve the linear system, decomposing the matrix to allocated buffer
        hhCorrXpXp.compute( corrXpXp[f] ); //unnecessary, should be able to update via Sherman-Morrison
        freqResp = hhCorrXpXp.solve( corrXpYf[f] );

        magResp = freqResp.array().abs();

        for (int c=0; c<nchan_block; c++) {
          if ( magResp(c) > maxLim )
            freqResp(c) = std::polar( maxLim, arg(freqResp(c)) );
          else if ( magResp(c) < minLim )
            freqResp(c) = std::polar( minLim, arg(freqResp(c)) );
        }

        //filter the blocked spectrum directly
        beamA->value(f,0).re = 0;
        beamA->value(f,0).im = 0;

        for (int c=0; c<nchan_block; c++) {
          freqRespM = ::set( freqRespM, freqResp(c) );
          beamA->value(f,0) += _conjugate(freqRespM) * blockSpec->value(f,c);
        }

        //in_place: compute Z by subtracting: Yf - Ya
        beamA->value(f,0) = beam1->value(f,0) - beamA->value(f,0);
      }
    }

    MHASignal::spectrum_t *prevSpecPost = enable_adaptive_beam ? beamA : beam1;
    if ( binaural_type_index==1 ) {
      //BIN_PR - phase reconstruction solution
      phasereconstruction(prevSpecPost);
    } else if ( binaural_type_index==2 ) {
      //BIN_PF - binaural postfilter solution
      postfilter(inSpec, prevSpecPost);
    }

    //to hear a previous stage, just copy this to output
    if ( !binaural_type_index ) {
      copyfixedbfoutput(prevSpecPost);
    }

    return outSpec;
  }



  void rohConfig::copyfixedbfoutput(MHASignal::spectrum_t* prevSpecPost){
    //copy the fixed bf output
    for (unsigned int f=0; f<outSpec->num_frames; f++) {
      for (unsigned int c=0; c<out_cfg.channels; c++) {
        outSpec->value(f,c) = prevSpecPost->value(f,0);
      }
    }
  }

  void rohConfig::phasereconstruction(MHASignal::spectrum_t* prevSpecPost) {
    mha_complex_t headModelL, headModelR;
    for (int f=0; f<nfreq; f++) {
      ::set( headModelL, (*headModel)(f,refL) );
      ::set( headModelR, (*headModel)(f,refR) );
      outSpec->value(f,0) = headModelL * prevSpecPost->value(f,0);
      outSpec->value(f,1) = headModelR * prevSpecPost->value(f,0);
    }
  }

  void rohConfig::postfilter(mha_spec_t *inSpec,MHASignal::spectrum_t *prevSpecPost){

    for (int f=0; f<nfreq; f++) {
      corrZZ(f) = alpha_postfilter * corrZZ(f) + (1 - alpha_postfilter) * abs2( prevSpecPost->value(f,0) );
    }

    //for the moment use hardcoded reference channels
    //estimate the power spectral densities of filtered, left, and right

    for (int f=0; f<nfreq; f++) {
      corrLL(f) = alpha_postfilter * corrLL(f) + (1 - alpha_postfilter) * abs2( value(inSpec,f,refL) );
      corrRR(f) = alpha_postfilter * corrRR(f) + (1 - alpha_postfilter) * abs2( value(inSpec,f,refR) );
    }

    //estimate the coefficients and apply them directly
    for (int f=0; f<nfreq; f++) {
      float propPowL = abs( (*headModel)(f,refL) );
      propPowL = propPowL * propPowL;
      float propPowR = abs( (*headModel)(f,refR) );
      propPowR = propPowR * propPowR;
      float H = ( propPowL + propPowR ) * corrZZ(f) / ( corrLL(f) + corrRR(f) );

      if ( is_denormal(H) ) { //try to catch bad filter, not working
        H = 0;
      }
      outSpec->value(f,0) = value(inSpec,f,refL) * H;
      outSpec->value(f,1) = value(inSpec,f,refR) * H;
    }
  }

  /** Constructs the beamforming plugin. */
  rohBeam::rohBeam(algo_comm_t & ac,
                   const std::string & chain_name,
                   const std::string & algo_name)
    : MHAPlugin::plugin_t<rohConfig>("Rohdenburg binaural beamformer",ac),

      prop_type("Propogation vector type.", "hm1", "[hm1 sampled]"),
      sampled_hrir_path("Path for a sampled hrir for the propogation vector.", ""),

      source_azimuth_degrees("Azimuth angle for the speech source.", "0", "[-180,180]"),
      mic_azimuth_degrees_vec("Azimuth angle for each mic, as vector.", "[0 0 0 0 0 0]", "[-180,180]"),
      head_model_sphere_radius_cm("Radius size of head model in meter", "8.2", "[0,100]"),
      intermic_distance_cm("Intermic distances in cm",
                           "[[0 0 0 0 0 0]; [0 0 0 0 0 0]; [0 0 0 0 0 0]; "
                           "[0 0 0 0 0 0]; [0 0 0 0 0 0]; [0 0 0 0 0 0]]", "[0,100]"),

      //TODO: add options for different propogation vectors
      //TODO: add option for free-field propogation vectors
      //TODO: add option for sampled hrir/propogation vector

      noise_field_model("Noise field model", "uncorr", "[uncorr diff2D diff3D intHRTF]"),
      enable_adaptive_beam("Whether adaptive beamformer is active.", "no"),
      binaural_type("Binaural adaptation type", "bin_pf", "[mono bin_pr bin_pf]"),
      diag_loading_mu("Diagonal loading constant mu for design of fixed beamformer.",
                      "0.1", "[0.000,2]"),
      enable_export("Whether filter design is exported as AC variables.", "no"),
      enable_wng_optimization("Whether beamform design uses white noise gain optimization.", "no"),
      tau_postfilter_ms("Smoothing time constant for postfilter in milliseconds.", "100", "[1,5000]"),
      tau_blocking_XkXi_ms("Time constant for estimation of noise cross-PSD in blocked signal.",
                           "30", "[1,5000]"),
      tau_blocking_XkY_ms("Time constant for estimation of filtered with blocked noise cross-PSD.",
                          "30", "[1,5000]"),
      prepared(false),
      beamExport(nullptr), noiseModelExport(nullptr)
  {
    //register variables
    insert_item("prop_type", &prop_type);
    insert_item("sampled_hrir_path", &sampled_hrir_path);

    insert_item("source_azimuth_degrees", &source_azimuth_degrees);
    insert_item("mic_azimuth_degrees_vec", &mic_azimuth_degrees_vec);
    insert_item("head_model_sphere_radius_cm", &head_model_sphere_radius_cm);
    insert_item("intermic_distance_cm", &intermic_distance_cm);
    insert_item("noise_field_model", &noise_field_model);
    insert_item("enable_adaptive_beam", &enable_adaptive_beam);
    insert_item("binaural_type", &binaural_type);
    insert_item("diag_loading_mu", &diag_loading_mu);
    insert_item("enable_export", &enable_export);
    insert_item("enable_wng_optimization", &enable_wng_optimization);
    insert_item("tau_postfilter_ms", &tau_postfilter_ms);
    insert_item("tau_blocking_XkXi_ms", &tau_blocking_XkXi_ms);
    insert_item("tau_blocking_XkY_ms", &tau_blocking_XkY_ms);

    patchbay.connect(&prop_type.valuechanged, this,
                     &rohBeam::on_model_param_valuechanged);
    patchbay.connect(&sampled_hrir_path.valuechanged, this,
                     &rohBeam::on_model_param_valuechanged);

    patchbay.connect(&source_azimuth_degrees.valuechanged, this,
                     &rohBeam::on_model_param_valuechanged);
    patchbay.connect(&mic_azimuth_degrees_vec.valuechanged, this,
                     &rohBeam::on_model_param_valuechanged);
    patchbay.connect(&head_model_sphere_radius_cm.valuechanged, this,
                     &rohBeam::on_model_param_valuechanged);
    patchbay.connect(&intermic_distance_cm.valuechanged, this,
                     &rohBeam::on_model_param_valuechanged);
    patchbay.connect(&noise_field_model.valuechanged, this,
                     &rohBeam::on_model_param_valuechanged);
    patchbay.connect(&enable_adaptive_beam.valuechanged, this,
                     &rohBeam::on_model_param_valuechanged);
    patchbay.connect(&binaural_type.valuechanged, this,
                     &rohBeam::on_model_param_valuechanged);
    patchbay.connect(&diag_loading_mu.valuechanged, this,
                     &rohBeam::on_model_param_valuechanged);
    patchbay.connect(&enable_export.valuechanged, this,
                     &rohBeam::on_model_param_valuechanged);
    patchbay.connect(&enable_wng_optimization.valuechanged, this,
                     &rohBeam::on_model_param_valuechanged);
    patchbay.connect(&tau_postfilter_ms.valuechanged, this,
                     &rohBeam::on_model_param_valuechanged);
    patchbay.connect(&tau_blocking_XkXi_ms.valuechanged, this,
                     &rohBeam::on_model_param_valuechanged);
    patchbay.connect(&tau_blocking_XkY_ms.valuechanged, this,
                     &rohBeam::on_model_param_valuechanged);
  }

  rohBeam::~rohBeam()
  {
    //delete initialized AC variables for export
    if ( beamExport != nullptr ) {
      delete beamExport;
    }
    if ( noiseModelExport != nullptr ) {
      delete noiseModelExport;
    }

  }


  /** Plugin preparation. This plugin checks that the input signal has the
   * spectral domain and contains at least one channel
   * @param signal_info
   *   Structure containing a description of the form of the signal (domain,
   *   number of channels, frames per block, sampling rate.
   */
  void rohBeam::prepare(mhaconfig_t & signal_info)
  {
    if (signal_info.domain != MHA_SPECTRUM)
      throw MHA_Error(__FILE__, __LINE__,
                      "This plugin can only process spectrum signals.");

    if (signal_info.channels < 6)
      throw MHA_Error(__FILE__,__LINE__,
                      "This plugin requires at least six input channels.");

    //this would be a good time to prepare the fixed beamformer
    //for the moment, use the assumption of decorrelated noise
    signal_info.channels = 2;

    //tell the plugin that it's ok to prepare configurations
    prepared = true;

    /* remember the transform configuration (i.e. channel numbers): */
    tftype = signal_info;
    /* make sure that a valid runtime configuration exists: */
    update_cfg();
  }

  /* when one of the angles or radii changes, recompute the head model */
  void rohBeam::on_model_param_valuechanged() {

    //only push configurations if prepare has already been called
    if ( prepared ) update_cfg();
  }

  void rohBeam::update_cfg() {

    std::unique_ptr<MatrixXcf> headModel =[&](){
                                            MatrixXcf* headModel=nullptr;

                                            if ( prop_type.isval("hm1") ) {
                                              headModel = compute_head_model_mat( source_azimuth_degrees.data );
                                            } else if ( prop_type.isval("sampled") ) {

                                              int nfreq = input_cfg().fftlen/2+1;

                                              //load only fftlen frames of the hrir
                                              MHASndFile::sf_wave_t sndfile(sampled_hrir_path.data, 0, input_cfg().fftlen);

                                              if ( input_cfg().channels != sndfile.num_channels )
                                                {
                                                  throw MHA_Error(__FILE__, __LINE__,
                                                                  "Number of channels in sampled HRIR (%u) does not match number"
                                                                  " of input channels (%u).",
                                                                  sndfile.num_channels, input_cfg().channels);
                                                }
                                              if( input_cfg().fftlen != sndfile.num_frames )
                                                {
                                                  throw MHA_Error(__FILE__,__LINE__,
                                                                  "Number of frames in sampled HRIR (%u) does not match fft length (%u).",
                                                                  sndfile.num_frames,input_cfg().fftlen);
                                                }
                                              if( input_cfg().srate != sndfile.samplerate )
                                                {
                                                  throw MHA_Error(__FILE__,__LINE__,
                                                                  "Sample rate in sampled HRIR (%d) does not match MHA sample rate (%f).",
                                                                  sndfile.samplerate,input_cfg().srate);
                                                }

                                              unsigned int nchan = sndfile.num_channels;

                                              //get an fft handle, allocate a buffer, and transform into HRTF
                                              mha_fft_t fft = mha_fft_new(input_cfg().fftlen);
                                              MHASignal::spectrum_t spec(nfreq,nchan);
                                              mha_fft_wave2spec(fft, &sndfile, &spec, true);
                                              // normalize spectrum to rms level, to avoid numerical problems:
                                              mha_real_t hrir_level(0.0);
                                              for( unsigned int k=0;k<nchan;k++)
                                                hrir_level += rmslevel(spec,k,input_cfg().fftlen);
                                              spec *= (mha_real_t)nchan/hrir_level;

                                              //copy to the Eigen type
                                              headModel = new MatrixXcf(nfreq, nchan);
                                              for (int f=0; f<nfreq; f++) {
                                                for (unsigned int c=0; c<nchan; c++) {
                                                  (*headModel)(f,c) = stdcomplex( spec.value(f,c) );
                                                }
                                              }

                                            }
                                            // check head model for denormals:
                                            for(int kr=0;kr<headModel->rows();kr++)
                                              for(int kc=0;kc<headModel->cols();kc++)
                                                if( is_denormal((*headModel)(kr,kc)) )
                                                  throw MHA_Error(__FILE__,__LINE__,"headModel contains denormals at (%d,%d).",
                                                                  kr,kc);

                                            return std::unique_ptr<MatrixXcf>(headModel);
                                          }();

    auto delayC=std::unique_ptr<MHASignal::matrix_t>(compute_delaycomp_vec( headModel.get() ));
    //whether to do wng optimization is built into compute_beamW
    auto beamW=std::unique_ptr<MHASignal::matrix_t>(compute_beamW( headModel.get() ));

    if ( enable_export.data ) {
      //copy and export the beamformer
      export_beam_design( *beamW.get(), *headModel.get() );
    }

    rohConfig *lastConfig = peek_config();
    rohConfig *config;

    //adjusted the time constants to work by hopsize (despite ref not doing this)
    //as the exp. filters are updated each frame, this should be correct
    float frame_rate = float( input_cfg().srate ) / float( input_cfg().fragsize );
    float alpha_postfilter =
      exp(-1/( frame_rate * tau_postfilter_ms.data * 0.001));
    float alpha_blocking_XkXi =
      exp(-1/( frame_rate * tau_blocking_XkXi_ms.data * 0.001));
    float alpha_blocking_XkY =
      exp(-1/( frame_rate * tau_blocking_XkY_ms.data * 0.001));

    //static initialization of the additional options
    configOptions options = { enable_adaptive_beam.data,
                              (int)binaural_type.data.get_index(),
                              alpha_postfilter,
                              alpha_blocking_XkXi,
                              alpha_blocking_XkY };

    if ( lastConfig == nullptr ) {

      //create a new configuration with an initial state
      config = new rohConfig( input_cfg(), output_cfg(),
                              std::move(headModel),
                              std::move(beamW),
                              std::move(delayC),
                              options );
    } else {

      //copy temporal state (noise estimations) while changing the filter setup
      config = new rohConfig( lastConfig,
                              input_cfg(), output_cfg(),
                              std::move(headModel),
                              std::move(beamW),
                              std::move(delayC),
                              options );
    }
    push_config( config );
  }

  /* this is the delay compensation vector that adjusts delays before blocking */
  MHASignal::matrix_t * rohBeam::compute_delaycomp_vec(MatrixXcf *headModel)
  {
    int nchan = input_cfg().channels;
    int nfreq = input_cfg().fftlen/2+1;

    //allocate the matrix
    MHASignal::matrix_t * delayCompMat =
      new MHASignal::matrix_t(headModel->rows(), headModel->cols());

    //iterate over frequencies and microphones
    for (int m=0; m<nchan; m++) {
      for (int f=0; f<nfreq; f++) {
        mha_complex_t modelVal;
        modelVal.re = real( (*headModel)(f,m) );
        modelVal.im = imag( (*headModel)(f,m) );
        (*delayCompMat)(f,m) =_conjugate( modelVal ) / abs2( modelVal );
      }
    }

    return delayCompMat;
  }

  //integrate the hrtf over all source angles and use it as noise model
  std::vector<MatrixXcf> *rohBeam::noise_integrate_hrtf()
  {
    int nchan = input_cfg().channels;
    int nfreq = input_cfg().fftlen/2+1;

    std::vector<MatrixXcf> *ret = new std::vector<MatrixXcf>( nfreq, MatrixXcf::Constant(nchan,nchan,0) );

    int nAngles = 0;
    //iterate over all azimuth angles
    for (int a=-180; a<180; a++) {

      nAngles=nAngles+1;

      //compute the hrtf for a source coming from angle 'a'
      MatrixXcf *prop = compute_head_model_mat( (float) a );

      for (int f=0; f<nfreq; f++) {

        VectorXcf propF = prop->row(f);
        MatrixXcf outer = propF * propF.adjoint();

        ret->at(f) = ret->at(f) + outer;
      }

      delete prop;
      prop = nullptr;
    }

    for (int f=0; f<nfreq; f++) {

      ret->at(f) = ret->at(f) / (float) nAngles;

      //assume symmetric head: set imaginary parts to 0
      (ret->at(f) += ret->at(f).conjugate()) /= 2.0f;
    }

    return ret;
  }

  /* Implement design of a row of the beamforming filter based on MVDR recipe.
   * Either we add a fixed amount of identity to the noise matrix,
   * or we iteratively optimize the White Noise Gain measure,
   * (adding different amounts of identity for different frequencies.)
   * */
  VectorXcf rohBeam::solve_MVDR(VectorXcf propVec, MatrixXcf noiseM)
  {
    unsigned int nchan = input_cfg().channels;

    if ( enable_wng_optimization.data ) {

      MatrixXcf Id = MatrixXcf::Identity(nchan,nchan);
      MatrixXcf nId = MatrixXcf::Constant(nchan,nchan,1) - Id;

      MatrixXcf invNoiseM = (noiseM.array() * Id.array()).matrix().inverse();

      VectorXcf freqRes = (invNoiseM * propVec) / ( propVec.adjoint() * invNoiseM * propVec);

      float maxwng = compute_wng(freqRes, propVec);
      float mu_dB = -80;
      int count = 0;
      float wng = 0;

      float wng_dB = -20;

      while (wng<std::min(wng_dB,maxwng)) {
        mu_dB=mu_dB+1;
        float mu=pow(10,mu_dB/20);
        count = count+1;
        invNoiseM = ( noiseM.array() * (Id + nId / (1+mu)).array() )
          .matrix().inverse();
        freqRes = (invNoiseM * propVec) / ( propVec.adjoint() * invNoiseM * propVec );
        wng = compute_wng(freqRes, propVec);
      }
      for(int kr=0;kr<freqRes.rows();kr++)
        for(int kc=0;kc<freqRes.cols();kc++)
          if( is_denormal(freqRes(kr,kc)) )
            throw MHA_Error(__FILE__,__LINE__,"Denormal in freqRes(%d,%d) (solve_MVDR,wng).",kr,kc);
      return freqRes;
    }
    else {
      //diagonal loading: to improve the condition/white noise gain
      noiseM += diag_loading_mu.data * MatrixXcf::Identity(nchan,nchan);

      //solution1: without matrix inversion
      VectorXcf freqRes = noiseM.fullPivLu().solve(propVec);

      for(int kr=0;kr<freqRes.rows();kr++)
        for(int kc=0;kc<freqRes.cols();kc++)
          if( is_denormal(freqRes(kr,kc)) )
            throw MHA_Error(__FILE__,__LINE__,"Denormal in freqRes(%d,%d) (solve_MVDR, before division,).",kr,kc);

      //divide the solution by denominator
      // fixme: this line causes denormals!
      auto d=propVec.adjoint() * freqRes;
      std::complex<float> denominator(scalarify(d));
      if( abs(denominator) > 0 )
        freqRes = freqRes / denominator;

      for(int kr=0;kr<freqRes.rows();kr++)
        for(int kc=0;kc<freqRes.cols();kc++)
          if( is_denormal(freqRes(kr,kc)) )
            throw MHA_Error(__FILE__,__LINE__,"Denormal in freqRes(%d,%d) (solve_MVDR).",kr,kc);
      return freqRes;
    }
  }

  MHASignal::matrix_t * rohBeam::compute_beamW(MatrixXcf *headModel)
  {
    unsigned int nfreq = input_cfg().fftlen/2+1;
    unsigned int nchan = input_cfg().channels;
    unsigned int fftlen = input_cfg().fftlen;
    unsigned int srate = input_cfg().srate;

    //poll the latest noise_field_model from current configuration
    int noise_option = noise_field_model.data.get_index();

    std::unique_ptr<MHASignal::matrix_t> beamW(new MHASignal::matrix_t(nfreq, nchan));

    //function pointer for noise model
    noiseFuncPtr nModel = get_noise_model_func();

    switch(noise_option) {
    case 0:
      //just copy headModel as a MHASignal::matrix_t
      //don't forget to normalize by number of microphones
      for (unsigned int c=0; c<nchan; c++) {
        for (unsigned int f=0; f<nfreq; f++) {
          ::set( (*beamW)(f,c), (*headModel)(f,c) / (float) nchan );
        }
      }
      break;
    case 1:
    case 2:

      for (unsigned int f=0; f<nfreq; f++) {

        float w = f * srate / fftlen * 2 * M_PI;

        //this is the noise correlation matrix from the noise model
        MatrixXf noiseM = (*this.*nModel)( w );

        VectorXcf propVec = headModel->row(f);
        VectorXcf freqRes = solve_MVDR(propVec, noiseM.cast<std::complex<float> >() );

        //copy to beamW
        for (unsigned int c=0; c<nchan; c++) {
          ::set( (*beamW)(f,c), freqRes[c] );
        }
      }

      break;
    case 3: //integrated hrtf option

      {
        std::vector <MatrixXcf> *noise = noise_integrate_hrtf();

        for (unsigned int f=0; f<nfreq; f++) {

          MatrixXcf noiseM = noise->at(f);
          VectorXcf propVec = headModel->row(f);
          VectorXcf freqRes = solve_MVDR(propVec, noiseM);

          //copy to beamW
          for (unsigned int c=0; c<nchan; c++) {
            ::set( (*beamW)(f,c), freqRes[c] );
          }
        }

        delete noise;
      }

      break;
    default:
      throw MHA_Error(__FILE__, __LINE__,
                      "Noise model option out of selected range.");
    }
    // check for NaN or Inf:
    for(unsigned int f=0;f<nfreq;f++)
      for(unsigned int c=0;c<nchan;c++){
        if( is_denormal((*beamW)(f,c)) )
          throw MHA_Error(__FILE__,__LINE__,"Beam weights contain denormals (bin %u, channel %u).",
                          f,c);
      }
    return beamW.release();
  }


  //to make a one-line routine for computing wng
  float rohBeam::compute_wng(VectorXcf freqRes, VectorXcf propVec) {
    float wngNom = pow( abs(std::complex<float>(scalarify(freqRes.adjoint()*propVec))), 2 );
    float wngDen = real( std::complex<float>(scalarify(freqRes.adjoint()*freqRes)));
    return 10*log10( wngNom / wngDen );
  }

  //for integrating hrtfs, we need to compute the hrtf at all angles
  MatrixXcf * rohBeam::compute_head_model_mat(float src_az_degrees) {

    mha_complex_t numer, denom, result1, result2;
    int nchan = input_cfg().channels;
    int nfreq = input_cfg().fftlen/2+1;
    int fftlen = input_cfg().fftlen;
    mha_real_t srate = input_cfg().srate;

    MatrixXcf * headModelMat = new MatrixXcf(nfreq, nchan);

    float w0 = CONST_C / (head_model_sphere_radius_cm.data / 100.0f);

    numer.re = 1;
    denom.re = 1;

    //iterate over frequencies and microphones
    for (int m=0; m<nchan; m++) {

      //compute T and a, which are based on angle difference
      //float s1 = source_azimuth_degrees.data;
      //float s2 = mic_azimuth_degrees_vec.data[m];
      float angle_diff = src_az_degrees - mic_azimuth_degrees_vec.data[m];

      if ( angle_diff > 180 ) {
        //if angle_diff > 180, go the other way around
        angle_diff -= 360;
      } else if ( angle_diff < -180 ) {
        //if the angle is too negative, add 360
        angle_diff += 360;
      }

      float angle_diff_rads = angle_diff * M_PI / (float) 180;
      float T = compute_head_model_T( angle_diff_rads );
      float alpha = compute_head_model_alpha( angle_diff_rads );

      float numer_factor = alpha / (2 * w0);

      for (int f=0; f<nfreq; f++) {

        float w = f * srate / fftlen * 2 * M_PI;

        numer.im = numer_factor * w;
        denom.im = w / (2 * w0);

        result1 = numer / denom;
        result2.re = 1;
        result2.im = 0;
        result2 = expi(result2, -w * T,1);

        result2 = result1 * result2;

        if( is_denormal(result2) )
          throw MHA_Error(__FILE__,__LINE__,"Denormal in head model (f=%d).",f);
        (*headModelMat)(f,m) = stdcomplex( result2 );
      }
    }

    return headModelMat;
  }

  //accepts the angle difference in radians
  float rohBeam::compute_head_model_T(float angle_rads) {
    float angle_abs = fabs(angle_rads);
    float rdivc = (head_model_sphere_radius_cm.data / 100.0f) / CONST_C;

    //cases need to be in radians
    if ( (0 <= angle_abs) && (angle_abs < M_PI/2.0) ) {
      return rdivc * cos( angle_rads );
    }
    else if ( M_PI/2.0 <= angle_abs && angle_abs < M_PI ) {
      return rdivc * ( angle_abs - M_PI/2 );
    }
    else  {
      throw MHA_Error(__FILE__, __LINE__,
                      "Difference angle out of expected range.");
    }
  }

  //accepts the angle difference in radians
  float rohBeam::compute_head_model_alpha(float angle_rads) {
    float a_min = 0.1f;
    float theta_min_rads = 150.0 / 180.0 * M_PI;
    float a_ret = (1 + a_min/2.0f) +
      (1 - a_min/2.0f) * cos( (angle_rads/theta_min_rads) * M_PI );
    return a_ret;
  }

  const MatrixXf rohBeam::compute_uncorr(float w) {

    //this function just returns identity
    //allocate an eigen matrix
    int ch_in = input_cfg().channels;
    MatrixXf m( ch_in, ch_in );

    for (int i=0; i<ch_in; i++) {
      for (int k=0; k<ch_in; k++) {
        m(i,k) = (i==k);
      }
    }

    return m;

  }

  const MatrixXf rohBeam::compute_diff2D(float w) {
    //allocate an eigen matrix
    int ch_in = input_cfg().channels;
    MatrixXf m( ch_in, ch_in );

    for (int i=0; i<ch_in; i++) {
      for (int k=0; k<ch_in; k++) {

        //implementation with Bessel functions of first kind
        float arg = w * (intermic_distance_cm.data[i][k]/100.0f) / CONST_C;
        m(i,k) = /*rohBeam::*/j0(arg);
        if( is_denormal(m(i,k) ) )
          throw MHA_Error(__FILE__,__LINE__,
                          "Denormals in diff2D noise model (%d,%d,w=%g,arg=%g).",
                          i,k,w,arg);
      }
    }

    return m;
  }

  const MatrixXf rohBeam::compute_diff3D(float w) {

    //allocate an eigen matrix
    MatrixXf m( input_cfg().channels, input_cfg().channels );

    for (unsigned int i=0; i<input_cfg().channels; i++) {
      for (unsigned int k=0; k<input_cfg().channels; k++) {

        //implementation with sinc functions
        float arg = w * (intermic_distance_cm.data[i][k]/100.0f) / CONST_C;
        if ( fabs(arg) < 0.000001 ) { //arg zero
          m(i,k) = 1.0f;
        } else {
          m(i,k) = sin( arg ) / arg;
        }
      }
    }

    return m;
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
  mha_spec_t * rohBeam::process(mha_spec_t * signal)
  {
    poll_config();
    return cfg->process(signal);
  }

  void rohBeam::export_beam_design( const MHASignal::matrix_t & beamW,
                                    const MatrixXcf &headModel )
  {
    unsigned int nfreq = beamW.size(0);
    unsigned int nchan = beamW.size(1);
    unsigned int fftlen = input_cfg().fftlen;
    unsigned int srate = input_cfg().srate;

    //assume export variables are allocated as a group
    if ( beamExport != nullptr ) {
      if ( beamExport->num_channels != nchan ||
           beamExport->num_frames != nfreq ) {

        //we must reallocate, for the size has changed
        delete beamExport;
        delete noiseModelExport;
        delete propExport;

        beamExport = nullptr;
        noiseModelExport = nullptr;
        propExport = nullptr;
      }
    }

    if ( beamExport == nullptr ) {
      beamExport = new MHA_AC::spectrum_t(ac, "beamExport", nfreq, nchan, false);
      noiseModelExport = new MHA_AC::waveform_t(ac, "noiseModelExport", nfreq, nchan*nchan, false);
      propExport = new MHA_AC::spectrum_t(ac, "propExport", nfreq, nchan, false);
    }

    //copy from the filter
    for (unsigned int f=0; f<nfreq; f++) {
      for (unsigned int c=0; c<nchan; c++) {
        beamExport->value(f,c) = beamW(f,c);
      }
    }

    //copy the noise model, blockwise across mic channels into export spectrum
    int noise_option = noise_field_model.data.get_index();

    if ( noise_option == 3 ) {

      std::vector <MatrixXcf> *noise = noise_integrate_hrtf();

      for (unsigned int f=0; f<nfreq; f++) {

        MatrixXcf noiseM = noise->at(f);

        for (unsigned int i=0; i<nchan; i++) {
          for (unsigned int k=0; k<nchan; k++) {

            //we take the real part, already done in the integration
            noiseModelExport->value(f,i*nchan+k) = real( noiseM(i,k) );
          }
        }
      }
      delete noise;

    } else {

      noiseFuncPtr nModel = get_noise_model_func();
      for (unsigned int f=0; f<nfreq; f++) {

        float w = f * srate / fftlen * 2 * M_PI;
        MatrixXf noiseM = (*this.*nModel)( w );

        for (unsigned int i=0; i<nchan; i++) {
          for (unsigned int k=0; k<nchan; k++) {
            noiseModelExport->value(f,i*nchan+k) = noiseM(i,k);
          }
        }
      }

    } //end of old noise models

    //copy the propogation vector
    for (unsigned int f=0; f<nfreq; f++) {
      for (unsigned int c=0; c<nchan; c++) {
        ::set( propExport->value(f,c), headModel(f,c) );
      }
    }

    beamExport->insert();
    noiseModelExport->insert();
    propExport->insert();
  }

  rohBeam::noiseFuncPtr rohBeam::get_noise_model_func()
  {
    //poll the latest noise_field_model from current configuration
    int noise_option = noise_field_model.data.get_index();
    //function pointer for noise model
    noiseFuncPtr nModel = nullptr;

    //compute the beamformer with diff2D/diff3D noise model
    if ( noise_option == 0 )
      nModel = &rohBeam::compute_uncorr;
    else if ( noise_option == 1 )
      nModel = &rohBeam::compute_diff2D;
    else
      nModel = &rohBeam::compute_diff3D;

    return nModel;
  }
} //namespace rohBeam
/*
 * This macro connects the plugin1_t class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(rohBeam,rohBeam::rohBeam,spec,spec)

/*
 * This macro creates code classification of the plugin and for
 * automatic documentation.
 *
 * The first argument to the macro is a space separated list of
 * categories, starting with the most relevant category. The second
 * argument is a LaTeX-compatible character array with some detailed
 * documentation of the plugin.
 */
MHAPLUGIN_DOCUMENTATION(rohBeam,
                        "beamforming binaural",
                        "Implements the binaural beamformer found in the PhD thesis of Thomas Rohdenburg. "
                        "Please see Chpt. 4,\"Multi-Channel Noise Reduction Schemes with Binaural Output"
                        "- Performance Evaluation and Optimization\" for an overview.\n\n"

                        "Rohdenburg's strategy is to use standard mono beamforming techniques, "
                        "but to use the mono output to estimate a linear time-varying filter that "
                        "approximately equalizes PSD of the reference input channels to the beamformer output. "

                        "This beamformer is implemented as a spectral domain plugin in MHA. "
                        "This algorithm has three main parts:\n\n"

                        "1. Fixed monoaural superdirectional beamformer based on "
                        "Minimum Variance Distortionless Response (MVDR). "
                        "This uses three ingredients to determine the fixed filter spectrum in each input channels:\n\n"
                        "a. Azimuth angle of the speech source relative to the listener.\n\n"
                        "b. Propogation vector that models the transfer function from source to each mic. "
                        "There are two options, head model HM1, or loading a HRTF from a WAV file.\n\n"
                        "c. Noise cross-correlation matrix, allows superdirectivity.\n\n"
                        "d. To avoid overamplification of noise in MVDR, "
                        "you can either use a fixed diagonal loading constant or an iterative per-frequency "
                        "WNG limited algorithm to choose the loading constants.\n\n"
                        "2. Adaptive Generalized Sidechain (GSC) adaptive beamformer.\n\n"
                        "This step estimates cross-correlations of output of (1) with inputs,"
                        "using a blocking strategy to distinguish signal from noise. "
                        "This component probably doesn't work as expected and might be dangerous; "
                        "please disable this unless you really know what you are doing.\n\n"
                        "3. Binaural output adaptation:\n\n"
                        "a. The preferred strategy, Binaural Postfilter, "
                        "estimates PSD of mono beamformed output and reference LR channels, "
                        "then chooses and applies an equalizing filter that splits the difference between LR channels.\n\n"
                        "b. One of the other strategies, phase reconstruction, is implemented for comparison.\n\n"
                        "c. It is also an option to simply output the monaural beamformer output.\n\n"
                        "d. The third strategy, bilateral beamforming is not supported, but can be easily "
                        "implemented by configuring two rohBeams.\n\n"

                        "Plausible features not implemented (but could be added) are:\n\n"
                        "a. Free Field for propogation vector. However you could do this by rendering your own "
                        "arbitrary HRTF and loading it via the \"sampled\" option for the propogation vector.\n\n"
                        "b. Null directions for the MVDR recipe."
                        )
