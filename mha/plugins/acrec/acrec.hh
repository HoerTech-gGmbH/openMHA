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


#include "mha_plugin.hh"
#include "mha_fifo.h"
#include <thread>
#include <fstream>

namespace plugins { namespace hoertech { namespace acrec {
/// acwriter_t decouples signal processing from writing to disk.
/// Data arriving in numeric AC variables is converted to data type double,
/// placed into a fifo pipeline to transport the data from the signal
/// processing thread to the disk writing thread, and finally written to
/// disk in chunks of at least minwrite numbers.  All numbers are written to
/// disk as binary doubles (8 bytes) in host byte order.
class acwriter_t {
public:
    /// The numeric data type used for outputting the data to disk.
    typedef double output_type;
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
    /// @param prefix   Path and start of output file name.  Will be extended
    ///                 with file name extension ".dat".
    /// @param use_date When true, the current date and time will be appended
    ///                 to the output file name before the file name extension.
    /// @param varname  Name of AC variable to save into file.  Can be accessed
    ///                 through getter method get_varname().  Stored here
    ///                 to avoid races between processing thread and
    ///                 configuration thread.
    acwriter_t(bool active, unsigned fifosize, unsigned minwrite,
               const std::string& prefix, bool use_date,
               const std::string& varname);
    /// Deallocates memory but does not terminate the write_thread.
    /// write_thread must be terminated before the destructor executes by
    /// calling exit_request.
    ~acwriter_t() = default;
    /// Place the data present in the algorithm communication variable into the
    /// fifo for output to disk.
    void process(comm_var_t*);
    /// Terminate output thread.
    void exit_request();
    /// getter for ac variable name @return name as char* as needed by get_var
    const char * get_varname() const {return varname.c_str();}
private:
    /// Main method of the disk writer thread.  Periodically wakes up and checks
    /// if data needs to be written to disk.
    void write_thread();
    /// Open data file for output. Combine prefix, date, and file name extension
    /// @param prefix   Path and start of output file name.  Will be extended
    ///                 with file name extension ".dat".
    /// @param use_date When true, the current date and time will be appended
    ///                 to the output file name before the file name extension.
    void create_datafile(const std::string& prefix, bool use_date);
    /// cross-thread-synchronization.  write_thread() terminates after this is
    /// set to true by exit_request().
    std::atomic<bool> close_session;
    /// The writer thread and the output file will only be created when active
    /// is true.
    const bool active;
    /// Fifo for decoupling signal processing thread from disk writer thread.
    std::unique_ptr<mha_fifo_lf_t<output_type> > fifo;
    /// Minimum number of samples that need to be waiting in the fifo before
    /// the disk writer thread writes them to disk.
    const unsigned int disk_write_threshold_min_num_samples;
    /// The thread that writes to disk.
    std::thread writethread;
    /// Intermediate buffer to receive data from fifo and store on disk.
    std::unique_ptr<output_type[]> diskbuffer;
    /// Ouput file.
    std::fstream outfile;
    /// Number of channels of AC variable using stride.  If the number of
    /// channels changes during processing, an exception is thrown.
    unsigned num_channels = 0U;
    /// The number of channels is determined during the first process callback.
    /// is_num_channels_known is set to true after the first process callback.
    bool is_num_channels_known = false;
    /// If the AC variable is of complex valued type or not.  If this changes
    /// during processing, then an exception is thrown.
    bool is_complex = false;
    /// The name of the ac variable to publish.
    const std::string varname;
};

/// Plugin interface class of plugin acrec.
class acrec_t : public MHAPlugin::plugin_t<acwriter_t> {
public:
    /// Process callback.  Pushes the data from one AC variable into the fifo.
    /// @return the unmodified input signal.
    /// @param s input signal.  The audio signal is not used or modified. 
    template <class mha_signal_t> mha_signal_t* process(mha_signal_t* s);
    /// Prepare callback.  acrec does not modify the signal parameters.
    /// @param cf The signal parameters.
    void prepare(mhaconfig_t& cf);
    /// Ensure recorded data is flushed to disk.
    void release();
    /// Plugin interface constructor.
    /// @param iac Algorithm communication variable space.
    acrec_t(algo_comm_t iac, const std::string & configured_name);
private:
    /// Configuration callback called whenever configuaration variable "record"
    /// is written to.
    void start_new_session();
    MHAParser::bool_t record =
        {"Recording session. Each write access will finalize the previous\n"
         "recording session. Each write access with value \"yes\" will start\n"
         "a new recording session into a new or re-opened output file.", "no"};
    MHAParser::int_t fifolen =
        {"Length of FIFO in samples","262144","[2,]"};
    MHAParser::int_t minwrite =       
        {"Minimal write length (must be less then fifolen)","65536","[1,]"};
    MHAParser::string_t prefix =
        {"Path (including path delimiter) and file prefix",""};
    MHAParser::string_t varname =
        {"Name of AC variable",""};
    MHAParser::bool_t use_date =
        {"Use date and time (yes), or only prefix (no)","yes"};
    MHAEvents::patchbay_t<acrec_t> patchbay;
    comm_var_t cv;
    algo_comm_t ac;
};

std::string to_iso8601(time_t tm);

    } //namespace acrec
  } // namespace hoertech
} // namespace plugins
