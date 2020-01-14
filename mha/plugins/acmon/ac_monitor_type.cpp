// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2009 2013 2014 2017 2018 2020 HörTech gGmbH
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

#include "ac_monitor_type.hh"

acmon::ac_monitor_t::ac_monitor_t(MHAParser::parser_t & parent,
                                  const std::string & name_,
                                  algo_comm_t ac,bool use_mat_)
    : name(name_),
      mon(""),
      mon_mat(""),
      mon_complex(""),
      mon_mat_complex(""),
      p_parser(parent),
      use_mat(use_mat_)
{
    comm_var_t v;
    if( ac.get_var(ac.handle,name.c_str(),&v) ){
        throw MHA_Error(__FILE__,__LINE__,
                        "No such variable: \"%s\"",name.c_str());
    }
    switch( v.data_type ){
        case MHA_AC_INT :
        case MHA_AC_FLOAT :
        case MHA_AC_DOUBLE :
        case MHA_AC_MHAREAL :
            if( use_mat )
                p_parser.insert_item(name,&mon_mat);
            else
                p_parser.insert_item(name,&mon);
            break;
        case MHA_AC_MHACOMPLEX :
            if( use_mat )
                p_parser.insert_item(name,&mon_mat_complex);
            else
                p_parser.insert_item(name,&mon_complex);
            break;
        default:
            break;
    }
    int rows(0);
    int cols(0);
    if( v.stride == 0 )
    v.stride = 1;
    if( (v.num_entries/v.stride)*v.stride != v.num_entries )
        v.stride = 1;
    cols = v.num_entries/v.stride;
    rows = v.stride;
    dimstr = MHAParser::StrCnv::val2str(cols)+"x"+MHAParser::StrCnv::val2str(rows);
}

void acmon::ac_monitor_t::getvar(algo_comm_t ac)
{
    unsigned int ndim, k, stride;
    comm_var_t v;
    if( ac.get_var(ac.handle,name.c_str(),&v) ){
        throw MHA_Error(__FILE__,__LINE__,
                        "No such variable: \"%s\"",name.c_str());
    }
    switch( v.data_type ){
        case MHA_AC_INT :
        case MHA_AC_FLOAT :
        case MHA_AC_DOUBLE :
        case MHA_AC_MHAREAL :
        case MHA_AC_MHACOMPLEX :
            ndim = v.num_entries;
            break;
        default:
            ndim = 0;
            break;
    }
    stride = v.stride;
    if( use_mat ){
        if( (stride == 0) || (stride > ndim) )
            throw MHA_Error(__FILE__,__LINE__,
                            "The variable \"%s\" has invalid stride settings (%u).",
                            name.c_str(),stride);
        mon_mat.data.resize(ndim/stride);
        mon_mat_complex.data.resize(ndim/stride);
        for(k=0;k<ndim/stride;k++){
            mon_mat.data[k].resize(stride);
            mon_mat_complex.data[k].resize(stride);
        }
    }
    mon.data.resize(ndim);
    mon_complex.data.resize(ndim);
    if( ndim == 0 )
        return;
    switch( v.data_type ){
        case MHA_AC_INT :
            for( k=0;k<ndim;k++)
                if( use_mat )
                    mon_mat.data[k/stride][k % stride] = ((int*)v.data)[k];
                else
                    mon.data[k] = ((int*)v.data)[k];
            break;
        case MHA_AC_FLOAT :
            for( k=0;k<ndim;k++)
                if( use_mat )
                    mon_mat.data[k/stride][k % stride] = ((float*)v.data)[k];
                else
                    mon.data[k] = ((float*)v.data)[k];
            break;
        case MHA_AC_DOUBLE :
            for( k=0;k<ndim;k++)
                if( use_mat )
                    mon_mat.data[k/stride][k % stride] = ((double*)v.data)[k];
                else
                    mon.data[k] = ((double*)v.data)[k];
            break;
        case MHA_AC_MHAREAL :
            for( k=0;k<ndim;k++)
                if( use_mat )
                    mon_mat.data[k/stride][k % stride] = ((mha_real_t*)v.data)[k];
                else
                    mon.data[k] = ((mha_real_t*)v.data)[k];
            break;
        case MHA_AC_MHACOMPLEX :
            for( k=0;k<ndim;k++)
                if( use_mat )
                    mon_mat_complex.data[k/stride][k % stride] = ((mha_complex_t*)v.data)[k];
                else
                    mon_complex.data[k] = ((mha_complex_t*)v.data)[k];
            break;
    }
}

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
