// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2015 2016 2018 2019 2020 HörTech gGmbH
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
 * This plugin receives a waveform through the ac space at each
 * iteration. Depending on the averaging method, the last consecutive
 * n frames are averaged and the computed average is send back to the
 * ac space. Also, the maximum of the computed average is written to
 * the ac space.
 */

#include "acPooling_wave.h"

#define PATCH_VAR(var) patchbay.connect(&var.valuechanged, this, &acPooling_wave::update_cfg)
#define INSERT_PATCH(var) insert_member(var); PATCH_VAR(var)

acPooling_wave_config::acPooling_wave_config(algo_comm_t &ac, const mhaconfig_t in_cfg, acPooling_wave *_pooling):
    ac(ac),
    raw_p_name(_pooling->p_name.data),
    p(ac, _pooling->pool_name.data.c_str(), _pooling->numsamples.data, 1, false),
    p_biased(ac, _pooling->p_biased_name.data.c_str(), _pooling->numsamples.data, 1, false),
    p_max(ac, _pooling->max_pool_ind_name.data.c_str(), 1, 1, false),
    like_ratio(ac, _pooling->like_ratio_name.data.c_str(), 1, 1, false),
    pooling_ind(0),
    pooling_option(_pooling->pooling_type.data.get_index()),
    pooling_size(std::max(2u, (unsigned int)(_pooling->pooling_wndlen.data * in_cfg.srate / (in_cfg.fragsize * 1000)))),
    up_thresh(_pooling->upper_threshold.data),
    low_thresh(_pooling->lower_threshold.data),
    neigh(_pooling->neighbourhood.data),
    alpha(_pooling->alpha.data),
    pool(_pooling->numsamples.data, std::max(2u, (unsigned int)(_pooling->pooling_wndlen.data * in_cfg.srate / (in_cfg.fragsize * 1000)))),
    prob_bias_func(_pooling->prob_bias.data)
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
    p_biased.insert();
    p_max.insert();
    like_ratio.insert();
}


//the actual processing implementation
mha_wave_t *acPooling_wave_config::process(mha_wave_t *wave)
{
    //do actual processing here using configuration state
    const mha_wave_t raw_p = MHA_AC::get_var_waveform(ac, raw_p_name.c_str());

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

        // apply the multiplicative bias function.
        p_biased(i, 0) = p.value(i, 0)*prob_bias_func(i, 0);

        // Find the direction with the maximum probability
        if (max < p_biased(i, 0)) {
            max = p_biased(i, 0);
            max_ind = i;

        }
    }

    // If the index of the most probable DOA cannot be found, we assign it to the middle,
    // which most probably correspond to 0°.
    if (max_ind == -1)
        max_ind = (int)((p.num_frames - 1) / 2);

    pooling_ind = (pooling_ind + 1)  % pooling_size;

    // Compute the normalized probability of the maximum
    like_ratio(0, 0) = p_biased(max_ind, 0) / mean_p;

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
    , numsamples("This parameter determines the length of the wave to be pooled in samples", "37", "]0,]")
    , pooling_wndlen("This parameter determines the length of the pooling window in msec.", "300", "]0,]")
    , pooling_type("This parameter determines the pooling method applied to the pooling window.", "mean", "[max sum mean]")
    , upper_threshold("This parameter sets a threshold for finding the maximum probability. If the maximum is above this threshold, it is taken, even if it is not in the neighbourhood of the last estimated direction.", "0.75", "[0, 1]")
    , lower_threshold("This parameter sets a threshold for finding the minimum probability. If the maximum probability is below this threshold, the estimated direction of the last iteration is taken.", "0", "[0, 1]")
    , neighbourhood("This parameter defines the neighbourhood of the allowed change of the estimated direction between iterations. -1 means no neighbourhood.", "2", "[-1,]")
    , alpha("This parameter simulates the forgetting effect by weighting the frames within the pooling window,  e.g. p(n + 1) = (1 - alpha) * p(n) + alpha * p_new. 0 means no weighting.", "0.1", "[0, 1]")
    , p_name("The name of the AC variable of the frame, which is going to be pooled.", "p")
    , p_biased_name("The name of the AC variable of the biased frame, after pooling.", "prob_biased")
    , pool_name("The name of the AC variable of the averaged (pooled) frame.", "pool")
    , max_pool_ind_name("The name of the AC variable for the index of the maximum of the averaged frames", "pool_max")
    , like_ratio_name("The name of the AC variable for the likelihood ratios of the averaged frames", "like_ratio")
    , prob_bias("A multiplicative probability bias", "[1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1]")
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
    INSERT_PATCH(p_biased_name);
    INSERT_PATCH(pool_name);
    INSERT_PATCH(max_pool_ind_name);
    INSERT_PATCH(like_ratio_name);
    INSERT_PATCH(prob_bias);
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

    if( signal_info.domain != MHA_WAVEFORM )
        throw MHA_Error(__FILE__, __LINE__,
                        "This plug-in requires time-domain signals.");

    if (pooling_wndlen.data * signal_info.srate / 1000 < signal_info.fragsize * 2)
        throw MHA_Error (__FILE__, __LINE__,
                         "Pooling window size in samples (%f) is not allowed to be smaller "
                         "than the twice of the fragsize (%u).",
                         pooling_wndlen.data * signal_info.srate / 1000, signal_info.fragsize * 2);
    
    if (prob_bias.data.size() != static_cast<unsigned int>(numsamples.data))
        throw MHA_Error(__FILE__, __LINE__,
                        "prob_bias must have \"numsamples\" (%i) elements",
                        numsamples.data);

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
MHAPLUGIN_DOCUMENTATION\
(acPooling_wave,
 "data-flow feature-extraction algorithm-communication",
        "This plugin computes an average over several consecutive frames using several different approaches (max, sum, mean, ...). Subsequently, the maximum of the average frame is delivered as well. The plugin receives the frames through the AC space and deliveres the averaged frame as well as its maximum to the AC space.\n"
        "\n"
        "This plugin is typically used together with the plugins {\\tt doasvm\\_feature\\_extraction} and {\\tt doasvm\\_classification} for estimating the arrival direction of an audio signal. Within this context, this plugin smooths the vector of estimated arrival directions over time using several estimation vectors. However, it can also used within other contexts for smoothing purposes of e.g. waveforms.\n"
        "\n"
        "The length of a frame is given by the configuration variable {\\bf numsamples}. In the context of localization, for an angular resolution of 5 degrees, a total number of 37 estimation values for the interval of possible arrival directions [-90, 90] would be produced by the {\\tt doasvm\\_classification} plugin.\n"
        "\n"
        "The frames to be smoothed have to be created by some other plugin in advance, e.g. {\\tt doasvm\\_classification} plugin for the localization, and should be available in the AC space to be read by this plugin. The name of the corresponding AC variable is defined using the configuration variable {\\bf p\\_name}.\n"
        "\n"
        "At each iteration, this plugin reads the AC variable corresponding to the frame to be smoothed and smooths it by averaging a number of such frames, which have been read in the past iterations and saved in a pool. The length of this pool in msec is defined using the configuration variable {\\bf pooling\\_wndlen}. Depending on the frame rate used within the current MHA configuration, the exact number of iterations, which fall into this pool is computed during the preparation of this plugin. Note that the length of this pool should be chosen carefully so as to make sure that more than one frame falls in.\n"
        "\n"
        "This plugin implements several pooling methods for smoothing the frames saved in the pool. The alternatives are {\\bf max, sum} and {\\bf mean}. The pooling method to be used is defined using the configuration variable {\\bf pooling\\_type}. For each value in the frame (e.g. for each possible arrival direction in the context of localization), the {\\bf max} pooling takes the maximum between the iterations. The {\\bf sum} pooling sums these values up. Finally, the {\\bf mean} pooling computes the arithmetic mean of these values. Once the pooling step has been completed, a smoothed vector of frames of the same size with a single frame from each iteration has been constucted. This vector is saved in another AC variable, which is defined by using the configuration variable {\\bf pool\\_name}.\n"
        "\n"
        "Optionally, after the smoothing step, certain areas within the smoothed frame can be weighted differently. In the localization context, this optional step can correspond to a prior probability to favour certain possible arrival directions more compared to others. For instance, a hearing aid waerer can expect that the person who he / she is talking to is infront of him / her. In that case, the prior probabilities for the frontal directions can be set higher than the other possible arrival directions. This can be defined by setting tha configuration variable {\\tt prob\\_bias}. The default values of this variable are all set to 1, hence uniform distribution. The size of this variable should be equal to the frame length given in the configuration variable {\\tt numsamples}. The smoothed and weighted frame is saved in yet another AC variable, which is defined by using the configuration variable {\\tt p\\_biased\\_name}.\n"
        "\n"
        "After the smoothed frame has been computed, the maximum value of this frame is found and saved in another AC variable. In the localization context, this maximum corresponds to the arrival direction of an audio signal. The name of this AC variable is defined using the configuration variable {\\bf max\\_pool\\_ind\\_name}.\n"
        "\n"
        "In the following, an example configuration within a localization context is given. In this configuration, an angular resolution of 5 degrees for the whole circle, namely the interval of [-180, 180] is considered. In that case, there are in total of 73 possible arrival directions.\n"
        "\n"
        "\\begin{verbatim}\n"
        "acPooling_wave.p_name = p\n"
        "acPooling_wave.pool_name = pool\n"
        "acPooling_wave.max_pool_ind_name = pool_max\n"
        "acPooling_wave.numsamples = 73\n"
        "acPooling_wave.pooling_wndlen = 300\n"
        "acPooling_wave.pooling_type = mean\n"
        "\\end{verbatim}\n"
 )

// Local Variables:
// c-basic-offset: 4
// indent-tabs-mode: nil
// compile-command: "make"
// coding: utf-8-unix
// End:
