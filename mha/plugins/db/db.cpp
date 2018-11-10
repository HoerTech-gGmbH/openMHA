// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2008 2009 2010 2014 2015 2017 HörTech gGmbH
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

#include "mha_signal.hh"
#include "mha_plugin.hh"
#include "mha_toolbox.h"
#include "mha_events.h"
#include "mha_os.h"
#include "mhapluginloader.h"

class db_t : public MHASignal::doublebuffer_t {
public:
    db_t(unsigned int outer_fragsize,
         unsigned int inner_fragsize,
         unsigned int nch_in,
         unsigned int nch_out,
         MHAParser::mhapluginloader_t& plug);
    mha_wave_t* inner_process(mha_wave_t*);
private:
    MHAParser::mhapluginloader_t& plugloader;
};

db_t::db_t(unsigned int outer_fragsize,
           unsigned int inner_fragsize,
           unsigned int nch_in,
           unsigned int nch_out,
           MHAParser::mhapluginloader_t& plug_)
    : MHASignal::doublebuffer_t(nch_in,nch_out,outer_fragsize,inner_fragsize),
      plugloader(plug_)
{
}

mha_wave_t* db_t::inner_process(mha_wave_t* s)
{
    plugloader.process(s,&s);
    return s;
}

class db_if_t : public MHAPlugin::plugin_t< db_t > {
public:
    db_if_t(algo_comm_t,std::string,std::string);
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release();
    ~db_if_t();
private:
    MHAEvents::patchbay_t< db_if_t > patchbay;
    MHAParser::int_t fragsize;
    MHAParser::mhapluginloader_t plugloader;
    std::string chain;
    std::string algo;
    bool bypass;
};

db_if_t::db_if_t(algo_comm_t iac,std::string th,std::string al)
    : MHAPlugin::plugin_t< db_t >("Synchronous double buffer plugin.",iac),
      fragsize("fragment size of client plugin","200","[0,]"), 
      plugloader(*this,iac),
      chain(th),
      algo(al),
      bypass(false)
{
    set_node_id("db");
    insert_item("fragsize",&fragsize);
}

db_if_t::~db_if_t()
{
}

void db_if_t::prepare(mhaconfig_t& conf)
{
    if( conf.domain != MHA_WAVEFORM )
        throw MHA_ErrorMsg("Doublebuffer: Only waveform data can be processed.");
    // remember the outer fragsize:
    unsigned int outer_fragsize = conf.fragsize;
    unsigned int inner_fragsize = fragsize.data;
    unsigned int input_channels = conf.channels;
    conf.fragsize = inner_fragsize;
    // sugest configuration to inner plugin, query requirements:
    plugloader.prepare(conf);
    if( conf.domain != MHA_WAVEFORM )
        throw MHA_ErrorMsg("Doublebuffer: Only waveform data can be processed.");
    // only fixed input/output fragsizes are allowed:
    if( inner_fragsize != conf.fragsize )
        throw MHA_ErrorMsg("Doublebuffer: Plugin modified the fragment size.");
    // no double buffering required:
    bypass = (inner_fragsize == 0 );
    // update the configuration, create an instance of the double buffer:
    push_config(new db_t(outer_fragsize,
                         inner_fragsize,
                         input_channels,
                         conf.channels,
                         plugloader));
    conf.fragsize = outer_fragsize;
}

void db_if_t::release()
{
    plugloader.release();
}

mha_wave_t* db_if_t::process(mha_wave_t* s)
{
    if( bypass )
        return s;
    poll_config();
    return cfg->outer_process(s);
}

MHAPLUGIN_CALLBACKS(db,db_if_t,wave,wave)
    MHAPLUGIN_DOCUMENTATION(db,"signalflow",
"The double buffer plugin allows changes of fragment size. It has an\n"
"outer layer (e.g.\\ framework) and an inner layer (e.g.\\ MHA kernel,\n"
"plugin). A configurable fragment size is used on the inner side, which\n"
"is independent from the outer fragment size. The input data is\n"
"buffered, and the data is processed when enough samples are available.\n"
"The configuration of the inner plugin is available via the {\\tt plug} prefix.\n"
"\n"
"Please note that double buffering adds an extra delay of the audio\n"
"stream. If both fragment sizes are identical, the double buffering is\n"
"bypassed.\n"
"\n"
"\\MHAfigure{Concept of the double buffer plugin. The outer fragments,\n"
"provided by the framework, are split up into smaller fragments for\n"
"processing in the kernel. For a continuous output stream, an extra\n"
"delay is needed, i.e.\\ the first fragment is filled with zeros at the\n"
"beginning.}{doublebuffer}\n"
"\n"
"\\paragraph{Warning:}\n"
"\n"
"If the inner fragment size is larger than the outer fragment size, the\n"
"maximal processing time is limited by the shorter fragment size. This\n"
"results in a maximal processor usage determined by the ratio of outer\n"
"to inner fragment size. This problem holds not for offline processing.\n"
"As an alternative, the asynchronous double-buffer plugin {\\tt\n"
"dbasync} (section \\ref{plug:dbasync}) can be used, which processes\n"
"the double-buffered signal in a separate thread. That plugin should be\n"
"preferred for real-time processing. If the inner thread should only be\n"
"used for signal analysis, please refer to the plugin {\\tt\n"
"analysispath} (section \\ref{plug:analysispath}).\n"

        )
// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
