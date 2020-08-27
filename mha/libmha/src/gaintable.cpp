// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2009 2013 2016 2017 2020 HörTech gGmbH
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

#include "gaintable.h"

#include <math.h>
#include <numeric>
#include <iostream>
#include "mha_signal.hh"

using namespace DynComp;

std::vector<mha_real_t> convert_f2logf(const std::vector<mha_real_t>& vF)
{
    std::vector<mha_real_t> ret;
    for(std::vector<mha_real_t>::const_iterator it=vF.begin();it!=vF.end();++it)
        ret.push_back(log(*it));
    return ret;
}

gaintable_t::gaintable_t(const std::vector<mha_real_t>& LInput,const std::vector<mha_real_t>& FCenter,unsigned int channels)
    : num_L(LInput.size()),
      num_F(FCenter.size()),
      num_channels(channels),
      vL(LInput),
      vF(FCenter),
      vFlog(convert_f2logf(FCenter))
{
    data.resize(num_channels);
    for(unsigned int ch=0;ch<num_channels;ch++){
        data[ch].resize(num_F);
        for(unsigned int kf=0;kf<num_F;kf++){
            data[ch][kf].resize(num_L);
            for(unsigned int kl=0;kl<num_L;kl++)
                data[ch][kf][kl] = 0;
        }
    }
}

gaintable_t::~gaintable_t()
{
}

std::vector<std::vector<mha_real_t> > gaintable_t::get_iofun() const
{
    std::vector<std::vector<mha_real_t> > retv;
    for(unsigned int ch=0;ch<num_channels;ch++)
        for(unsigned int kf=0;kf<num_F;kf++)
            retv.push_back(data[ch][kf]);
    return retv;
}


mha_real_t gaintable_t::get_gain(mha_real_t Lin,mha_real_t Fin,unsigned int channel)
{
    return interp2(vFlog,vL,data[channel],log(Fin),Lin);
}

mha_real_t gaintable_t::get_gain(mha_real_t Lin, unsigned int band, unsigned int channel)
{
    return interp1(vL,data[channel][band],Lin);
}

void gaintable_t::get_gain(const mha_wave_t& Lin,mha_wave_t& Gain)
{
    MHA_assert_equal(Lin.num_channels,num_channels*num_F);
    MHA_assert_equal(Lin.num_channels,Gain.num_channels);
    MHA_assert_equal(Lin.num_frames,Gain.num_frames);
    for(unsigned int ch=0;ch<num_channels;ch++)
        for(unsigned int kf=0;kf<num_F;kf++)
            for(unsigned int kt=0;kt<Lin.num_frames;kt++)
                value(Gain,kt,num_F*ch+kf) = get_gain(value(Lin,kt,num_F*ch+kf),kf,ch);
}

bool isempty(const std::vector<std::vector<mha_real_t> > & arg)
{
    if( !arg.size() )
        return true;
    if( (arg.size() == 1) && (!arg[0].size()) )
        return true;
    return false;
}

void gaintable_t::update(std::vector<std::vector<std::vector<mha_real_t> > > newGain)
{
    if( data.size() != newGain.size() )
        throw MHA_Error(__FILE__,__LINE__,
                        "The gain table size cannot change (expected %zu channels, got %zu).",
                        data.size(),newGain.size());
    for(unsigned int ch=0;ch<num_channels;ch++){
        if( isempty(newGain[ch]) ){
            newGain[ch].resize(data[ch].size());
            for( unsigned int kf=0;kf<data[ch].size();kf++){
                newGain[ch][kf].resize(data[ch][kf].size());
            }
        }
        if( data[ch].size() != newGain[ch].size() )
            throw MHA_Error(__FILE__,__LINE__,
                            "The gain table size cannot change (expected %zu frequencies in channel %u, got %zu).",
                            data[ch].size(),ch,newGain[ch].size());
        for( unsigned int kf=0;kf<num_F;kf++){
            if( data[ch][kf].size() != newGain[ch][kf].size() )
                throw MHA_Error(__FILE__,__LINE__,
                                "The gain table size cannot change (expected %zu levels in band %u channel %u, got %zu).",
                                data[ch][kf].size(),kf,ch,newGain[ch][kf].size());
        }
    }
    data = newGain;
}

mha_real_t DynComp::interp1(const std::vector<mha_real_t>& vX, const std::vector<mha_real_t>& vY, mha_real_t X)
{
    if( vX.size() != vY.size() )
        throw MHA_ErrorMsg("Mismatching size.");
    if( vX.size() == 0 )
        throw MHA_ErrorMsg("Empty data (interp1).");
    if( vX.size() == 1 )
        return vY[0];
    // first search for optimal X:
    std::vector<mha_real_t>::const_iterator itX = vX.begin();
    std::vector<mha_real_t>::const_iterator itY = vY.begin();
    while( (itX < vX.end()) && (*itX < X) ){
        ++itX;
        ++itY;
    }
    if( itX == vX.end() ){
        --itX;
        --itY;
    }
    // search neighbour of optimal X:
    std::vector<mha_real_t>::const_iterator itXn = itX;
    std::vector<mha_real_t>::const_iterator itYn = itY;
    if( itXn == vX.begin() ){
        ++itYn;
        ++itXn;
    }else{
        --itXn;
        --itYn;
    }
    // interpolate between optimal and neighbour:
    return *itYn + (*itY - *itYn)*(X - *itXn)/(*itX - *itXn);
}


mha_real_t DynComp::interp2(const std::vector<mha_real_t>& vX, const std::vector<mha_real_t>& vY, const std::vector<std::vector<mha_real_t> >& mZ, mha_real_t X, mha_real_t Y)
{
    if( (!vX.size()) || (!vY.size()) )
        throw MHA_ErrorMsg("Empty data (interp2).");
    if( vX.size() != mZ.size() )
        throw MHA_ErrorMsg("Mismatching size.");
    // return if singular data point in database:
    if( mZ.size() == 1 ){
        if( mZ[0].size() == 1 )
            return mZ[0][0];
        else
            return DynComp::interp1(vY,mZ[0],Y);
    }
    // first search for optimal X:
    std::vector<mha_real_t>::const_iterator itX = vX.begin();
    std::vector<std::vector<mha_real_t> >::const_iterator itZY = mZ.begin();
    while( (itX < vX.end()) && (*itX < X) ){
        ++itX;
        ++itZY;
    }
    if( itX == vX.end() ){
        --itX;
        --itZY;
    }
    // check size of optimal entry:
    if( vY.size() != itZY->size() )
        throw MHA_ErrorMsg("Mismatching size.");
    // search neighbour of optimal X:
    std::vector<mha_real_t>::const_iterator itXn = itX;
    std::vector<std::vector<mha_real_t> >::const_iterator itZYn = itZY;
    if( itXn == vX.begin() ){
        ++itXn;
        ++itZYn;
    }else{
        --itXn;
        --itZYn;
    }
    // check size of neighbour entry:
    if( vY.size() != itZYn->size() )
        throw MHA_ErrorMsg("Mismatching size.");
    // now search for optimal Y:
    std::vector<mha_real_t>::const_iterator itY = vY.begin();
    std::vector<mha_real_t>::const_iterator itZ_rt = itZY->begin();
    std::vector<mha_real_t>::const_iterator itZ_lt = itZYn->begin();
    while( (itY < vY.end()) && (*itY < Y) ){
        ++itY;
        ++itZ_rt;
        ++itZ_lt;
    }
    std::vector<mha_real_t>::const_iterator itYn = itY;
    std::vector<mha_real_t>::const_iterator itZ_rb = itZ_rt;
    std::vector<mha_real_t>::const_iterator itZ_lb = itZ_lt;
    if( itYn == vY.begin() ){
        ++itYn;
        ++itZ_rb;
        ++itZ_lb;
    }else{
        --itYn;
        --itZ_rb;
        --itZ_lb;
    }
    // now calculate the average, spanned by the four points rt, rb, lt, lb
    mha_real_t rel_Xpos = (X - *itXn)/(*itX - *itXn);
    mha_real_t xt = *itZ_lt + (*itZ_rt - *itZ_lt)*rel_Xpos;
    mha_real_t xb = *itZ_lb + (*itZ_rb - *itZ_lb)*rel_Xpos;
    mha_real_t result = xb + (xt - xb)*(Y - *itYn)/(*itY - *itYn);
    return result;
}


// Local Variables:
// compile-command: "make -C .."
// coding: utf-8-unix
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
