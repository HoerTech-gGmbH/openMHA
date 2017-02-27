// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2013 2014 2016 HörTech gGmbH
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

#ifndef _WINDOWSELECTOR_H_
#define _WINDOWSELECTOR_H_

#include "mha_parser.hh"
#include "mha_signal.hh"
#include "mha_event_emitter.h"
#include "mha_events.h"

class windowselector_t {
public:
    windowselector_t(const std::string& default_type);
    ~windowselector_t();
    void set_length(unsigned int len);
    const MHAWindow::base_t& current();
    void insert_items(MHAParser::parser_t* p);
    MHAEvents::emitter_t updated;
private:
    void update();
    void update_parser();
    MHAWindow::base_t* wnd;
    MHAParser::kw_t wndtype;
    MHAParser::float_t wndexp;
    MHAParser::vfloat_t userwnd;
    MHAEvents::patchbay_t<windowselector_t> patchbay;
    unsigned int length;
};

#endif

// Local Variables:
// mode: c++
// coding: utf-8-unix
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
