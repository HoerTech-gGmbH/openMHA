// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2009 2010 2013 2014 2015 2018 2019 2020 HörTech gGmbH
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
#include "mha_plugin.hh"
#include "mhapluginloader.h"
#include "mha_defs.h"
#include <map>
#include <algorithm>

/** Single class implementing plugin altconfig.
 * altconfig loads another plugin and can send configuration commands to that plugin.
 * altconfig does not need a separate runtime configuration class,
 * template parameter char is used as a placeholder.
 * Uses parser variable names "algos" and "select" even though the alternatives that can be
 * selected in this plugin are not algorithms or plugins but configuration commands
 * in order to be interface-compatible with plugin altplugs. mhacontrol has a UI area that
 * automatically fills with the alternatives found in either altplugs or altconfig if the complete
 * MHA loads exactly one plugin with this interface.
 */
class altconfig_t : public MHAPlugin::plugin_t<char>, public MHAParser::mhapluginloader_t
{
public:
    /** Constructor initializes an instance of the altconfig plugin.
     * @param iac Algorithm communication variable space, not used by this plugin except to initialize base class.
     * @param algo Configured name of this plugin. */
    altconfig_t(algo_comm_t iac,const char* chain,const char* algo);

    /** Invoked by MHA when this plugin should prepare for signal processing.
     * altconfig delegates to the loaded plugin and does not need to do more work to prepare.
     * @param cf signal dimensions, forwarded to loaded plugin which may change the signal dimensions.
     */
    void prepare(mhaconfig_t& cf) { MHAParser::mhapluginloader_t::prepare(cf); };

    /** Invoked by MHA when this plugin should call its release function.
     * altconfig delegates to the loaded plugin.
     */
    void release() { MHAParser::mhapluginloader_t::release(); };
private:
    /** Callback executed when the configuration variable "algos" is written to at run time.
     * Adds the configuration variables that store the alternative configuration commands based on the new names and
     * sets the allowed values of configuration variable "select" */
    void on_set_algos();

    /** Callback executed when the configuration variable "select" is successfully written to at run time.
     * Causes the execution of the stored command for the selected condition in the context of the loaded plugin.
     */
    void on_set_select();

    /** Callback executed when the configuration variable "selectall" is written to at run time.
     * When set to yes, iterates once through all stored configurations in the order they appear in
     * configuration variable algos. */
    void event_select_all();
    MHAParser::vstring_t parser_algos;
    MHAParser::kw_t select_plug;
    MHAParser::bool_t selectall;

    /** Storage for alternative configuration commands.
     * New entries are registered with the plugin's parser when
     * parser_algos is updated.*/
    // We have to use a map here because std::map does not invalidate
    // references (insert_item takes an address) and provides indexing
    std::map<std::string,MHAParser::string_t> configs;

    /** Configuration event broker */
    MHAEvents::patchbay_t<altconfig_t> patchbay;

    // Because we touch more than just algos_parser, the basic "restore the variable written to" policy for
    // exception safety does not suffice here. algo_parser and configs are coupled, so we need to roll back
    // changes to configs too.
    /** Save the old state of the user-defined sub-parsers for
     * eventual restoration later. operator= does not suffice to store/restore the old state because we need
     * to re-insert the string_t's that were removed, but a copy of the string_t keeps a pointer to its parent
     * and complains if we try to insert_item it again. So we just copy the relevant data into a new string_t.
     * @param old_state old parser state
     * @returns Copy of the old state
     */
    std::map<std::string,MHAParser::string_t> save_state();

    /** Restore the old parser state from a saved state
     * @param state old parser state
     */
  void restore_state(std::map<std::string,MHAParser::string_t>& state,std::map<std::string,MHAParser::string_t>& failed_state);
};
