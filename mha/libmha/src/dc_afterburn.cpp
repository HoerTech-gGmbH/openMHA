// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2010 2011 2013 2016 HörTech gGmbH
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

#include "dc_afterburn.h"
#include "gaintable.h"

/**
   \namespace DynComp
   \brief dynamic compression related classes and functions
 */


DynComp::dc_afterburn_vars_t::dc_afterburn_vars_t()
    : f("Sample frequencies of data / Hz.","[1000]","[0,]"),
      drain("Drain caused by vent / dB.","[0]"),
      conflux("Conflux caused by vent / dB.","[-120]"),
      maxgain("Maximum allowed gain / dB.","[80]"),
      mpo("Maximum allowed output level / dB SPL (see notes in plugin doc).","[120]"),
      taugain("Time constant of afterburn gain modifier lowpass / s.","0.2","[0,]"),
      commit("Commit changes of configuration variables.","commit","[commit]"),
      bypass("Bypass afterburn stage.","no")
{
    insert_member(f);
    insert_member(drain);
    insert_member(conflux);
    insert_member(maxgain);
    insert_member(mpo);
    insert_member(taugain);
    insert_member(commit);
    insert_member(bypass);
    set_node_id("dc_afterburn");
}


DynComp::dc_afterburn_t::dc_afterburn_t()
    : _channels(0),
      _srate(1.0f),
      commit_pending(false),
      fb_pars_configured(false)
{
    patchbay.connect(&commit.writeaccess,this,&dc_afterburn_t::update);
}

float mylogf( float x )
{
    return logf(std::max(1e-5f,x));
}

DynComp::dc_afterburn_rt_t::dc_afterburn_rt_t(const std::vector<float>& cf,unsigned int channels,float srate, const dc_afterburn_vars_t& vars)
{
    if( vars.f.data.size() < 1 )
        throw MHA_Error(__FILE__,__LINE__,"At least one entry is expected in after burner variable 'f'.");
    if( vars.drain.data.size() < 1 )
        throw MHA_Error(__FILE__,__LINE__,"At least one entry is expected in after burner variable 'drain'.");
    if( vars.conflux.data.size() < 1 )
        throw MHA_Error(__FILE__,__LINE__,"At least one entry is expected in after burner variable 'conflux'.");
    if( vars.maxgain.data.size() < 1 )
        throw MHA_Error(__FILE__,__LINE__,"At least one entry is expected in after burner variable 'maxgain'.");
    if( vars.mpo.data.size() < 1 )
        throw MHA_Error(__FILE__,__LINE__,"At least one entry is expected in after burner variable 'mpo'.");
    // first check that all variables have same dimension:
    MHA_assert_equal(vars.f.data.size(),vars.drain.data.size());
    MHA_assert_equal(vars.f.data.size(),vars.conflux.data.size());
    MHA_assert_equal(vars.f.data.size(),vars.maxgain.data.size());
    MHA_assert_equal(vars.f.data.size(),vars.mpo.data.size());
    // now do frequency interpolation (linear interpolation on log scale):
    std::vector<float> fin_log = vars.f.data;
    std::for_each(fin_log.begin(),fin_log.end(),mylogf);
    unsigned int bands = cf.size();
    for( unsigned int k=0;k<bands;k++){
        float cflog = mylogf(cf[k]);
        drain_inv.push_back(powf(10.0f,-0.05*DynComp::interp1(fin_log,vars.drain.data,cflog)));
        conflux.push_back(powf(10.0f,0.05*DynComp::interp1(fin_log,vars.conflux.data,cflog)));
        maxgain.push_back(powf(10.0f,0.05*DynComp::interp1(fin_log,vars.maxgain.data,cflog)));
        mpo_inv.push_back(powf(10.0f,-0.05*DynComp::interp1(fin_log,vars.mpo.data,cflog))/2e-5f);
    }
    for( unsigned int k=0;k<channels;k++)
        lp.push_back(MHAFilter::o1flt_lowpass_t(std::vector<float>(bands,vars.taugain.data),srate));
}

void DynComp::dc_afterburn_t::update()
{
    if( fb_pars_configured ){
        push_config(new dc_afterburn_rt_t(_cf, _channels, _srate, *this));
        commit_pending = false;
    }else{
        commit_pending = true;
    }
}

void DynComp::dc_afterburn_t::set_fb_pars(const std::vector<float>& cf, unsigned int channels, float srate)
{
    _cf = cf;
    _channels = channels;
    _srate = srate;
    fb_pars_configured = true;
    update();
}

void DynComp::dc_afterburn_t::unset_fb_pars()
{
    fb_pars_configured = false;
}

/*
 * Local Variables:
 * mode: c++
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * compile-command: "make -C .."
 * coding: utf-8-unix
 * End:
 */
