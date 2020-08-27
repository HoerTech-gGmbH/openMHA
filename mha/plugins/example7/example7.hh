// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2007 2009 2010 2012 2013 2014 2015 2017 2018
//             2020 HörTech gGmbH
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


#ifndef EXAMPLE7_H
#define EXAMPLE7_H

#include "mha_plugin.hh"

class example7_t : public MHAPlugin::plugin_t<int> {
public:
  example7_t(algo_comm_t&,
             const std::string&,
             const std::string&);

  void release(void);
  void prepare(mhaconfig_t&);
  mha_wave_t* process(mha_wave_t*);

};

#endif
