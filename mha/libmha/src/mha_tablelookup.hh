// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2013 2016 2017 HörTech gGmbH
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

#ifndef __TABLELOOKUP_H__
#define __TABLELOOKUP_H__

#include <vector>
#include <map>
#include "mha.h"

#ifdef __cplusplus

/**
    \ingroup mhatoolbox
    \file mha_tablelookup.hh
    \brief Header file for table lookup classes
*/

/** \ingroup mhatoolbox
    \brief Namespace for table lookup classes
*/
namespace MHATableLookup {
    using namespace std;

    class table_t {
    public:
        table_t(void);
        virtual ~table_t(void);
        virtual mha_real_t lookup(mha_real_t) const =0;
        virtual mha_real_t interp(mha_real_t) const =0;
    protected:
        virtual void clear(void) = 0;
    };

    class linear_table_t : public table_t {
    public:
        linear_table_t(void);
        mha_real_t lookup(mha_real_t) const;
        mha_real_t interp(mha_real_t) const;
        ~linear_table_t(void);
        void set_xmin(mha_real_t);
        void set_xmax(mha_real_t);
        void add_entry(mha_real_t);
        void prepare(void);
        void clear(void);
    protected:
        mha_real_t* vy;
        unsigned int len;
    private:
        vector<mha_real_t> vec_y;
        mha_real_t xmin;
        mha_real_t xmax;
        mha_real_t scalefac;
    };

    /** 
        \brief Class for interpolation with non-equidistant x values
        
        Linear interpolation of the x-y table is performed. A
        transformation of x and y-values is possible; if a
        transformation function is provided for the x-values, the same
        function is applied to the argument of xy_table_t::interp()
        and xy_table_t::lookup().  The transformation of y values is
        applied only during insertion into the table. Two functions
        for y-transformation can be provided: a simple transformation
        which depends only on the y values, or a transformation which
        takes both (non-transformed) x and y value as an argument. The
        two-argument transformation is applied before the one-argument
        transformation.
     */
    class xy_table_t : public table_t {
    public:
        xy_table_t();
        mha_real_t lookup(mha_real_t x) const;
        mha_real_t interp(mha_real_t x) const;
        void add_entry( mha_real_t x, mha_real_t y );
        void add_entry( mha_real_t* pVX, mha_real_t* pVY, unsigned int len );
        void clear();
        void set_xfun(float (*pXFun)(float));
        void set_yfun(float (*pYFun)(float));
        void set_xyfun(float (*pYFun)(float,float));
    private:
        std::map<mha_real_t,mha_real_t> mXY;
        float (*xfun)(float);
        float (*yfun)(float);
        float (*xyfun)(float,float);
    };

}

#endif

#endif

// Local Variables:
// mode: c++
// compile-command: "make -C .."
// c-basic-offset: 4
// coding: utf-8-unix
// indent-tabs-mode: nil
// End:
