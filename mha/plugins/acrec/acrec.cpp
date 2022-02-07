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

#include "acrec.hh"
#include <ctime>
#include <atomic>
#include <vector>

using namespace plugins::hoertech::acrec;

std::string plugins::hoertech::acrec::to_iso8601(time_t tm){
    // time zone handling is too painful, omit time zone
    char buf[18];
    int res=std::strftime(buf,sizeof(buf),"%Y-%m-%dT%H%M%S",std::localtime(&tm));
    if(res==0)
        throw MHA_Error(__FILE__,__LINE__,"Bug: Date string does not fit in buffer.");
    return std::string(buf);
}

acrec_t::acrec_t(algo_comm_t iac, const std::string & )
    : MHAPlugin::plugin_t<acwriter_t>("ac variable file recorder",iac),
      ac(iac)
{
    insert_member(fifolen);
    insert_member(minwrite);
    insert_member(prefix);
    insert_member(use_date);
    insert_member(varname);
    insert_member(record);
    patchbay.connect(&record.writeaccess,this,&acrec_t::start_new_session);
}

template <class mha_signal_t> mha_signal_t* acrec_t::process(mha_signal_t* s)
{
    poll_config();
    cv = ac.handle->get_var(cfg->get_varname());
    cfg->process(&cv);
    return s;
}

void acrec_t::prepare(mhaconfig_t&)
{
    // Need to ensure there's a valid configuration after prepare()
    // If record has not been set, there is no configuration, so create one.
    // This can not cause spurious creation of new files because setting record
    // to true would have caused the creation of a new config.
    if(!peek_config())
        push_config(new acwriter_t(record.data, fifolen.data, minwrite.data,
                                   prefix.data, use_date.data, varname.data));
}

void acrec_t::release()
{
    auto latest_cfg=peek_config();
    if(latest_cfg)
        latest_cfg->exit_request();
}

void acrec_t::start_new_session()
{
    auto latest_cfg=peek_config();
    if(latest_cfg)
        latest_cfg->exit_request();
    push_config(new acwriter_t(record.data, fifolen.data, minwrite.data,
                               prefix.data, use_date.data, varname.data));
}

void acwriter_t::create_datafile(const std::string& prefix, bool use_date)
{
    std::string fname;
    if( use_date ){
        fname = prefix+"-"+to_iso8601(std::time(nullptr))+".dat";
    }
    else{
        fname=prefix+".dat";
    }
    outfile=std::fstream(fname, std::ios::out | std::ios::binary);
    if( !outfile.good() )
        throw MHA_Error(__FILE__,__LINE__,"Unable to create file %s.",fname.c_str());
}

acwriter_t::acwriter_t(bool active,unsigned fifosize,unsigned minwrite,
                       const std::string& prefix, bool use_date,
                       const std::string& varname)
    : close_session(false),
      active(active),
      disk_write_threshold_min_num_samples(minwrite),
      varname(varname)
{
    if (active) {
        if (disk_write_threshold_min_num_samples >= fifosize) {
            throw MHA_Error(__FILE__,__LINE__,
                            "minwrite must be less than fifosize "
                            "(minwrite: %u, fifosize: %u)",
                            disk_write_threshold_min_num_samples,
                            fifosize);
        }
        create_datafile(prefix, use_date);
        diskbuffer = std::make_unique<output_type[]>(fifosize);
        fifo = std::make_unique<mha_fifo_lf_t<output_type> >(fifosize);
        writethread=std::thread(&acwriter_t::write_thread, this);
    } else {
        // When inactive, sizes do not need to be checked, no output file,
        // no fifo, no disk buffer, and no thread needs to be created.
    }
    // std::string implementations may allocate memory on first invocation
    // of c_str() after a change.  Avoid allocation during processing:
    (void)get_varname();
}

void acwriter_t::process(comm_var_t* s)
{
    if( active ) {
        if (not is_num_channels_known) {
            num_channels = s->stride;
            is_complex = (s->data_type == MHA_AC_MHACOMPLEX);
            is_num_channels_known = true;
        }
        if (num_channels != s->stride) {
            throw MHA_Error(__FILE__,__LINE__,"Number of channels of AC"
                            " variable %s has changed from %u to %u",
                            get_varname().c_str(), num_channels, s->stride);
        }
        if (is_complex != (s->data_type == MHA_AC_MHACOMPLEX)) {
            throw MHA_Error(__FILE__,__LINE__,"AC variable %s has changed from"
                            " %s numeric type to %s numeric type",
                            get_varname().c_str(),
                            is_complex ? "complex" : "real",
                            (s->data_type==MHA_AC_MHACOMPLEX)?"complex":"real");
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
                     s->num_entries * complex_factor);
        constexpr unsigned PUSH_ARRAY_SIZE = 32U;
        static_assert(PUSH_ARRAY_SIZE % 2U == 0U,
                      "Needs to be even for the complex numbers");

        output_type value_to_push[PUSH_ARRAY_SIZE];
        for (unsigned index = 0;
             index < number_of_values_to_push_to_fifo;
             index += PUSH_ARRAY_SIZE) {
            const unsigned num_values_to_copy =
                std::min(PUSH_ARRAY_SIZE,
                         number_of_values_to_push_to_fifo - index);
            for (unsigned subindex = 0; subindex<num_values_to_copy; ++subindex)
                switch (s->data_type) {
                case MHA_AC_INT:
                    value_to_push[subindex] =
                        static_cast<int*>(s->data)[index + subindex];
                    break;
                case MHA_AC_FLOAT:
                    value_to_push[subindex] =
                        static_cast<float*>(s->data)[index + subindex];
                    break;
                case MHA_AC_DOUBLE:
                    value_to_push[subindex] =
                        static_cast<double*>(s->data)[index + subindex];
                    break;
                case MHA_AC_MHAREAL:
                case MHA_AC_MHACOMPLEX: // alternating real and imag mha_real_t
                    value_to_push[subindex] =
                        static_cast<mha_real_t*>(s->data)[index + subindex];
                    break;
                default:
                    throw MHA_Error(__FILE__,__LINE__,
                                    "Data type not supported: %u",
                                    s->data_type);
                }
            fifo->write(value_to_push, num_values_to_copy);
        }
    }
}

void acwriter_t::exit_request(){
    if(!close_session.load()){
        close_session.store(true);
        if( active ){
            writethread.join();
            outfile.close();
            // Deallocate disk buffer.
            diskbuffer = nullptr;
            // We cannot deallocate the fifo here because process() may still
            // use it concurrently.  Will be deallocated by destructor.
        }
    }
    return;
}

void acwriter_t::write_thread()
{
    auto flush = // Writes fifo content to disk.
        [&]()
        {
            // Allow saving AC vars with stride zero, interpret as stride one.
            unsigned num_ch_effective =
                std::max(num_channels, 1U) * (is_complex ? 2U : 1U);
            unsigned frames = fifo->get_fill_count() / num_ch_effective;
            if (outfile.good() && frames > 0U) {
                fifo->read(diskbuffer.get(),frames*num_ch_effective);
                outfile.write(reinterpret_cast<const char *>(diskbuffer.get()),
                              frames*num_ch_effective*sizeof(output_type));
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

MHAPLUGIN_CALLBACKS(acrec,acrec_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(acrec,acrec_t,spec,spec)
MHAPLUGIN_DOCUMENTATION\
(acrec,
 "data-export disk-files",
 "AC variable file recorder plugin. This plugin writes the values of an ac variable to"
 " a file in a thread-safe manner. It always records the value that is current when its process"
 " callback is called, i.e. if an ac variable is written to by multiple plugins, only the final values"
 " are committed to file, intermediary values are lost."
 " A new data file is opened every time the record variable is set to yes. The file is"
 " closed on any of \"cmd=release\", \"cmd=quit\" or \"record=no\". Note that \"cmd=stop\" does not"
 " close the data file. After the the close command is given, it can take an unspecified, but usually small amount"
 " amount of time until the file is actually closed and ready for further processing. \n"
 " The name (and path) of the output file is chosen by the prefix configuration variable. By default the current"
 " date and time and the file name extension \".dat\" are appended to the file"
 " name, this behaviour can be influenced by the \"use\\_date\" variable. The date"
 " format follows ISO 8601 extended format omitting colons and time zone information, "
 " so e.g. November 5, 1994, 8:15:30 corresponds to 1994-11-05T081530. \n"
 " Only AC variables of numeric types can be stored into a file by this plugin."
 " Regardless of the data type of the AC variable, the data is converted to "
 " data type double and stored as binary data in host byte order.\n"
 " Complex data are stored storing real part and imaginary part consecutively."
 " No metadata is stored in the file.\n\n"
 " The AC variable may change the number of elements that it contains from"
 " one process call to the next, but its stride (e.g. number of channels or"
 " number of bins) must remain constant.\n\n"
 " If more data arrives through the process callback than can be written to"
 " disk in the same time, then some of the incoming data will have to be"
 " discarded before writing to disk continues.\n"
 " This may e.g. happen with slow disks like network drives or SD cards, or"
 " with very high data rates.\n\n"
 "The \"fifolen\" and \"minwrite\" variables control the behaviour of the fifo buffer and should usually remain unchanged.")

/*
 * Local Variables:
 * compile-command: "make"
 * c-basic-offset: 4
 * End:
 */
