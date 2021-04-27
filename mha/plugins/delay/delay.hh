// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2007 2008 2009 2010 2013 2014 2015 2017 2018 HörTech gGmbH
// Copyright © 2019 2020 2021 HörTech gGmbH
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
#include "mha_events.h"

namespace delay {
    class interface_t : public MHAPlugin::plugin_t<MHASignal::delay_t> {
        public:
            interface_t(algo_comm_t iac, const std::string & configured_name);
            void prepare(mhaconfig_t&);
            mha_wave_t* process(mha_wave_t*);
        private:
            void update();
            MHAParser::vint_t delays;
            MHAEvents::patchbay_t<interface_t> patchbay;
    };
}

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
