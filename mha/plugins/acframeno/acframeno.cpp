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

/** The class which implements the acframeno_t plugin.
 */
class acframeno_t : public MHAPlugin::plugin_t<int> {
public:
  /// Plugin constructor
  acframeno_t(algo_comm_t iac, const std::string &configured_name);
  void prepare(mhaconfig_t &);
  mha_wave_t *process(mha_wave_t *);
  mha_spec_t *process(mha_spec_t *);

private:
  void process();
  MHA_AC::int_t cnt;
};

acframeno_t::acframeno_t(algo_comm_t iac, const std::string &configured_name)
    : MHAPlugin::plugin_t<int>("Frame counter as AC variable.", iac),
      cnt(iac, configured_name) {}

void acframeno_t::prepare(mhaconfig_t &) { cnt.insert(); }

mha_wave_t *acframeno_t::process(mha_wave_t *s) {
  ++cnt.data;
  return s;
}

mha_spec_t *acframeno_t::process(mha_spec_t *s) {
  ++cnt.data;
  return s;
}

MHAPLUGIN_CALLBACKS(acframeno, acframeno_t, wave, wave)
MHAPLUGIN_PROC_CALLBACK(acframeno, acframeno_t, spec, spec)
MHAPLUGIN_DOCUMENTATION(
    acframeno, "frame counter as AC variable",
    "The \\texttt{acframeno} plugin can be used to count frames. The frame "
    "number will be set to zero when the plugin is loaded."
    "\n\n"
    "")

/*
 * Local Variables:
 * c-basic-offset: 4
 * compile-command: "make"
 * indent-tabs-mode: nil
 * End:
 */
