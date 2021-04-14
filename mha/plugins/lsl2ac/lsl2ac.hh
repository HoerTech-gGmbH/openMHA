// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2020 2021 HörTech gGmbH
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
#include "mha_parser.hh"
#include "mha_plugin.hh"
#include "mha_os.h"
#include "mha_events.h"
#include "mha_defs.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "lsl_cpp.h"
#pragma GCC diagnostic pop

#include <memory>
#include <map>
#include <string>
#include <exception>
#include <vector>
#include <chrono>

class Test_save_var_t; // Forward declaration
namespace lsl2ac{
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
     * @param nchannels_ Number of channels in the AC variable must be zero or equal to number of channels in LSL stream
     * @param nsamples_ Number of samples per channel in the AC variable. Zero means resize as needed
     */
    save_var_t(const lsl::stream_info& info_, const algo_comm_t& ac_,
               overrun_behavior ob_, int buffersize_, int chunksize_,  int nchannels_, int nsamples_);
    ~save_var_t()=default;
    /** Get stream info object from stream inlet */
    lsl::stream_info info();
    /** Receive a samples from lsl and copy to AC space. Handling of underrun is configuration-dependent */
    void receive_frame();
  private:
    /** LSL stream outlet. Interface to lsl */
    lsl::stream_inlet stream;
    /** Data buffer of the ac variable. */
    std::vector<mha_real_t> buf;
    /** Timestamp buffer */
    std::vector<double> ts_buf;
    /** Handle to AC space */
    const algo_comm_t& ac;
    /** Timeseries AC variable */
    comm_var_t cv;
    /** Timestamp AC variable */
    comm_var_t ts;
    /** Current time correction */
    double tc=0.0;
    /** Timestamp AC variable name */
    std::string ts_name;
    /** Time correction AC variable name */
    std::string tc_name;
    /** Number of new samples AC variable name */
    std::string new_name;
    /** time point of last time correction pull */
    std::chrono::time_point<std::chrono::steady_clock> tic;
    /** Should the variable be skipped in future process calls? Only true when error occured. */
    bool skip=false;
    /** Behavior on stream overrun */
    overrun_behavior ob;
    /** Name of stream. Must be saved separately because the stream info might be unrecoverable in error cases */
    const std::string name;
    /** Number of channels */
    int32_t nchannels;
    /** Number of samples per channel in the AC variable. Zero means resize as needed. */
    std::size_t nsamples;
    /** Maximal chunk size of lsl stream */
    std::size_t chunksize;
    /** Total buffer size of the ac variable buffer. nsamples*nchannels if nsamples is set,
     * chunksize*nchannels otherwise if chunksize is set, nchannels if neither nsamples and
     * chunksize are set.
     */
    std::size_t bufsize;
    /** Number of most recently pulled samples per channel */
    int n_new_samples;
    /** Pull new samples, ignore overrun. If nsamples=0, leaves the buffers in a state
     * where the newest samples are at the beginning of the buffers, the state of the older
     * samples is undefined and n_new_samples contains the number of new samples per channel.
     * If nsamples is non-zero, the buffers are rotated so the oldest samples are the first in the buffer
     * and n_new_samples contains the number of new samples per channel. */
    void pull_samples_ignore();
    /** Pull new samples as long as there are samples ready for pickup in the
     * LSL buffers. If nsamples=0, leaves the buffers in a state where the
     * newest samples are at the beginning of the buffers, the state of the
     * older samples is undefined and n_new_samples contains total number of new
     * samples per channel. If nsamples is non-zero, the buffers are rotated so
     * the oldest samples come first. n_samples_new then contains the total number
     * of new samples per channel. If n_new_samples is larger than the number of samples in
     * the AC variable that means samples had be discarded. */
    void pull_samples_discard();
    /** Refresh time correction value every 5s */
    void get_time_correction();
    /** Insert stream value, time stamp and time offset into ac space*/
    void insert_vars();
  };

  /** Runtime configuration class of the lsl2ac plugin */
  class cfg_t {
    /** Maps variable name to unique ptr's of lsl to ac bridges. */
    std::map<std::string, std::unique_ptr<save_var_t>> varlist;
  public:
    /** C'tor of lsl2ac run time configuration
     * @param ac_          AC space, data from LSL will be inserted as AC variables
     * @param overrun_     Overrun behavior
     * @param bufsize_     Buffer size of the LSL stream inlet
     * @param chunksize_   Chunk size of the LSL stream
     * @param streamnames_ Names of LSL streams to be subscribed to
     * @param nchannels_   Number of channels to expect in the the LSL streams. Zero means accept any.
     * @param nsamples_    Number of samples per channel in the AC variable. Zero means resize as needed.
     */
    cfg_t(const algo_comm_t& ac_, overrun_behavior overrun_,
          int bufsize_, int chunksize_,const std::vector<std::string>& streamnames_,
          int nchannels_, int nsamples_);
    void process();

  };

  /** Plugin class of lsl2ac */
  class lsl2ac_t : public MHAPlugin::plugin_t<cfg_t>
  {
  public:
    lsl2ac_t(algo_comm_t iac, const std::string & configured_name);
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
    /** Config variable for overrun behavior. */
    MHAParser::kw_t overrun_behavior={"How to handle overrun","discard","[discard ignore]"};
    /** Config variable for maximum buffer size of LSL */
    MHAParser::int_t buffersize={"The maximum amount of data for LSL to buffer."
                             " In seconds if there is a nominal sampling rate,"
                             " otherwise x100 in samples","360","[0,]"};
    /** Config variable for maximum chunk size of LSL */
    MHAParser::int_t chunksize={"The maximum granularity, in samples, at which chunks are transmitted by LSL."
                                " Default means sender decides","0","[0,]"};
    /** Config variable for number of channels to expect from LSL stream */
    MHAParser::int_t nchannels = {
        "The number of channels to expect of the LSL stream, also determines stride of AC variable. "
        " Default means sender decides",
        "0", /* Make sure nchannels fits in int32*/
        [](){return "[0," + std::to_string(std::min(std::numeric_limits<int32_t>::max(),
                                                    std::numeric_limits<int>::max())) +"]";}()};
    /** Config variable for maximum chunk size of LSL */
    MHAParser::int_t nsamples = {
        "The number of samples per channel to be stored in the AC variable."
        " Default means the AC variable will be resized as needed "
        " to accommodate the samples received, up to a maximum of chunksize.",
        "0", "[0,]"};
    /** Patchbay for configuration callbacks. */
    MHAEvents::patchbay_t<lsl2ac_t> patchbay;
    /** Monitor variable containing all available streams. */
    MHAParser::vstring_mon_t available_streams={"List of all available LSL streams"};
  };
}
