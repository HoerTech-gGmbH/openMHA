#ifndef SHYNKCONFIG_H
#define SHYNKCONFIG_H

#include "mha_plugin.hh"

class adaptive_filter_t {

public:
  adaptive_filter_t(algo_comm_t &, const mhaconfig_t,
                    int lenOldSamps, bool doCircularComp,
                    float mu, float alp, bool);

  //this configuration copies filter state, dimensions must stay the same
  adaptive_filter_t(adaptive_filter_t &old,
                    bool doCircularComp, float mu,
                    float alp, bool useVAD);

  ~adaptive_filter_t()=default;
  mha_wave_t* process(mha_wave_t*);


private:

  void insert();

  algo_comm_t ac;
  mhaconfig_t in_cfg;

  unsigned int lenOldSamps;
  unsigned int lenNewSamps;
  unsigned int bufSize;
  float frac_old;

  mha_fft_t mha_fft;

  unsigned int nfreq;
  unsigned int nchan;

  unsigned int desired_chan;

  bool doCircularComp;

  float mu;
  float alp;

  bool useVAD;

  MHA_AC::waveform_t x;
  MHA_AC::spectrum_t X;
  MHA_AC::spectrum_t W;
  MHA_AC::spectrum_t Y;
  MHA_AC::waveform_t y;
  MHA_AC::waveform_t d;
  MHA_AC::waveform_t e;
  MHA_AC::spectrum_t E;
  MHA_AC::spectrum_t E2;
  MHA_AC::waveform_t grad;
  MHA_AC::spectrum_t Grad;
  MHA_AC::waveform_t e_out;
  MHA_AC::waveform_t P;
  MHA_AC::waveform_t Psum;

  MHA_AC::waveform_t ref;
  MHA_AC::spectrum_t Xinit;

};



class adaptive_filter_if_t : public MHAPlugin::plugin_t<adaptive_filter_t> {

public:
  adaptive_filter_if_t(algo_comm_t & ac,const std::string & chain_name,
                       const std::string & algo_name);
  ~adaptive_filter_if_t()=default;
  mha_wave_t* process(mha_wave_t*);
  void prepare(mhaconfig_t&);
  void release(void) {/* Do nothing in release */}

private:
  void update_cfg();

  /* patch bay for connecting configuration parser
     events with local member functions: */
  MHAEvents::patchbay_t<adaptive_filter_if_t> patchbay;

  MHAParser::int_t lenOldSamps;
  MHAParser::bool_t doCircularComp;
  MHAParser::float_t mu;
  MHAParser::float_t alp;
  MHAParser::bool_t useVAD;

  bool prepared;

  void on_model_param_valuechanged();

};

#endif // TIMOSMOOTH_H
