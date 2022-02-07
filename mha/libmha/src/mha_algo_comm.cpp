// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2005 2006 2007 2008 2009 2010 2011 2013 2016 HörTech gGmbH
// Copyright © 2017 2018 2019 2020 HörTech gGmbH
// Copyright © 2021 2022 Hörzentrum Oldenburg gGmbH
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

#include "mha_algo_comm.hh"
#include "mha_defs.h"

/** 

\ingroup algocomm
\struct comm_var_t

\brief Algorithm communication variable structure
    
Algorithm communication variables (AC variables) are objects of this
type. The member data is a pointer to the variable `data'. This
pointer has to be valid for the lifetime of this AC variable. The
member `data_type' can be one of the predefined types or any user
defined type. The member `num_entries' describes the number of
elements of this base type stored at the pointer address.
    
    An AC variable can be registered with the \ref
    algo_comm_class_t::insert_var method.
    
*/
/** \var comm_var_t::data_type
    \brief Type of data.
    
    This can be one of the predefined types
    \li MHA_AC_CHAR
    \li MHA_AC_INT
    \li MHA_AC_MHAREAL
    \li MHA_AC_FLOAT
    \li MHA_AC_DOUBLE
    \li MHA_AC_MHACOMPLEX
    \li MHA_AC_VEC_FLOAT
    or any user defined type with a value greater than
    \li MHA_AC_USER
*/

/** \var comm_var_t::num_entries
    \brief Number of entries 
*/
/** \var comm_var_t::stride
    \brief length of one row (C interpretation)
           or of one column (Fortran interpretation) 
*/     
/** \var comm_var_t::data
    \brief Pointer to variable data 
*/


/** \defgroup algocomm Communication between algorithms

Algorithms within one chain can share variables for communication
with other algorithms. This mechanism allows interaction between
algorithms (i.e. separation of noise estimation and noise reduction
algorithms, combination of dynamic compression and noise
estimation). Through a set of simple C functions, algorithms can
propagate variables of any type, even C++ classes, to other
algorithms.

An algorithm communication handle (algo_comm_t) is passed at
initialisation time to the constructor of each plugin 
class \ref MHAPlugin::plugin_t "constructor".  This handle
contains a reference handle, \ref algo_comm_t::handle, and a number of
function pointers, \ref algo_comm_t::insert_var etc.. An algorithm
communication variable is accessed through objects of type \ref comm_var_t.

For \mha users, \mha provides generic plugins to inspect and store
AC variables of numeric types:
 - plugin acmon mirrors AC variables of numeric types in readonly configuration
   variables (called monitors),
 - plugin acsave stores AC variables into Matlab or text files.
Plugin developers may also want to use these plugins to inspect any AC
variables published by their own plugins during testing.

As a developer of \mha plugin(s), please observe the following best practices
in plugins using AC variables:
  -# Plugins publishing AC variables:
     - insert all variables during prepare()
     - re-insert all variables during each process()
     - memory used for storing AC variable values is allocated and owned
       by the publishing plugin and needs to remain valid until the next
       call to process() or release() of the same plugin.
  -# Plugins consuming AC variable published by other plugins:
     - poll required variables (and check validity) again during each
       process() before accessing their values.
*/

void MHAKernel::comm_var_map_t::insert(const std::string & name,
                                       const comm_var_t & var)
{
    // If we are not replacing an entry, then we must be creating a new entry.
    const bool creating_new_entry = not has_key(name);

    // Creating new entry is not permitted when MHA is prepared.
    if (is_prepared && creating_new_entry)
        throw MHA_Error(__FILE__, __LINE__, "Attempt to create AC variable "
                        "\"%s\" "
                        "during live signal processing.  The plugin that "
                        "tried to do this is misbehaving and should be fixed.",
                        name.c_str());

    if (name.empty() || name.find(' ') != std::string::npos)
        throw MHA_Error(__FILE__,__LINE__,"Invalid algorithm communication "
                        "variable name \"%s\", cannot insert into AC space",
                        name.c_str());

    // Create or replace.
    map[name] = var;

    // Update the list of entries if we have just extended it.
    if (creating_new_entry)
        update_entries();
}

void MHAKernel::comm_var_map_t::erase_by_name(const std::string & name)
{
    // Removing AC variables is not permitted while MHA is prepared.
    if (is_prepared)
        throw MHA_Error(__FILE__,__LINE__,"Attempt to remove AC variable "
                        "\"%s\" "
                        "during live signal processing.  The plugin that "
                        "tried to do this is misbehaving and should be fixed.",
                        name.c_str());
    // When not perpared, it is permitted, do it.
    map.erase(name);
    update_entries();
}

void MHAKernel::comm_var_map_t::erase_by_pointer(void * ptr)
{
    std::map<std::string,comm_var_t>::iterator current_iterator, next_iterator;

    // The same pointer may be used by multiple AC variables.  Collect
    // the names of all AC variables here in case we have to throw an exception
    // so that we can give the user some information from which they may be
    // able to deduce which plugin misbehaved.
    std::string erased_variables;

    // This counter is used for both cases, removal permitted or not permitted,
    // but variables will not actually be erased when not permitted, they are
    // only counted.
    size_t num_erased_variables = 0U;

    // The following loop finds all AC variables that point to the same address
    // as ptr.  When variable removal is not permitted, then it collects the
    // names of the AC variables that would be erased, otherwise it performs
    // the erasure but does not collect the names.
    for(current_iterator = map.begin();
        current_iterator != map.end();
        current_iterator = next_iterator) 
    {
        // We may invalidate the current iterator later, better get next now
        next_iterator = current_iterator;
        ++next_iterator;

        if( current_iterator->second.data == ptr ) { // Found a match
            ++num_erased_variables;   // Increase counter.
            if (is_prepared)  // operation forbidden, add info to error message
                erased_variables += current_iterator->first + ", ";
            else              // operation allowed, delete AC variable entry
                map.erase(current_iterator);
        }
    }
    if (is_prepared) { // Operation forbidden while MHA prepared, raise error.
        if (erased_variables.size()) // remove ", " after last entry
            erased_variables.resize(erased_variables.size() - 2U);
        switch (num_erased_variables) {
        case 1: 
            throw MHA_Error(__FILE__,__LINE__,"Attempt to remove AC variable "
                            "%s by address %p during live signal processing.  "
                            "The plugin that tried to do this is misbehaving "
                            "and should be fixed.",
                            erased_variables.c_str(), ptr);
        case 0:
            throw MHA_Error(__FILE__,__LINE__,"Attempt to remove unknown AC "
                            "variable by address %p during live signal process"
                            "ing. The plugin that tried to do this is "
                            "misbehaving and should be fixed.", ptr);
        default:
            throw MHA_Error(__FILE__,__LINE__,"Attempt to remove AC variables "
                            "%s by address %p during live signal processing.  "
                            "The plugin that tried to do this is misbehaving "
                            "and should be fixed.",
                            erased_variables.c_str(), ptr);
        }
    }
    if (num_erased_variables)
        update_entries();
}

const comm_var_t &
MHAKernel::comm_var_map_t::retrieve(const std::string & name) const
{
    if (has_key(name)) 
        return map.at(name);
    else
        throw MHA_Error(__FILE__,__LINE__,
                        "No algorithm communication variable \"%s\".",
                        name.c_str());
}

const std::vector<std::string> & MHAKernel::comm_var_map_t::get_entries() const
{
    return entries;
}

void MHAKernel::comm_var_map_t::update_entries()
{
    if (is_prepared == true) {
        // Should not happen, this is a private method, the caller should make
        // sure that the object is in correct state when this method is called.
        throw MHA_Error(__FILE__, __LINE__, "Internal error: comm_var_map_t::"
                        "update_entries was called while is_prepared == true");
    }
    entries.clear();
    for(const auto & pair : map) { // Loop over all AC space entries.
        entries.push_back(pair.first); // Append name of current AC variable to list.
    }
}

MHAKernel::algo_comm_class_t::algo_comm_class_t()
{
    ac.handle = this;
}

algo_comm_t MHAKernel::algo_comm_class_t::get_c_handle()
{
    return ac;
}

MHAKernel::algo_comm_class_t::~algo_comm_class_t()
{
}

void MHAKernel::algo_comm_class_t::
insert_var(const std::string & name, comm_var_t var)
{
    vars.insert(name, var);
}

void MHAKernel::algo_comm_class_t::
insert_var_int(const std::string & name, int* ptr)
{
    insert_var(name, {MHA_AC_INT, 1, 1, ptr});
}

void MHAKernel::algo_comm_class_t::
insert_var_float(const std::string & name, float* ptr)
{
    insert_var(name, {MHA_AC_FLOAT, 1, 1, ptr});
}

void MHAKernel::algo_comm_class_t::
insert_var_double(const std::string & name, double* ptr)
{
    insert_var(name, {MHA_AC_DOUBLE, 1, 1, ptr});
}

void MHAKernel::algo_comm_class_t::
insert_var_vfloat(const std::string & name,std::vector<float>& vec)
{
    comm_var_t cv{MHA_AC_FLOAT, 1, 1, vec.data()};
    cv.num_entries = vec.size();
    if (cv.num_entries != vec.size()) { // overflow, too many elements
        throw MHA_Error(__FILE__, __LINE__, "Error inserting vector as "
                        "algorithm communication variable \"%s\" into AC "
                        "space: too many elements (%zu), maximum is %u.",
                        name.c_str(), vec.size(), --(cv.num_entries=0));
    }
    insert_var(name, cv);
}

void MHAKernel::algo_comm_class_t::remove_var(const std::string & name)
{
    vars.erase_by_name(name);
}

void MHAKernel::algo_comm_class_t::remove_ref(void* addr)
{
    vars.erase_by_pointer(addr);
}

comm_var_t MHAKernel::algo_comm_class_t::
get_var(const std::string & name) const
{
    return vars.retrieve(name);
}

bool MHAKernel::algo_comm_class_t::is_var(const std::string & name) const
{
    return vars.has_key(name);
}

const std::vector<std::string> & MHAKernel::algo_comm_class_t::
get_entries() const
{
    return vars.get_entries();
}

size_t MHAKernel::algo_comm_class_t::size() const
{
    return vars.size();
}

int MHAKernel::algo_comm_class_t::get_var_int(const std::string & name) const
{
    comm_var_t var = get_var(name);
    if( var.data_type != MHA_AC_INT )
        throw MHA_Error(__FILE__, __LINE__, "algo_comm_class_t::get_var_int: "
                        "AC variable \"%s\" has unexpected data type %u, "
                        "expected MHA_AC_INT (%d).", name.c_str(),
                        var.data_type, MHA_AC_INT);
    if( var.num_entries != 1 )
        throw MHA_Error(__FILE__, __LINE__, "algo_comm_class_t::get_var_int: "
                        "AC variable \"%s\" has unexpected size %u, "
                        "expected 1.", name.c_str(), var.num_entries);
    return *static_cast<int*>(var.data);
}

float MHAKernel::algo_comm_class_t::
get_var_float(const std::string & name) const
{
    comm_var_t var = get_var(name);
    if( var.data_type != MHA_AC_FLOAT )
        throw MHA_Error(__FILE__, __LINE__,"algo_comm_class_t::get_var_float: "
                        "AC variable \"%s\" has unexpected data type %u, "
                        "expected MHA_AC_FLOAT (%d).", name.c_str(),
                        var.data_type, MHA_AC_FLOAT);
    if( var.num_entries != 1 )
        throw MHA_Error(__FILE__, __LINE__,"algo_comm_class_t::get_var_float: "
                        "AC variable \"%s\" has unexpected size %u, "
                        "expected 1.", name.c_str(), var.num_entries);
    return *static_cast<float*>(var.data);
}

double MHAKernel::algo_comm_class_t::
get_var_double(const std::string & name) const
{
    comm_var_t var = get_var(name);
    if( var.data_type != MHA_AC_DOUBLE )
        throw MHA_Error(__FILE__,__LINE__,"algo_comm_class_t::get_var_double: "
                        "AC variable \"%s\" has unexpected data type %u, "
                        "expected MHA_AC_DOUBLE (%d).", name.c_str(),
                        var.data_type, MHA_AC_DOUBLE);
    if( var.num_entries != 1 )
        throw MHA_Error(__FILE__,__LINE__,"algo_comm_class_t::get_var_double: "
                        "AC variable \"%s\" has unexpected size %u, "
                        "expected 1.", name.c_str(), var.num_entries);
    return *static_cast<double*>(var.data);
}

void MHAKernel::algo_comm_class_t::set_prepared(bool prepared)
{
    vars.is_prepared = prepared;
}

mha_spec_t MHA_AC::get_var_spectrum(algo_comm_t ac,const std::string& n)
{
    comm_var_t var = ac.handle->get_var(n);
    if( (var.stride == 0) || (var.num_entries!=0 && var.stride > var.num_entries) )
        throw MHA_Error(__FILE__,__LINE__,
                        "The variable \"%s\" has invalid stride settings (%u).",
                        n.c_str(),var.stride);
    if( var.num_entries == 0 )
        throw MHA_Error(__FILE__,__LINE__,"The variable \"%s\" contains no data.",n.c_str());
    mha_spec_t s;
    memset(&s,0,sizeof(s));
    s.num_frames = var.stride;
    s.num_channels = var.num_entries / var.stride;
    if( s.num_channels * s.num_frames != var.num_entries )
        throw MHA_Error(__FILE__,__LINE__,
                        "The variable \"%s\" has invalid stride settings (%u): Not an integer fraction of entries.",
                        n.c_str(),var.stride);
    if( var.data_type != MHA_AC_MHACOMPLEX )
        throw MHA_Error(__FILE__,__LINE__,"The variable \"%s\" has invalid data type.",n.c_str());
    s.buf = (mha_complex_t*)var.data;
    return s;
}

mha_wave_t MHA_AC::get_var_waveform(algo_comm_t ac,const std::string& n)
{
    comm_var_t var = ac.handle->get_var(n);
    if( (var.stride == 0) || (var.num_entries!=0 && var.stride > var.num_entries) )
        throw MHA_Error(__FILE__,__LINE__,"The variable \"%s\" has invalid stride settings (%u).",n.c_str(),var.stride);
    mha_wave_t s;
    memset(&s,0,sizeof(s));
    s.num_channels = var.stride;
    s.num_frames = var.num_entries / var.stride;
    if( s.num_channels * s.num_frames != var.num_entries )
        throw MHA_Error(__FILE__,__LINE__,
                        "The variable \"%s\" has invalid stride settings (%u): Not an integer fraction of entries.",
                        n.c_str(),var.stride);
    if( var.data_type == MHA_AC_MHAREAL ){
        s.buf = (mha_real_t*)var.data;
        return s;
    }
    if( sizeof(mha_real_t) == sizeof(float) ){
        if( var.data_type == MHA_AC_FLOAT ){
            s.buf = (float*)var.data;
            return s;
        }
    }
    throw MHA_Error(__FILE__,__LINE__,"The variable \"%s\" has invalid data type.",n.c_str());
}

int MHA_AC::get_var_int(algo_comm_t ac,const std::string& n)
{
    return ac.handle->get_var_int(n);
}

std::vector<float> MHA_AC::get_var_vfloat(algo_comm_t ac,const std::string& name)
{
    comm_var_t cv = ac.handle->get_var(name);
    unsigned types[2] = {MHA_AC_FLOAT, MHA_AC_FLOAT};
    if (std::is_same<float,mha_real_t>::value)
        types[0] = MHA_AC_MHAREAL;
    if (cv.data_type != types[0] && cv.data_type != types[1]) {
        throw MHA_Error(__FILE__,__LINE__,
                        "Algorithm communication variable %s has unexpected"
                        " data type %u", name.c_str(), cv.data_type);
    }
    std::vector<float> vfloat;
    vfloat.resize(cv.num_entries);
    std::copy(static_cast<float*>(cv.data), 
              static_cast<float*>(cv.data) + cv.num_entries,
              vfloat.begin());
    return vfloat;
}

float MHA_AC::get_var_float(algo_comm_t ac,const std::string& n)
{
    return ac.handle->get_var_float(n);
}

MHA_AC::spectrum_t::spectrum_t(algo_comm_t iac,
                               const std::string & iname,
                               unsigned int bins,
                               unsigned int channels,
                               bool insert_now)
    : MHASignal::spectrum_t(bins,channels),
      ac(iac),
      name(iname),
      remove_during_destructor(insert_now)
{
    if( insert_now )
        insert();
}

MHA_AC::spectrum_t::~spectrum_t()
{
    if (remove_during_destructor) {
        try {
            remove();
        }
        catch (...) {
            // ignore all exceptions because we are in destructor
        }
    }
}
void MHA_AC::spectrum_t::remove()
{
    ac.handle->remove_ref(buf);
}


MHA_AC::waveform_t::waveform_t(algo_comm_t iac,
                               const std::string & iname,
                               unsigned int frames,
                               unsigned int channels,
                               bool insert_now)
    : MHASignal::waveform_t(frames,channels),
      ac(iac),
      name(iname),
      remove_during_destructor(insert_now)
{
    if( insert_now )
        insert();
}

MHA_AC::waveform_t::~waveform_t()
{
    if (remove_during_destructor) {
        try {
            remove();
        }
        catch (...) {
            // ignore all exceptions because we are in destructor
        }
    }
}
void MHA_AC::waveform_t::remove()
{
    ac.handle->remove_ref(buf);
}

MHA_AC::stat_t::stat_t(algo_comm_t ac,const std::string& name,
                       const unsigned int& frames, const unsigned int& channels,
                       bool insert_now)
    : MHASignal::stat_t(frames,channels),
      mean(ac,name+"_mean",frames,channels,false),
      std(ac,name+"_std",frames,channels,false)
{
    if( insert_now )
        insert();
}

void MHA_AC::stat_t::insert()
{
    mean.insert();
    std.insert();
}

void MHA_AC::stat_t::update()
{
    mean_std(mean,std);
}

void MHA_AC::waveform_t::insert()
{
    comm_var_t var;
    var.data_type = MHA_AC_MHAREAL;
    var.num_entries = num_frames * num_channels;
    var.stride = num_channels;
    var.data = buf;
    ac.handle->insert_var(name,var);
}

void MHA_AC::spectrum_t::insert()
{
    comm_var_t var;
    var.data_type = MHA_AC_MHACOMPLEX;
    var.num_entries = num_frames * num_channels;
    var.stride = num_frames;
    var.data = buf;
    ac.handle->insert_var(name,var);
}

MHA_AC::ac2matrix_helper_t::ac2matrix_helper_t(algo_comm_t iac,const std::string& iname)
    : ac(iac),
      size(2)
{
    MHAParser::expression_t cfgname(iname, ":");
    if(!cfgname.rval.size())
        cfgname.rval = cfgname.lval;
    name = cfgname.lval;
    username = cfgname.rval;
    getvar();
    is_complex = acvar.data_type == MHA_AC_MHACOMPLEX;
    size[0] = acvar.stride;
    size[1] = acvar.num_entries / acvar.stride;
}

void MHA_AC::ac2matrix_helper_t::getvar()
{
    acvar = ac.handle->get_var(name);
    if( acvar.stride == 0 )
        throw MHA_Error(__FILE__,__LINE__,"Stride of AC variable %s is zero.",name.c_str());
    switch( acvar.data_type ){
    case MHA_AC_MHACOMPLEX :
    case MHA_AC_MHAREAL :
    case MHA_AC_FLOAT :
    case MHA_AC_DOUBLE :
    case MHA_AC_INT :
        break;
    default:
        throw MHA_Error(__FILE__,__LINE__,
                        "Unsupported AC data format (%u).",
                        acvar.data_type);
    }
}

MHA_AC::ac2matrix_t::ac2matrix_t(algo_comm_t iac,const std::string& iname)
    : MHA_AC::ac2matrix_helper_t(iac,iname),
      MHASignal::matrix_t(MHA_AC::ac2matrix_helper_t::size,MHA_AC::ac2matrix_helper_t::is_complex)
{
    update();
}

void MHA_AC::ac2matrix_t::update()
{
    getvar();
    MHASignal::matrix_t::operator=(acvar);
}

void MHA_AC::ac2matrix_t::insert(algo_comm_t ac)
{
    ac.handle->insert_var(getname(),get_comm_var());
}

MHA_AC::acspace2matrix_t::acspace2matrix_t(algo_comm_t iac,const std::vector<std::string>& names)
    : len(names.size()),data(NULL),frameno(0)
{
    std::vector<std::string> entries(names);
    if( len == 0 ){
        entries = iac.handle->get_entries();
        len = entries.size();
    }
    data = new MHA_AC::ac2matrix_t*[mha_min_1(len)];
    for(unsigned int k=0;k<len;k++)
        data[k] = new MHA_AC::ac2matrix_t(iac,entries[k]);
}

MHA_AC::acspace2matrix_t::acspace2matrix_t(const MHA_AC::acspace2matrix_t& src)
    : len(src.len),data(NULL),frameno(0)
{
    data = new MHA_AC::ac2matrix_t*[mha_min_1(len)];
    for(unsigned int k=0;k<len;k++)
        data[k] = new MHA_AC::ac2matrix_t(src[k]);
}

MHA_AC::acspace2matrix_t::~acspace2matrix_t()
{
    for(unsigned int k=0;k<len;k++)
        delete data[k];
    delete [] data;
}

MHA_AC::acspace2matrix_t& MHA_AC::acspace2matrix_t::operator=(const MHA_AC::acspace2matrix_t& src)
{
    if( src.len != len )
        throw MHA_Error(__FILE__,__LINE__,
                        "left value has %u entries, right value has %u.",
                        len,src.len);
    for(unsigned int k=0;k<len;k++)
        *data[k] = *(src.data[k]);
    return *this;
}

void MHA_AC::acspace2matrix_t::insert(algo_comm_t oac)
{
    for(unsigned int k=0;k<size();k++)
        (*this)[k].insert(oac);
}

// Local Variables:
// compile-command: "make -C .."
// c-basic-offset: 4
// coding: utf-8-unix
// indent-tabs-mode: nil
// End:
