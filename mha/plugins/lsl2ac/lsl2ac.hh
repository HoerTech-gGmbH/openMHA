// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2020 HörTech gGmbH
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

#include "mha_algo_comm.h"
#include "mha_plugin.hh"
#include "mha_os.h"
#include "mha_events.h"
#include "mha_defs.h"

#include "lsl_cpp.h"

#include <memory>
#include <map>
#include <string>
#include <exception>
#include <vector>
#include <chrono>

class Test_save_var_t; // Forward declaration
namespace lsl2ac{
  enum class underrun_behavior { Zero=0, Copy, Abort};
  enum class overrun_behavior { Discard=0, Ignore};


  /** LSL to AC bridge variable */
  class save_var_t {
  public:
    // Do not allow copies or moves to avoid reasoning about copies or moves of streams and ac vars
    save_var_t(const save_var_t&)=delete;
    save_var_t(save_var_t&&)=delete;
    save_var_t& operator=(const save_var_t&)=delete;
    save_var_t& operator=(save_var_t&&)=delete;
    /** C'tor of lsl to ac bridge.
     * @param name_ Name of LSL stream to be received
     * @param info_ LSL stream info object containing metadata
     * @param ac_ Handle to ac space
     * @param ub_ Underrun behavior. 0=Zero out, 1=Copy, 2=Abort
     * @param ob_ Overrun behavior. 0=Discard oldest, 1=Ignore
     * @param buffersize_ LSL buffer size
     * @param chunksize_ LSL chunk size
     */
    save_var_t(const lsl::stream_info& info_, const algo_comm_t& ac_, underrun_behavior ub_,
               overrun_behavior ob_, int buffersize_, int chunksize_);
    ~save_var_t()=default;
    /** Get stream info object from stream inlet */
    lsl::stream_info info();
    /** Get number of entries in the stream object*/
    unsigned num_entries();
    /** Receive a samples from lsl and copy to AC space. Handling of underrun is configuration-dependent */
    void receive_frame();
  private:
    /** LSL stream outlet. Interface to lsl */
    lsl::stream_inlet stream;
    /** Pointer to data buffer of the ac variable. */
    std::unique_ptr<mha_real_t[]> buf;
    /** Handle to AC space */
    const algo_comm_t& ac;
    /** Timeseries AC variable */
    comm_var_t cv;
    /** Current timestamp */
    double ts=0.0;
    /** Current time correction */
    double tc=0.0;
    /** Timestamp name */
    std::string ts_name;
    /** Time correction name */
    std::string tc_name;
    /** time point of last time correction pull */
    std::chrono::time_point<std::chrono::steady_clock> tic;
    /** Should the variable be skipped in future process calls? Only true when error occured. */
    bool skip=false;
    /** Behavior on stream underrun */
    underrun_behavior ub;
    /** Behavior on stream overrun */
    overrun_behavior ob;
    /** Name of stream. Must be saved separately because the stream info might be unrecoverable in error cases */
    const std::string name;
    /** Pull sample from stream and save it in buf.
     * @returns timestamp or saved sample
     */
    double pull_sample();
    /** Refresh time correction value every 5s */
    void get_time_correction();
    /** Insert stream value, time stamp and time offset into ac space*/
    void insert_vars();
    /** Receive samples from lsl and copy to AC space. Insert zeros on underrun */
    void impl_receive_frame_zero();
    /** Receive samples from lsl and copy to AC space. Insert last values on underrun */
    void impl_receive_frame_copy();
    /** Receive samples from lsl and copy to AC space. Throw exception on underrun */
    void impl_receive_frame_abort();
  };

  /** Runtime configuration class of the lsl2ac plugin */
  class cfg_t {
    /** Maps variable name to unique ptr's of lsl to ac bridges. */
    std::map<std::string, std::unique_ptr<save_var_t>> varlist;
  public:

    /** C'tor of lsl2ac run time configuration
     * @param ac_          AC space, data from LSL will be inserted as AC variables
     * @param underrun_    Underrun behavior
     * @param overrun_     Overrun behavior
     * @param bufsize_     Buffer size of the LSL stream inlet
     * @param chunksize_   Chunk size of the LSL stream
     * @param streamnames_ Names of LSL streams to be subscribed to
     */
    cfg_t(const algo_comm_t& ac_, underrun_behavior underrun_, overrun_behavior overrun_,
          int bufsize_, int chunksize_,const std::vector<std::string>& streamnames_);
    void process();

  };

  /** Plugin class of lsl2ac */
  class lsl2ac_t : public MHAPlugin::plugin_t<cfg_t>
  {
  public:
    lsl2ac_t(algo_comm_t iac,const char* chain, const char* algo);
    /** Prepare constructs the vector of bridge variables and locks
     * the configuration, then calls update(). */
    void prepare(mhaconfig_t&);
    /** Processing fct for waveforms. Calls process(void). */
    mha_wave_t* process(mha_wave_t* s) {process();return s;};
    /** Processing fct for spectra. Calls process(void). */
    mha_spec_t* process(mha_spec_t* s) {process();return s;};
    /** Process function. */
    void process();
    /** Release fct. Unlocks variable name list */
    void release();
  private:
    /** Retrieves all stream names from LSL and fills them into
        available_streams.
    */
    void get_all_stream_names();
    /** Construct new runtime configuration */
    void update();
    /** Convencience function lock/unlock all config variables
     * @param lock_ True to lock, False for unlock
     */
    void setlock(bool lock_);
    /** Config variable for list of streams to be saved. */
    MHAParser::vstring_t streams={"List of LSL streams to be saved, empty for all.","[]"};
    /** Config variable for activation/deactivation of plugin. */
    MHAParser::bool_t activate={"Receive from network?","yes"};
    /** Config variable setting how underruns should be handled. */
    MHAParser::kw_t underrun_behavior={"How to handle underrun","zero","[zero copy abort]"};
    /** Config variable for overrun behavior. */
    MHAParser::kw_t overrun_behavior={"How to handle overrun","discard","[discard ignore]"};
    /** Config variable for maximum buffer size of LSL */
    MHAParser::int_t buffersize={"The maximum amount of data for LSL to buffer."
                             " In seconds if there is a nominal sampling rate,"
                             " otherwise x100 in samples","360","[0,]"};
    /** Config variable for maximum chunk size of LSL */
    MHAParser::int_t chunksize={"The maximum size, in samples, at which chunks are transmitted by LSL."
                                " Default means sender decides","0","[0,]"};
    /** Patchbay for configuration callbacks. */
    MHAEvents::patchbay_t<lsl2ac_t> patchbay;
    /** Monitor variable containing all available streams. */
    MHAParser::vstring_mon_t available_streams={"List of all available LSL streams"};
  };
}
