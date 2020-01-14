// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2007 2013 2016 2017 2018 2020 HörTech gGmbH
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

MHAWindow::base_t::base_t(unsigned int n)
    : MHASignal::waveform_t(n,1)
{
}

MHAWindow::base_t::base_t(const MHAWindow::base_t& src)
    : MHASignal::waveform_t(src.num_frames,1)
{
    if( src.num_channels != 1 )
        throw MHA_Error(__FILE__,__LINE__,"Invalid window base (%u channels).",src.num_channels);
    for( unsigned int k=0;k<num_frames;k++)
        buf[k] = src[k];
}

void MHAWindow::base_t::operator()(mha_wave_t& s) const
{
    if( s.num_frames != num_frames )
        throw MHA_Error(__FILE__,__LINE__,
                        "Window function (overloaded): invalid number of frames (got %u, expected %u).",
                        s.num_frames,num_frames);
    unsigned int k;
    for(unsigned int ch=0;ch<s.num_channels;ch++)
        for(k=0;k<num_frames;k++)
            ::value(s,k,ch) *= value(k,0);
}

void MHAWindow::base_t::operator()(mha_wave_t* s) const
{
    if( s->num_frames != num_frames )
        throw MHA_Error(__FILE__,__LINE__,
                        "Window function (overloaded): invalid number of frames (got %u, expected %u).",
                        s->num_frames,num_frames);
    unsigned int k;
    for(unsigned int ch=0;ch<s->num_channels;ch++)
        for(k=0;k<num_frames;k++)
            ::value(s,k,ch) *= value(k,0);
}

void MHAWindow::base_t::ramp_begin(mha_wave_t& s) const
{
    if( s.num_frames < num_frames )
        throw MHA_Error(__FILE__,__LINE__,"Cannot apply ramp to a signal which is shorter than the ramp (%u<%u)",
                        s.num_frames, num_frames);
    unsigned int k;
    for(unsigned int ch=0;ch<s.num_channels;ch++)
        for(k=0;k<num_frames;k++)
            ::value(s,k,ch) *= value(k,ch);
}

void MHAWindow::base_t::ramp_end(mha_wave_t& s) const
{
    if( s.num_frames < num_frames )
        throw MHA_Error(__FILE__,__LINE__,"Cannot apply ramp to a signal which is shorter than the ramp (%u<%u)",
                        s.num_frames, num_frames);
    unsigned int k;
    unsigned int k0 = s.num_frames-num_frames;
    for(unsigned int ch=0;ch<s.num_channels;ch++)
        for(k=0;k<num_frames;k++)
            ::value(s,k+k0,ch) *= value(k,ch);
}

float MHAWindow::rect(float x)
{
    if( (x < -1) || (x >= 1) )
        return 0;
    return 1;
}

float MHAWindow::bartlett(float x)
{
    if( (x < -1) || (x >= 1) )
        return 0;
    if( x < 0 )
        return x+1.0f;
    return 1.0f-x;
}

float MHAWindow::hanning(float x)
{
    if( (x < -1) || (x >= 1) )
        return 0;
    return 0.5 + 0.5 * cos( M_PI*x );
}

float MHAWindow::hamming(float x)
{
    if( (x < -1) || (x >= 1) )
        return 0;
    return 0.54 + 0.46 * cos( M_PI*x );
}

float MHAWindow::blackman(float x)
{
    if( (x < -1) || (x >= 1) )
        return 0;
    return std::max(0.42-0.5*cos(M_PI*(x+1.0))+0.08*cos(2.0*M_PI*(x+1.0)),0.0);
}

MHAWindow::user_t::user_t(const std::vector<mha_real_t>& wnd)
    : MHAWindow::base_t(wnd.size())
{
    for(unsigned int k=0;k<wnd.size();k++)
        buf[k] = wnd[k];
}

MHAWindow::fun_t::fun_t(unsigned int n,float (*fun)(float),float xmin,float xmax,bool min_included,bool max_included)
    : MHAWindow::base_t(n)
{
    float x;
    float virtn = (float)mha_min_1(n-min_included-max_included+1);
    float xmin1 = xmin;
    if( !min_included )
        xmin1 += (xmax-xmin)/virtn;
    for(unsigned int k=0;k<n;k++){
        x = xmin1 + k*(xmax-xmin)/virtn;
        buf[k] = fun(x);
    }
}

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
        // During C++ construction, a user window is initialized with
        // all ones (rectangle).  We can replace the ones by applying
        // the window to the ones.
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
