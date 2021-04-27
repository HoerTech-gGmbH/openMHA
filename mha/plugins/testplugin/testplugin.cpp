// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2014 2015 2018 2019 2021 HörTech gGmbH
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

#include <algorithm>
#include "mha_plugin.hh"
#include "mha_algo_comm.hh"
#include "mha_multisrc.h"
#include "mhapluginloader.h"

namespace testplugin {

  class config_parser_t : public MHAParser::parser_t {
  public:
    MHAParser::int_t channels;
    MHAParser::kw_t domain;
    MHAParser::int_t fragsize;
    MHAParser::int_t wndlen;
    MHAParser::int_t fftlen;
    MHAParser::float_t srate;

    void setlock(const bool & b) {
      channels.setlock(b); domain.setlock(b); fragsize.setlock(b);
      wndlen.setlock(b);   fftlen.setlock(b); srate.setlock(b);
    }
    
    config_parser_t() :
      MHAParser::parser_t("signal domain and dimensions"),
      channels("Number of audio channels", "2", "[0,]"),
      domain("Signal domain", "MHA_WAVEFORM", "[MHA_WAVEFORM MHA_SPECTRUM]"),
      fragsize("Fragment size of waveform data","200","[0,]"),
      wndlen("Window length of spectral data","400","[0,]"),
      fftlen("FFT length of spectral data","512","[0,]"),
      srate("Sampling rate in Hz","44100","]0,]")
    {
      insert_member(channels); insert_member(domain); insert_member(fragsize);
      insert_member(wndlen);   insert_member(fftlen); insert_member(srate);
    }

    mhaconfig_t get() const {
      mhaconfig_t c;
      c.channels = channels.data;
      c.domain = (domain.data.get_index() == 0) ? MHA_WAVEFORM : MHA_SPECTRUM;
      c.fragsize = fragsize.data; c.wndlen = wndlen.data;
      c.fftlen = fftlen.data;     c.srate = srate.data;
      return c;
    }
    void set(mhaconfig_t c) {
      channels.data = c.channels;
      domain.data.set_value((c.domain == MHA_WAVEFORM)
                            ? "MHA_WAVEFORM" : "MHA_SPECTRUM");
      fragsize.data = c.fragsize; wndlen.data = c.wndlen;
      fftlen.data = c.fftlen;     srate.data = c.srate;
    }
  };

  class ac_parser_t : public MHAParser::parser_t,
                      public MHAKernel::algo_comm_class_t {
  public:
    MHAParser::string_t insert_var;
    MHAParser::string_t get_var;
    MHAParser::kw_t data_type;
    MHAParser::int_t num_entries;
    MHAParser::int_t stride;
    MHAParser::string_t char_data;
    MHAParser::vint_t int_data;
    MHAParser::vfloat_t float_data;
    MHAParser::vcomplex_t complex_data;
    
    MHAEvents::patchbay_t<ac_parser_t> patchbay;
    enum data_type_t {_MHA_AC_CHAR,_MHA_AC_INT,_MHA_AC_MHAREAL,
                      _MHA_AC_FLOAT,_MHA_AC_DOUBLE,_MHA_AC_MHACOMPLEX,_unknown};
    ac_parser_t() :
      MHAParser::parser_t("Insert and retrieve AC variables"),
      insert_var("Setting this inserts an AC variable into the AC space",""),
      get_var("Setting this retrieves an AC variable from the AC space",""),
      data_type("Type of data."
                " No support for MHA_AC_USER and MHA_AC_DOUBLE data access",
                "unknown", 
                "[MHA_AC_CHAR MHA_AC_INT MHA_AC_MHAREAL"
                " MHA_AC_FLOAT MHA_AC_DOUBLE MHA_AC_MHACOMPLEX unknown]"),
      num_entries("Number of entries", "1", "[0,]"),
      stride("length of one row (C interpretation)"
             " or of one column (Fortran interpretation)",
             "1","[0,]"),
      char_data("data of ac variable if data_type is MHA_AC_CHAR",""),
      int_data("data of ac variable if data_type is MHA_AC_INT","[]","[,]"),
      float_data("data of ac variable if data_type is MHA_AC_FLOAT or MHA_AC_MHAREAL","[]","[,]"),
      complex_data("data of ac variable if data_type is MHA_AC_MHACOMPLEX","[]","[,]")
    {
      insert_member(insert_var); insert_member(get_var);
      insert_member(data_type);  insert_member(num_entries);
      insert_member(stride);     insert_member(char_data);
      insert_member(int_data);   insert_member(float_data);
      insert_member(complex_data);

      patchbay.connect(&insert_var.writeaccess, this,
                       &ac_parser_t::do_insert_var);
      patchbay.connect(&get_var.writeaccess, this,
                       &ac_parser_t::do_get_var);
    }

    /** Insert variable into AC space. This leaks memory by design, as the 
        plugin is for testing only */
    void do_insert_var() {
      comm_var_t c;
      c.num_entries = num_entries.data; c.stride = stride.data;
      c.data_type = MHA_AC_FLOAT;
      switch (data_type.data.get_index()) {
      case _MHA_AC_CHAR: {
        char * cdata = new char[char_data.data.length()+1];
        std::copy(char_data.data.begin(), char_data.data.end(), cdata);
        cdata[char_data.data.length()] = '\0';
        c.data = cdata;
        c.data_type = MHA_AC_CHAR;
      } break;
      case _MHA_AC_INT: {
        int * idata = new int[int_data.data.size()];
        std::copy(int_data.data.begin(), int_data.data.end(), idata);
        c.data = idata;
        c.data_type = MHA_AC_INT;
      } break;
      case _MHA_AC_MHAREAL: 
        c.data_type = MHA_AC_MHAREAL;
        // fall through
      case _MHA_AC_FLOAT: {
        float * fdata = new float[float_data.data.size()];
        std::copy(float_data.data.begin(), float_data.data.end(), fdata);
        c.data = fdata;
      } break;
      case _MHA_AC_MHACOMPLEX: {
        mha_complex_t * zdata = new mha_complex_t[complex_data.data.size()];
        std::copy(complex_data.data.begin(), complex_data.data.end(), zdata);
        c.data = zdata;
        c.data_type = MHA_AC_MHACOMPLEX;
      } break;
      default:
        throw MHA_Error(__FILE__,__LINE__,
                        "insertion of ac variables of type %s is not supported"
                        " by this plugin", data_type.data.get_value().c_str());
      }
      local_insert_var(insert_var.data.c_str(), c);
    }

    void do_get_var() {
      comm_var_t c;
      local_get_var(get_var.data.c_str(), &c);

      char_data.data.clear();  int_data.data.clear();
      float_data.data.clear(); complex_data.data.clear();

      num_entries.data = c.num_entries;
      stride.data = c.stride;

      switch (c.data_type) {
      case MHA_AC_CHAR:
        data_type.data.set_value("MHA_AC_CHAR");
        char_data.data.assign(static_cast<const char *>(c.data), c.num_entries);
        break;
      case MHA_AC_INT: {
        data_type.data.set_value("MHA_AC_INT");
        const int * ibegin = static_cast<const int *>(c.data);
        const int * iend = ibegin + c.num_entries;
        int_data.data.assign(ibegin, iend);
      } break;
      case MHA_AC_MHAREAL: {
        data_type.data.set_value("MHA_AC_MHAREAL");
        const mha_real_t * rbegin = static_cast<const mha_real_t *>(c.data);
        const mha_real_t * rend = rbegin + c.num_entries;
        float_data.data.assign(rbegin, rend);
      } break;
      case MHA_AC_FLOAT: {
        data_type.data.set_value("MHA_AC_FLOAT");
        const float * fbegin = static_cast<const float *>(c.data);
        const float * fend = fbegin + c.num_entries;
        float_data.data.assign(fbegin, fend);
      } break;
      case MHA_AC_DOUBLE: 
        data_type.data.set_value("MHA_AC_DOUBLE");
        break;
      case MHA_AC_MHACOMPLEX: {
        data_type.data.set_value("MHA_AC_MHACOMPLEX");
        const mha_complex_t * zbegin =
          static_cast<const mha_complex_t *>(c.data);
        const mha_complex_t * zend = zbegin + c.num_entries;
        complex_data.data.assign(zbegin, zend);
      } break;
      default:
        data_type.data.set_value("unknown");
        break;
      }
    }
  };

  class signal_parser_t : public MHAParser::parser_t {
  public:
    MHAParser::mfloat_t input_wave;
    MHAParser::mcomplex_t input_spec;
    MHAParser::mfloat_mon_t output_wave;
    MHAParser::mcomplex_mon_t output_spec;
    signal_parser_t() :
      MHAParser::parser_t("signal input and output"),
      input_wave("waveform input signal. Writing data will cause processing",
                 "[]","[,]"),
      input_spec("spectrum input signal. Writing data will cause processing",
                 "[]","[,]"),
      output_wave("waveform output signal from last processing"),
      output_spec("spectrum output signal from last processing")
    {
      insert_member(input_wave);  insert_member(input_spec);
      insert_member(output_wave); insert_member(output_spec);
    }
  };

  class if_t : public MHAPlugin::plugin_t<int> {
  public:
    if_t(algo_comm_t iac, const std::string & configured_name);
    mha_spec_t* process(mha_spec_t * s_in) {return s_in;}
    mha_wave_t* process(mha_wave_t * s_in) {return s_in;}
    void prepare(mhaconfig_t&) {}
  private:
    config_parser_t config_in, config_out;
    ac_parser_t ac;
    signal_parser_t signal;
    MHAParser::bool_t _prepare;
    MHAEvents::patchbay_t<if_t> patchbay;
    MHAParser::mhapluginloader_t plug;

    void test_prepare();
    void test_process();
  };

  if_t::if_t(algo_comm_t iac, const std::string &)
    : MHAPlugin::plugin_t<int>("loads a plugin for testing",iac),
      _prepare("for preparing/releasing the loaded plugin","no"),
      plug(*this, ac.get_c_handle())
  {
    insert_member(config_in); insert_member(config_out);
    insert_member(ac);        insert_member(signal);
    insert_item("prepare", &_prepare);
    patchbay.connect(&signal.writeaccess, this, &if_t::test_process);
    patchbay.connect(&_prepare.valuechanged, this, &if_t::test_prepare);
    config_out.setlock(true);
  }

  void if_t::test_prepare() {
    if (_prepare.data) {
      mhaconfig_t signal_params = config_in.get();
      plug.prepare(signal_params);
      config_out.set(signal_params);
      config_in.setlock(true);
    } else {
      plug.release();
      config_in.setlock(false);
    }
  }
  
  void if_t::test_process() {
    if (_prepare.data == false)
      throw MHA_Error(__FILE__,__LINE__,
                      "test plugin cannot process in unprepared state");
    signal.output_wave.data.clear();
    signal.output_spec.data.clear();
    if (config_in.domain.data.get_value() == "MHA_WAVEFORM") {
      unsigned num_frames_in = signal.input_wave.data.size();
      unsigned num_channels_in = num_frames_in ?
        signal.input_wave.data[0].size() : config_in.fragsize.data;
      MHASignal::waveform_t s_in(num_frames_in, num_channels_in);
      for (unsigned k = 0; k < num_frames_in; ++k) {
        if (signal.input_wave.data[k].size() != num_channels_in)
          throw MHA_Error(__FILE__,__LINE__,
                          "inconsistent number of channels in input_wave");
        for (unsigned ch = 0; ch < num_channels_in; ++ch)
          s_in(k,ch) = signal.input_wave.data[k][ch];
      }
      if (config_out.domain.data.get_value() == "MHA_WAVEFORM") {
        mha_wave_t * s_out = 0;
        plug.process(&s_in, &s_out);
        signal.output_wave.data.resize(s_out->num_frames);
        for (unsigned k = 0; k < s_out->num_frames; ++k) {
          signal.output_wave.data[k].resize(s_out->num_channels);
          for (unsigned ch = 0; ch < s_out->num_channels; ++ch)
            signal.output_wave.data[k][ch] = value(s_out,k,ch);
        }
      } else { // MHA_SPECTRUM output
        mha_spec_t * s_out = 0;
        plug.process(&s_in, &s_out);
        signal.output_spec.data.resize(s_out->num_frames);
        for (unsigned k = 0; k < s_out->num_frames; ++k) {
          signal.output_spec.data[k].resize(s_out->num_channels);
          for (unsigned ch = 0; ch < s_out->num_channels; ++ch)
            signal.output_spec.data[k][ch] = value(s_out,k,ch);
        }
      }
    } else { // MHA_SPECTRUM input
      unsigned num_frames_in = signal.input_spec.data.size();
      unsigned num_channels_in = num_frames_in ?
        signal.input_spec.data[0].size() : (config_in.fftlen.data / 2 + 1);
      MHASignal::spectrum_t s_in(num_frames_in, num_channels_in);
      for (unsigned k = 0; k < num_frames_in; ++k) {
        if (signal.input_spec.data[k].size() != num_channels_in)
          throw MHA_Error(__FILE__,__LINE__,
                          "inconsistent number of channels in input_wave");
        for (unsigned ch = 0; ch < num_channels_in; ++ch)
          s_in(k,ch) = signal.input_spec.data[k][ch];
      }
      if (config_out.domain.data.get_value() == "MHA_WAVEFORM") {
        mha_wave_t * s_out = 0;
        plug.process(&s_in, &s_out);
        signal.output_wave.data.resize(s_out->num_frames);
        for (unsigned k = 0; k < s_out->num_frames; ++k) {
          signal.output_wave.data[k].resize(s_out->num_channels);
          for (unsigned ch = 0; ch < s_out->num_channels; ++ch)
            signal.output_wave.data[k][ch] = value(s_out,k,ch);
        }
      } else { // MHA_SPECTRUM output
        mha_spec_t * s_out = 0;
        plug.process(&s_in, &s_out);
        signal.output_spec.data.resize(s_out->num_frames);
        for (unsigned k = 0; k < s_out->num_frames; ++k) {
          signal.output_spec.data[k].resize(s_out->num_channels);
          for (unsigned ch = 0; ch < s_out->num_channels; ++ch)
            signal.output_spec.data[k][ch] = value(s_out,k,ch);
        }
      }
    }
  }
}

MHAPLUGIN_CALLBACKS(testplugin,testplugin::if_t,spec,spec)
MHAPLUGIN_PROC_CALLBACK(testplugin,testplugin::if_t,wave,wave)
MHAPLUGIN_DOCUMENTATION\
(testplugin,
 "test-tool feature-extraction",
 "")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 2
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
