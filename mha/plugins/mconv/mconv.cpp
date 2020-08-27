// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2007 2009 2010 2013 2014 2015 2018 2019 2020 HörTech gGmbH
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

#include "mha_plugin.hh"
#include "mha_signal.hh"
#include "mha_events.h"
#include "mha_defs.h"
#include "mha_filter.hh"


namespace mconv {
    /**\internal
     * class implements plugin for partitioned convolution.
     * A matrix of impulse responses, filtering n input channels to m output
     * channels, is supported.
     */
    class MConv :  public MHAPlugin::plugin_t<MHAFilter::partitioned_convolution_t>
    {
    public:
        /** Plugin constructor.
         * @param iac handle and function pointers for algorithm communication
         * @param chainname Name of processing chain
         * @param algoname The name by which the chain refers to this algorithm */
        MConv(const algo_comm_t & iac,
              const std::string & chainname,
              const std::string & algoname);
        /** Prepare this plugin for processing.
         * @param mhaconfig Configuration for this plugin (Input/Output parameter)
         * Sample rate, fragment size, number of channels are detailed here. */
        void prepare(mhaconfig_t & mhaconfig);
        void release();
        mha_wave_t* process(mha_wave_t*);
    private:
        /** Update is needed only once, since this plugin allows only
         * change of irs after prepare(). */
        void update();
        /**  This function updates the irs without allowing a change
         * of its size after prepare(). */
        void update_irs();
        /** Number of output channels to produce */
        MHAParser::int_t nchannels_out;
        /** Vector of input channel indices.
         * Each element in this vector identifies the input channel to
         * which to apply the corresponding impulse response in irs. */
        MHAParser::vint_t inch;
        /** Vector of output channel indices.
         * Each element in this vector identifies the output channel to
         * which the result of filtering with the corresponding impulse
         * response in irs is mixed. */
        MHAParser::vint_t outch;
        /** Impulse responses, one per row.  For each row, the
         * corresponding element of inch identifies the source channel,
         * and the corresponding element of outch identifies the target
         * channel. */
        MHAParser::mfloat_t irs;

        /** Number of input channels, set during prepare. */
        unsigned int nchannels_in;

        /** Fragsize, set during prepare, is used as the partition length in the
         * partitioned convolution */
        unsigned int fragsize;

        MHAEvents::patchbay_t<MConv> patchbay;
    };

    MConv::MConv(const algo_comm_t & iac,
                 const std::string & chainname,
                 const std::string & algoname)
        : MHAPlugin::plugin_t<MHAFilter::partitioned_convolution_t>
        ("FFT based FIR filter using partitioned convolution\n"
         "  This plugin filters its input channels using partitioned fast\n"
         "convolution. The variables in this plugin define a sparse matrix of\n"
         "impulse responses. The number of elements in the vectors inch and\n"
         "outch and the number of rows in irs have to be equal.\n", iac),
        nchannels_out("Number of output channels to produce", "1", "[0,["),
        inch("Vector of input channel indices.\n"
             "  Each element in this vector identifies the input channel to\n"
             "which to apply the corresponding impulse response in irs.",
             "[0]", "[0,["),
        outch("Vector of output channel indices.\n"
              "  Each element in this vector identifies the output channel to\n"
              "which the result of filtering with the corresponding impulse\n"
              "response in irs is mixed.", "[0]", "[0,["),
        irs("Impulse responses, one per row.  For each row, the corresponding\n"
            "element of inch identifies the source channel, and the\n"
            "corresponding element of outch identifies the target channel.",
            "[[1]]"),
        nchannels_in(0),
        fragsize(0)
    {
        insert_item("nchannels_out", &nchannels_out);
        insert_item("inch", &inch);
        insert_item("outch", &outch);
        insert_item("irs", &irs);

        patchbay.connect(&irs.writeaccess, this, &MConv::update_irs);
    }

    void MConv::prepare(mhaconfig_t & mhaconfig)
    {
        if (mhaconfig.domain != MHA_WAVEFORM)
            throw MHA_ErrorMsg("Plugin supports waveform processing only.");
        nchannels_in = mhaconfig.channels;
        mhaconfig.channels = nchannels_out.data;
        fragsize = mhaconfig.fragsize;
        update();
        inch.setlock(true);
        outch.setlock(true);
        nchannels_out.setlock(true);
    }

    void MConv::release()
    {
        nchannels_out.setlock(false);
        inch.setlock(false);
        outch.setlock(false);
    }

    void MConv::update()
    {
        if (irs.data.size() != inch.data.size()
            || irs.data.size() != outch.data.size())
            throw MHA_Error(__FILE__, __LINE__,
                            "Sizes of irs (%zu), inch (%zu), and outch (%zu) do not match",
                            irs.data.size(), inch.data.size(), outch.data.size());

        MHAFilter::transfer_function_t tf;
        MHAFilter::transfer_matrix_t tm;
        for (size_t index = 0; index < irs.data.size(); ++index) {
            if (inch.data[index] < 0 || inch.data[index] >= (int)nchannels_in)
                throw MHA_Error(__FILE__,__LINE__,
                                "Source channel index inch[%d]=%d is out of range",
                                int(index), inch.data[index]);
            tf.source_channel_index = inch.data[index];
            if (outch.data[index] < 0 || outch.data[index] >= nchannels_out.data)
                throw MHA_Error(__FILE__,__LINE__,
                                "Target channel index outch[%d]=%d is out of range",
                                int(index), outch.data[index]);
            tf.target_channel_index = outch.data[index];
            tf.impulse_response = irs.data[index];
            tm.push_back(tf);
        }

        if (is_prepared())
            push_config(new MHAFilter::partitioned_convolution_t(fragsize,
                                                             nchannels_in,
                                                             nchannels_out.data,
                                                             tm));
    }

    void MConv::update_irs()
    {
        if (!is_prepared())
            return;
        if (irs.data.size() != inch.data.size()
            || irs.data.size() != outch.data.size())
            throw MHA_Error(__FILE__, __LINE__,
                            "Sizes of irs (%zu), inch (%zu), and outch (%zu) do not match",
                            irs.data.size(), inch.data.size(), outch.data.size());

        MHAFilter::transfer_function_t tf;
        MHAFilter::transfer_matrix_t tm;
        for (size_t index = 0; index < irs.data.size(); ++index) {
            tf.source_channel_index = inch.data[index];
            tf.target_channel_index = outch.data[index];
            tf.impulse_response = irs.data[index];
            tm.push_back(tf);
        }

        push_config(new MHAFilter::partitioned_convolution_t(fragsize,
                                                             nchannels_in,
                                                             nchannels_out.data,
                                                             tm));
    }

    mha_wave_t* MConv::process(mha_wave_t * s_in)
    {
        poll_config();
        mha_wave_t * s_out = cfg->process(s_in);
        return s_out;
    }
}

MHAPLUGIN_CALLBACKS(mconv,mconv::MConv, wave, wave)
MHAPLUGIN_DOCUMENTATION(mconv,
                        "filter",
                        "The plugin {\\em mconv} performs partitioned convolution, using a"
                        " sparse matrix of impulse responses.\n\n"
                        " The partition size used for the partitioned convolution is equal to"
                        " fragsize, the number of samples per channel in one block of audio. "
                        " The impulse responses are separated into partitions, and each partition"
                        " is applied with the appropriate delay. Each partition is applied using the"
                        "overlap-save method. The FFT length used is 2*fragsize."
                        "For efficiency reasons, fragsize should be a power of two.\n\n"
                        " This implementation discards impulse response partitions where the coefficients are all zero."
                        )


// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
