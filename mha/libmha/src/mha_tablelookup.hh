// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2013 2016 2017 2018 HörTech gGmbH
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

    /** Class for interpolation with equidistant x values.

        This class can be used for linear interpolation tasks where
        the mesh points are known for equidistant x values.
        
        Before the class can be used for interpolation, it has to be filled
        with the y values for the mesh points, the x range has to be specified,
        and when all values are given, the prepare method has to be called
        so that the object can determine the distance between x values from
        the range and the number of mesh points given.

        Only after prepare has returned, the object may be used for
        interpolation.
     */
    class linear_table_t : public table_t {
    public:
        /** contructor creates an empty linear_table_t object.
         * add_entry, set_xmin, set_xmax and prepare methods have to be called
         * before the object can be used to lookup and interpolate values. */
        linear_table_t(void);

        /** look up the y value that is stored for the mesh point where x
         * is lower than or equal to the x value given here.
         * 
         * This method does not extrapolate, so for x < xmin, the y
         * value for xmin is returned. For all x greater than the x of
         * the last mesh point, the y value of the last mesh point is
         * returned.
         *
         * @pre prepare must have been called before lookup may be called. */
        mha_real_t lookup(mha_real_t x) const;

        /** interpolate y value for the given x value. The y values
         * for the neighbouring mesh points are looked up and linearly
         * interpolated.  For x values outside the range of mesh
         * points, the y value is extrapolated from the nearest two
         * mesh points.
         *
         * @pre prepare must have been called before interp may be called. */
        mha_real_t interp(mha_real_t x) const;

        /** destructor */
        ~linear_table_t(void);

        /** set the x value for the first mesh point.  Must be called
         * before prepare can be called. */
        void set_xmin(mha_real_t xmin);

        /** set the y value for the next mesh point.  Must be called at least
         * twice before prepare can be called. */
        void add_entry(mha_real_t y);

        /** this sets the x value for a past-the-end, not added mesh point.
         * Example:
         *
         *     t.set_xmin(100);
         *     t.add_entry(0); // mesh point {100,0}
         *     t.add_entry(1); // mesh point {110,1}
         *     // the next mesh point would be at x=120, but we do not add this
         *     t.set_xmax(120); // the x where the next mesh point would be
         *     t.prepare();
         *
         * now, t.interp(100) == 0; t.interp(110) == 1; t.interp(105) == 0.5;
         */ 
        void set_xmax(mha_real_t xmax );

        /** prepare computes the x distance of the mesh points based
         * on the values given to set_xmin, set_xmax, and the number
         * of times that add_entry was called.
         *
         * @pre set_xmin, set_xmax, add_entry functions must have been
         *      called before calling prepare, add_entry must have
         *      been called at least twice.
         * 
         * Only after this method has been called, interp or lookup
         * may be called.
         */
        void prepare(void);

        /** clear resets the state of this object to the state directly after
         * construction.  mesh entries and x range are deleted.
         * 
         * interp and lookup may not be called after this function has
         * been called unless prepare and before that its precondition
         * methods are called again.
         */
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
        /** returns the min and max x of all mesh points that are
         * stored in the lookup table, i.e. after transformation with
         * xfun, if any. Not real-time safe */
        std::pair<mha_real_t,mha_real_t> get_xlimits() const
        { return std::pair<mha_real_t,mha_real_t>(mXY.begin()->first,
                                                  mXY.rbegin()->first); }
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
