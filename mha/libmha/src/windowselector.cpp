// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2009 2013 2014 2016 2017 2018 2020 HörTech gGmbH
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

#include "windowselector.h"
#include "mha_error.hh"

windowselector_t::windowselector_t(const std::string& default_type)
    : wnd(NULL),
      wndtype("window type",default_type,"[rect bartlett hanning hamming blackman user]"),
      wndexp("window exponent to be applied to all elements of window function","1"),
      userwnd("user provided window","[]","")
{
    patchbay.connect(&wndtype.writeaccess,this,&windowselector_t::update_parser);
    patchbay.connect(&userwnd.writeaccess,this,&windowselector_t::update_parser);
    patchbay.connect(&wndexp.writeaccess,this,&windowselector_t::update_parser);
}

windowselector_t::~windowselector_t()
{
    invalidate_window_data();
}

void windowselector_t::invalidate_window_data()
{
    if( wnd )
        delete wnd;
    wnd = nullptr;
}

void windowselector_t::insert_items(MHAParser::parser_t* p)
{
    p->insert_item("wndtype",&wndtype);
    p->insert_item("wndexp",&wndexp);
    p->insert_item("userwnd",&userwnd);
}

void windowselector_t::setlock(bool b_) {
    wndtype.setlock(b_);
    wndexp.setlock(b_);
    userwnd.setlock(b_);
}

void windowselector_t::update_parser()
{
    invalidate_window_data();
    updated();
}

const MHAWindow::base_t& windowselector_t::get_window_data(unsigned length)
{
    if( !wnd ) {
        switch(wndtype.data.get_index()){
        case 0 : // rect
            wnd = new MHAWindow::rect_t(length);
            break;
        case 1 : // bartlett
            wnd = new MHAWindow::bartlett_t(length);
            break;
        case 2 : // hanning
            wnd = new MHAWindow::hanning_t(length);
            break;
        case 3 : // hamming
            wnd = new MHAWindow::hamming_t(length);
            break;
        case 4 : // blackman
            wnd = new MHAWindow::blackman_t(length);
            break;
        case 5 : // user
            if( userwnd.data.size() != length )
                throw MHA_Error(__FILE__,__LINE__,
                                "wave2spec: User window size (%zu) is not window length (%u).",
                                userwnd.data.size(),length);
            wnd = new MHAWindow::user_t(userwnd.data);
            break;
        default:
            throw MHA_ErrorMsg("Unknown window type.");
        }
        *wnd ^= wndexp.data;
    }
    return *wnd;
}

// Local Variables:
// coding: utf-8-unix
// indent-tabs-mode: nil
// c-basic-offset: 4
// End:
