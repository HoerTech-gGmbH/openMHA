// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2009 2013 2016 2017 2018 2020 HörTech gGmbH
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

#include "auditory_profile.h"
#include "mha_error.hh"

using namespace AuditoryProfile;

std::vector<mha_real_t> AuditoryProfile::fmap_t::get_frequencies() const
{
    std::vector<mha_real_t> f;
    for(std::map<mha_real_t,mha_real_t>::const_iterator it = begin();it != end(); ++it)
        f.push_back(it->first);
    return f;
}

std::vector<mha_real_t> AuditoryProfile::fmap_t::get_values() const
{
    std::vector<mha_real_t> v;
    for(std::map<mha_real_t,mha_real_t>::const_iterator it = begin();it != end(); ++it)
        v.push_back(it->second);
    return v;
}

AuditoryProfile::parser_t::fmap_t::fmap_t(const std::string& name, const std::string& help)
    : MHAParser::parser_t(help),
      f("Sample frequencies / Hz","[]","],]"),
      value(help,"[]"),
      name_(name)
      // \todo define range of value
{
    patchbay.connect(&value.writeaccess,this,&AuditoryProfile::parser_t::fmap_t::validate);
    insert_member(f);
    insert_item(name,&value);
}

AuditoryProfile::fmap_t AuditoryProfile::parser_t::fmap_t::get_fmap() const
{
    // \todo rethink if it is ok not to check equality of dimension, otherwise call validate()
    AuditoryProfile::fmap_t fmap;
    for(unsigned int k=0;k<f.data.size();k++)
        fmap[f.data[k]] = value.data[k];
    return fmap;
}

void AuditoryProfile::parser_t::fmap_t::validate()
{
    if( value.data.size() != f.data.size() )
        throw MHA_Error(__FILE__,__LINE__,"Mismatching size of %s (%zu) and frequency vector (%zu)."
                        " Please consider setting frequency vector first.",
                        name_.c_str(),value.data.size(),f.data.size());
}

AuditoryProfile::parser_t::ear_t::ear_t()
    : HTL("HTL","Hearing threshold in dB HL."),
      UCL("UCL","Uncomfortable level in dB HL.")
{
    insert_member(HTL);
    insert_member(UCL);
}

AuditoryProfile::profile_t::ear_t AuditoryProfile::parser_t::ear_t::get_ear() const
{
    AuditoryProfile::profile_t::ear_t ear;
    ear.HTL = HTL.get_fmap();
    ear.UCL = UCL.get_fmap();
    return ear;
}

AuditoryProfile::parser_t::parser_t()
{
    insert_member(L);
    insert_member(R);
}

AuditoryProfile::profile_t AuditoryProfile::parser_t::get_current_profile()
{
    AuditoryProfile::profile_t aud;
    aud.L = L.get_ear();
    aud.R = R.get_ear();
    return aud;
}

void AuditoryProfile::profile_t::ear_t::convert_empty2normal()
{
    if( HTL.isempty() ){
        HTL[125] = 0;
        HTL[250] = 0;
        HTL[500] = 0;
        HTL[1000] = 0;
        HTL[2000] = 0;
        HTL[4000] = 0;
        HTL[8000] = 0;
    }
    if( UCL.isempty() ){
        UCL[125] = 106.9;
        UCL[250] = 106.9;
        UCL[500] = 108.5;
        UCL[750] = 103.4;
        UCL[1000] = 99.4;
        UCL[1500] = 99.5;
        UCL[2000] = 96.6;
        UCL[3000] = 94.5;
        UCL[4000] = 97.9;
        UCL[6000] = 102.4;
        UCL[8000] = 94.9;
    }
}

// Local Variables:
// compile-command: "make -C .."
// coding: utf-8-unix
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
