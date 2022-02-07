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

namespace lsl2ac{

  enum class overrun_behavior { Discard=0, Ignore};

  class save_var_base_t {
  public:
    virtual ~save_var_base_t()=default;
    /** Get stream info object from stream inlet */
    virtual lsl::stream_info info()=0;
    /** Receive a samples from lsl and copy to AC space. Handling of underrun is configuration-dependent */
    virtual void receive_frame()=0;
  };

  /** LSL to AC bridge variable */
  template<typename T>
  class save_var_t : public save_var_base_t {
  public:
    // Do not allow copies or moves to avoid reasoning about copies or moves of streams and ac vars
    save_var_t(const save_var_t&)=delete;
    save_var_t(save_var_t&&)=delete;
    virtual ~save_var_t()=default;
    save_var_t& operator=(const save_var_t&)=delete;
    save_var_t& operator=(save_var_t&&)=delete;
    /** C'tor of lsl to ac bridge.
     * @param name_ Name of LSL stream to be received
     * @param info_ LSL stream info object containing metadata
     * @param ac_ Handle to ac space
     * @param ob_ Overrun behavior. 0=Discard oldest, 1=Ignore
     * @param type_ Type tag of the AC variable
     * @param buflen_ LSL buffer size
     * @param chunksize_ LSL chunk size
     * @param nchannels_ Number of channels in the AC variable must be zero or equal to number of channels in LSL stream
     * @param nsamples_ Number of samples per channel in the AC variable. Zero means resize as needed
     */
    // This is a function try block. Really ugly syntax but the only way to handle exceptions
    // in the ctor initializer list. See http://www.gotw.ca/gotw/066.htm
    save_var_t(const lsl::stream_info &info_,
               const algo_comm_t &ac_,
               overrun_behavior ob_,
               int type_,
               int buflen_,
               int chunksize_,
               int nchannels_,
               int nsamples_) try
      : stream(info_, buflen_, chunksize_),
          ac(ac_),
          ts_name(info_.name() + "_ts"),
          tc_name(info_.name() + "_tc"),
          new_name(info_.name() + "_new"),
          tic(std::chrono::steady_clock::now()),
          ob(ob_),
          name(info_.name()),
          nchannels(nchannels_), /* Conversion int->int32_t is okay b/c parser only accepts values that fit*/
          nsamples(nsamples_),   /* Parser only accepts values>0, so no problems b/c of signedness */
          chunksize(chunksize_), /* Parser only accepts values>0, so no problems b/c of signedness */
          bufsize(0),
          n_new_samples(0)
          {
            if (nchannels != 0 && nchannels != info_.channel_count())
              throw MHA_Error(__FILE__, __LINE__,
                              "Expected %i channels in lsl stream, got %zu", nchannels,
                              nsamples);
            // channel_count will not change, we can set it and forget about it
            if (nchannels == 0)
              nchannels = info_.channel_count();
            // Precedence for channel bufsize=maximum number of elements in AC variable:
            // 1. nsamples*nchannels (fixed size)
            // 2. chunksize*nchannels (maximum number of samples that can arrive simultaneously)
            // 3. nchannels (can not go smaller)
            if(nsamples)
              bufsize = nsamples * nchannels;
            else if(chunksize)
              bufsize = chunksize*nchannels;
            else
              bufsize=nchannels;
            buf.resize(bufsize);
            std::fill(buf.begin(), buf.end(), 0.0f);
            ts_buf.resize(bufsize / nchannels);
            std::fill(ts_buf.begin(),ts_buf.end(),0.0);
            stream.open_stream();
            // According to lsl doc the first call to time_correction takes several ms,
            // subsequent calls are 'instant'. Calling it here to avoid blocking later.
            tc = stream.time_correction();
            cv.stride = info_.channel_count();
            cv.num_entries = info_.channel_count() * nsamples; // Not problematic if zero initially - will be reset on first pull
            cv.data_type = type_;
            cv.data = buf.data();
            ac.handle->insert_var(info_.name(), cv);
            ts.stride = 1;
            ts.num_entries = nsamples;
            ts.data_type = MHA_AC_DOUBLE;
            ts.data = ts_buf.data();
            insert_vars();
          } catch (MHA_Error &e) {
      // The framework can handle MHA_Errors. Just re-throw
      throw;
    } catch (std::exception &e) {
      // LSL does not document its exceptions very well, but all exceptions I
      // encountered inherit from std::exception Rethrow as MHA_Error
      throw MHA_Error(__FILE__, __LINE__, "Could not subscribe to stream %s: %s",
                      info_.name().c_str(), e.what());
    } catch (...) {
      // No matter what happened before, throw an MHA_Error so that the framework
      // can handle that
      throw MHA_Error(__FILE__, __LINE__,
                      "Could not subscribe to stream %s: Unknown error",
                      info_.name().c_str());
    };
    /** Get stream info object from stream inlet */
    lsl::stream_info info() override {return stream.info();};
    /** Receive a samples from lsl and copy to AC space. Handling of underrun is configuration-dependent */
    void receive_frame() override{
      if(skip){
        return;
      }
      else {
        n_new_samples=0;
        if (ob == overrun_behavior::Ignore) {
          pull_samples_ignore();
        } else if (ob == overrun_behavior::Discard) {
          pull_samples_discard();
        } else {
          throw MHA_Error(__FILE__, __LINE__,
                          "Bug: Unknown overrun behavior %i",
                          static_cast<int>(ob));
        }
        get_time_correction();
        insert_vars();
      }};
  private:
    /** LSL stream outlet. Interface to lsl */
    lsl::stream_inlet stream;
    /** Data buffer of the ac variable. */
    std::vector<T> buf;
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
    void pull_samples_ignore() {
      try{
        std::size_t n = 0;
        n = stream.pull_chunk_multiplexed(buf.data(), ts_buf.data(), bufsize,
                                          bufsize / nchannels,
                                          /*timeout =*/0.0);
        if (n<bufsize and nsamples != 0) {
          std::rotate(buf.begin(), buf.begin() + n, buf.end());
          std::rotate(ts_buf.begin(), ts_buf.begin() + n / nchannels, ts_buf.end());
        } else {
          cv.num_entries = n;
          ts.num_entries = n / nchannels;
        }
        n_new_samples = n / nchannels;
      }
      // If the stream is recoverable, lsl does not throw, instead tries to
      // recover.
      // Handle any other exception as a permanent underrun
      catch (...) {
        n_new_samples = 0;
        stream.close_stream();
        skip = true;
      }
    };
    /** Pull new samples as long as there are samples ready for pickup in the
     * LSL buffers. If nsamples=0, leaves the buffers in a state where the
     * newest samples are at the beginning of the buffers, the state of the
     * older samples is undefined and n_new_samples contains total number of new
     * samples per channel. If nsamples is non-zero, the buffers are rotated so
     * the oldest samples come first. n_samples_new then contains the total number
     * of new samples per channel. If n_new_samples is larger than the number of samples in
     * the AC variable that means samples had be discarded. */
    void pull_samples_discard() {
      try{
        std::size_t n = 0 ;
        do {
          n = stream.pull_chunk_multiplexed(buf.data(), ts_buf.data(), bufsize,
                                            bufsize / nchannels,
                                            /*timeout =*/0.0);
          if (n < bufsize and nsamples != 0) {
            std::rotate(buf.begin(), buf.begin() + n, buf.end());
            std::rotate(ts_buf.begin(), ts_buf.begin() + n / nchannels, ts_buf.end());
          } else {
            cv.num_entries = n;
            ts.num_entries = n / nchannels;
          }
          n_new_samples += n / nchannels;
        } while (stream.samples_available());
      }
      // If the stream is recoverable, lsl does not throw, instead tries to
      // recover.
      // Handle any other exception as a permanent underrun
      catch (...) {
        n_new_samples = 0;
        stream.close_stream();
        skip = true;
      }
    };
    /** Refresh time correction value every 5s */
    void get_time_correction() {
      auto toc=std::chrono::steady_clock::now();
      if(toc-tic>std::chrono::seconds(5)){
        tc=stream.time_correction();
        tic=toc;
      }
    };
    /** Insert stream value, time stamp and time offset into ac space*/
    void insert_vars(){
      ac.handle->insert_var(name,cv);
      ac.handle->insert_var(ts_name, ts);
      ac.handle->insert_var_double(tc_name, &tc);
      ac.handle->insert_var_int(new_name, &n_new_samples);
    };
  };

  /** Specialication for marker streams */
  template<>
  class save_var_t<std::string> : public save_var_base_t {
  public:
    // Do not allow copies or moves to avoid reasoning about copies or moves of streams and ac vars
    save_var_t(const save_var_t&)=delete;
    save_var_t(save_var_t&&)=delete;
    virtual ~save_var_t()=default;
    save_var_t& operator=(const save_var_t&)=delete;
    save_var_t& operator=(save_var_t&&)=delete;
    /** C'tor of lsl to ac bridge.
     * @param name_ Name of LSL stream to be received
     * @param info_ LSL stream info object containing metadata
     * @param ac_ Handle to ac space
     * @param ob_ Overrun behavior. 0=Discard oldest, 1=Ignore
     * @param buflen_ LSL buffer size
     * @param chunksize_ LSL chunk size
     * @param strlen_ Length string buffer. 
     */
    // This is a function try block. Really ugly syntax but the only way to handle exceptions
    // in the ctor initializer list. See http://www.gotw.ca/gotw/066.htm
    save_var_t(const lsl::stream_info &info_,
               const algo_comm_t &ac_,
               overrun_behavior ob_,
               int buflen_,
               int chunksize_,
               int strlen_) try
      : stream(info_, buflen_, chunksize_),
          ac(ac_),
          ts_name(info_.name() + "_ts"),
          tc_name(info_.name() + "_tc"),
          new_name(info_.name() + "_new"),
          tic(std::chrono::steady_clock::now()),
          ob(ob_),
          name(info_.name())
          {
            stream.open_stream();
            if(stream.info().channel_count()!=1)
              throw MHA_Error(__FILE__,__LINE__,"Expected exactly one channel for marker streams. Got %i",stream.info().channel_count());
            // According to lsl doc the first call to time_correction takes several ms,
            // subsequent calls are 'instant'. Calling it here to avoid blocking later.
            tc = stream.time_correction();
            str.reserve(strlen_);
            buf.resize(strlen_);
            cv.stride = 1;
            cv.num_entries = 0; // Not problematic if zero initially - will be reset on first pull
            cv.data_type = MHA_AC_CHAR;
            cv.data = &buf[0];
            ac.handle->insert_var(info_.name(), cv);
            insert_vars();
          } catch (MHA_Error &e) {
      // The framework can handle MHA_Errors. Just re-throw
      throw;
    } catch (std::exception &e) {
      // LSL does not document its exceptions very well, but all exceptions I
      // encountered inherit from std::exception Rethrow as MHA_Error
      throw MHA_Error(__FILE__, __LINE__, "Could not subscribe to stream %s: %s",
                      info_.name().c_str(), e.what());
    } catch (...) {
      // No matter what happened before, throw an MHA_Error so that the framework
      // can handle that
      throw MHA_Error(__FILE__, __LINE__,
                      "Could not subscribe to stream %s: Unknown error",
                      info_.name().c_str());
    };
    /** Get stream info object from stream inlet */
    lsl::stream_info info() override {return stream.info();};
    /** Receive a samples from lsl and copy to AC space. Handling of underrun is configuration-dependent */
    void receive_frame() override{
      if(skip){
        return;
      }
      else {
        if (ob == overrun_behavior::Ignore) {
          pull_samples_ignore();
        } else if (ob == overrun_behavior::Discard) {
          pull_samples_discard();
        } else {
          throw MHA_Error(__FILE__, __LINE__,
                          "Bug: Unknown overrun behavior %i",
                          static_cast<int>(ob));
        }
        get_time_correction();
        insert_vars();
      }};
  private:
    /** LSL stream outlet. Interface to lsl */
    lsl::stream_inlet stream;
    /** Temporary storage for marker string */
    std::string str;
    /** Data buffer of the ac variable. */
    std::vector<char> buf;
    /** Timestamp */
    double ts;
    /** Handle to AC space */
    const algo_comm_t& ac;
    /** Timeseries AC variable */
    comm_var_t cv;
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
    /** Copy string to AC buffer, stop at the latest at buffer end, make sure that the string is always zero-terminated
     * @return The number of characters copied into the AC buffer, also the number entries in the AC variable
     */
    std::size_t copy_string_safe(){
      std::size_t n=std::min(str.size(),buf.size());
      if(n==0)
        return 0;
      std::copy_n(str.begin(),n,buf.begin());
      if(n<buf.size()-1){
        buf[n]='\0';
        // return n+1 -> include the zero byte into the regular entries of the AC variable
        // this is safe b/c we just checked that buf.size()>n-1, i.e. we own at least n+1 bytes
        return n+1;
      }
      else
        buf.back()='\0';
      return n;
    }
    /** Pull new samples, ignore overrun, i.e. only pull one sample from the buffer, do not check if there are newer ones waiting*/
    void pull_samples_ignore() {
      try{
        ts=stream.pull_sample(&str,1,0);
        if (ts!=0) {
          cv.num_entries = copy_string_safe();
        }
        else
          {
            cv.num_entries=0;
          }
      }
      // If the stream is recoverable, lsl does not throw, instead tries to
      // recover.
      // Handle any other exception as a permanent underrun
      catch (...) {
        cv.num_entries=0;
        stream.close_stream();
        skip = true;
      }
    };
    /** Pull new samples as long as there are samples ready for pickup in the
     * LSL buffers. Overwrite old markers */
    void pull_samples_discard() {
      try{
        do {
          ts=stream.pull_sample(&str,1,0);
        } while (stream.samples_available());
        if(ts!=0){
          cv.num_entries = copy_string_safe();
        }
        else {
          cv.num_entries=0;
        }

      }
      // If the stream is recoverable, lsl does not throw, instead tries to
      // recover.
      // Handle any other exception as a permanent underrun
      catch (...) {
        cv.num_entries=0;
        stream.close_stream();
        skip = true;
      }
    };
    /** Refresh time correction value every 5s */
    void get_time_correction() {
      auto toc=std::chrono::steady_clock::now();
      if(toc-tic>std::chrono::seconds(5)){
        tc=stream.time_correction();
        tic=toc;
      }
    };
    /** Insert stream value, time stamp and time offset into ac space*/
    void insert_vars(){
      cv.data = &buf[0];

      ac.handle->insert_var(name,cv);
      ac.handle->insert_var_double(ts_name, &ts);
      ac.handle->insert_var_double(tc_name, &tc);
    };
  };


  /** Runtime configuration class of the lsl2ac plugin */
  class cfg_t {
    /** Maps variable name to unique ptr's of lsl to ac bridges. */
    std::map<std::string, std::unique_ptr<save_var_base_t>> varlist;
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
