// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2009 2013 2014 2016 HörTech gGmbH
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
      userwnd("user provided window","[]",""),
      length(1)
{
    patchbay.connect(&wndtype.writeaccess,this,&windowselector_t::update_parser);
    patchbay.connect(&userwnd.writeaccess,this,&windowselector_t::update_parser);
    patchbay.connect(&wndexp.writeaccess,this,&windowselector_t::update_parser);
    update();
}

windowselector_t::~windowselector_t()
{
    if( wnd )
        delete wnd;
}

void windowselector_t::insert_items(MHAParser::parser_t* p)
{
    p->insert_item("wndtype",&wndtype);
    p->insert_item("wndexp",&wndexp);
    p->insert_item("userwnd",&userwnd);
}


void windowselector_t::set_length(unsigned int len)
{
    length = len;
    update();
}

const MHAWindow::base_t& windowselector_t::current()
{
    if( !wnd )
        throw MHA_Error(__FILE__,__LINE__,"currently no valid window is available (check for previous errors).");
    return *wnd;
}

void windowselector_t::update_parser()
{
    update();
    updated();
}

void windowselector_t::update()
{
    if( wnd )
        delete wnd;
    wnd = NULL;
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
                            "wave2spec: User window size (%d) is not window length (%d).",
                            userwnd.data.size(),length);
        wnd = new MHAWindow::user_t(userwnd.data);
        break;
    default:
        throw MHA_ErrorMsg("Unknown window type.");
    };
    *wnd ^= wndexp.data;
}

// Local Variables:
// coding: utf-8-unix
// End:
