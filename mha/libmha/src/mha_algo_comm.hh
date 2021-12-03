// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2005 2013 2016 2017 2018 2019 2020 HörTech gGmbH
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

#ifndef MHA_ALGO_COMM_HH
#define MHA_ALGO_COMM_HH

#include "mha_algo_comm.h"
#include "mha_toolbox.h"
#include <map>

#define ALGO_COMM_ID_STR "MFVK3jL5rmeus1XtggEI971aXCR/GU7RRehKz4kQtrg="

extern algo_comm_t algo_comm_default;

namespace MHAKernel {

    /** Storage class for the AC variable space.  Uses an std::map for
     * associating AC variable names with AC variable metadata (\c comm_var_t).
     * Acts as a delegator for the std::map storage.  Allows operations that
     * may require memory allocations/deallocations only when
     * is_prepared == false. */
    class comm_var_map_t {
        /** The std::map used for organizing the AC space */
        std::map<std::string, comm_var_t> map;

        /// A string containing the names of all AC variables, space-separated.
        std::string entries;

        /* In order to avoid complicated size types, assert that the map's
         * size_type is the same as size_t. */
        static_assert(std::is_same<std::map<std::string,comm_var_t>::size_type,
                                   size_t>::value);

        /** Update the string entries because a variable has been inserted or
         * removed.  Only permitted if is_prepared == false. */
        void update_entries();
    public:
        /** is_prepared stores whether the provider of the AC space has entered
         * MHA state "prepared" or not.  Operations on \c map that require
         * memory allocations or deallocations are only allowed when not
         * prepared.  Needs to be set by the containing \n algo_comm_class_t AC
         * space instance. */
        bool is_prepared = {false};

        /** Query the map if some AC variable name is present in the AC space.
         * @param name Name of AC variable to check.
         * @return true if the variable is present in the AC space.
         * @return false if no variable with this name exists in the AC space.
         */
        bool has_key(const char * name) const
        {return map.find(name) != map.end();}

        /** Create or replace variable.  Creating is only permitted if
         * is_prepared == false.
         * @param name Name of the AC variable to create or update.
         * @param var  Metadata of the AC variable.
         * @throw MHA_Error if asked to create in prepared state. */
        void insert(const char * name, const comm_var_t & var);

        /** Remove variable. Only permitted if is_prepared == false.
         * @param name Name of the AC variable to remove.
         * @throw MHA_Error if called while prepared. */
        void erase_by_name(const char * name);

        /** Find variables that point to the given address. Erase all that
         * are found. It is not an error if no variable points there.  Only
         * permitted if is_prepared == false. 
         * @ptr Pointer to memory where the variables data is stored.
         * @throw MHA_Error if called while prepared. */
        void erase_by_pointer(void * ptr);

        /** Get the comm_var_t of an existing variable.
         * @param name The name of the AC variable.
         * @throw MHA_Error if no such variable exists in the AC space. */
        const comm_var_t & retrieve(const char * name) const;

        /** @return A string with names of all AC variables in this AC
         *          space, separated by spaces. */
        const std::string & get_entries() const;

        /** @return number of stored AC variables */
        size_t size() const {return map.size();}
    };

    /** AC variable space.  This class is used by AC variable space providers
     * to create and manage their AC space.  AC variable space users, i.e.
     * plugins that provide, read, or alter AC variables, have to use the
     * \c algo_comm_t handle that they receive as a constructor parameter
     * instead of this class. */
    class algo_comm_class_t {
    public:

        /// AC variable space constructor
        algo_comm_class_t();

        /// AC variable space destructor
        virtual
        ~algo_comm_class_t();

        /// Generates the client handle for users for this AC space.
        virtual algo_comm_t get_c_handle();

        /** Trampoline to be referenced by algo_comm_t::insert_var.
         * Redirects to local_insert_var(). */
        static int insert_var(void*,const char*,comm_var_t);

        /** Convenience function for algo_comm_t::insert_var_int.
         * Creates suitable comm_var_t and forwards to local_insert_var(). */
        static int insert_var_int(void*,const char*,int*);

        /** Convenience function for algo_comm_t::insert_var_vfloat.
         * Creates suitable comm_var_t and forwards to local_insert_var(). */
        static int insert_var_vfloat(void* handle,const char* name, std::vector<float>& ivar);

        /** Convenience function for algo_comm_t::insert_var_float.
         * Creates suitable comm_var_t and forwards to local_insert_var(). */
        static int insert_var_float(void*,const char*,float*);

        /** Convenience function for algo_comm_t::insert_var_double.
         * Creates suitable comm_var_t and forwards to local_insert_var(). */
        static int insert_var_double(void*,const char*,double*);

        /** Trampoline to be referenced by algo_comm_t::remove_var.
         * Redirects to local_remove_var(). */
        static int remove_var(void*,const char*);

        /** Trampoline to be referenced by algo_comm_t::remove_ref.
         * Redirects to local_insert_var_int(). */
        static int remove_ref(void*,void*);

        /** Trampoline to be referenced by algo_comm_t::is_var.
         * Redirects to local_is_var(). */
        static int is_var(void*,const char*);

        /** Trampoline to be referenced by algo_comm_t::get_var.
         * Redirects to local_get_var(). */
        static int get_var(void*,const char*,comm_var_t*);

        /** Convenience function accessible through algo_comm_t::get_var_int.
         * Forwards to local_get_var(), checks type and stores result. */
        static int get_var_int(void*,const char*,int*);

        /** Convenience function accessible through algo_comm_t::get_var_float.
         * Forwards to local_get_var(), checks type and stores result. */
        static int get_var_float(void*,const char*,float*);

        /** Convenience function accessible through algo_comm_t::get_var_double
         * Forwards to local_get_var(), checks type and stores result. */
        static int get_var_double(void*,const char*,double*);

        /** Trampoline to be referenced by algo_comm_t::get_entries.
         * Redirects to local_get_entries(). */
        static int get_entries(void*,char*,unsigned int);

        /** Trampoline to be referenced by algo_comm_t::get_error.
         * Redirects to local_get_error(). */
        static const char* get_error(int);
        
        /** Interacts with AC space storage to create or replace an AC variable
         * When the AC space is prepared, only replacing is permitted. */
        virtual
        void local_insert_var(const char*,comm_var_t);

        /** Interacts with AC space storage to remove an AC variable.  Only
         * permitted when AC space is not prepared. */
        virtual
        void local_remove_var(const char*);

        /** Interacts with AC space storage to remove an AC variable. Only
         * permitted when AC space is not prepared. */
        virtual
        void local_remove_ref(void*);

        /** Interacts with AC space storage to check if an AC variable with
         * the given name exists. Always permitted. */
        virtual
        bool local_is_var(const char*) const;

        /** Interacts with AC space storage to retrieve the metadata for an
         * AC variable with the given name. Always permitted. */
        virtual
        void local_get_var(const char*,comm_var_t*) const;

        /** Interacts with AC space storage to retrieve a single string
         * containing the names of all existing AC variables. */
        virtual
        const std::string & local_get_entries() const;

        /** Interacts with AC space storage to return the number of AC
         * variables currently stored in the AC space.  Always permitted. */
        virtual
        size_t size() const;

        /** The provider of this AC space must set the AC space to prepared at
         * the end of its own prepare() operation and to not prepared at the
         * beginning of its own release() operation. */
        virtual void set_prepared(bool prepared);

        char* algo_comm_id_string;
    private:
        algo_comm_t ac;
        int algo_comm_id_string_len;
        comm_var_map_t vars;
    };

    algo_comm_class_t* algo_comm_safe_cast(void*);

}

#endif
/*
 * Local variables:
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * coding: utf-8-unix
 * End:
 */
