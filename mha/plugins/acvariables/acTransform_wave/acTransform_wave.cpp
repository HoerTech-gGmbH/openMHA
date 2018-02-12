/*
 * The rotation angle comes from another plugin written by Giso Grimm. It basically measures the head rotation angle and this plugin compensates for it.
 */

#include "acTransform_wave.h"

#define PATCH_VAR(var) patchbay.connect(&var.valuechanged, this, &acTransform_wave::update_cfg)
#define INSERT_PATCH(var) insert_member(var); PATCH_VAR(var)

acTransform_wave_config::acTransform_wave_config(algo_comm_t &ac, const mhaconfig_t in_cfg, acTransform_wave *_transform):
    ac(ac),
    ang_name(_transform->ang_name.data),
    raw_p_name(_transform->raw_p_name.data),
    raw_p_max_name(_transform->raw_p_max_name.data),
    rotated_p(ac, _transform->rotated_p_name.data.c_str(), _transform->numsamples.data, 1, true),
    rotated_i(ac, _transform->rotated_p_max_name.data.c_str(), _transform->numsamples.data / 2),
    offset(_transform->numsamples.data / 2),
    resolution(ceil(360 / _transform->numsamples.data))
{
    //initialize plugin state for a new configuration
    if (_transform->to_from.data)
        to_from = 1;
    else
        to_from = -1;
}

acTransform_wave_config::~acTransform_wave_config() {}

//the actual processing implementation
mha_wave_t *acTransform_wave_config::process(mha_wave_t *wave)
{
    //do actual processing here using configuration state
    unsigned int ang_i = 0;
    unsigned int offset_i = 0;

    // get the waveform to be rotated
    const mha_wave_t raw_p = MHA_AC::get_var_waveform(ac, raw_p_name.c_str());

    // get the maximum of the waveform to be rotated
    const int raw_i = MHA_AC::get_var_int(ac, raw_p_max_name.c_str());

    // get the rotation angle
    const float ang = MHA_AC::get_var_float(ac, ang_name.c_str());
    ang_i = (unsigned int)(floor(to_from * ang / resolution) + raw_p.num_frames / 2) % raw_p.num_frames;

     // Add the new raw p values into the pool
    for( unsigned int i = 0; i < raw_p.num_frames; ++i ) {

        offset_i = (ang_i - offset + i + raw_p.num_frames) % raw_p.num_frames;
        rotated_p(offset_i, 0) = value(raw_p, i, 0);

        if (i == (unsigned int)raw_i)
            rotated_i.data = offset_i;

    }


    //return current fragment
    return wave;
}

/** Constructs our plugin. */
acTransform_wave::acTransform_wave(algo_comm_t & ac,
                                   const std::string & chain_name,
                                   const std::string & algo_name)
    : MHAPlugin::plugin_t<acTransform_wave_config>("Transform Plugin Between Coordinate Systems for Waveforms",ac)
    , ang_name("This parameter has the name of the AC variable having the rotation angle", "head_ang")
    , raw_p_name("This parameter has the name of the AC variable having the waveform to be rotated", "p")
    , raw_p_max_name("This parameter has the name of the AC variable having the maximum of the waveform to be rotated", "p_max")
    , rotated_p_name("This parameter has the name of the AC variable having the waveform after rotation", "rotated_p")
    , rotated_p_max_name("This parameter has the name of the AC variable having the maximum of the waveform after rotation", "rotated_p_max")
    , numsamples("This parameter determines the length of the wave to be pooled in samples.", "73", "]0, 360]")
    , to_from("This parameter tells whether the rotation will be performed to the given angle or from it", "yes")
{
    //add parser variables and connect them to methods here
    //INSERT_PATCH(foo_parser);

    // make the plug-in findable via "?listid"
    set_node_id(algo_name);

    INSERT_PATCH(ang_name);
    INSERT_PATCH(raw_p_name);
    INSERT_PATCH(raw_p_max_name);
    INSERT_PATCH(rotated_p_name);
    INSERT_PATCH(rotated_p_max_name);
    INSERT_PATCH(numsamples);
    INSERT_PATCH(to_from);
}

acTransform_wave::~acTransform_wave() {}

/** Plugin preparation.
 *  An opportunity to validate configuration parameters before instantiating a configuration.
 * @param signal_info
 *   Structure containing a description of the form of the signal (domain,
 *   number of channels, frames per block, sampling rate.
 */
void acTransform_wave::prepare(mhaconfig_t & signal_info)
{
    //good idea: restrict input type and dimension
    /*
    if (signal_info.channels != 2)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin must have 2 input channels: (%d found)\n"
                        "[Left, Right].", signal_info.channels);
    */

    if (signal_info.domain != MHA_WAVEFORM)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin can only process spectrum signals.");


    /* make sure that a valid runtime configuration exists: */
    update_cfg();
}

void acTransform_wave::update_cfg()
{
    if ( is_prepared() ) {

        //when necessary, make a new configuration instance
        //possibly based on changes in parser variables
        acTransform_wave_config *config;
        config = new acTransform_wave_config( ac, input_cfg(), this);
        push_config( config );
    }
}

/**
 * Checks for the most recent configuration and defers processing to it.
 */
mha_wave_t * acTransform_wave::process(mha_wave_t * signal)
{
    //this stub method defers processing to the configuration class
    return poll_config()->process( signal );
}

/*
 * This macro connects the plugin1_t class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(acTransform_wave,acTransform_wave,wave,wave)

/*
 * This macro creates code classification of the plugin and for
 * automatic documentation.
 *
 * The first argument to the macro is a space separated list of
 * categories, starting with the most relevant category. The second
 * argument is a LaTeX-compatible character array with some detailed
 * documentation of the plugin.
 */
MHAPLUGIN_DOCUMENTATION(acTransform_wave,
        "analysis",
        "This plugin transforms a waveform in the AC space from one coordinate system into another.  For this it receives an angle also saved in the AC space. Then, the plugin rotates the axes into the direction of the given angle."
        )

