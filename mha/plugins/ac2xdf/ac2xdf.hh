// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2022 Hoerzentrum Oldenburg gGmbH
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
#define _CRT_SECURE_NO_WARNINGS
#include "mha_plugin.hh"
#include "mha_fifo.h"
#include "xdfwriter.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <thread>
#include <fstream>
#include <mutex>

namespace ac2xdf {

  std::string to_iso8601(time_t tm){
    // time zone handling is too painful, omit time zone
    char buf[18];
    auto res=std::strftime(buf,sizeof(buf),"%Y-%m-%dT%H%M%S",std::localtime(&tm));
    if(res==0)
      throw MHA_Error(__FILE__,__LINE__,"Bug: Date string does not fit in buffer.");
    return std::string(buf);
  };

  /// output_file_t represents one XDF output file. It wraps around the XDFWriter class,
  /// which handles the conversion of a stream into the bits and bytes on disk. Access
  /// to the output file protected by a lock. There's usually one output file per plugin
  /// instance shared by all acwriters. 
  class output_file_t {
  public:
    /// Constructor
    /// @param prefix   Path and start of output file name.  Will be extended
    ///                 with file name extension ".xdf".
    /// @param use_date When true, the current date and time will be appended
    ///                 to the output file name before the file name extension.
    output_file_t(const std::string& prefix, bool use_date);

    /// Initialize stream. Writes a minimal stream header and a boundary chunk
    /// @param stream_id Numerical stream id.
    /// @param varname Human-readable stream id. Gets saved as stream name in metadata
    /// @param channel_format Data type of the stream, gets written into the channel_format metadata.
    /// must be one of {"int8", "int16", "int32", "int64","float32" ,"double64" , "string"}
    /// @param num_channels Number of channels in stream to be written in metadata
    /// @param sampling_rate Nominal sampling rate in Hz. To be written in metadata.
    /// Zero means irregular rate
    void initialize_stream(uint32_t stream_id,const std::string& varname,
                           const std::string& channel_format, unsigned num_channels,
                           double sampling_rate=0);

    /// Write data chunk to the stream with id stream_id.
    /// @param stream_id The stream id.
    /// @param buf Pointer to buffer containing frames entries. The caller retains ownership of buf.
    /// @param Pointer to buffer containing \ref frames * \ref channels values. Interleaved storage:
    /// The first \ref channels values in memory contain the values of the first frame, etc.
    /// The caller retains ownership of buf.
    /// @param frames Number of entries per channel in buf.
    /// @param channels Number of channels in buf.
    template<typename T=double>
    void write(uint32_t stream_id, const T* buf, std::size_t frames, std::size_t channels);
    /// Close stream with id stream_id by writing stream footer
    /// @param stream_id Numeric ID of the stream to be closed
    void close_stream(uint32_t stream_id);
  private:
    /// Mutex to protect write access to the output file
    std::mutex write_lock;
    /// XDFWriter. Handles the translation into the xdf format and disk writes
    std::unique_ptr<XDFWriter> outfile;
  };

  /// Base class for all acwriter_t's. This class decouples signal processing
  /// from writing to disk. There's one acwriter per AC variable to be written to
  /// disk. Each instance of acwriter spawns its own writer thread and has its own
  /// internal FIFO to safely move the samples of the AC variable out of the processing thread.
  /// All acwriters share an output file. It's not problematic when an acwriter has
  /// to wait for write access because the waiting does happen in its own thread,
  /// not in the audio thread.
  class acwriter_base_t {
  public:
    /// Place the data present in the algorithm communication variable into the
    /// fifo for output to disk.
    virtual void process(comm_var_t&)=0;
    /// Terminate output thread. Returns after exit thread has joined
    virtual void exit_request()=0;
    /// getter for ac variable name @return name as char* as needed by get_var
    virtual const char * get_varname() const = 0;
    virtual ~acwriter_base_t()=default;
  };


  template<typename T>
  class acwriter_t : public acwriter_base_t {
  public:
    /// Constructor allocates fifo and disk output buffer.  It spawns a new
    /// thread for writing data to disk when active==true.  In order to
    /// terminate the thread, method exit_request <b>must</b> be called
    /// before this object is destroyed.
    /// @param active Only write data to disk when this is true.
    /// @param fifosize Capacity of both the fifo pipeline and of the
    ///                 disk buffer.
    /// @param minwrite Wait for a fifo fill count of at least minwrite doubles
    ///                 before flushing the contents of the fifo to disk.
    ///                 Fifo is also flushed before this object is destroyed.
    /// @param varname  Name of AC variable to save into file.  Can be accessed
    ///                 through getter method get_varname().  Stored here
    ///                 to avoid races between processing thread and
    ///                 configuration thread.
    /// @param outfile  Handle to the output file
    /// @param stream_id Numerical id of the stream.
    acwriter_t(bool active,unsigned fifosize,
               unsigned minwrite,
               const std::string& varname,
               double sampling_rate,
               output_file_t* outfile,
               uint32_t stream_id);

    acwriter_t(const acwriter_t&)=delete;
    acwriter_t(acwriter_t&&)=delete;
    /// Deallocates memory but does not terminate the write_thread.
    /// write_thread must be terminated before the destructor executes by
    /// calling exit_request.
    virtual ~acwriter_t() = default;
    void process(comm_var_t & s) override;
    void exit_request() override;
    const char * get_varname() const override {return varname.c_str();}
  private:
    /// Main method of the disk writer thread.  Periodically wakes up and checks
    /// if data needs to be written to disk.
    void write_thread();
    /// cross-thread-synchronization.  write_thread() terminates after this is
    /// set to true by exit_request().
    std::atomic<bool> close_session;
    /// The writer thread and the output file will only be created when active
    /// is true.
    const bool active;
    /// Fifo for decoupling signal processing thread from disk writer thread.
    std::unique_ptr<mha_fifo_lf_t<T> > fifo;
    /// Minimum number of samples that need to be waiting in the fifo before
    /// the disk writer thread writes them to disk.
    const unsigned int disk_write_threshold_min_num_samples;
    /// The thread that writes to disk.
    std::thread writethread;
    /// Intermediate buffer to receive data from fifo and store on disk.
    std::unique_ptr<T[]> diskbuffer;
    /// Ouput file.
    output_file_t* outfile;
    /// Number of channels of AC variable using stride.  If the number of
    /// channels changes during processing, an exception is thrown.
    unsigned num_channels = 0U;
    /// The number of channels is determined during the first process callback.
    /// is_num_channels_known is set to true after the first process callback.
    bool is_num_channels_known = false;
    /// If the AC variable is of complex valued type or not.  If this changes
    /// during processing, then an exception is thrown.
    bool is_complex = false;
    /// Data type of the ac variable. Used to protect against data type change during processing
    unsigned int data_type = MHA_AC_UNKNOWN;
    /// The name of the ac variable to publish.
    const std::string varname;
    const uint32_t stream_id;
    bool is_stream_initialized=false;
    double sampling_rate;
  };
} //namespace ac2xdf
