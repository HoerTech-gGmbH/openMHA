// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2008 2009 2013 2016 HörTech gGmbH
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

#ifndef MHA_EVENT_EMITTER_H
#define MHA_EVENT_EMITTER_H

#ifdef __cplusplus

#include <list>
#include <string>

namespace MHAEvents {

    class connector_base_t {
    public:
        connector_base_t();
        virtual ~connector_base_t();
        virtual void emit_event();
        virtual void emit_event(const std::string&);
        virtual void emit_event(const std::string&,unsigned int,unsigned int);
        void emitter_die();
    protected:
        bool emitter_is_alive;
    };
    
    /**
       \brief Claas for emitting MHA events

       Use the template claas MHAEvents::patchbay_t for connecting to
       an emitter.
     */
    class emitter_t {
    public:
        ~emitter_t();
        /**
           \brief Emit an event without parameter
         */
        void operator()();
        /**
           \brief Emit an event with string parameter
         */
        void operator()(const std::string&);
        /**
           \brief Emit an event with string parameter and two unsigned int parameters
         */
        void operator()(const std::string&,unsigned int,unsigned int);
        void connect(connector_base_t*);
        void disconnect(connector_base_t*);
    private:
        std::list<connector_base_t*> connections;
    };

}

#endif /* __cplusplus */

#endif /* MHA_EVENT_EMITTER_H */

/*
 Local Variables:
 mode: c++
 coding: utf-8-unix
 End:
 */
