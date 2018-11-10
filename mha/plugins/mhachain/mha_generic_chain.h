// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2010 2014 2016 HörTech gGmbH
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

#define MHAPLUGIN_OVERLOAD_OUTDOMAIN
#include "mha_defs.h"
#include "mha_plugin.hh"
#include "mha_events.h"
#include "mha_profiling.h"
#include "mhapluginloader.h"

namespace mhachain {

    class plugs_t {
    public:
        plugs_t( std::vector<std::string> algos,
                 mhaconfig_t cfin,
                 mhaconfig_t cfout,
                 bool do_prepare,
                 MHAParser::parser_t& p,
                 algo_comm_t iac,
                 std::string ichain,
                 bool use_profiling);
        ~plugs_t();
        void prepare(mhaconfig_t&);
        void release();
        void process(mha_wave_t*,mha_spec_t*,mha_wave_t**,mha_spec_t**);
        bool prepared() const {return b_prepared;};
    private:
        void alloc_plugs(std::vector<std::string> algos);
        void cleanup_plugs();
        void update_proc_load();
        bool b_prepared;
        std::vector< PluginLoader::mhapluginloader_t* > algos;
        MHAParser::parser_t& parser;
        algo_comm_t ac;
        std::string chain;
        MHAParser::parser_t profiling;
        MHAParser::vstring_mon_t prof_algos;
        MHAParser::vfloat_mon_t prof_init;
        MHAParser::vfloat_mon_t prof_prepare;
        MHAParser::vfloat_mon_t prof_release;
        MHAParser::vfloat_mon_t prof_process;
        MHAParser::float_mon_t prof_process_tt;
        MHAParser::vfloat_mon_t prof_process_load;
        unsigned int proc_cnt;
        mhaconfig_t prof_cfg;
        MHAEvents::connector_t<mhachain::plugs_t> prof_load_con;
        MHAEvents::connector_t<mhachain::plugs_t> prof_tt_con;
        bool b_use_profiling;
        mha_platform_tictoc_t tictoc;
    };

    class chain_base_t : public MHAPlugin::plugin_t<mhachain::plugs_t> {
    public:
        chain_base_t(algo_comm_t,const std::string &,const std::string &);
        void process(mha_wave_t*,mha_wave_t**);
        void process(mha_spec_t*,mha_wave_t**);
        void process(mha_wave_t*,mha_spec_t**);
        void process(mha_spec_t*,mha_spec_t**);
        void prepare(mhaconfig_t &);
        void release();
    private:
        void update();
    protected:
        MHAParser::bool_t bprofiling;
        MHAParser::vstring_t algos;
    private:
        std::vector<std::string> old_algos;
        MHAEvents::patchbay_t < mhachain::chain_base_t > patchbay;
        mhaconfig_t cfin, cfout;
        bool b_prepared;
        std::string chain;
    };

}

/*
 * Local Variables:
 * compile-command: "make"
 * mode: c++
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * coding: utf-8-unix
 * End:
 */
