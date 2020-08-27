// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2013 2016 2017 2018 HörTech gGmbH
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

#include "mha_events.h"

void MHAEvents::emitter_t::operator()()
{
    std::list<connector_base_t*>::iterator connector;
    for(connector=connections.begin(); connector != connections.end(); ++connector)
        (*connector)->emit_event();
}

void MHAEvents::emitter_t::operator()(const std::string& arg)
{
    std::list<connector_base_t*>::iterator connector;
    for(connector=connections.begin(); connector != connections.end(); ++connector)
        (*connector)->emit_event(arg);
}

void MHAEvents::emitter_t::operator()(const std::string& arg,unsigned int arg2,unsigned int arg3)
{
    std::list<connector_base_t*>::iterator connector;
    for(connector=connections.begin(); connector != connections.end(); ++connector)
        (*connector)->emit_event(arg,arg2,arg3);
}

void MHAEvents::emitter_t::connect(connector_base_t* c)
{
    connections.push_back(c);
}

    
MHAEvents::emitter_t::~emitter_t()
{
    std::list<connector_base_t*>::iterator connector;
    for(connector=connections.begin(); connector != connections.end(); ++connector)
        (*connector)->emitter_die();
}

void MHAEvents::emitter_t::disconnect(connector_base_t* c)
{
    std::list<connector_base_t*>::iterator connector, next_connector;
    for(connector=connections.begin(); connector != connections.end(); connector = next_connector){
        next_connector = connector;
        ++next_connector;
        if( *connector == c ){
            connections.erase(connector);
        }
    }
}

MHAEvents::connector_base_t::connector_base_t()
    : emitter_is_alive(true)
{
}

MHAEvents::connector_base_t::~connector_base_t()
{
}

void MHAEvents::connector_base_t::emit_event()
{
}

void MHAEvents::connector_base_t::emit_event(const std::string&)
{
}

void MHAEvents::connector_base_t::emit_event(const std::string&,unsigned int,unsigned int)
{
}

void MHAEvents::connector_base_t::emitter_die()
{
    emitter_is_alive = false;
}

// Local Variables:
// coding: utf-8-unix
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
