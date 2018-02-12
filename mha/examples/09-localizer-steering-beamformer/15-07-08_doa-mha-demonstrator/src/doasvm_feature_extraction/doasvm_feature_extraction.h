#ifndef DOASVM_FEATURE_EXTRACTION_H
#define DOASVM_FEATURE_EXTRACTION_H

#include "mha_plugin.hh"
#include <mha_toolbox.h>

class doasvm_feature_extraction;

//runtime config
class doasvm_feature_extraction_config {

public:
    doasvm_feature_extraction_config(algo_comm_t &ac, const mhaconfig_t in_cfg, doasvm_feature_extraction *_doagcc);
    ~doasvm_feature_extraction_config();

    mha_wave_t* process(mha_wave_t*);

    //declare data necessary for processing state here
    doasvm_feature_extraction *doagcc;

    unsigned int wndlen;
    unsigned int fftlen;
    unsigned int G_length;
    unsigned int GCC_start;
    unsigned int GCC_end;

    MHA_AC::waveform_t vGCC_ac;

    mha_fft_t fft;
    mha_fft_t ifft;

    double hifftwin_sum;

    MHASignal::waveform_t proc_wave;
    MHASignal::waveform_t hwin;
    MHASignal::waveform_t hifftwin;
    MHASignal::waveform_t vGCC;

    MHASignal::spectrum_t in_spec;
    MHASignal::spectrum_t G;

};

class doasvm_feature_extraction : public MHAPlugin::plugin_t<doasvm_feature_extraction_config> {

public:
    doasvm_feature_extraction(algo_comm_t & ac,const std::string & chain_name,
                              const std::string & algo_name);
    ~doasvm_feature_extraction();
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release(void) {/* Do nothing in release */}

    //declare MHAParser variables here
    MHAParser::int_t fftlen;
    MHAParser::int_t max_lag;
    MHAParser::int_t nupsample;
    MHAParser::string_t vGCC_name;

private:
    void update_cfg();

    /* patch bay for connecting configuration parser
       events with local member functions: */
    MHAEvents::patchbay_t<doasvm_feature_extraction> patchbay;

};

#endif // DOASVM_FEATURE_EXTRACTION_H

