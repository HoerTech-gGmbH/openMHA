// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2007 2008 2009 2012 2013 2016 2017 2018 2019 2020 HörTech gGmbH
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

#ifndef MHAPLUGINLOADER_H
#define MHAPLUGINLOADER_H

#include "mha_toolbox.h"
#include "mha_os.h"

namespace PluginLoader {

    const char* mhastrdomain(mha_domain_t);

    void mhaconfig_compare(const mhaconfig_t& req, const mhaconfig_t& avail,const std::string& pref = "");

    class config_file_splitter_t
    {
    public:
        config_file_splitter_t(const std::string& name);
        const std::string& get_configname() const {return configname;};
        const std::string& get_libname() const {return libname;};
        const std::string& get_origname() const {return origname;};
        const std::string& get_configfile() const {return configfile;};
    private:
        std::string libname;
        std::string configname;
        std::string origname;
        std::string configfile;
    };

/** This abstract class defines the interface for classes
 * that implement all types of signal domain processing supported by
 * the MHA: wave2wave, spec2spec, wave2spec, and spec2wave.
 * 
 * For supporting different output domains for the same input domain,
 * the processing methods are overloaded with respect to input domain
 * and output domain. */
    class fourway_processor_t {
    public:
        /** Pure waveform processing
         * @param s_in input waveform signal
         * @param s_out output waveform signal */
        virtual void process(mha_wave_t * s_in, mha_wave_t ** s_out) = 0;
        /** Pure spectrum processing
         * @param s_in input spectrum signal
         * @param s_out output spectrum signal */
        virtual void process(mha_spec_t * s_in, mha_spec_t ** s_out) = 0;
        /** Signal processing with domain transformation from waveform to spectrum.
         * @param s_in input waveform signal
         * @param s_out output spectrum signal */
        virtual void process(mha_wave_t * s_in, mha_spec_t ** s_out) = 0;
        /** Signal processing with domain transformation from spectrum to waveform.
         * @param s_in input spectrum signal
         * @param s_out output waveform signal */
        virtual void process(mha_spec_t * s_in, mha_wave_t ** s_out) = 0;
        /** 
         * Prepares the processor for signal processing.
         * @param settings domain and dimensions of the signal.  The
         * contents of settings may be modified by the prepare
         * implementation.  Upon calling fourway_processor_t#prepare,
         * settings reflects domain and dimensions of the input signal.
         * When fourway_processor_t#prepare returns, settings reflects
         * domain and dimensions of the output signal. */
        virtual void prepare(mhaconfig_t & settings) = 0;
        /** 
         * Resources allocated for signal processing in 
         * fourway_processor_t#prepare are released here in
         * fourway_processor_t#release. */
        virtual void release() = 0;
        /** Parser interface */
        virtual std::string parse(const std::string & query) = 0;
        /** Classes with virtual methods need virtual destructor.
         * This destructor is empty. */
        virtual ~fourway_processor_t() {}
    };

    class mhapluginloader_t
        : public config_file_splitter_t,
          public MHAParser::c_ifc_parser_t,
          public fourway_processor_t
    {
    public:
        std::string parse(const std::string & str) override;
        /** Loads and initializes mha plugin and establishes interface.
         * @param iac
         *   AC space (algorithm communication variables)
         *
         * @param libname
         *   Either file name of MHA plugin without platform-specific
         *   extension (i.e. "identity" for "identity.so" or
         *   "identity.dll") to be found on the MHA_LIBRARY_PATH
         *   (which is an environment variable).  Or the same file
         *   name without extension followed by a colon ":" followed
         *   by the "configuration name" of the MHA plugin, which may
         *   be used to differentiate between multiple identical MHA
         *   plugins or to give the plugin a self-documenting name
         *   that fits its purpose. The library name - configuration
         *   name expression can be followed by a "<" followed by a
         *   configuration file name, which will be read after
         *   initialization of the plugin.
         *
         *   Example: "overlapadd:agc<compression.cfg" will load the
         *   plugin "overlapadd.so" or "overlapadd.dll", insert it as
         *   the configuration node "agc", and reads the configuration
         *   file "compression.cfg" into that node.
         *
         * @param check_version
         *   Pluginloader will not check that the plugin was built using
         *   a known compatible MHA version if this flag is set to false.
         *   Disabling version check is discouraged.
         */
        mhapluginloader_t(algo_comm_t iac, const std::string& libname, bool check_version=true);
        ~mhapluginloader_t() throw();
        bool has_process(mha_domain_t in,mha_domain_t out) const;
        bool has_parser() const;
        mha_domain_t input_domain() const;
        mha_domain_t output_domain() const;
        void prepare(mhaconfig_t&) override;
        void release() override;
        void process(mha_wave_t*,mha_wave_t**) override;
        void process(mha_spec_t*,mha_spec_t**) override;
        void process(mha_wave_t*,mha_spec_t**) override;
        void process(mha_spec_t*,mha_wave_t**) override;
        std::string getfullname() const {return lib_handle.getname();};
        std::string get_documentation() const {return plugin_documentation;};
        std::vector<std::string> get_categories() const {return plugin_categories;};
        bool is_prepared() const {return b_is_prepared;};
    protected:
        void test_error();
        void test_version();
        void mha_test_struct_size(unsigned int s);
        void resolve_and_init();
        int lib_err;
        algo_comm_t ac;
        dynamiclib_t lib_handle;
        void* lib_data;
        // callback handles:
        MHAGetVersion_t MHAGetVersion_cb;
        MHAInit_t MHAInit_cb;
        MHADestroy_t MHADestroy_cb;
        MHAPrepare_t MHAPrepare_cb;
        MHARelease_t MHARelease_cb;
        MHAProc_wave2wave_t MHAProc_wave2wave_cb;
        MHAProc_spec2spec_t MHAProc_spec2spec_cb;
        MHAProc_wave2spec_t MHAProc_wave2spec_cb;
        MHAProc_spec2wave_t MHAProc_spec2wave_cb;
        MHASet_t MHASet_cb;
        MHASetcpp_t MHASetcpp_cb;
        MHAStrError_t MHAStrError_cb;
        mhaconfig_t cf_input;
        mhaconfig_t cf_output;
        std::string plugin_documentation;
        std::vector<std::string> plugin_categories;
        bool b_check_version;
        bool b_is_prepared;
    };
}

namespace MHAParser {

    /**
       \brief Class to create a plugin loader in a parser, including the load logic.
     */
    class mhapluginloader_t {
    public:
        mhapluginloader_t(MHAParser::parser_t& parent,const algo_comm_t& ac,
                          const std::string& plugname_name = "plugin_name", const std::string& prefix = "");
        ~mhapluginloader_t();
        void prepare(mhaconfig_t& cf);
        void release();
        void process(mha_wave_t* sIn,mha_wave_t** sOut){ plug->process(sIn,sOut);};
        void process(mha_spec_t* sIn,mha_spec_t** sOut){ plug->process(sIn,sOut);};
        void process(mha_wave_t* sIn,mha_spec_t** sOut){ plug->process(sIn,sOut);};
        void process(mha_spec_t* sIn,mha_wave_t** sOut){ plug->process(sIn,sOut);};
        mhaconfig_t get_cfin() const {return cf_in_;};
        mhaconfig_t get_cfout() const {return cf_out_;};
        const std::string & get_last_name() const {return last_name;}
    protected:
        PluginLoader::mhapluginloader_t* plug;
    private:
        void load_plug();
        MHAParser::parser_t& parent_;
        MHAParser::string_t plugname;
        std::string prefix_;
        MHAEvents::connector_t<mhapluginloader_t> connector;
        algo_comm_t ac_;
        std::string last_name;
        std::string plugname_name_;
        mhaconfig_t cf_in_;
        mhaconfig_t cf_out_;
        static double bookkeeping; // last time the dongle was queried
    };

}

#endif

/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * compile-command: "make -C .."
 * coding: utf-8-unix
 * End:
 */
