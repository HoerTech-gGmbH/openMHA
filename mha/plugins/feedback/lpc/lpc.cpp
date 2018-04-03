// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2014 2015 2017 HörTech gGmbH
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
 * This plugin estimates the autocorrelation of each block.
 * It then produces the inverse filter using the Levinson recursion.
 */

#include "lpc.h"

#define PATCH_VAR(var) patchbay.connect(&var.valuechanged, this, &lpc::update_cfg)
#define INSERT_PATCH(var) insert_member(var); PATCH_VAR(var)

lpc_config::lpc_config(algo_comm_t &ac, const mhaconfig_t in_cfg,
                       std::string & algo_name, unsigned int _order, unsigned int _lpc_buffer_size, bool _shift, unsigned int _comp_each_iter, bool _norm) :
    norm(_norm), shift(_shift), comp_each_iter(_comp_each_iter), order(_order), lpc_buffer_size(_lpc_buffer_size), N(in_cfg.fragsize), //copy the LPC order and fragsize
    comp_iter(0),
    R(_order + 1, 0 ), A(_order + 1, 0), //store the estimated autocorrelation and filters
    inwave((_order + 1) > _lpc_buffer_size ? (_order + 1) : _lpc_buffer_size, in_cfg.channels, 0),
    lpc_out(ac, algo_name, _order + 1, in_cfg.channels, false),
    corr_out(ac, algo_name + "_corr", _order + 1, in_cfg.channels, false)
{
    //initialize plugin state for a new configuration    
    sample.buf = new mha_real_t[1 * in_cfg.channels];
    sample.num_channels = in_cfg.channels;
    sample.num_frames = 1;

    if (N > order + 1)
        shift = false;

    if (!shift)
        comp_each_iter = 1;

    if (order + 1 > lpc_buffer_size)
        lpc_buffer_size = order + 1;

    lpc_out.assign_frame(0, 1);
}

lpc_config::~lpc_config()
{
    delete[] sample.buf;
}

void lpc_config::insert()
{
    lpc_out.insert();
    corr_out.insert();
}

void Levinson2(unsigned int P, const std::vector<mha_real_t> &R, std::vector<mha_real_t> &A);

//the actual processing implementation
mha_wave_t *lpc_config::process(mha_wave_t *wave)
{
    if (shift) {
        if (inwave.contained_frames() == lpc_buffer_size)
            inwave.discard(N);
        inwave.write(*wave);
    }
    else {
        int num_to_write;

        if (inwave.contained_frames() == lpc_buffer_size)
            inwave.discard(lpc_buffer_size);

        if (N < lpc_buffer_size)
            num_to_write = lpc_buffer_size - inwave.contained_frames() >= N ? N: lpc_buffer_size - inwave.contained_frames();
        else
            num_to_write = lpc_buffer_size;

        // Write the necessary amount of samples of the incoming signal into the LPC buffer sample by sample
        for(int i = 0; i < num_to_write; i++) {
            for (unsigned int ch = 0; ch < wave->num_channels; ch++)
                sample.buf[ch] = value(wave, i, ch);

            inwave.write(sample);

        }

        if (inwave.contained_frames() <= order)
            return wave;
    }

    if (comp_each_iter - 1 > comp_iter) {
        comp_iter++;
        return wave;
    }

    //step 1: estimate the autocorrelation for this frame
    for (unsigned int ch = 0; ch < wave->num_channels; ch++) {
        for (unsigned int i=0; i < order + 1; ++i) //over all lags within order+1
        {
            R[i] = 0;
            for (unsigned int j=0; j < lpc_buffer_size - i; ++j) //all samples minus lag
            {
                R[i] += inwave.value(j,ch) * inwave.value(j+i,ch);
            }

            if (norm)
                R[i] /= (mha_real_t) lpc_buffer_size; //divide by blocksize
        }

        //step 2: find IIR coefficients to invert the system
        Levinson2(order, R, A);

        //step 3: copy coefficients to AC space
        for (unsigned int i=0; i<=order; i++)
        {
            value(lpc_out,i,ch) = A[i];
            value(corr_out,i,ch) = R[i];
        }
    }

    comp_iter = 0;

    // Insert the updated AC variables into the AC space
    insert();

    //return current fragment
    return wave;
}

/** Constructs our plugin. */
lpc::lpc(algo_comm_t & ac,
         const std::string & chain_name,
         const std::string & algo_name)
    : MHAPlugin::plugin_t<lpc_config>("This plugin implements the linear predictive coding analysis (LPC) by using the Levinson-Durbin recursion.",ac),
      algo_name(algo_name), //copy algo name
      lpc_order("LPC filter order", "20", "[0,500]"),
      lpc_buffer_size("Size of the buffer in samples for which the autocorrelation matrix will be computed", "21", "]0,501]"),
      shift("Refill the LPC buffer completely with new input signal by ignoring the old samples (no) or shift the old buffer as large as the block size of the input signal and read in the current input signal (yes).", "yes"),
      comp_each_iter("Reestimate the LPC coefficients each <comp_each_iter> iterations, default value is 1", "1", "]0,]"),
      norm("Normalize the auto correlation matrix with the LPC order", "no")
{
    // make the plug-in findable via "?listid"
    set_node_id(algo_name);

    //add parser variables and connect them to methods here
    INSERT_PATCH(lpc_order);
    INSERT_PATCH(lpc_buffer_size);
    INSERT_PATCH(shift);
    INSERT_PATCH(comp_each_iter);
    INSERT_PATCH(norm);
}

lpc::~lpc() {}

/** Plugin preparation.
 *  An opportunity to validate configuration parameters before instantiating a configuration.
 * @param signal_info
 *   Structure containing a description of the form of the signal (domain,
 *   number of channels, frames per block, sampling rate.
 */
void lpc::prepare(mhaconfig_t & signal_info)
{
    //good idea: restrict input type and dimension
    if (signal_info.domain != MHA_WAVEFORM)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin can only process spectrum signals.");

    /* make sure that a valid runtime configuration exists: */
    update_cfg();
    poll_config()->insert();
}

void lpc::update_cfg()
{
    if ( is_prepared() ) {

        //when necessary, make a new configuration instance
        //possibly based on changes in parser variables
        lpc_config *config;
        config = new lpc_config( ac, input_cfg(), algo_name, lpc_order.data, lpc_buffer_size.data, shift.data, comp_each_iter.data, norm.data);
        push_config( config );
    }
}

/**
 * Checks for the most recent configuration and defers processing to it.
 */
mha_wave_t * lpc::process(mha_wave_t * signal)
{
    //this stub method defers processing to the configuration class
    return poll_config()->process( signal );
}

//rewritten code, based on tutorial by Cedrick Collomb
void Levinson2(unsigned int P, const std::vector<mha_real_t> &R, std::vector<mha_real_t> &A)
{
    //step 1: initialize A to be a unit vector
    //the rest of the coefficients will be computed from the main recursion
    for (unsigned int k=0; k<P+1; ++k)
    {
        A[k] = (k==0) ? 1.0 : 0.0;
    }

    //step 2: set E0 to R0-
    //each error component comes from explaining components of the autocorrelation
    double Ek = R[0];

//    std::vector<mha_real_t> Atemp(P+1,0); //temp for A recursion

    //step 3: Levinson recursion.
    //This uses coefficients of order k to compute order k+1
    for (unsigned int k=0; k<P; ++k)
    {
        //step 4: update lambda given equation (4)
        double lambda = 0;
        for (unsigned int j=0; j<k+1; ++j)
        {
            lambda += A[j] * R[k+1-j];
        }
        lambda /= -Ek; //change sign and divide by Ek

        //step 5: update Ak[k+1 coeffs] to Ak+1[k+2 coeffs] by summing extensions
        //these are Uk+1, from expanding Ak with a zero
        //and Vk+1, which is just Uk+1 flipped
//        Atemp = A; //copy prev values
//        for (unsigned int j=0; j<k+2; ++j)
//        {
//            //for each new element of Ak+1
//            //Ak+1 = Uk+1 + lambda * Vk+1
//            A[j] = Atemp[j] + lambda * Atemp[k+1-j];
//        }

        for (unsigned int j = 0; j <= (k + 1) / 2; j++)
        {
            double temp = A[k + 1 - j] + lambda * A[j];
            A[j] = A[j] + lambda * A[k + 1 - j];
            A[k + 1 - j] = temp;
        }

        //step 6: update Ek given equation (6)
        Ek = (1.0 - lambda*lambda) * Ek;

    } //end of levinson recursion
} //end of Levinson2

/*
 * This macro connects the plugin1_t class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(lpc,lpc,wave,wave)

/*
 * This macro creates code classification of the plugin and for
 * automatic documentation.
 *
 * The first argument to the macro is a space separated list of
 * categories, starting with the most relevant category. The second
 * argument is a LaTeX-compatible character array with some detailed
 * documentation of the plugin.
 */
MHAPLUGIN_DOCUMENTATION(lpc,
        "adaptive",
        "This plugin estimates the autocorrelation of each block. "
        "It then produces the inverse filter using the Levinson-Durbin recursion."
        )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
