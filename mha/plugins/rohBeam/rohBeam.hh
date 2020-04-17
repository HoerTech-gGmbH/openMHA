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

/*
 * Implements the binaural beamforming algorithm described in
 * Development and Objective Perceptual Quality
 * Assessment of Monaural and Binaural Noise
 * Reduction Schemes for Hearing Aids,
 * PhD thesis from Thomas Rohdenburg
 */

#ifndef ROHBEAM_HH
#define ROHBEAM_HH

#include "mha_plugin.hh"
#include "mhasndfile.h"
#define NDEBUG //supposed to speed up Eigen

// Need to disable Wduplicated-branches via pragma as opposed to
// command line because the unit test runner needs this disabled too
#include <eigen3/Eigen/Dense>

#include <cmath>
#include <memory>
#include <fstream>

namespace rohBeam {

  /// Cylindrical bessel function of the first kind of order 0
  /// @param x the argument of the function
  /// @returns j0(x)
  double j0(double x);

  //a constant for the speed of sound m/s at 10C in air
  //constexpr float CONST_C   337.31f
  //speed of sound as used in Rohdenburg reference implementation
  constexpr float CONST_C = 343.0115f;
  constexpr int refL = 0;
  constexpr int refR = 3;
  auto scalarify=[](auto t){return t(0);};
  struct configOptions {
    bool enable_adaptive_beam;
    int binaural_type_index;
    float alpha_postfilter;
    float alpha_blocking_XkXi;
    float alpha_blocking_XkY;
  };

  class rohConfig {
  public:
    rohConfig(const mhaconfig_t in_cfg,const mhaconfig_t out_cfg,
              std::unique_ptr<Eigen::MatrixXcf> headModel_,
              std::unique_ptr<MHASignal::matrix_t> beamW_,
              std::unique_ptr<MHASignal::matrix_t> delayComp_,
              const configOptions& options);

    //this one copies state from previous
    rohConfig(rohConfig *lastConfig,
              const mhaconfig_t,const mhaconfig_t out_cfg,
              std::unique_ptr<Eigen::MatrixXcf> headModel_,
              std::unique_ptr<MHASignal::matrix_t> beamW_,
              std::unique_ptr<MHASignal::matrix_t> delayComp_,
              const configOptions& options);

    ~rohConfig();
    rohConfig(const rohConfig&)=delete;
    rohConfig& operator=(const rohConfig&)=delete;
    mha_spec_t* process(mha_spec_t*);
    void init_dynamic();

  private:

    void phasereconstruction(MHASignal::spectrum_t*);
    void postfilter(mha_spec_t *,MHASignal::spectrum_t*);
    void copyfixedbfoutput(MHASignal::spectrum_t*);
    int nfreq;
    int nchan_block;

    mhaconfig_t in_cfg;
    mhaconfig_t out_cfg;
    bool enable_adaptive_beam;
    int binaural_type_index;


    //the prop vector
    std::unique_ptr<Eigen::MatrixXcf> headModel;

    // the beamforming matrix in MHA complex form
    std::unique_ptr<MHASignal::matrix_t> beamW;
    std::unique_ptr<MHASignal::matrix_t> delayComp;

    //
    MHASignal::spectrum_t *beam1; //output of beamforming1, Y_f
    MHASignal::spectrum_t *beamA; //output of adaptive beamforming, Y_a
    MHASignal::spectrum_t *blockSpec; //blocked spectrum, X'
    MHASignal::spectrum_t *outSpec;

    float alpha_postfilter;
    float alpha_blocking_XkXi;
    float alpha_blocking_XkY;

    /* Eigen matrices for recursive estimation of the noise characteristics
     * of blocking and fixed beamforming residual output */
    std::vector<Eigen::MatrixXcf> corrXpXp;
    std::vector<Eigen::VectorXcf> corrXpYf;

    //power spectral densities for binaural postfilter
    Eigen::VectorXf corrZZ;
    Eigen::VectorXf corrLL;
    Eigen::VectorXf corrRR;

    //all below is volatile and not important

    //buffer to allocate (in construction) for matrix decomposition
    //use a householder QR decomposition,
    //which seems to be fastest (from Eigen docs) w/o special matrix structure
    Eigen::HouseholderQR<Eigen::MatrixXcf> hhCorrXpXp;

    Eigen::VectorXcf nextXpYf;
    Eigen::VectorXcf blockXp;

    Eigen::VectorXcf freqResp;
    Eigen::ArrayXf magResp;

    float minLim;
    float maxLim;
  };



  class rohBeam : public MHAPlugin::plugin_t<rohConfig> {

  public:
    rohBeam(algo_comm_t & ac,const std::string & chain_name,
            const std::string & algo_name);
    ~rohBeam();
    mha_spec_t* process(mha_spec_t*);
    void prepare(mhaconfig_t&);
    void release(void) {/* Do nothing in release */}

  private:
    void update_cfg();

    float compute_head_model_T(float);
    float compute_head_model_alpha(float);
    Eigen::MatrixXcf * compute_head_model_mat(float src_az_degrees);
    MHASignal::matrix_t * compute_delaycomp_vec(Eigen::MatrixXcf *headModel);
    std::vector<Eigen::MatrixXcf> * noise_integrate_hrtf();
    Eigen::VectorXcf solve_MVDR(Eigen::VectorXcf propVec, Eigen::MatrixXcf noiseM);

    const Eigen::MatrixXf compute_uncorr(float w);
    const Eigen::MatrixXf compute_diff2D(float);
    const Eigen::MatrixXf compute_diff3D(float);

    MHASignal::matrix_t * compute_beamW(Eigen::MatrixXcf*);
    float compute_wng(Eigen::VectorXcf freqRes, Eigen::VectorXcf propVec);
    void export_beam_design( const MHASignal::matrix_t & beamW,const Eigen::MatrixXcf &headModel );

    typedef const Eigen::MatrixXf (rohBeam::*noiseFuncPtr) (float);
    noiseFuncPtr get_noise_model_func(void);

    MHAParser::kw_t prop_type;
    MHAParser::string_t sampled_hrir_path;

    MHAParser::float_t source_azimuth_degrees;
    MHAParser::vfloat_t mic_azimuth_degrees_vec;
    MHAParser::float_t head_model_sphere_radius_cm;
    MHAParser::mfloat_t intermic_distance_cm;
    MHAParser::kw_t noise_field_model;
    MHAParser::bool_t enable_adaptive_beam;
    MHAParser::kw_t binaural_type;

    MHAParser::float_t diag_loading_mu;
    MHAParser::bool_t enable_export;
    MHAParser::bool_t enable_wng_optimization;
    MHAParser::float_t tau_postfilter_ms;
    MHAParser::float_t tau_blocking_XkXi_ms;
    MHAParser::float_t tau_blocking_XkY_ms;

    /* patch bay for connecting configuration parser
       events with local member functions: */
    MHAEvents::patchbay_t<rohBeam> patchbay;

    bool prepared;

    //for exporting beamforming design
    MHA_AC::spectrum_t * beamExport;
    MHA_AC::waveform_t * noiseModelExport; //change to spectrum if we need complex PSD
    MHA_AC::spectrum_t * propExport;

    void on_model_param_valuechanged();

  };

}
#endif // ROHBEAM_HH
