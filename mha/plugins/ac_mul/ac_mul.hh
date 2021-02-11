// This file is part of the open HörTech Master Hearing Aid (openMHA)
// Copyright © 2006 2009 2010 2013 2014 2015 2018 2020 2021 HörTech gGmbH
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
#include <memory>

/// Indicates whether the factors of the product are real or complex valued.
typedef enum {
    ARG_RR, ///< Both factors are real.
    ARG_RC, ///< First factor is real, second is complex.
    ARG_CR, ///< First factor is complex, second is real.
    ARG_CC  ///< Both factors are complex.
} arg_type_t;

/// Indicates whether an AC variable contains real or complex values.
typedef enum {
    VAL_REAL,   ///< AC variable contains real values.
    VAL_COMPLEX ///< AC variable contains complex values.
} val_type_t;

/** The class which implements the ac_mul_t plugin.  Different from most 
 * other plugins, the ac_mul plugin's interface class does not inherit
 * from plugin_t<>, but from MHAParser::string_t.  This way, it does
 * not get inserted into the MHA configuration tree as a parser node wich
 * can have multiple variables, but as a string variable.
 * 
 * The ac_mul_t variable multiplies two AC variables element-wise. */
class ac_mul_t : public MHAParser::string_t {
public:
    /// Plugin constructor
    ac_mul_t(algo_comm_t,std::string,std::string);
    /// Prepare method, called prepare_() with trailing underscore because 
    /// ac_mul_t does not inherit from plugin_t<>.  Leaves signal dimensions
    /// unchanged.  The AC variables contained in the string expression must
    /// exist at this point.
    void prepare_(mhaconfig_t&);
    void release_();
    mha_wave_t* process(mha_wave_t*);
    mha_spec_t* process(mha_spec_t*);
private:
    void scan_syntax();
    void get_arg_type_and_dimension();
    void get_arg_type_and_dimension(const std::string&,val_type_t&,unsigned int&,unsigned int&);
    void process();
    void process_rr();
    void process_rc();
    void process_cr();
    void process_cc();
    //
    algo_comm_t ac;
    std::string algo;
    arg_type_t argt;
    std::string str_a;
    std::string str_b;
    MHA_AC::waveform_t* res_r;
    MHA_AC::spectrum_t* res_c;
    unsigned int num_frames;
    unsigned int num_channels;
};

/*
 * Local Variables:
 * c-basic-offset: 4
 * compile-command: "make"
 * indent-tabs-mode: nil
 * End:
 */
