// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2020 2021 HörTech gGmbH
// Copyright © 2022 Hörzentrum Oldenburg gGmbH
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


#include "lsl_cpp.h"
#include "mha_algo_comm.h"
#include "mha_plugin.hh"
#include "mha_fifo.h"

/** namespace for trigger2lsl plugin*/
namespace trigger2lsl {
  /** real-time configuration class for trigger2lsl plugin*/
  class trigger2lsl_rt_t {
  public:
    /** C'tor of rt configuration
     * @param stream_name_ Name of the output stream
     * @param rising_edge_ String to be sent on detection of a rising edge
     * @param falling_edge_ String to be sent on detection of a falling edge
     * @param threshold_ Threshold for state transition
     * @param channel_ Channel index where to look for threshold crossings
     * @param sampling_rate_ Sampling rate of the input signal. Needed for timestamp offeset correction
     * @param use_edge_position_ Flag wether to use the position of the edge within the signal block to correct the timestamp of the output marker
     * @param min_debounce_ Minimum number of consecutive samples that need to cross the threshold to initiate a state transition
     */
    trigger2lsl_rt_t(const std::string& stream_name_,
                     const std::string& rising_edge_,
                     const std::string& falling_edge_,
                     mha_real_t threshold_,
                     int channel_,
                     mha_real_t sampling_rate_,
                     bool use_edge_position_,
                     int min_debounce_);
    void process(mha_wave_t* wave);
  private:
    /** Outlet stream */
    const lsl::stream_outlet stream;
    /** String to be sent when a rising edge is detected */
    const std::string rising_edge;
    /** String to be sent when a falling edge is detected */
    const std::string falling_edge;
    /** Threshold for state transition */
    const mha_real_t threshold;
    /** Channel number where to look for threshold crossings */
    const int channel;
    /** Current state. false means HIGH and true means LOW. LOW state means we are below
     * the threshold, looking for rising edges, HIGH state means we are above, looking for falling edges.
     */
    const bool state=false;
    /** Sampling rate of the input signal. Needed for timestamp offeset correction */
    const mha_real_t sampling_rate;
    /** Flag wether to use the position of the edge within the signal block to correct the timestamp of the output marker. */
    const bool use_edge_position=true;
    /** Minimum number of consecutive samples that need to cross the threshold to initiate a state transition */
    const int min_debounce;
    /** Debounce counter */
    int debounce_counter=0;
  };

  /// Plugin interface class of plugin trigger2lsl.
  class trigger2lsl_if_t : public MHAPlugin::plugin_t<trigger2lsl_rt_t> {
  public:

    // Generic process callback
    template <class mha_signal_t> mha_signal_t* process(mha_signal_t* s);
    /// Prepare callback.  trigger2lsl does not modify the signal parameters.
    /// @param cf The signal parameters.
    void prepare(mhaconfig_t& cf);
    /// Ensure recorded data is flushed to disk.
    void release();
    /// Plugin interface constructor.
    /// @param iac Algorithm communication variable space.
    trigger2lsl_if_t(algo_comm_t iac, const std::string & configured_name);
  private:
    void update();
    MHAEvents::patchbay_t<trigger2lsl_if_t> patchbay;
    MHAParser::string_t rising_edge{"Marker string to be sent when a rising edge is detected","START"};
    MHAParser::string_t falling_edge{"Marker string to be sent when a falling edge is detected","STOP"};
    MHAParser::float_t threshold{"Threshold","0.5","[0,]"};
    MHAParser::int_t channel{"Channel index where edge detection should be run","0","[0,]"};
    MHAParser::string_t stream_name{"Name of the output stream",""};
    MHAParser::bool_t use_edge_position{"Offset timestamp by position of edge within block","no"};
    MHAParser::int_t min_debounce{"Number of consecutive samples the threshold must have been crossed before a trigger is issued","3","[0,]"};
  };


} //namespace trigger2lsl
