// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2009 2012 2013 2016 2017 HörTech gGmbH
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

#ifndef AUDITORY_PROFILE_H
#define AUDITORY_PROFILE_H

#include "mha_parser.hh"
#include "mha_events.h"
#include "mha_error.hh"

/**
   \brief Namespace for classes and functions around the auditory profile (e.g., audiogram handling)

   The auditory profile as defined by HearCom or BMBF Modellbasierte
   Hoergeraete is stored in the class
   AuditoryProfile::profile_t. Until a complete definition is
   available, only the currently needed elements are implemented.
 */
namespace AuditoryProfile {

    /**
       \brief A class to store frequency dependent data (e.g., HTL and UCL).
     */
    class fmap_t : public std::map<mha_real_t,mha_real_t> {
    public:
        std::vector<mha_real_t> get_frequencies() const; ///< \brief Return configured frequencies.
        std::vector<mha_real_t> get_values() const; ///< \brief Return stored values corresponding to the frequencies.
        bool isempty() const {return size() == 0;}; ///< \brief Test against emptyness.
    };

    /**
       \brief The Auditory Profile class.

       See definition of auditory profile

       \todo Give more documentation; implement all parts of the auditory profile.

       Currently only the audiogram data is stored.
     */
    class profile_t {
    public:
        /**
           \brief Class for ear-dependent parameters, e.g., audiograms or unilateral loudness scaling.
         */
        class ear_t {
        public:
            AuditoryProfile::fmap_t HTL;
            AuditoryProfile::fmap_t UCL;
            void convert_empty2normal();
        };
        AuditoryProfile::profile_t::ear_t L; ///< \brief Left ear data.
        AuditoryProfile::profile_t::ear_t R; ///< \brief Right ear data.
        /**
           \brief Return ear information of channel number
        */
        AuditoryProfile::profile_t::ear_t get_ear(unsigned int channel) const { 
            if( channel==0 ) return L;
            if( channel==1 ) return R;
            throw MHA_Error(__FILE__,__LINE__,"No support for more than two ears!");
        };
    };

    /**
       \brief Class to make the auditory profile accessible through the parser interface.
     */
    class parser_t : public MHAParser::parser_t {
    public:
        class fmap_t : public MHAParser::parser_t
        {
        public:
            fmap_t(const std::string& name, const std::string& help);
            AuditoryProfile::fmap_t get_fmap() const;
        private:
            void validate();
            MHAEvents::patchbay_t<AuditoryProfile::parser_t::fmap_t> patchbay;
            MHAParser::vfloat_t f;
            MHAParser::vfloat_t value;
            std::string name_;
        };
        class ear_t : public MHAParser::parser_t {
        public:
            ear_t();
            AuditoryProfile::profile_t::ear_t get_ear() const;
        private:
            AuditoryProfile::parser_t::fmap_t HTL;
            AuditoryProfile::parser_t::fmap_t UCL;
        };
        parser_t();
        AuditoryProfile::profile_t get_current_profile();
    private:
        AuditoryProfile::parser_t::ear_t L;
        AuditoryProfile::parser_t::ear_t R;
    };


}

#endif


// Local Variables:
// mode: c++
// compile-command: "make -C .."
// c-basic-offset: 4
// coding: utf-8-unix
// indent-tabs-mode: nil
// End:
