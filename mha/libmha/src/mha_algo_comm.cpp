// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2005 2006 2007 2008 2009 2010 2011 2013 2016 HörTech gGmbH
// Copyright © 2017 2018 2019 2020 HörTech gGmbH
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
\struct algo_comm_t
\brief A reference handle for algorithm communication variables.

This structure contains a coontrol handle and a set of function
pointers for sharing variables within one processing chain. See also
section \ref algocomm.

*/

/** \var algo_comm_t::handle
    \brief AC variable control handle
*/
/** \var algo_comm_t::insert_var
    \brief Register an AC variable.
    
    This function can register a variable to be shared within one
    chain. If a variable of this name exists it will be
    overwritten.
    
    \param h    AC handle
    \param n    name of variable.
                May not be empty. Must not contain space character.
                The name is copied, therefore it is allowed that the
                char array pointed to gets invalid after return.
    \param v    variable handle of type \ref comm_var_t
    \return Error code or zero on success
*/

/** \var algo_comm_t::insert_var_int
    \brief Register an int as an AC variable.
    
    This function can register an int variable to be shared with other
    algorithms. It behaves similar to ac.insert_var.
    
    \param h    AC handle
    \param n    name of variable
    \param v    pointer on the variable
    \return Error code or zero on success
*/
/** \var algo_comm_t::insert_var_float
    \brief Register a float as an AC variable.
    
    This function can register a float variable to be shared with
    other algorithms. It behaves similar to ac.insert_var.
    
    \param h    AC handle
    \param n    name of variable
    \param v    pointer on the variable
    \return Error code or zero on success
*/
/** \var algo_comm_t::remove_var
    \brief Remove an AC variable

    Remove (unregister) an AC variable. After calling this
    function, the variable is not available to ac.is_var or
    ac.get_var. The data pointer is not affected.
    
    \param h    AC handle
    \param n    name of variable to be removed
    \return Error code or zero on success
*/
/** \var algo_comm_t::remove_ref
    \brief Remove all AC variable which refer to address
    
    This function removes all AC variables whos data field points
    to the given address.
    
    \param h    AC handle
    \param p    address which should not be referred to any more
    \return Error code or zero on success
*/
/** \var algo_comm_t::is_var
    \brief Test if an AC variable exists
    
    This function tests if an AC variable of a given name
    exists. Use ac.get_var to get information about the
    variables type and dimension.
    
    \param h    AC handle
    \param n    name of variable
    \return 1 if the variable exists, 0 otherwise
*/
/** \var algo_comm_t::get_var
    \brief Get the variable handle of an AC variable
  
    This function returns the variable handle \ref comm_var_t of a
    variable of the given name. If no variable of that name
    exists, an error code is returned.
    
    \param h    AC handle
    \param n    name of variable
    \param v    pointer to a AC variable object
    \return Error code or zero on success
*/
/** \var algo_comm_t::get_var_int
    \brief Get the value of an int AC variable
            
    This function returns the value of an int AC variable of the given
    name. If no variable exists, the variable type is mismatching or
    more than one entry is registered, a corresponding error code is
    returned. This is a special version of ac.get_var.
    
    \param h AC handle   
    \param n    name of variable
    \param v    pointer on an int variable to store the result
    \return Error code or zero on success
*/
/** \var algo_comm_t::get_var_float
    \brief Get the value of a float AC variable
            
    This function returns the value of a float AC variable of the
    given name. If no variable exists, the variable type is
    mismatching or more than one entry is registered, a corresponding
    error code is returned. This is a special version of ac.get_var.
    
    \param h AC handle   
    \param n    name of variable
    \param v    pointer on a float variable to store the result
    \return Error code or zero on success
*/
/** \var algo_comm_t::get_entries
    \brief Return a space separated list of all variable names
            
    This function returns the names of all registered variables,
    separated by a single space.
    
    \param h    AC handle
    \retval ret Character buffer for return value
    \param len length of character buffer
    \return Error code or zero on success.
             -1: invalid ac handle.
             -3: not enough room in character buffer to store all variable
                 names.
*/
/** \var algo_comm_t::get_error
    \brief Convert AC error codes into human readable error messages
    
    \param e    Error code
    \return Error message
*/

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
    algo_comm_t::insert_var "insert_var" function.
    
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

#define AC_SUCCESS 0
#define AC_INVALID_HANDLE -1
#define AC_INVALID_NAME -2
#define AC_STRING_TRUNCATED -3
#define AC_INVALID_OUTPTR -4
#define AC_TYPE_MISMATCH -5
#define AC_DIM_MISMATCH -6

const char* MHAKernel::algo_comm_class_t::get_error(int e)
{
    switch( e ){
    case AC_SUCCESS:
        return "Success";
    case AC_INVALID_HANDLE:
        return "Invalid handle";
    case AC_INVALID_NAME:
        return "Invalid or non-existing variable name";
    case AC_STRING_TRUNCATED:
        return "string truncated";
    case AC_INVALID_OUTPTR:
        return "Invalid output pointer";
    case AC_TYPE_MISMATCH:
        return "The variable has unexpected type";
    case AC_DIM_MISMATCH:
        return "The variable has unexpected dimension";
    default:
        return "Unknwon error";
    }
}

algo_comm_t algo_comm_default = {
    NULL,
    MHAKernel::algo_comm_class_t::insert_var,
    MHAKernel::algo_comm_class_t::insert_var_int,
    MHAKernel::algo_comm_class_t::insert_var_float,
    MHAKernel::algo_comm_class_t::remove_var,
    MHAKernel::algo_comm_class_t::remove_ref,
    MHAKernel::algo_comm_class_t::is_var,
    MHAKernel::algo_comm_class_t::get_var,
    MHAKernel::algo_comm_class_t::get_var_int,
    MHAKernel::algo_comm_class_t::get_var_float,
    MHAKernel::algo_comm_class_t::get_entries,
    MHAKernel::algo_comm_class_t::get_error
};

MHAKernel::algo_comm_class_t* MHAKernel::algo_comm_safe_cast(void* h)
{
    if( !h )
        return NULL;
    algo_comm_class_t* p = (algo_comm_class_t*)h;
    if( strncmp(p->algo_comm_id_string,ALGO_COMM_ID_STR,strlen(ALGO_COMM_ID_STR)) != 0 )
        return NULL;
    return p;
}

MHAKernel::algo_comm_class_t::algo_comm_class_t()
    : algo_comm_id_string(NULL),
      algo_comm_id_string_len(strlen(ALGO_COMM_ID_STR)+1)
{
    algo_comm_id_string = new char[algo_comm_id_string_len];
    memset(algo_comm_id_string,0,algo_comm_id_string_len);
    strncpy(algo_comm_id_string,ALGO_COMM_ID_STR,algo_comm_id_string_len-1);
    ac = algo_comm_default;
    ac.handle = this;
    vars.clear();
}

algo_comm_t MHAKernel::algo_comm_class_t::get_c_handle()
{
    return ac;
}

MHAKernel::algo_comm_class_t::~algo_comm_class_t()
{
    memset(algo_comm_id_string,0,algo_comm_id_string_len);
    delete [] algo_comm_id_string;
}

void MHAKernel::algo_comm_class_t::local_insert_var(const char* name,comm_var_t var)
{
    if (name == 0)
        throw MHA_ErrorMsg("NULL is not a valid name for an algo_comm variable");
    if (strlen(name) == 0)
        throw MHA_ErrorMsg("Empty string is not a valid name " \
                         "for an algo_comm variable");
    if (strchr(name, ' ') != 0)
        throw MHA_ErrorMsg("algo_comm variable name " \
                         "may not contain space character");
    vars[name] = var;
}

void MHAKernel::algo_comm_class_t::local_remove_var(const char* name)
{
    if (name == 0) {
        throw MHA_ErrorMsg("String pointer for variable name must not be NULL");
    }
    if (vars.has_key(name))
        vars.erase(name);
    else
        throw MHA_Error(__FILE__,__LINE__,
                        "A variable of name \"%s\" was not found.",name);
}

void MHAKernel::algo_comm_class_t::local_remove_ref(void* addr)
{
    comm_var_map_t::iterator current_iterator, next_iterator;
    for(current_iterator=vars.begin();
        current_iterator != vars.end();
        current_iterator = next_iterator){
        next_iterator = current_iterator;
        ++next_iterator;
        if( current_iterator->second.data == addr ){
            vars.erase(current_iterator);
        }
    }
}

void MHAKernel::algo_comm_class_t::local_get_var(const char* name,comm_var_t* var)
{
    if( !var ){
        throw MHA_ErrorMsg("Invalid variable pointer.");
    }
    if (name == 0) {
        throw MHA_ErrorMsg("String pointer for variable name must not be NULL");
    }
    if (vars.has_key(name)) 
        *var = vars[name];
    else
        throw MHA_Error(__FILE__,__LINE__,
                        "No algorithm communication variable \"%s\".",
                        name);
}

bool MHAKernel::algo_comm_class_t::local_is_var(const char* name)
{
    if (name == 0) return false;
    return vars.has_key(name);
}

std::string MHAKernel::algo_comm_class_t::local_get_entries()
{
    std::string r("");
    comm_var_map_t::iterator it;
    for(it=vars.begin();it != vars.end();++it){
        r += it->first + " ";
    }
    if( r.rfind(" ") < r.size() )
        r.erase(r.rfind(" "),1);
    return r;
}

MHAKernel::comm_var_map_t::size_type MHAKernel::algo_comm_class_t::size() const
{
    return vars.size();
}

/*
 *
 * Here begins the static function implementation:
 *
 */

int MHAKernel::algo_comm_class_t::insert_var(void* handle,const char* name,comm_var_t var)
{
    try{
        algo_comm_class_t* p = algo_comm_safe_cast(handle);
        if(!p) 
            return AC_INVALID_HANDLE;
        p->local_insert_var(name,var);
        return AC_SUCCESS;
    }
    catch(MHA_Error&e){
        (void)e;
        return AC_INVALID_NAME;
    }
}

int MHAKernel::algo_comm_class_t::insert_var_int(void* handle,const char* name,int* ivar)
{
    try{
        algo_comm_class_t* p = algo_comm_safe_cast(handle);
        if(!p) 
            return AC_INVALID_HANDLE;
        comm_var_t var;
        var.data_type = MHA_AC_INT;
        var.num_entries = 1;
        var.stride = 1;
        var.data = ivar;
        p->local_insert_var(name,var);
        return AC_SUCCESS;
    }
    catch(MHA_Error&e){
        (void)e;
        return AC_INVALID_NAME;
    }
}

int MHAKernel::algo_comm_class_t::insert_var_float(void* handle,const char* name,float* ivar)
{
    try{
        algo_comm_class_t* p = algo_comm_safe_cast(handle);
        if(!p) 
            return AC_INVALID_HANDLE;
        comm_var_t var;
        var.data_type = MHA_AC_FLOAT;
        var.num_entries = 1;
        var.stride = 1;
        var.data = ivar;
        p->local_insert_var(name,var);
        return AC_SUCCESS;
    }
    catch(MHA_Error& e){
        (void)e;
        return AC_INVALID_NAME;
    }
}

int MHAKernel::algo_comm_class_t::insert_var_vfloat(void* handle,const char* name,std::vector<float>& ivar)
{
    try{
        algo_comm_class_t* p = algo_comm_safe_cast(handle);
        if(!p)
            return AC_INVALID_HANDLE;
        comm_var_t var;
        var.data_type = MHA_AC_FLOAT;
        var.num_entries = ivar.size();
        var.stride = 1;
        var.data = static_cast<void*>(const_cast<float*>(ivar.data()));
        p->local_insert_var(name,var);
        return AC_SUCCESS;
    }
    catch(MHA_Error&e){
        (void)e;
        return AC_INVALID_NAME;
    }
}

int MHAKernel::algo_comm_class_t::remove_var(void* handle,const char* name)
{
    try{
        algo_comm_class_t* p = algo_comm_safe_cast(handle);
        if(!p) 
            return AC_INVALID_HANDLE;
        p->local_remove_var(name);
        return AC_SUCCESS;
    }
    catch(MHA_Error&e){
        (void)e;
        return AC_INVALID_NAME;
    }
}

int MHAKernel::algo_comm_class_t::remove_ref(void* handle,void* ref)
{
    try{
        algo_comm_class_t* p = algo_comm_safe_cast(handle);
        if(!p) 
            return AC_INVALID_HANDLE;
        p->local_remove_ref(ref);
        return AC_SUCCESS;
    }
    catch(MHA_Error&e){
        (void)e;
        return AC_INVALID_NAME;
    }
}

int MHAKernel::algo_comm_class_t::get_var(void* handle,const char* name,comm_var_t* var)
{
    try{
        algo_comm_class_t* p = algo_comm_safe_cast(handle);
        if (!p)
            return AC_INVALID_HANDLE;
        if (!p->local_is_var(name))
            return AC_INVALID_NAME;
        p->local_get_var(name,var);
        return AC_SUCCESS;
    }
    catch(MHA_Error&e){
        (void)e;
        return AC_INVALID_NAME;
    }
}

int MHAKernel::algo_comm_class_t::get_var_int(void* handle,const char* name,int* ivar)
{
    try{
        algo_comm_class_t* p = algo_comm_safe_cast(handle);
        if (!p)
            return AC_INVALID_HANDLE;
        if (!p->local_is_var(name))
            return AC_INVALID_NAME;
        comm_var_t var;
        if( !ivar ){
            return AC_INVALID_OUTPTR;
        }
        p->local_get_var(name,&var);
        if( var.data_type != MHA_AC_INT )
            return AC_TYPE_MISMATCH;
        if( var.num_entries != 1 )
            return AC_DIM_MISMATCH;
        *ivar = *((int*)(var.data));
        return AC_SUCCESS;
    }
    catch(MHA_Error&e){
        (void)e;
        return AC_INVALID_NAME;
    }
}

int MHAKernel::algo_comm_class_t::get_var_float(void* handle,const char* name,float* ivar)
{
    try{
        algo_comm_class_t* p = algo_comm_safe_cast(handle);
        if (!p)
            return AC_INVALID_HANDLE;
        if (!p->local_is_var(name))
            return AC_INVALID_NAME;
        comm_var_t var;
        if( !ivar )
            return AC_INVALID_OUTPTR;
        p->local_get_var(name,&var);
        if( var.data_type != MHA_AC_FLOAT )
            return AC_TYPE_MISMATCH;
        if( var.num_entries != 1 )
            return AC_DIM_MISMATCH;
        *ivar = *((float*)(var.data));
        return AC_SUCCESS;
    }
    catch(MHA_Error&e){
        (void)e;
        return AC_INVALID_NAME;
    }
}


int MHAKernel::algo_comm_class_t::get_entries(void* handle,char* ret,unsigned int len)
{
    algo_comm_class_t* p = algo_comm_safe_cast(handle);
    if(!p) 
        return AC_INVALID_HANDLE;
    memset(ret,0,len);
    std::string sres = p->local_get_entries();
    strncpy(ret,sres.c_str(),len-1);
    if( sres.size() >= len )
        return AC_STRING_TRUNCATED;
    return AC_SUCCESS;
}

int MHAKernel::algo_comm_class_t::is_var(void* handle,const char* name)
{
    algo_comm_class_t* p = algo_comm_safe_cast(handle);
    if(!p)
        return 0;
    return p->local_is_var(name);
}

mha_spec_t MHA_AC::get_var_spectrum(algo_comm_t ac,const std::string& n)
{
    comm_var_t var;
    int err = ac.get_var(ac.handle,n.c_str(),&var);
    if( err )
        throw MHA_Error(__FILE__,__LINE__,"AC error (%s): %s",n.c_str(),ac.get_error(err));
    if( (var.stride == 0) || (var.stride > var.num_entries) )
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
    comm_var_t var;
    int err = ac.get_var(ac.handle,n.c_str(),&var);
    if( err )
        throw MHA_Error(__FILE__,__LINE__,"AC error (%s): %s",n.c_str(),ac.get_error(err));
    if( (var.stride == 0) || (var.stride > var.num_entries) )
        throw MHA_Error(__FILE__,__LINE__,"The variable \"%s\" has invalid stride settings (%u).",n.c_str(),var.stride);
    if( var.num_entries == 0 )
        throw MHA_Error(__FILE__,__LINE__,"The variable \"%s\" contains no data.",n.c_str());
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
    comm_var_t var;
    int err = ac.get_var(ac.handle,n.c_str(),&var);
    if( err )
        throw MHA_Error(__FILE__,__LINE__,"AC error (%s): %s",n.c_str(),ac.get_error(err));
    if( var.num_entries != 1 )
        throw MHA_Error(__FILE__,__LINE__,"The variable \"%s\" contains not exactly one entry.",n.c_str());
    if( var.data_type != MHA_AC_INT )
        throw MHA_Error(__FILE__,__LINE__,"The variable \"%s\" has invalid data type.",n.c_str());
    return *((int*)var.data);
}

std::vector<float> MHA_AC::get_var_vfloat(algo_comm_t ac,const std::string& name)
{
    comm_var_t cv;
    int err = ac.get_var(ac.handle,name.c_str(),&cv);
    if( err )
        throw MHA_Error(__FILE__,__LINE__,"AC error (%s): %s",name.c_str(),ac.get_error(err));
    if (cv.data_type != MHA_AC_MHAREAL) {
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
    float ret;
    comm_var_t var;
    int err = ac.get_var(ac.handle,n.c_str(),&var);
    if( err )
        throw MHA_Error(__FILE__,__LINE__,"AC error (%s): %s",n.c_str(),ac.get_error(err));
    if( var.num_entries != 1 )
        throw MHA_Error(__FILE__,__LINE__,"The variable \"%s\" contains not exactly one entry.",n.c_str());
    switch( var.data_type ){
    case MHA_AC_INT :
        ret = *((int*)var.data);
        break;
    case MHA_AC_FLOAT :
        ret = *((float*)var.data);
        break;
    case MHA_AC_DOUBLE :
        ret = *((double*)var.data);
        break;
    case MHA_AC_MHAREAL :
        ret = *((mha_real_t*)var.data);
        break;
    default:
        throw MHA_Error(__FILE__,__LINE__,"The variable \"%s\" has invalid data type.",n.c_str());
    }
    return ret;
}

MHA_AC::spectrum_t::spectrum_t(algo_comm_t iac,
                               std::string iname,
                               unsigned int frames,
                               unsigned int channels,
                               bool insert_now)
    : MHASignal::spectrum_t(frames,channels),
      ac(iac),
      name(iname)
{
    if( insert_now )
        insert();
}

MHA_AC::spectrum_t::~spectrum_t()
{
    ac.remove_ref(ac.handle,buf);
}


MHA_AC::waveform_t::waveform_t(algo_comm_t iac,
                               std::string iname,
                               unsigned int frames,
                               unsigned int channels,
                               bool insert_now)
    : MHASignal::waveform_t(frames,channels),
      ac(iac),
      name(iname)
{
    if( insert_now )
        insert();
}

MHA_AC::waveform_t::~waveform_t()
{
    ac.remove_ref(ac.handle,buf);
}

MHA_AC::int_t::int_t(algo_comm_t iac,std::string n,int v)
    : data(v),
      ac(iac)
{
    if( int err = ac.insert_var_int(ac.handle,n.c_str(),&data) )
        throw MHA_Error(__FILE__,__LINE__,
                        "Not able to insert AC variable 'int %s':\n%s",
                        n.c_str(),ac.get_error(err));
}

MHA_AC::int_t::~int_t()
{
    ac.remove_ref(ac.handle,&data);
}

MHA_AC::float_t::float_t(algo_comm_t iac,std::string n,float v)
    : data(v),
      ac(iac)
{
    if( int err = ac.insert_var_float(ac.handle,n.c_str(),&data) )
        throw MHA_Error(__FILE__,__LINE__,
                        "Not able to insert AC variable 'float %s':\n%s",
                        n.c_str(),ac.get_error(err));
}

MHA_AC::float_t::~float_t()
{
    ac.remove_ref(ac.handle,&data);
}

MHA_AC::double_t::double_t(algo_comm_t iac,std::string n,double v)
    : data(v),
      ac(iac)
{
    comm_var_t acv;
    memset(&acv,0,sizeof(acv));
    acv.data_type = MHA_AC_DOUBLE;
    acv.num_entries = 1;
    acv.stride = 0;
    acv.data = &data;
    int err = ac.insert_var(ac.handle, n.c_str(), acv);
    if( err )
        throw MHA_Error(__FILE__,__LINE__,
                        "Not able to insert AC variable 'double %s':\n%s",
                        n.c_str(),ac.get_error(err));
}

MHA_AC::double_t::~double_t()
{
    ac.remove_ref(ac.handle,&data);
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
    if( name.size() ){
        comm_var_t var;
        var.data_type = MHA_AC_MHAREAL;
        var.num_entries = num_frames * num_channels;
        var.stride = num_channels;
        var.data = buf;
        int err;
        if( (err = ac.insert_var(ac.handle,name.c_str(),var)) )
            throw MHA_Error(__FILE__,__LINE__,
                            "Not able to insert AC variable 'waveform %s':\n%s",
                            name.c_str(),ac.get_error(err));
    }
}

void MHA_AC::spectrum_t::insert()
{
    if( name.size() ){
        comm_var_t var;
        var.data_type = MHA_AC_MHACOMPLEX;
        var.num_entries = num_frames * num_channels;
        var.stride = num_frames;
        var.data = buf;
        int err;
        if( (err = ac.insert_var(ac.handle,name.c_str(),var)) )
            throw MHA_Error(__FILE__,__LINE__,
                            "Not able to insert AC variable 'spectrum %s':\n%s",
                            name.c_str(),ac.get_error(err));
    }
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
    int err = ac.get_var(ac.handle,name.c_str(),&acvar);
    if( err )
        throw MHA_Error(__FILE__,__LINE__,"AC error (%s): %s",name.c_str(),ac.get_error(err));
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
    ac.insert_var(ac.handle,getname().c_str(),get_comm_var());
}

MHA_AC::acspace2matrix_t::acspace2matrix_t(algo_comm_t iac,const std::vector<std::string>& names)
    : len(names.size()),data(NULL),frameno(0)
{
    std::vector<std::string> entries(names);
    if( len == 0 ){
        unsigned int cstr_len = 8192;
        char* temp_cstr = new char[cstr_len];
        iac.get_entries(iac.handle,temp_cstr,cstr_len);
        std::string entr = temp_cstr;
        delete [] temp_cstr;
        entr = std::string("[") + entr + std::string("]");
        MHAParser::StrCnv::str2val(entr,entries);
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
