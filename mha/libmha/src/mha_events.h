// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2013 2016 HörTech gGmbH
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

#ifndef MHA_EVENTS_H
#define MHA_EVENTS_H

#ifdef __cplusplus

#include "mha_event_emitter.h"

/**
   \brief Collection of event handling classes.
 */
namespace MHAEvents {

    template<class receiver_t> class connector_t : public connector_base_t {
    public:
        connector_t(emitter_t*,receiver_t*,void (receiver_t::*)());
        connector_t(emitter_t*,receiver_t*,void (receiver_t::*)(const std::string&));
        connector_t(emitter_t*,receiver_t*,void (receiver_t::*)(const std::string&,unsigned int,unsigned int));
        ~connector_t();
    private:
        void emit_event();
        void emit_event(const std::string&);
        void emit_event(const std::string&,unsigned int,unsigned int);
        emitter_t* emitter;
        receiver_t* receiver;
        void (receiver_t::*eventhandler)();
        void (receiver_t::*eventhandler_s)(const std::string&);
        void (receiver_t::*eventhandler_suu)(const std::string&,unsigned int,unsigned int);
    };

    /** 
        \brief Patchbay which connects any event emitter with any member function of the parameter class

        The connections created by the connect() function are hold
        until the destructor is called. To avoid access to invalid
        function pointers, it is required to destruct the patchbay
        before the receiver, usually by declaring the patchbay as a
        member of the receiver.

        The receiver can be any claas or structure; the event callback
        can be either a member function without arguments or with
        const std::string& argument.
        
    */
    template<class receiver_t> class patchbay_t {
    public:
        ~patchbay_t();
        /**
           \brief Connect a receiver member function void
           (receiver_t::*)() with an event emitter.
         */
        void connect(emitter_t*,receiver_t*,void (receiver_t::*)());
        /**
           \brief Connect a receiver member function void
           (receiver_t::*)(const std::string&) with an event emitter.
         */
        void connect(emitter_t*,receiver_t*,void (receiver_t::*)(const std::string&));
        void connect(emitter_t*,receiver_t*,void (receiver_t::*)(const std::string&,unsigned int,unsigned int));
    private:
        std::list<connector_t<receiver_t>*> cons;
    };

}

template<class receiver_t> MHAEvents::connector_t<receiver_t>::connector_t(emitter_t* e,receiver_t* r,void (receiver_t::*rfun)())
    : emitter(e),
      receiver(r),
      eventhandler(rfun),
      eventhandler_s(NULL),
      eventhandler_suu(NULL)
{
    emitter->connect(this);
}

template<class receiver_t> MHAEvents::connector_t<receiver_t>::connector_t(emitter_t* e,receiver_t* r,void (receiver_t::*rfun)(const std::string&))
    : emitter(e),
      receiver(r),
      eventhandler(NULL),
      eventhandler_s(rfun),
      eventhandler_suu(NULL)
{
    emitter->connect(this);
}

template<class receiver_t> MHAEvents::connector_t<receiver_t>::connector_t(emitter_t* e,receiver_t* r,void (receiver_t::*rfun)(const std::string&,unsigned int,unsigned int))
    : emitter(e),
      receiver(r),
      eventhandler(NULL),
      eventhandler_s(NULL),
      eventhandler_suu(rfun)
{
    emitter->connect(this);
}

template<class receiver_t> MHAEvents::connector_t<receiver_t>::~connector_t()
{
    if( emitter_is_alive )
        emitter->disconnect(this);
}


template<class receiver_t> void MHAEvents::connector_t<receiver_t>::emit_event()
{
    if( receiver && eventhandler )
        (receiver->*eventhandler)();
}

template<class receiver_t> void MHAEvents::connector_t<receiver_t>::emit_event(const std::string& arg)
{
    if( receiver && eventhandler_s )
        (receiver->*eventhandler_s)(arg);
}

template<class receiver_t> void MHAEvents::connector_t<receiver_t>::emit_event(const std::string& arg,unsigned int arg2,unsigned int arg3)
{
    if( receiver && eventhandler_suu )
        (receiver->*eventhandler_suu)(arg,arg2,arg3);
}

template<class receiver_t> MHAEvents::patchbay_t<receiver_t>::~patchbay_t()
{
    typename std::list<connector_t<receiver_t>*>::iterator con;
    for(con=cons.begin();con!=cons.end();++con)
        delete *con;
}

/** 
    \brief Create a connection
            
    The connection is removed when the patchbay is destructed.
            
    \param e Pointer to an event emitter
    \param r Pointer to the receiver
    \param rfun Pointer to a member function of the receiver class
            
*/
template<class receiver_t> void MHAEvents::patchbay_t<receiver_t>::connect(emitter_t* e,receiver_t* r,void (receiver_t::*rfun)())
{
    cons.push_back(new connector_t<receiver_t>(e,r,rfun));
}

/** 
    \brief Create a connection
            
    The connection is removed when the patchbay is destructed.
            
    \param e Pointer to an event emitter
    \param r Pointer to the receiver
    \param rfun Pointer to a member function of the receiver class
            
*/
template<class receiver_t> void MHAEvents::patchbay_t<receiver_t>::connect(emitter_t* e,receiver_t* r,void (receiver_t::*rfun)(const std::string&))
{
    cons.push_back(new connector_t<receiver_t>(e,r,rfun));
}

template<class receiver_t> void MHAEvents::patchbay_t<receiver_t>::connect(emitter_t* e,receiver_t* r,void (receiver_t::*rfun)(const std::string&,unsigned int,unsigned int))
{
    cons.push_back(new connector_t<receiver_t>(e,r,rfun));
}

#endif

#endif

/*
  Local Variables:
  mode: c++
  coding: utf-8-unix
  c-basic-offset: 4
  compile-command: "make -C .."
  indent-tabs-mode: nil
  End:
*/
