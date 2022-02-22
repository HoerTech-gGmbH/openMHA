// This file is part of the HörTech Open Master Hearing Aid (openMHA)
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

#include "ac2xdf.hh"
#include <sstream>
#include <unordered_map>
#include <typeindex>
#include <typeinfo>
#include <algorithm>
namespace ac2xdf {

  const std::unordered_map<std::type_index, std::string>
  types={
    {std::type_index(typeid(int)),"int32"},
    {std::type_index(typeid(mha_real_t)),"float32"},
    {std::type_index(typeid(double)),"double64"},
    {std::type_index(typeid(char)),"string"}
  };

  output_file_t::output_file_t(const std::string& prefix, bool use_date){
    std::string fname;
    if( use_date ){
      fname = prefix+"-"+to_iso8601(std::time(nullptr))+".xdf";
    }
    else{
      fname=prefix+".xdf";
    }
    try{
      outfile=std::make_unique<XDFWriter>(fname);
    }
    catch(...){
      throw MHA_Error(__FILE__,__LINE__,"Unable to create file %s.",fname.c_str());
    }
  }

  void output_file_t::initialize_stream(uint32_t stream_id,
                                        const std::string& varname,
                                        const std::string& channel_format,
                                        unsigned num_channels,
                                        double sampling_rate){
    std::string formats[]={"int8", "int16", "int32", "int64","float32" ,"double64" , "string"};
    if(std::none_of(std::begin(formats),std::end(formats),[&](auto s){return s==channel_format;}))
      throw MHA_Error(__FILE__,__LINE__,"Channel format %s is unknown",channel_format.c_str());
    std::unique_lock flock(write_lock);
    outfile->write_stream_header(stream_id, std::string("<?xml version=\"1.0\"?><info><name>")+
                                 varname+std::string("</name><channel_count>")+
                                 std::to_string(num_channels)+
                                 "</channel_count><nominal_srate>"+std::to_string(sampling_rate)+"</nominal_srate>"
                                 "<channel_format>"+channel_format+
                                 "</channel_format></info>");
    outfile->write_boundary_chunk();
  }

  template<typename T>
  void output_file_t::write(uint32_t stream_id, const T* buf, std::size_t frames, std::size_t channels){
    std::unique_lock flock(write_lock);
    std::vector<double> timestamps(frames,0);
    outfile->write_data_chunk(stream_id,timestamps,buf,frames, channels);
  }

  void output_file_t::close_stream(uint32_t stream_id){
    std::unique_lock flock(write_lock);
    outfile->write_boundary_chunk();
    const std::string footer=std::string("<?xml version=\"1.0\"?>""<info>""</info>");
    outfile->write_stream_footer(stream_id, footer);;
  }


  template<typename T>
  acwriter_t<T>::acwriter_t(bool active,unsigned fifosize,unsigned minwrite,
                            const std::string& varname,
                            double sampling_rate,
                            output_file_t* outfile,
                            uint32_t stream_id)
    : close_session(false),
      active(active),
      disk_write_threshold_min_num_samples(minwrite),
      outfile(outfile),
      varname(varname),
      stream_id(stream_id),
      sampling_rate(sampling_rate)
  {
    if (active) {
      if (disk_write_threshold_min_num_samples >= fifosize) {
        throw MHA_Error(__FILE__,__LINE__,
                        "minwrite must be less than fifosize "
                        "(minwrite: %u, fifosize: %u)",
                        disk_write_threshold_min_num_samples,
                        fifosize);
      }
      diskbuffer = std::make_unique<T[]>(fifosize);
      fifo = std::make_unique<mha_fifo_lf_t<T> >(fifosize);
      writethread=std::thread(&acwriter_t::write_thread, this);
      // Would normaly initialize stream now, but need to know number of channels
      // and data type -> Defer until we first write a chunk to file.
    } else {
      // When inactive, sizes do not need to be checked, no output file,
      // no fifo, no disk buffer, and no thread needs to be created.
    }
    // std::string implementations may allocate memory on first invocation
    // of c_str() after a change.  Avoid allocation during processing:
    (void)get_varname();
  }

  template<typename T>
  void acwriter_t<T>::process(comm_var_t& s) {
    if( active ) {
      if (not is_num_channels_known) {
        num_channels = s.stride;
        data_type = s.data_type;
        is_complex = (s.data_type == MHA_AC_MHACOMPLEX);
        is_num_channels_known = true;
      }
      if (num_channels != s.stride) {
        throw MHA_Error(__FILE__,__LINE__,"Number of channels of AC"
                        " variable %s has changed from %u to %u",
                        get_varname(), num_channels, s.stride);
      }
      if (data_type!= s.data_type) {
         throw MHA_Error(__FILE__,__LINE__,"AC variable %s has changed from"
                         " %u type to %u type",get_varname(),data_type,
                         s.data_type);
      }
      if (is_complex != (s.data_type == MHA_AC_MHACOMPLEX)) {
        throw MHA_Error(__FILE__,__LINE__,"AC variable %s has changed from"
                        " %s numeric type to %s numeric type",
                        get_varname(), is_complex ? "complex" : "real",
                        (s.data_type==MHA_AC_MHACOMPLEX)?"complex":"real");
      }
      // We save complex values as alternating real and imaginary part, i.e.
      // each complex is stored as two values.
      const unsigned complex_factor = is_complex ? 2U : 1U;
      // Allow saving AC vars with stride zero, interpret as stride one.
      const unsigned num_ch_effective =
        std::max(num_channels, 1U) * complex_factor;
      const unsigned number_of_values_to_push_to_fifo =
        std::min(fifo->get_available_space() / num_ch_effective
                 * num_ch_effective, // only write multiples of num_channels
                 s.num_entries * complex_factor);
      fifo->write(static_cast<T*>(s.data),number_of_values_to_push_to_fifo);
    }
  }

  /// Terminate output thread.
  template<typename T> void acwriter_t<T>::exit_request() {
    // exchange() to prevent trying to join() twice
    if(!close_session.exchange(true)){
      if( active ){
        writethread.join();
        outfile->close_stream(stream_id);
        // Deallocate disk buffer.
        diskbuffer = nullptr;
        // We cannot deallocate the fifo here because process() may still
        // use it concurrently.  Will be deallocated by destructor.
      }
    }
    return;
  }

  template<typename T> void acwriter_t<T>::write_thread()  {
    auto flush = // Writes fifo content to disk.
      [&]()
      {
        // Allow saving AC vars with stride zero, interpret as stride one.
        unsigned num_ch_effective =
          std::max(num_channels, 1U) * (is_complex ? 2U : 1U);
        if (not is_stream_initialized){
          outfile->initialize_stream(stream_id,varname,types.at(std::type_index(typeid(T))),num_ch_effective, sampling_rate);
          is_stream_initialized=true;
        }
        unsigned frames = fifo->get_fill_count() / num_ch_effective;
        if (frames > 0U) {
          fifo->read(diskbuffer.get(),frames*num_ch_effective);
          if constexpr(std::is_same<std::decay_t<T>,char>::value){
            std::stringstream s;
            // Nota bene: stringstreams are not FILEs. The read and write positions are independent!
            s.write((char*)diskbuffer.get(),frames*num_ch_effective);
            for (std::string line; std::getline(s, line, '\0'); ) {
              outfile->write(stream_id,&line,1,1);
            }
          }
          else{
            outfile->write(stream_id,diskbuffer.get(),frames,num_ch_effective);
          }
        }
      };

    while(!close_session.load()){
      mha_msleep(1);
      if (fifo->get_fill_count() > disk_write_threshold_min_num_samples) {
        flush();
      }
    }
    flush();
  }


  class ac2xdf_rt_t {
  public:
    ac2xdf_rt_t(const std::string prefix,
                bool use_date, bool active,
                unsigned fifosize,
                unsigned minwrite,
                const std::vector<std::string>& varnames,
                const std::vector<mha_real_t>& sampling_rates,
                algo_comm_t & iac):
      ac(iac){
      try{
        MHASignal::dupvec_chk(sampling_rates,varnames.size());
      } catch(MHA_Error & e) {
        throw MHA_Error(__FILE__,__LINE__,
                        "Sampling rate vector must have 1 or %zu elements. Got : %zu",
                        varnames.size(),
                        sampling_rates.size());
      }
      if(active){
        outfile=std::make_unique<output_file_t>(prefix, use_date);
        
        for(std::size_t ii=0;ii<varnames.size();++ii){
          unsigned id=ii+1;
          auto cv=ac.get_var(varnames[ii]);
            switch(cv.data_type){
            case MHA_AC_DOUBLE:
              vars.emplace_back(std::make_unique<acwriter_t<double>>(active,fifosize,minwrite,varnames[ii],sampling_rates[ii],outfile.get(),id));
              break;
            case MHA_AC_FLOAT:
              vars.emplace_back(std::make_unique<acwriter_t<float>>(active,fifosize,minwrite,varnames[ii],sampling_rates[ii],outfile.get(),id));
              break;
            case MHA_AC_MHAREAL:
              vars.emplace_back(std::make_unique<acwriter_t<mha_real_t>>(active,fifosize,minwrite,varnames[ii],sampling_rates[ii],outfile.get(),id));
              break;
            case MHA_AC_CHAR:
              vars.emplace_back(std::make_unique<acwriter_t<char>>(active,fifosize,minwrite,varnames[ii],sampling_rates[ii],outfile.get(),id));
              break;
            case MHA_AC_INT:
              vars.emplace_back(std::make_unique<acwriter_t<int>>(active,fifosize,minwrite,varnames[ii],sampling_rates[ii],outfile.get(),id));
              break;
            case MHA_AC_MHACOMPLEX:
              vars.emplace_back(std::make_unique<acwriter_t<mha_real_t>>(active,fifosize,minwrite,varnames[ii],sampling_rates[ii],outfile.get(),id));
              break;
            default:
              throw MHA_Error(__FILE__,__LINE__,"Unknown data type of variable %s: %u",varnames[ii].c_str(),cv.data_type);
            }
        }
      }
    }
    
    void process(){
      for(auto& var : vars){
        auto cv=ac.get_var(var->get_varname());
        var->process(cv);
      }
    }

    void exit_request(){
      for(auto& var : vars){
        var->exit_request();
      }
    }

  private:
    MHA_AC::algo_comm_t & ac;
    std::vector<std::unique_ptr<acwriter_base_t>> vars;
    std::unique_ptr<output_file_t> outfile;
  };


  /// Plugin interface class of plugin ac2xdf.
  class ac2xdf_if_t : public MHAPlugin::plugin_t<ac2xdf_rt_t> {
  public:
    /// Process callback.  Pushes the data from one AC variable into the fifo.
    /// @return the unmodified input signal.
    /// @param s input signal.  The audio signal is not used or modified.
    template <class mha_signal_t> mha_signal_t* process(mha_signal_t* s){
      poll_config();
      cfg->process();
      return s;
    }
    /// Prepare callback.  ac2xdf does not modify the signal parameters.
    /// @param cf The signal parameters.
    void prepare(mhaconfig_t&){
      // We want to start recording, so push a new config (start_new_session returns early before prepare())
      if(record.data)
        push_config(new ac2xdf_rt_t(prefix.data, use_date.data,record.data, fifolen.data, minwrite.data,
                                    varnames.data, nominal_sampling_rates.data, ac));
      // We need to ensure there's a valid configuration after prepare()
      // If record has not been set, there is no configuration, so
      // push a dummy config that's set to inactive, so it does nothing
      else if(!peek_config()){
        push_config(new ac2xdf_rt_t("",false,false,0,0,{},{},ac));
      }
      else{
        ; // Record is false, but we already have a valid config: Do nothing
      }
    }

    /// Ensure recorded data is flushed to disk.
    void release(){
      // Ensure there's an exit request. Multiple exit requests for the same config do not hurt.
      auto latest_cfg=peek_config();
      if(latest_cfg)
        latest_cfg->exit_request();
    }

    /// Plugin interface constructor.
    /// @param iac Algorithm communication variable space.
    ac2xdf_if_t(algo_comm_t & iac, const std::string &)
      : MHAPlugin::plugin_t<ac2xdf_rt_t>("ac variable file recorder",iac)
    {
      insert_member(fifolen);
      insert_member(minwrite);
      insert_member(prefix);
      insert_member(use_date);
      insert_member(varnames);
      insert_member(record);
      patchbay.connect(&record.writeaccess,this,&ac2xdf_if_t::start_new_session);
    }

  private:
    /// Configuration callback called whenever configuaration variable "record"
    /// is written to. Always flush the old file if there's one.
    void start_new_session(){
      auto latest_cfg=peek_config();
      if(latest_cfg)
        latest_cfg->exit_request();
      // Only push config after prepare to ensure the AC variables are already created.
      // We do not need to push a config before prepare as we can not record anyway.
      if(is_prepared())
        push_config(new ac2xdf_rt_t(prefix.data, use_date.data,record.data, fifolen.data, minwrite.data,
                                    varnames.data, nominal_sampling_rates.data, ac));
    }
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
    MHAParser::vstring_t varnames =
      {"Name of AC variables",""};
    MHAParser::vfloat_t nominal_sampling_rates =
      {"Nominal sampling rates. Must be >0 or 0 for irregular sampling rate","[0]","[0,]"};
    MHAParser::bool_t use_date =
      {"Use date and time (yes), or only prefix (no)","yes"};
    MHAEvents::patchbay_t<ac2xdf_if_t> patchbay;
  };


  template void output_file_t::write<double>(uint32_t stream_id, const double* buf, std::size_t frames, std::size_t channels);
  template void output_file_t::write<int>(uint32_t stream_id, const int* buf, std::size_t frames, std::size_t channels);
  template void output_file_t::write<char>(uint32_t stream_id, const char* buf, std::size_t frames, std::size_t channels);
  template void output_file_t::write<mha_real_t>(uint32_t stream_id, const mha_real_t* buf, std::size_t frames, std::size_t channels);

  template class acwriter_t<double>;
  template class acwriter_t<int>;
  template class acwriter_t<char>;
  template class acwriter_t<mha_real_t>;

} // namespace ac2xdf





MHAPLUGIN_CALLBACKS(ac2xdf,ac2xdf::ac2xdf_if_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(ac2xdf,ac2xdf::ac2xdf_if_t,spec,spec)

MHAPLUGIN_DOCUMENTATION\
(ac2xdf,
 "data-export disk-files",
 "The 'ac2xdf plugin saves the contents of algorithm communication (AC) variables into XDF files."
 " This plugin writes the values of an ac variable to"
 " an XDF file in a thread-safe manner. It always records the value that is current when its process"
 " callback is called, i.e. if an ac variable is written to by multiple plugins, only the final values"
 " are committed to file, intermediary values are lost. Numeric und character valued AC variables are supported."
 " Complex data are stored storing real part and imaginary part consecutively."
 " The AC variable may change the number of elements that it contains from"
 " one process call to the next, but its stride (e.g. number of channels or"
 " number of bins) must remain constant.\n\n"
 " A new data file is opened every time the \"record\" variable is set to yes after \"cmd=prepare\" is issued. The file is"
 " closed on any of \"cmd=release\", \"cmd=quit\" or \"record=no\". Note that \"cmd=stop\" does not"
 " close the data file. After the the close command is given, it can take an unspecified, but usually small amount"
 " amount of time until the file is actually closed and ready for further processing. \n"
 " The name (and path) of the output file is chosen by the \"prefix\" configuration variable. By default the current"
 " date and time and the file name extension \".dat\" are appended to the file"
 " name, this behaviour can be influenced by the \"use\\_date\" variable. The date"
 " format follows ISO 8601 extended format omitting colons and time zone information, "
 " so e.g. November 5, 1994, 8:15:30 corresponds to 1994-11-05T081530. \n"
 " If more data arrives through the process callback than can be written to"
 " disk in the same time, then some of the incoming data will have to be"
 " discarded before writing to disk continues.\n"
 " This may e.g. happen with slow disks like network drives or SD cards, or"
 " with very high data rates.\n\n"
 "The \"fifolen\" and \"minwrite\" variables control the behaviour of the fifo buffer and should usually remain unchanged."
 )
