// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2009 2013 2016 2017 2018 2020 HörTech gGmbH
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

#include "mha_tablelookup.hh"
#include "mha_error.hh"
#include <math.h>

using namespace MHATableLookup;

table_t::table_t(){}
table_t::~table_t(){}

xy_table_t::xy_table_t()
    : xfun(NULL), yfun(NULL), xyfun(NULL)
{
}

/** \brief Clear the table and transformation functions.
 */
void xy_table_t::clear()
{
    mXY.clear();
    xfun = NULL;
    yfun = NULL;
    xyfun = NULL;
}

/**
   \brief Set transformation function for x values.
   \param fun Transformation function.
*/
void xy_table_t::set_xfun( float (*fun)(float) )
{
    xfun = fun;
}

/**
   \brief Set transformation function for y values during insertion.
   \param fun Transformation function.
*/
void xy_table_t::set_yfun( float (*fun)(float) )
{
    yfun = fun;
}

/**
   \brief Set transformation function for y values during insertion, based on x and y values.
   \param fun Transformation function.
*/
void xy_table_t::set_xyfun( float (*fun)(float,float) )
{
    xyfun = fun;
}

/**
   \brief Add a single x-y pair entry.
   \param x x value
   \param y corresponding y value
*/
void xy_table_t::add_entry(mha_real_t x, mha_real_t y)
{
    if( xyfun )
        y = xyfun( x, y );
    if( xfun )
        x = xfun(x);
    if( yfun )
        y = yfun(y);
    mXY[x] = y;
}

/**
   \brief Add multiple entries at once.
   \param pVX array of x values
   \param pVY array of y values
   \param uLength Length of x and y arrays
*/
void xy_table_t::add_entry(mha_real_t* pVX, mha_real_t* pVY, unsigned int uLength)
{
    for( unsigned int k=0;k<uLength;k++)
        add_entry( pVX[k], pVY[k] );
}

/**
   \brief Linear interpolation function
   \param x x value
   \return interpolated y value
*/
mha_real_t xy_table_t::interp( mha_real_t x ) const
{
    if( !mXY.size() )
        throw MHA_ErrorMsg("the xy table has no entries");
    // border cases, single entry:
    if( mXY.size() == 1 )
        return mXY.begin()->second;
    // transform x value:
    if( xfun )
        x = xfun(x);
    std::map<mha_real_t,mha_real_t>::const_iterator mXYit1(mXY.lower_bound(std::max(mXY.begin()->first,std::min(mXY.rbegin()->first,x))));
    std::map<mha_real_t,mha_real_t>::const_iterator mXYit2(mXYit1);
    if( mXYit1 == mXY.begin() )
        mXYit2++;
    else
        mXYit1--;
    // 
    return mXYit1->second + (mXYit2->second - mXYit1->second)*(x-mXYit1->first)/(mXYit2->first-mXYit1->first);
}


/**
   \brief Return the y-value at the position of the nearest x value below input.
   
   \param x Input value
   \return y value at nearest x value below input.
*/
mha_real_t xy_table_t::lookup( mha_real_t x ) const
{
    if( !mXY.size() )
        throw MHA_ErrorMsg("the xy table has no entries");
    // border cases, single entry:
    if( mXY.size() == 1 )
        return mXY.begin()->second;
    // transform x value:
    if( xfun )
        x = xfun(x);
    // border cases, below range:
    if( x <= mXY.begin()->first )
        return mXY.begin()->second;
    // border cases, above range:
    if( x >= mXY.rbegin()->first )
        return mXY.rbegin()->second;
    // value is in range:
    std::map<mha_real_t,mha_real_t>::const_iterator mXYit(mXY.lower_bound(x));
    if( x == mXYit->first )
        return mXYit->second;
    if( mXYit == mXY.begin() )
        return mXYit->second;
    std::map<mha_real_t,mha_real_t>::const_iterator mXYitL(mXYit);
    mXYitL--;
    if( fabs(x-mXYitL->first) <= fabs(x-mXYit->first) )
        return mXYitL->second;
    return mXYit->second;
}

linear_table_t::linear_table_t(void)
    : table_t(),
      vy(NULL),
      len(0),
      xmin(0.0),
      xmax(1.0),
      scalefac(0)
{}

linear_table_t::~linear_table_t(void)
{
    if( vy )
        delete [] vy;
}

mha_real_t linear_table_t::lookup(mha_real_t x) const
{
    mha_real_t ind;
    ind = x - xmin;
    ind *= scalefac;
    ind = std::max(ind, 0.0f);
    ind = std::min(ind, (mha_real_t)len-1.0f);
    return vy[(unsigned int)ind];
}

mha_real_t linear_table_t::interp(mha_real_t x) const
{
    mha_real_t ind, frac, ret;
    unsigned int dw_ind;
    ind = x - xmin;
    ind *= scalefac;
    if( len < 2 ){
        throw MHA_Error(__FILE__,__LINE__,"Invalid table length.");
    }
    if( ind < 0 ){
        frac = ind;
        ret = vy[0] + frac * (vy[1] - vy[0]);
    }else if( ind > (mha_real_t)len - 2.0 ){
        frac = ind - (mha_real_t)len + 2.0;
        dw_ind = len-2;
        ret = vy[dw_ind] + frac * (vy[dw_ind+1] - vy[dw_ind]);
    }else{
        frac = ind;
        ind = floor(ind);
        frac -= ind;
        dw_ind = (unsigned int)ind;
        ret = vy[dw_ind];
        if( ind+1 < (mha_real_t)len )
            ret += frac * (vy[dw_ind+1] - vy[dw_ind]);
    }
    return ret;
}

void linear_table_t::set_xmin(mha_real_t x)
{
    xmin = x;
}

void linear_table_t::set_xmax(mha_real_t x)
{
    xmax = x;
}

void linear_table_t::add_entry(mha_real_t x)
{
    vec_y.push_back(x);
}

void linear_table_t::prepare(void)
{
    unsigned int k;
    if( xmax <= xmin ){
        throw MHA_ErrorMsg("invalid x range"); 
    }
    if( vy )
        delete [] vy;
    vy = NULL;
    len = vec_y.size();
    if( len > 1 ){
        vy = new mha_real_t[len];
        scalefac = (mha_real_t)len/(xmax - xmin);
        for( k=0; k<len; k++)
            vy[k] = vec_y[k];
    }else{
        throw MHA_Error(__FILE__,__LINE__,
                        "The table is empty.");
    }
}

void linear_table_t::clear(void)
{
    len = 0;
    if( vy )
        delete [] vy;
    vy = NULL;
    vec_y.clear();
}


// Local Variables:
// compile-command: "make -C .."
// coding: utf-8-unix
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
