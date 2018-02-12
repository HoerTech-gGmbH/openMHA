/*
 * This plugin receives a waveform through the ac space at each iteration. Depending on the averaging method, the last consecutive n frames are averaged and the computed average is send back to the ac space. Also, the maximum of the computed average is written to the ac space.
 */

#include "acPooling_wave.h"

#define PATCH_VAR(var) patchbay.connect(&var.valuechanged, this, &acPooling_wave::update_cfg)
#define INSERT_PATCH(var) insert_member(var); PATCH_VAR(var)

acPooling_wave_config::acPooling_wave_config(algo_comm_t &ac, const mhaconfig_t in_cfg, acPooling_wave *_pooling):
    ac(ac),
    raw_p_name(_pooling->p_name.data),
    p(ac, _pooling->pool_name.data.c_str(), _pooling->numsamples.data, 1, false),
    p_max(ac, _pooling->max_pool_ind_name.data.c_str(), 1, 1, false),
    like_ratio(ac, _pooling->like_ratio_name.data.c_str(), 1, 1, false),
    pooling_ind(0),
    pooling_option(_pooling->pooling_type.data.get_index()),
    pooling_size(_pooling->pooling_wndlen.data * in_cfg.srate / (in_cfg.fragsize * 1000)),
    up_thresh(_pooling->upper_threshold.data),
    low_thresh(_pooling->lower_threshold.data),
    neigh(_pooling->neighbourhood.data),
    alpha(_pooling->alpha.data),
    pool(_pooling->numsamples.data, _pooling->pooling_wndlen.data * in_cfg.srate / (in_cfg.fragsize * 1000))
{
    //initialize plugin state for a new configuration
    pool.assign(0);

    // initialize the estimated DOA to be in the middle, preferable steering forward
    p_max.assign((_pooling->numsamples.data - 1) / 2);
}


acPooling_wave_config::~acPooling_wave_config() {}

void acPooling_wave_config::insert()
{
    p.insert();
    p_max.insert();
    like_ratio.insert();
}


//the actual processing implementation
mha_wave_t *acPooling_wave_config::process(mha_wave_t *wave)
{
    //do actual processing here using configuration state
    const mha_wave_t raw_p = MHA_AC::get_var_waveform(ac, raw_p_name.c_str());

    // map to probability using a sigmoid transformation
    // find the max probability
    mha_real_t max = 0, sample_max, mean_p = 0;
    int  max_ind = -1;

    // Subtract the last raw p values from the pool before overwriting them
    for (unsigned int i = 0;  i < p.num_frames; ++i) {

        switch(pooling_option) {

        case 0: // sum

            if (alpha > 0 )
                p(i, 0) -= pow(1 - alpha, pooling_size - 1) * alpha * pool.value(i, pooling_ind);
            else
                p(i, 0) -= pool.value(i, pooling_ind);

            break;

        case 2: // mean

            if (alpha > 0)
                p(i, 0) -= pow(1 - alpha, pooling_size - 1) * alpha * pool.value(i, pooling_ind) / pooling_size;
            else
                p(i, 0) -= ( pool.value(i, pooling_ind) / pooling_size );

            break;

        }

    }

     // Add the new raw p values into the pool
    for( unsigned int i = 0; i < p.num_frames; ++i ) {

        pool(i, pooling_ind) =  value(raw_p, i, 0);

        switch (pooling_option) {

        case 0: // sum

            if (alpha > 0) 
                p(i, 0) = (1 - alpha) * p(i, 0) + alpha * pool.value(i, pooling_ind);
            else
                p(i, 0) += pool.value(i, pooling_ind);

            break;

        case 1: // max

            sample_max = 0;

            for (unsigned int j = 0; j < pooling_size; j++) {
                if (sample_max < pool.value(i, j))
                    sample_max = pool.value(i, j);

                p(i, 0) = sample_max;
            }

            break;

        case 2: // mean

            if (alpha > 0)
                p(i, 0) = (1 - alpha) * p(i, 0) + alpha * pool.value(i, pooling_ind) / pooling_size;
            else
                p(i, 0) += ( pool.value(i, pooling_ind) / pooling_size );

            break;

        }

        mean_p += p.value(i, 0);

        // Find the direction with the maximum probability
        if (max < p.value(i, 0)) {
            max = p.value(i, 0);
            max_ind = i;

        }
    }

    // If the index of the most probable DOA cannot be found, we assign it to the middle,
    // which most probably correspond to 0°.
    if (max_ind == -1)
        max_ind = (int)((p.num_frames - 1) / 2);

    pooling_ind = (pooling_ind + 1)  % pooling_size;

    // Compute the normalized probability of the maximum
    like_ratio(0, 0) = p(max_ind, 0) / mean_p;

    if ( like_ratio(0, 0) >= up_thresh )
        p_max(0, 0) = max_ind;
    else if (like_ratio(0, 0) >= low_thresh) {
        // If the new maximum within the neighbourhood lyes
        if (neigh == -1)
            p_max(0, 0) = max_ind;
        else if (std::abs(max_ind - (int)p_max(0, 0)) <= neigh)
            p_max(0, 0) = max_ind;
    }

    // Insert the updated AC variables into the AC space
    insert();

    //return current fragment
    return wave;
}

/** Constructs our plugin. */
acPooling_wave::acPooling_wave(algo_comm_t & ac,
                               const std::string & chain_name,
                               const std::string & algo_name)
    : MHAPlugin::plugin_t<acPooling_wave_config>("Pooling of several consecutive time frames",ac)
    , numsamples("This parameter determines the length of the wave to be pooled in samples", "33", "]0, 360]")
    , pooling_wndlen("This parameter determines the length of the pooling window in msec.", "300", "[0, 5000]")
    , pooling_type("This parameter determines the pooling method applied to the pooling window.", "mean", "[max sum mean]")
    , upper_threshold("This parameter sets a threshold for finding the maximum probability. If the maximum is above this threshold, it is taken, even if it is not in the neighbourhood of the last estimated direction.", "0.75", "[0, 1]")
    , lower_threshold("This parameter sets a threshold for finding the minimum probability. If the maximum probability is below this threshold, the estimated direction of the last iteration is taken.", "0", "[0, 1]")
    , neighbourhood("This parameter defines the neighbourhood of the allowed change of the estimated direction between iterations. -1 means no neighbourhood.", "2", "[-1, 360]")
    , alpha("This parameter simulates the forgetting effect by weighting the frames within the pooling window,  e.g. p(n + 1) = (1 - alpha) * p(n) + alpha * p_new. 0 means no weighting.", "0.1", "[0, 1]")
    , p_name("The name of the AC variable of the frame, which is going to be pooled.", "p")
    , pool_name("The name of the AC variable of the averaged (pooled) frame.", "pool")
    , max_pool_ind_name("The name of the AC variable for the index of the maximum of the averaged frames", "pool_max")
    , like_ratio_name("The name of the AC variable for the likelihood ratios of the averaged frames", "like_ratio")
{
    //add parser variables and connect them to methods here
    //INSERT_PATCH(foo_parser);

    // make the plug-in findable via "?listid"
    set_node_id(algo_name);

    INSERT_PATCH(numsamples);
    INSERT_PATCH(pooling_wndlen);
    INSERT_PATCH(pooling_type);
    INSERT_PATCH(upper_threshold);
    INSERT_PATCH(lower_threshold);
    INSERT_PATCH(neighbourhood);
    INSERT_PATCH(alpha);
    INSERT_PATCH(p_name);
    INSERT_PATCH(pool_name);
    INSERT_PATCH(max_pool_ind_name);
    INSERT_PATCH(like_ratio_name);
}

acPooling_wave::~acPooling_wave() {}

/** Plugin preparation.
 *  An opportunity to validate configuration parameters before instantiating a configuration.
 * @param signal_info
 *   Structure containing a description of the form of the signal (domain,
 *   number of channels, frames per block, sampling rate.
 */
void acPooling_wave::prepare(mhaconfig_t & signal_info)
{
    //good idea: restrict input type and dimension
    /*
    if (signal_info.channels != 2)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin must have 2 input channels: (%d found)\n"
                        "[Left, Right].", signal_info.channels);

    if (signal_info.domain != MHA_SPECTRUM)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin can only process spectrum signals.");
                        */

    if( signal_info.domain != MHA_WAVEFORM )
        throw MHA_Error(__FILE__, __LINE__,
                        "This plug-in requires time-domain signals.");

    /* make sure that a valid runtime configuration exists: */
    update_cfg();
    poll_config()->insert();
}

void acPooling_wave::update_cfg()
{
    if ( is_prepared() ) {

        //when necessary, make a new configuration instance
        //possibly based on changes in parser variables
        acPooling_wave_config *config;
        config = new acPooling_wave_config( ac, input_cfg(), this );
        push_config( config );
    }
}

/**
 * Checks for the most recent configuration and defers processing to it.
 */
mha_wave_t * acPooling_wave::process(mha_wave_t * signal)
{
    //this stub method defers processing to the configuration class
    return poll_config()->process( signal );
}

/*
 * This macro connects the plugin1_t class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(acPooling_wave,acPooling_wave,wave,wave)

/*
 * This macro creates code classification of the plugin and for
 * automatic documentation.
 *
 * The first argument to the macro is a space separated list of
 * categories, starting with the most relevant category. The second
 * argument is a LaTeX-compatible character array with some detailed
 * documentation of the plugin.
 */
MHAPLUGIN_DOCUMENTATION(acPooling_wave,
        "AC-variables",
        "This plugin computes an average over several consecutive frames using several different approaches (max, sum, mean, ...). Subsequently, the maximum of the average frame is delivered as well. The plugin receives the frames through the ac space and deliveres the averaged frame as well as its maximum to the ac space."
        )

