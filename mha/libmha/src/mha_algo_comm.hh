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

    class comm_var_map_t : public std::map<std::string, comm_var_t> {
    public:
        bool has_key(const std::string & name) {return find(name) != end();}
    };
      

    class algo_comm_class_t {
    public:
        algo_comm_class_t();
        virtual
        ~algo_comm_class_t();
        algo_comm_t get_c_handle();
        static int insert_var(void*,const char*,comm_var_t);
        static int insert_var_int(void*,const char*,int*);
        static int insert_var_vfloat(void* handle,const char* name, std::vector<float>& ivar);
        static int insert_var_float(void*,const char*,float*);
        static int remove_var(void*,const char*);
        static int remove_ref(void*,void*);
        static int is_var(void*,const char*);
        static int get_var(void*,const char*,comm_var_t*);
        static int get_var_int(void*,const char*,int*);
        static int get_var_float(void*,const char*,float*);
        static int get_entries(void*,char*,unsigned int);
        static const char* get_error(int);
        virtual
        void local_insert_var(const char*,comm_var_t);
        virtual
        void local_remove_var(const char*);
        virtual
        void local_remove_ref(void*);
        virtual
        bool local_is_var(const char*);
        virtual
        void local_get_var(const char*,comm_var_t*);
        virtual
        std::string local_get_entries();
        virtual
        comm_var_map_t::size_type size() const;
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
