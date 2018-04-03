// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2009 2014 2017 HörTech gGmbH
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

#include "mha_plugin.hh"
#include "mha_signal.hh"
#include "mha_defs.h"
#include <math.h>

/** Namespace for displaying ac variables as parser monitors */
namespace acmon {
    /// A class for converting AC variables to Parser monitors of correct type
    class ac_monitor_t {
    public:
        /** Converts AC variable to parser monitor
         * \param parent The parser to insert a monitor into
         * \param name_ The name of the AC variable and the monitor variable
         * \param ac Handle to algorithm communication space
         * \param use_matrix Indicates if a matrix monitor type should be used.
         */
        ac_monitor_t(MHAParser::parser_t & parent,
                     const std::string & name_,
                     algo_comm_t ac,
                     bool use_matrix);

        /** Update values of monitor
         * \param ac Handle to algorithm communication space */
        void getvar(algo_comm_t ac);

        std::string name; /**< name of AC variable and parser monitor */
        std::string dimstr; /**< columns x rows */
        MHAParser::vfloat_mon_t mon; /**< Monitor used for real vectors */
        MHAParser::mfloat_mon_t mon_mat; /**< Monitor used for real matrices */
        MHAParser::vcomplex_mon_t mon_complex; /**< monitor used for complex vectors */
        MHAParser::mcomplex_mon_t mon_mat_complex; /**< monitor used for complex matrices */
        MHAParser::parser_t& p_parser; /**< parent parser to insert monitor into */
    private:
        bool use_mat; /**< if true, use matrix monitor, else use vector monitor */
    };
}

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
