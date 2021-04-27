// This file is part of the HörTech open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2009 2010 2013 2014 2015 2018 2020 2021 HörTech gGmbH
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


// A plugin that makes a double precision number available to AC space.
// Behaves as identity in signal processing.

#include "mha_plugin.hh"

namespace double2acvar {

// For usage simplicity, we do not inherit from MHAPlugin::plugin_t.
// Advantage is that user can assign to the parser node of this
// plugin and does not have to remember a variable name.
/** Plugin interface class for double2acvar */
class double2acvar_t : public MHAParser::string_t, 
                       public MHAPlugin::config_t<double> {
public:
    /** Standard plugin constructor.
     * @param iac             Algorithm communication variable space.
     * @param configured_name Configured name of this plugin, also used as name
     *                        of the AC variable. */
    double2acvar_t(algo_comm_t iac, const std::string & configured_name);
    ~double2acvar_t() = default;
    /** process() does not alter the signal and has same implementation
     * regardless of signal domain.
     * @param s Pointer to input signal structure, mha_wave_t or mha_spec_t.
     * @return s, unaltered. */
    template <class T>  T* process(T* s);
    /** Called from process() and, when allowed, also from
     * on_configuration_update(). poll_latest_value_and_reinsert()
     * retrieves the latest configured value and reinserts the AC
     * variable into the AC space. */
    void poll_latest_value_and_reinsert();
    // Trailing underscores in names because we do not inherit from plugin_t:
    /** Prepare method as expected by the plugin interface macros.
     * Parameter is not used nor altered.  Sets is_prepared flag. */
    void prepare_(mhaconfig_t&);
    /** Release method as expected by the plugin interface macros.
     * Resets is_prepared flag. */
    void release_();
    /** Callback function on write access to the string configuration value. */
    void on_configuration_update();
private:
    /** AC variable inserted by this plugin. */
    MHA_AC::double_t ac_double;
    /** Callback router. */
    MHAEvents::patchbay_t<double2acvar_t> patchbay;
    /** Flag to keep track if we are currently prepared.  If we are, then
     * signal processing is active and AC variables may only be accessed when
     * MHA is currently executing out process() method. */
    bool is_prepared;
};

double2acvar_t::double2acvar_t(algo_comm_t iac, const std::string & configured_name) :
    MHAParser::string_t("Converts configuration variable of type string\n"
                        "containing a decimal floating point number literal\n"
                        "to algorithm communication variable of type double.\n"
                        "Name of the AC variable is the configured algorithm\n"
                        "name.", "0"),
    ac_double(iac, configured_name, 0.0),
    is_prepared(false)
{
    patchbay.connect(&writeaccess,this,
                     &double2acvar_t::on_configuration_update);
    // inserts AC variable immediately with default value.
    on_configuration_update();
}

void double2acvar_t::prepare_(mhaconfig_t&)
{
    // Calling on_configuration_update() here is unnecessary because all
    // it was called in the constructor and in response to any configuration
    // updates since then. Therefore not calling it here.

    // Prevent immediate updates of the AC variable on configuration changes
    // during signal processing. AC updates will now be triggered by process().
    is_prepared=true;
}

void double2acvar_t::release_()
{
    // Signal processing is finished, allow immediate updates of AC variable.
    is_prepared=false;

    // There may have been a configuration update between the last call to
    // process and the release. In that case, this update would not have been
    // propagated to the AC variable yet. Propagate now.
    poll_latest_value_and_reinsert();
}

template<class T> T* double2acvar_t::process(T* s)
{
    poll_latest_value_and_reinsert();
    return s;
}

void double2acvar_t::poll_latest_value_and_reinsert()
{
    ac_double.data = *poll_config();
    ac_double.insert();
}

void double2acvar_t::on_configuration_update()
{
    double new_double = atof(this->data.c_str());
    char converted_back_to_string[0x100];
    snprintf(converted_back_to_string,0xFF,"%.17g",new_double);
    converted_back_to_string[0xFF] = '\0';
    this->data = converted_back_to_string;
    
    push_config(new double(new_double));
    if (not is_prepared) {
        // before prepare/after release: safe to update ac variable immediately
        poll_latest_value_and_reinsert();
    }
}

}

MHAPLUGIN_CALLBACKS(double2acvar,double2acvar::double2acvar_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(double2acvar,double2acvar::double2acvar_t,spec,spec)
MHAPLUGIN_DOCUMENTATION
(double2acvar,"data-import",
 "Publishes an AC variable of type \\texttt{double}."
 " Because the \\mha{} configuration language does not have configuration"
 " variables of type \\texttt{double}, we are using a configuration variable"
 " of type string.  The string is converted to double by the C function"
 " \\texttt{atof()} which means:\n"
 " \\begin{enumerate}\n"
 " \\item The number format uses the standard C locale.\n"
 " \\item Leading white space is skipped.\n"
 " \\item The string is converted up to the first non-conversible character.\n"
 " \\item Empty strings and strings with only con-conversible characters"
 "        convert to value 0.0.\n"
 " \\end{enumerate}\n")

// Local Variables:
// c-basic-offset: 4
// indent-tabs-mode: nil
// compile-command: "make"
// End:
