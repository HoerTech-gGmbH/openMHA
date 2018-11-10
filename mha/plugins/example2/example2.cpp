// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2005 2006 2007 2009 2010 2013 2014 2015 2017 HörTech gGmbH
// Copyright © 2018 HörTech gGmbH
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
 * A simple example of an \mha plugin written in C++
 *
 * This plugin scales one channel of the input signal, working in the
 * time domain. The scale factor and the scaled channel number is made
 * accessible to the configuration structure.
 */

#include "mha_plugin.hh"

/** This C++ class implements the second example plugin for the
 * step-by-step tutorial.  It extends the first example by using
 * configuration language variables to influence the processing. */
class example2_t : public MHAPlugin::plugin_t<int> {
    /** Index of audio channel to scale. */
    MHAParser::int_t scale_ch;
    /** The scaling factor applied to the selected channel. */
    MHAParser::float_t factor;
public:
    /** This constructor initializes the configuration language
     * variables and inserts them into the \mha configuration tree. */
    example2_t(algo_comm_t & ac,
               const std::string & chain_name,
               const std::string & algo_name);

    /** Plugin preparation. This plugin checks that the input signal
     * has the waveform domain and contains enough channels.
     * @param signal_info 
     *   Structure containing a description of the form of the signal
     *   (domain, number of channels, frames per block, sampling rate.
     */
    void prepare(mhaconfig_t & signal_info);

    /** Undo restrictions posed in prepare. */
    void release(void);

    /** Signal processing performed by the plugin.  
     * This plugin multiplies the signal in the selected audio channel by
     * the configured factor. 
     * @param signal
     *   Pointer to the input signal structure.
     * @return
     *   Returns a pointer to the input signal structure,
     *   with a the signal modified by this plugin.
     *   (In-place processing)
     */
    mha_wave_t * process(mha_wave_t * signal);
};

example2_t::example2_t(algo_comm_t & ac,
                       const std::string & chain_name,
                       const std::string & algo_name)
    : MHAPlugin::plugin_t<int>("This plugin multiplies the sound signal"
                               " in one audio channel by a factor",ac),
      scale_ch("Index of audio channel to scale. Indices start from 0.",
               "0",
               "[0,["),
      factor("The scaling factor that is applied to the selected channel.",
             "0.1", 
             "[0,[")
{
    insert_item("channel", &scale_ch);
    insert_item("factor", &factor);
}

void example2_t::prepare(mhaconfig_t & signal_info)
{
    if (signal_info.domain != MHA_WAVEFORM)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin can only process waveform signals.");
    // The user may have configured scale_ch before prepare is called.
    // Check that the configured channel is present in the input signal.
    if (signal_info.channels <= unsigned(scale_ch.data))
        throw MHA_Error(__FILE__,__LINE__,
                        "This plugin requires at least %d input channels.",
                        scale_ch.data + 1);
    // Adjust the range of the channel configuration variable so that it
    // cannot be set to an out-of-range value during processing.
    using MHAParser::StrCnv::val2str;
    scale_ch.set_range("[0," + val2str(int(signal_info.channels)) + "[");
}

void example2_t::release(void)
{
    scale_ch.set_range("[0,[");
}

mha_wave_t * example2_t::process(mha_wave_t * signal)
{
    unsigned int frame;
    for(frame = 0; frame < signal->num_frames; frame++)
        value(signal,frame,scale_ch.data) *= factor.data;
    return signal;
}

MHAPLUGIN_CALLBACKS(example2,example2_t,wave,wave)
MHAPLUGIN_DOCUMENTATION(example2,"example","")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
