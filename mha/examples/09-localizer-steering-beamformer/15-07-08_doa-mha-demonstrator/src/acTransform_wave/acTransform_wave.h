#ifndef ACTRANSFORM_WAVE_H
#define ACTRANSFORM_WAVE_H

#include "mha_plugin.hh"

class acTransform_wave;

//runtime config
class acTransform_wave_config {

public:
    acTransform_wave_config(algo_comm_t &ac, const mhaconfig_t in_cfg, acTransform_wave *_transform);
    ~acTransform_wave_config();

    mha_wave_t* process(mha_wave_t*);

    //declare data necessary for processing state here
    algo_comm_t &ac;
    std::string ang_name;
    std::string raw_p_name;
    std::string raw_p_max_name;
    MHA_AC::waveform_t rotated_p;
    MHA_AC::int_t rotated_i;

    unsigned int offset;
    unsigned int resolution;
    unsigned int to_from;

};

class acTransform_wave : public MHAPlugin::plugin_t<acTransform_wave_config> {

public:
    acTransform_wave(algo_comm_t & ac,const std::string & chain_name,
                     const std::string & algo_name);
    ~acTransform_wave();
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release(void) {/* Do nothing in release */}

    //declare MHAParser variables here
    MHAParser::string_t ang_name;
    MHAParser::string_t raw_p_name;
    MHAParser::string_t raw_p_max_name;
    MHAParser::string_t rotated_p_name;
    MHAParser::string_t rotated_p_max_name;
    MHAParser::int_t numsamples;
    MHAParser::bool_t to_from;

private:
    void update_cfg();

    /* patch bay for connecting configuration parser
       events with local member functions: */
    MHAEvents::patchbay_t<acTransform_wave> patchbay;

};

#endif // ACTRANSFORM_WAVE_H

