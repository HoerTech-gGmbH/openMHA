// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2007 2013 2016 2018 HörTech gGmbH
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

#include "mha_windowparser.h"

float (*wnd_funs[])(float) = {
    MHAWindow::rect,
    MHAWindow::hanning,
    MHAWindow::hamming,
    MHAWindow::blackman,
    MHAWindow::bartlett,
    MHAWindow::rect
};

MHAParser::window_t::window_t(const std::string& help)
    : MHAParser::parser_t(help),
      wtype("Window type.","hanning","[rect hanning hamming blackman bartlett user]"),
      user("User provided window (used if window type==user).","[]","")
{
    insert_item("type",&wtype);
    insert_item("user",&user);
}

MHAWindow::base_t MHAParser::window_t::get_window(unsigned int len) const
{
    MHAParser::window_t::wtype_t wt(get_type());
    MHAWindow::fun_t w(len,wnd_funs[wt]);
    if( wt == MHAParser::window_t::wnd_user ){
        // a user window is initialized with all ones (rectangle).
        // We can replace the ones by applying the window to the ones.
        MHAWindow::user_t user_wnd(user.data);
        user_wnd(w);
    }
    return w;
}

MHAWindow::base_t MHAParser::window_t::get_window(unsigned int len,float xmin) const
{
    MHAParser::window_t::wtype_t wt(get_type());
    MHAWindow::fun_t w(len,wnd_funs[wt],xmin);
    if( wt == MHAParser::window_t::wnd_user ){
        MHAWindow::user_t user_wnd(user.data);
        user_wnd(w);
    }
    return w;
}

MHAWindow::base_t MHAParser::window_t::get_window(unsigned int len,float xmin,float xmax) const
{
    MHAParser::window_t::wtype_t wt(get_type());
    MHAWindow::fun_t w(len,wnd_funs[wt],xmin,xmax);
    if( wt == MHAParser::window_t::wnd_user ){
        MHAWindow::user_t user_wnd(user.data);
        user_wnd(w);
    }
    return w;
}

MHAWindow::base_t MHAParser::window_t::get_window(unsigned int len,float xmin,float xmax,bool minincluded) const
{
    MHAParser::window_t::wtype_t wt(get_type());
    MHAWindow::fun_t w(len,wnd_funs[wt],xmin,xmax,minincluded);
    if( wt == MHAParser::window_t::wnd_user ){
        MHAWindow::user_t user_wnd(user.data);
        user_wnd(w);
    }
    return w;
}

MHAWindow::base_t MHAParser::window_t::get_window(unsigned int len,float xmin,float xmax,bool minincluded,bool maxincluded) const
{
    MHAParser::window_t::wtype_t wt(get_type());
    MHAWindow::fun_t w(len,wnd_funs[wt],xmin,xmax,minincluded,maxincluded);
    if( wt == MHAParser::window_t::wnd_user ){
        MHAWindow::user_t user_wnd(user.data);
        user_wnd(w);
    }
    return w;
}

MHAParser::window_t::wtype_t MHAParser::window_t::get_type() const
{
    MHAParser::window_t::wtype_t wt = (MHAParser::window_t::wtype_t)wtype.data.get_index();
    return wt;
}

/*
 * Local Variables:
 * compile-command: "make -C .."
 * coding: utf-8-unix
 * End:
 */
