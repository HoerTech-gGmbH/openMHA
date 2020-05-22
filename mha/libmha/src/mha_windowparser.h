// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2007 2013 2016 2017 2018 HörTech gGmbH
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
#include "mha_algo_comm.h"

/**
   \ingroup mhasignal
   \brief Collection of Window types
*/
namespace MHAWindow {

    /**
       \brief Common base for window types
    */
    class base_t : public MHASignal::waveform_t {
    public:
        /** 
            \brief Constructor
            \param len Window length in samples.
        */
        base_t(unsigned int len);
        /**
           \brief Copy constructor
           \param src Source to be copied
        */
        base_t(const MHAWindow::base_t& src);
        void operator()(mha_wave_t&) const;/**< \brief Apply window to waveform segment (reference) */
        void operator()(mha_wave_t*) const;/**< \brief Apply window to waveform segment (pointer) */
        void ramp_begin(mha_wave_t&) const;/**< \brief Apply a ramp at the begining */
        void ramp_end(mha_wave_t&) const;/**< \brief Apply a ramp at the end */
    };

    float rect(float);///<\brief Rectangular window function
    float bartlett(float);///<\brief Bartlett window function
    float hanning(float);///<\brief Hanning window function
    float hamming(float);///<\brief Hamming window function
    float blackman(float);///<\brief Blackman window function

    /**
       \brief Generic window based on a generator function

       The generator function should return a valid window function in
       the interval [-1,1[.
    */
    class fun_t : public MHAWindow::base_t {
    public:
        /**
           \brief Constructor.
           \param n Window length
           \param fun Generator function, i.e. MHAWindow::hanning()
           \param xmin Start value of window, i.e. -1 for full window or 0 for fade-out ramp.
           \param xmax Last value of window, i.e. 1 for full window
           \param min_included Flag if minimum value is included
           \param max_included Flag if maximum value is included
        */
        fun_t(unsigned int n,float (*fun)(float),float xmin=-1,float xmax=1,bool min_included=true,bool max_included=false);
    };

    /** \brief Rectangular window */
    class rect_t : public MHAWindow::fun_t {
    public:
        rect_t(unsigned int n):MHAWindow::fun_t(n,MHAWindow::rect){};
    };

    /** \brief Bartlett window */
    class bartlett_t : public MHAWindow::fun_t {
    public:
        bartlett_t(unsigned int n):MHAWindow::fun_t(n,MHAWindow::bartlett){};
    };

    /** \brief von-Hann window */
    class hanning_t : public MHAWindow::fun_t {
    public:
        hanning_t(unsigned int n):MHAWindow::fun_t(n,MHAWindow::hanning){};
    };

    /** \brief Hamming window */
    class hamming_t : public MHAWindow::fun_t {
    public:
        hamming_t(unsigned int n):MHAWindow::fun_t(n,MHAWindow::hamming){};
    };

    /** \brief Blackman window */
    class blackman_t : public MHAWindow::fun_t {
    public:
        blackman_t(unsigned int n):MHAWindow::fun_t(n,MHAWindow::blackman){};
    };

    /** \brief User defined window */
    class user_t : public MHAWindow::base_t {
    public:
        /** \brief Constructor
            \param wnd User defined window
        */
        user_t(const std::vector<mha_real_t>& wnd);
    };

}

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
        void setlock(bool b){
            wtype.setlock(b);
            user.setlock(b);
        }
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
