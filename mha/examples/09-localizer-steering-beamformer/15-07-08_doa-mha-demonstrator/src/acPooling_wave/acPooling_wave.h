#ifndef ACPOOLING_WAVE_H
#define ACPOOLING_WAVE_H

#include "mha_plugin.hh"

class acPooling_wave;

//runtime config
class acPooling_wave_config {

public:
    acPooling_wave_config(algo_comm_t &ac, const mhaconfig_t in_cfg, acPooling_wave *_pooling);
    ~acPooling_wave_config();

    mha_wave_t* process(mha_wave_t*);

    //declare data necessary for processing state here
    algo_comm_t &ac;
    std::string raw_p_name;
    MHA_AC::waveform_t p;
    MHA_AC::int_t p_max;
    MHA_AC::float_t like_ratio;
    mha_wave_t c;
    unsigned int pooling_ind;
    unsigned int pooling_option;
    unsigned int pooling_size;
    float up_thresh;
    float low_thresh;
    int neigh;
    float alpha;
    MHASignal::waveform_t pool;

};

class acPooling_wave : public MHAPlugin::plugin_t<acPooling_wave_config> {

public:
    acPooling_wave(algo_comm_t & ac,const std::string & chain_name,
                   const std::string & algo_name);
    ~acPooling_wave();
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release(void) {/* Do nothing in release */}

    //declare MHAParser variables here
    MHAParser::int_t numsamples;
    MHAParser::int_t pooling_wndlen;
    MHAParser::kw_t pooling_type;
    MHAParser::float_t upper_threshold;
    MHAParser::float_t lower_threshold;
    MHAParser::int_t neighbourhood;
    MHAParser::float_t alpha;
    MHAParser::string_t p_name;
    MHAParser::string_t pool_name;
    MHAParser::string_t max_pool_ind_name;
    MHAParser::string_t like_ratio_name;

private:
    void update_cfg();

    /* patch bay for connecting configuration parser
       events with local member functions: */
    MHAEvents::patchbay_t<acPooling_wave> patchbay;

};

#endif // ACPOOLING_WAVE_H

