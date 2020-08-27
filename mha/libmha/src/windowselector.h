// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2013 2014 2016 2017 2018 HörTech gGmbH
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
#include "mha_windowparser.h"

/** A combination of mha parser variables to describe an overalapadd analysis
 * window.  Provides a method to get the window samples as an instance of
 * MHAWindow::base_t when needed. */
class windowselector_t {
public:
    /** constructor creates the mha parser variables that describe an
     * overlapadd analysis window.
     * @param default_type name of the default analysis window type.
     *                     Must be one of: "rect", "bartlett", "hanning",
     *                     "hamming", "blackman"
     */
    windowselector_t(const std::string& default_type);

    /** destructor frees window data that were allocated */
    ~windowselector_t();

    /** re-computes the window if required.
     * @param length the desired window length in samples
     * return the window's samples as a constref to MHAWindow::base_t instance.
     *        The referenced instance lives until the window parameters are
     *        changed, or this windowselector_t instance is destroyed. */
    const MHAWindow::base_t& get_window_data(unsigned length);

    /** insert the window parameters "wndtype", "wndexp", and "userwnd" as 
     * mha configuration parameters into the given mha configuration parser.
     * @param p The configuration parser where to insert the window parameters.
     *          E.g. the plugin wave2spec's interface class. */
    void insert_items(MHAParser::parser_t* p);

    /** Lock/Unlock variables
     * @param b_ Desired lock state
     */
    void setlock(bool b_);

    /** A collector event that fires when any of the window parameters managed
     * here is written to. */
    MHAEvents::emitter_t updated;
private:
    /** invalidates any allocated window samples. */
    void invalidate_window_data();
    
    /** invoked when a parser parameter changes.  Calls
     * invalidate_window_data() and emits the updated event. */
    void update_parser();

    /** Storage for the window data returned by get_window_data() */
    MHAWindow::base_t* wnd;

    /** parser variable for window type */
    MHAParser::kw_t wndtype;

    /** parser variable for window exponent */
    MHAParser::float_t wndexp;

    /** parser variable for user window samples to use */
    MHAParser::vfloat_t userwnd;

    /** patchbay to watch for changes for the parser variables */
    MHAEvents::patchbay_t<windowselector_t> patchbay;
};

#endif

// Local Variables:
// compile-command: "make -C .."
// mode: c++
// coding: utf-8-unix
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
