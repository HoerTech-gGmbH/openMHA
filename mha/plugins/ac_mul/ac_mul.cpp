// This file is part of the open HörTech Master Hearing Aid (openMHA)
// Copyright © 2006 2009 2010 2013 2014 2015 2018 2020 2021 HörTech gGmbH
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

#include "ac_mul.hh"

ac_mul_t::ac_mul_t(MHA_AC::algo_comm_t & iac,
                   const std::string & configured_name)
    : MHAParser::string_t("Element-wise multiplication expression in style"
                          " \"a * b\",\n"
                          "where \"a\" and \"b\" are AC variables.","a * b"),
      ac(iac),
      algo(configured_name),
      argt(ARG_RR),
      str_a(""),
      str_b(""),
      res_r(NULL),
      res_c(NULL),
      num_frames(0),
      num_channels(0)
{}

void ac_mul_t::scan_syntax()
{
    auto m1 = std::make_unique<char[]>(data.size()+1);
    auto m2 = std::make_unique<char[]>(data.size()+1);
    int cnt = 0;
    cnt = sscanf( data.c_str(), "%[^+-*/ ] * %[^+-*/ ]", m1.get(), m2.get() );
    if( cnt != 2 )
        throw MHA_Error(__FILE__,__LINE__,
                        "Invalid syntax: \"%s\"", data.c_str());
    str_a = static_cast<const char *>(m1.get());
    str_b = static_cast<const char *>(m2.get());
}

void ac_mul_t::prepare_(mhaconfig_t&)
{
    scan_syntax();
    get_arg_type_and_dimension();
    switch( argt ){
    case ARG_RR :
        res_r = new MHA_AC::waveform_t(ac,algo,num_frames,num_channels,true);
        break;
    case ARG_RC :
    case ARG_CR :
    case ARG_CC :
        res_c = new MHA_AC::spectrum_t(ac,algo,num_frames,num_channels,true);
        break;
    }
}

void ac_mul_t::release_()
{
    if( res_r ){
        delete res_r;
        res_r = NULL;
    }
    if( res_c ){
        delete res_c;
        res_c = NULL;
    }
}

mha_wave_t* ac_mul_t::process(mha_wave_t* s)
{
    process();
    return s;
}

mha_spec_t* ac_mul_t::process(mha_spec_t* s)
{
    process();
    return s;
}

void ac_mul_t::process()
{
    switch( argt ){
    case ARG_RR :
        process_rr();
        break;
    case ARG_RC :
        process_rc();
        break;
    case ARG_CR :
        process_cr();
        break;
    case ARG_CC :
        process_cc();
        break;
    }
}

void ac_mul_t::process_rr()
{
    mha_wave_t a = MHA_AC::get_var_waveform(ac,str_a);
    mha_wave_t b = MHA_AC::get_var_waveform(ac,str_b);
    res_r->copy(a);
    *res_r *= b;
    res_r->insert();
}

void ac_mul_t::process_rc()
{
    mha_wave_t a = MHA_AC::get_var_waveform(ac,str_a);
    mha_spec_t b = MHA_AC::get_var_spectrum(ac,str_b);
    res_c->copy(b);
    *res_c *= a;
    res_c->insert();
}

void ac_mul_t::process_cr()
{
    mha_spec_t a = MHA_AC::get_var_spectrum(ac,str_a);
    mha_wave_t b = MHA_AC::get_var_waveform(ac,str_b);
    res_c->copy(a);
    *res_c *= b;
    res_c->insert();
}

void ac_mul_t::process_cc()
{
    mha_spec_t a = MHA_AC::get_var_spectrum(ac,str_a);
    mha_spec_t b = MHA_AC::get_var_spectrum(ac,str_b);
    res_c->copy(a);
    *res_c *= b;
    res_c->insert();
}

void ac_mul_t::get_arg_type_and_dimension()
{
    val_type_t vt_a;
    val_type_t vt_b;
    unsigned int num_frames_a;
    unsigned int num_frames_b;
    unsigned int num_channels_a;
    unsigned int num_channels_b;
    get_arg_type_and_dimension(str_a,vt_a,num_frames_a,num_channels_a);
    get_arg_type_and_dimension(str_b,vt_b,num_frames_b,num_channels_b);
    if( (vt_a == VAL_REAL) && (vt_b == VAL_REAL) )
        argt = ARG_RR;
    else if( (vt_a == VAL_REAL) && (vt_b == VAL_COMPLEX) )
        argt = ARG_RC;
    else if( (vt_a == VAL_COMPLEX) && (vt_b == VAL_REAL) )
        argt = ARG_CR;
    else if( (vt_a == VAL_COMPLEX) && (vt_b == VAL_COMPLEX) )
        argt = ARG_CC;
    else
        throw MHA_ErrorMsg("This can't happen.");
    if( (num_frames_a != num_frames_b) || (num_channels_a != num_channels_b) )
        throw MHA_Error(__FILE__,__LINE__,
                        "The signal dimension mismatched: %s: %ux%u %s: %ux%u",
                        str_a.c_str(),num_frames_a,num_channels_a,
                        str_b.c_str(),num_frames_b,num_channels_b);
    num_frames = num_frames_a;
    num_channels = num_channels_a;
}

void ac_mul_t::get_arg_type_and_dimension(
    const std::string& name,
    val_type_t& vt,
    unsigned int& num_frames,
    unsigned int& num_channels
    )
{
    MHA_AC::comm_var_t v = ac.get_var(name);
    switch( v.data_type ){
    case MHA_AC_MHAREAL :
    case MHA_AC_FLOAT :
        vt = VAL_REAL;
        mha_wave_t datar;
        datar = MHA_AC::get_var_waveform(ac,name);
        num_frames = datar.num_frames;
        num_channels = datar.num_channels;
        break;
    case MHA_AC_MHACOMPLEX :
        vt = VAL_COMPLEX;
        mha_spec_t datac;
        datac = MHA_AC::get_var_spectrum(ac,name);
        num_frames = datac.num_frames;
        num_channels = datac.num_channels;
        break;
    default:
        throw MHA_Error(__FILE__,__LINE__,"Invalid data type for %s: %u",
                        name.c_str(),v.data_type);
    }
}

MHAPLUGIN_CALLBACKS(ac_mul,ac_mul_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(ac_mul,ac_mul_t,spec,spec)
MHAPLUGIN_DOCUMENTATION
(ac_mul, "data-flow math algorithm-communication",
 "The \\texttt{ac\\_mul} plugin can be used to multiply two AC variables"
 " element-wise and store the result in another AC variable.  The result AC"
 " variable is named after the configured name of this plugin (see example"
 " below).  The matrix dimensions of both input AC variables must be"
 " identical, and must not change during signal processing.  Multiplying two"
 " real-valued input matrices results in a real-valued output matrix.  If at"
 " least one of the input matrices is complex-valued, then the result is also"
 " complex-valued."
 "\n\n"

 "The names of the AC variables to multiply are extracted from the expression"
 " string assigned to this plugin."
 "\n\n"

 "Example: in order to compute the squared samples of a time-domain input"
 " signal with this plugin, one could include the following excerpt in an"
 " \\mha{} configuration:"
 "\n\n"

 "\\texttt{mhachain.algos = [save\\_wave:signal ac\\_mul:squared\\_signal] \\\\"
 "         mhachain.squared\\_signal = signal * signal}"

 "\n\n"
 "which will store the squared samples of the signal in AC variable "
 "\\texttt{squared\\_signal}."
 "")

/*
 * Local Variables:
 * c-basic-offset: 4
 * compile-command: "make"
 * indent-tabs-mode: nil
 * End:
 */
