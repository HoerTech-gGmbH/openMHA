// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2007 2013 2016 2017 HörTech gGmbH
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

#ifndef MHA_WINDOWPARSER_H
#define MHA_WINDOWPARSER_H

#include "mha_parser.hh"
#include "mha_signal.hh"

namespace MHAParser {

    /**
       \ingroup mhasignal
       \brief MHA configuration interface for a window function generator.

       This class implements a configuration interface (sub-parser)
       for window type selection and user-defined window type. It
       provides member functions to generate an instance of
       MHAWindow::base_t based on the values provided by the
       configuration interface.

       The configuration interface is derived from MHAParser::parser_t
       and can thus be inserted into the configuration tree using the
       insert_item() method of the parent parser.

       If one of the pre-defined window types is used, then the window
       is generated using the MHAWindow::fun_t class constructor; for
       the user-defined type the values from the "user" variable are
       copied.
     */
    class window_t : public MHAParser::parser_t
    {
    public:
        typedef enum { wnd_rect=0,
                       wnd_hann=1,
                       wnd_hamming=2,
                       wnd_blackman=3,
                       wnd_bartlett=4,
                       wnd_user=5 } wtype_t;
        /// \brief Constructor to create parser class.
        window_t(const std::string& help="Window type configuration.");
        /// \brief Create a window instance, use default parameters.
        MHAWindow::base_t get_window(unsigned int len) const;
        /// \brief Create a window instance.
        MHAWindow::base_t get_window(unsigned int len,float xmin) const;
        /// \brief Create a window instance.
        MHAWindow::base_t get_window(unsigned int len,float xmin,float xmax) const;
        /// \brief Create a window instance.
        MHAWindow::base_t get_window(unsigned int len,float xmin,float xmax,bool minincluded) const;
        /// \brief Create a window instance.
        MHAWindow::base_t get_window(unsigned int len,float xmin,float xmax,bool minincluded,bool maxincluded) const;
        /// \brief Return currently selected window type.
        MHAParser::window_t::wtype_t get_type() const;
    private:
        MHAParser::kw_t wtype;
        MHAParser::vfloat_t user;
    };

}

#endif
/*
 * Local Variables:
 * mode: c++
 * compile-command: "make -C .."
 * coding: utf-8-unix
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
