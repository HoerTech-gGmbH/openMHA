// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2007 2009 2011 2013 2016 2017 2018 HörTech gGmbH
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

#include "mhafw_lib.h"
#include "mhapluginloader.h"
#include "mha_algo_comm.hh"
#include <exception>
#include <memory>

std::string strdom( mha_domain_t d )
{
    switch( d ){
    case MHA_WAVEFORM : return "waveform";
    case MHA_SPECTRUM : return "spectrum";
    default: return "unknwon";
    }
}

void print_ac(MHAKernel::algo_comm_class_t& ac,std::string txt)
{
    std::string stmp;
    std::vector<std::string> vstmp;
    stmp = ac.local_get_entries();
    if( stmp.size() ){
        std::cout << "AC variables " << txt << ":" << std::endl;
        MHAParser::StrCnv::str2val(std::string("[")+stmp+std::string("]"),vstmp);
        for( unsigned int k=0;k<vstmp.size();k++)
            std::cout << "  " << vstmp[k] << std::endl;
        std::cout << std::endl;
    }else{
        std::cout << "empty AC space " << txt << "." << std::endl;
    }
}

int document_plugin(MHAKernel::algo_comm_class_t& ac, PluginLoader::mhapluginloader_t &load, int argc,
    char **argv) {
  for (int karg = 2; karg < argc; karg++)
    std::cout << load.parse(argv[karg]) << std::endl;
  std::cout << "-- general information ---------------------------\n";
  std::cout << "Full name:\n" << load.getfullname() << std::endl << std::endl;
  std::cout << "Categories:\n"
            << MHAParser::StrCnv::val2str(load.get_categories()) << std::endl
            << std::endl;
  std::cout << "Has LaTeX documentation?\n";
  if (load.get_documentation().size())
    std::cout << "yes" << std::endl << std::endl;
  else
    std::cout << "no" << std::endl << std::endl;
  std::cout << "Supported domains:" << std::endl;
  mha_domain_t indom, outdom;
  for (indom = 0; indom < MHA_DOMAIN_MAX; indom++)
    for (outdom = 0; outdom < MHA_DOMAIN_MAX; outdom++)
      if (load.has_process(indom, outdom))
        std::cout << "  " << strdom(indom) << " to " << strdom(outdom)
                  << std::endl;
  std::cout << std::endl;
  mhaconfig_t sample_cf_wave;
  memset(&sample_cf_wave, 0, sizeof(mhaconfig_t));
  sample_cf_wave.domain = MHA_WAVEFORM;
  sample_cf_wave.channels = 2;
  sample_cf_wave.fragsize = 64;
  sample_cf_wave.srate = 44100;
  mhaconfig_t sample_cf_spec;
  sample_cf_spec = sample_cf_wave;
  sample_cf_spec.domain = MHA_SPECTRUM;
  sample_cf_spec.fftlen = 256;
  sample_cf_spec.wndlen = 128;
  std::cout << "-- waveform processing ---------------------------\n";
  try {
    print_ac(ac, "before prepare");
    load.prepare(sample_cf_wave);
    std::cout << "Successfully prepared for waveform processing." << std::endl;
    print_ac(ac, "after prepare");
    load.release();
    print_ac(ac, "after release");
  } catch (MHA_Error &e) {
    std::cout << "Prepare for waveform processing failed with message:"
              << std::endl;
    std::cout << e.what() << std::endl << std::endl;
  }
  std::cout << "-- spectrum processing ---------------------------\n";
  try {
    print_ac(ac, "before prepare");
    load.prepare(sample_cf_spec);
    std::cout << "Successfully prepared for spectrum processing." << std::endl
              << std::endl;
    print_ac(ac, "after prepare");
    load.release();
    print_ac(ac, "after release");
  } catch (MHA_Error &e) {
    std::cout << "Prepare for spectrum processing failed with message:"
              << std::endl;
    std::cout << e.what() << std::endl << std::endl;
  }
  std::cout << "-- configuration interface -----------------------\n";
  if (load.has_parser()) {
    std::cout << load.parse("?");
  } else {
    std::cout << "A parser interface is not available." << std::endl;
  }
  return 0;
}

void document_io_plugin(char *lib_name) {
    fw_t lib;
    lib.parse(std::string("iolib=") + std::string(lib_name));
    std::cout << lib.parse("io?");
}

int main(int argc,char** argv)
{
    try{
        if( argc < 2 )
            throw MHA_Error(__FILE__,__LINE__,"Usage: analysemhaplugin <plugin> [parser args]");
        try {
            MHAKernel::algo_comm_class_t ac;
            PluginLoader::mhapluginloader_t load(ac.get_c_handle(), argv[1]);
            return document_plugin(ac, load, argc, argv);
        } catch (MHA_Error &e) {
            // We maybe have an i/o plugin, try to document as io plugin. NOTE: This is a hacky way to
            // to detect if we have a 'normal' or i/o plugin, but there's no built-in way to distinguish
            // these.
            document_io_plugin(argv[1]);
        }
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}

// Local Variables:
// coding: utf-8-unix
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
