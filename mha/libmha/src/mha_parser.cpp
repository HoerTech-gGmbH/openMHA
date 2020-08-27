// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2003 2004 2005 2006 2007 2008 2009 2010 2011 HörTech gGmbH
// Copyright © 2012 2013 2014 2016 2017 2018 2019 2020 HörTech gGmbH
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

#include <stdio.h>
#include <sstream>
#include <ctype.h>
#include <algorithm>
#include "mha_parser.hh"
#include "mha_error.hh"
#include "mha_defs.h"
#include "mha.hh"
#include "mha_os.h"
#include <fstream>
#include "mha_signal.hh"

#ifdef _WIN32
#include <float.h>
#endif

#ifndef MHAPLATFORM
#ifdef _WIN32
#define MHAPLATFORM "Win32"
#else
#define MHAPLATFORM "undefined-linux"
#endif
#endif

/** \defgroup mhascript The \mha configuration language

\mha Plugins that should use the \mha configuration language for their
configuration have to be implemented in C++ and need to include
mha_parser.hh. All required classes and functions for parser access
are declared in the namespace \ref MHAParser. The plugin class should
be derived from the class \ref MHAParser::parser_t (or
MHAPlugin::plugin_t), which symbolises a sub-parser node in the \mha
script hierarchy. Variables of many types can be registered to the
sub-parser node by calling the member function \ref
MHAParser::parser_t::insert_item() "insert_item". 

The \mha Plugin template class \ref MHAPlugin::plugin_t together with
the Plugin macro \ref MHAPLUGIN_CALLBACKS provide the callback
mappings and correct inheritance. If your plugin is based on that
template class, you simply have to use the insert_item command to give
access to your variables, everything else is managed internally.

A complete list of all \mha script items is given in the description of
the \ref MHAParser namespace.

*/

/**
   \ingroup mhatoolbox
   \namespace MHAParser
   \brief Name space for the \mha-Parser configuration language

   This namespace contains all classes which are needed for the
   implementation of the \mha configuration language. For details on the script
   language itself please see section \ref mhascript.
   
   \section sec_mhaparser_items List of valid MHAParser items

\li <b>Sub-parser</b>: \ref MHAParser::parser_t "parser_t"
\li <b>Variables</b>:<br>
Numeric variables:
\ref MHAParser::int_t "int_t",
\ref MHAParser::vint_t "vint_t",
\ref MHAParser::float_t "float_t",
\ref MHAParser::vfloat_t "vfloat_t",
\ref MHAParser::mfloat_t "mfloat_t"<br>
Other variables:
\ref MHAParser::string_t "string_t",
\ref MHAParser::vstring_t "vstring_t",
\ref MHAParser::kw_t "kw_t",
\ref MHAParser::bool_t "bool_t"
\li <b>Monitors</b>:<br>
Numeric monitors:
\ref MHAParser::int_mon_t "int_mon_t",
\ref MHAParser::vint_mon_t "vint_mon_t",
\ref MHAParser::float_mon_t "float_mon_t",
\ref MHAParser::vfloat_mon_t "vfloat_mon_t"<br>
\ref MHAParser::mfloat_mon_t "mfloat_mon_t"<br>
\ref MHAParser::mcomplex_mon_t "mcomplex_mon_t"<br>
Other monitors:
\ref MHAParser::bool_mon_t "bool_mon_t",
\ref MHAParser::string_mon_t "string_mon_t",
\ref MHAParser::vstring_mon_t "vstring_mon_t"

Members can be inserted into the configuration namespace by using
MHAParser::insert_item() or the insert_member() macro.

*/

/**

   \class MHAParser::commit_t

   \brief Parser variable with event-emission functionality

   The commit_t variable can register an event receiver in its
   constructor, which is called whenever the variable is set to
   "commit".

 */

namespace MHAParser {
    int get_precision();

    namespace StrCnv {
        int num_brackets(const std::string& s);
        int bracket_balance(const std::string& s);
    }
}

int MHAParser::get_precision()
{
    int prec = 9;
    std::string env_prec = mha_getenv("MHA_PARSER_PRECISION");
    if( env_prec.size() ){
        prec = strtol( env_prec.c_str(), NULL, 0 );
    }
    return prec;
}

MHAParser::entry_t::entry_t( const std::string & n, base_t * e )
    :name( n ), entry( e )
{
}

void MHAParser::trim( std::string & s )
{
    if( s.size(  ) == 0 )
        return;
    while( isspace( s.c_str(  )[0] ) ) {
        s.erase( 0, 1 );
        if( s.size(  ) == 0 )
            return;
    }
    while( isspace( s.c_str(  )[s.size(  ) - 1] ) ) {
        s.erase( s.size(  ) - 1, 1 );
        if( s.size(  ) == 0 )
            return;
    }
}

std::string MHAParser::commentate( const std::string & s )
{
    if( !s.size(  ) )
        return "";
    std::string r = std::string( "# " ) + s + std::string( "\n" );
    for( std::string::size_type k = r.size(  ) - 2; k > 0; k-- ) {
        if( r[k] == '\n' )
            r.insert( k + 1, "# " );
    }
    return r;
}

/**

\brief string replace function

\param s      target string
\param arg    search pattern
\param rep    replace pattern
*/
void MHAParser::strreplace( std::string & s, const std::string & arg, const std::string & rep )
{
    std::string out_string("");
    std::string::size_type len = arg.size(  );
    std::string::size_type pos;
    while( (pos = s.find(arg)) < s.size() ){
        out_string += s.substr(0,pos);
        out_string += rep;
        s.erase(0,pos+len);
    }
    s = out_string + s;
}

void MHAParser::envreplace( std::string & s )
{
    unsigned int p1, p2;
    std::string rep, tok;
    while( (p2 = s.find("}")) < s.size() ){
        p1 = s.find("${");
        if( p1 < p2 ){
            tok = s.substr(p1+2,p2-p1-2);
            rep = mha_getenv(tok.c_str());
            s.replace(p1,p2-p1+1,rep);
        }else{
            return;
        }
    }
}

/**************************************************************************/
/*   base_t                                                              **/
/**************************************************************************/
/** Constructor for base class of all parser nodes.
 * @param h Help text describing this parser node.
 *   This help text is accessible to the configuration language through
 *   the "?help" query command. */
MHAParser::base_t::base_t( const std::string & h )
    : data_is_initialized(false),
      help( h ), 
      id_str(""), 
      nested_lock( false ), 
      parent( NULL ), 
      thefullname( "" )
{
    operators["?"] = &base_t::op_query;
    operators["="] = &base_t::op_setval;
    operators["."] = &base_t::op_subparse;
    activate_query( "", &base_t::query_dump );
    activate_query( "type", &base_t::query_type );
    activate_query( "help", &base_t::query_help );
    activate_query( "subst", &base_t::query_subst );
    activate_query( "cmds", &base_t::query_cmds );
    activate_query( "addsubst", &base_t::query_addsubst );
    activate_query( "id", &base_t::query_id );
    activate_query( "version", &base_t::query_version );
    add_replace_pair( "PLATFORM", MHAPLATFORM );
}

MHAParser::base_t::base_t(const MHAParser::base_t& src)
    : data_is_initialized( src.data_is_initialized ),
      help( src.help ), 
      id_str( src.id_str ), 
      nested_lock( src.nested_lock ), 
      parent( src.parent ), 
      thefullname( src.thefullname )
{
    operators["?"] = &base_t::op_query;
    operators["="] = &base_t::op_setval;
    operators["."] = &base_t::op_subparse;
    activate_query( "", &base_t::query_dump );
    activate_query( "type", &base_t::query_type );
    activate_query( "help", &base_t::query_help );
    activate_query( "subst", &base_t::query_subst );
    activate_query( "cmds", &base_t::query_cmds );
    activate_query( "addsubst", &base_t::query_addsubst );
    activate_query( "id", &base_t::query_id );
    activate_query( "version", &base_t::query_version );
    add_replace_pair( "PLATFORM", MHAPLATFORM );
}

void MHAParser::base_t::add_parent_on_insert( parser_t * p, std::string n )
{
    if( parent ) {
        throw MHA_Error( __FILE__, __LINE__, "The variable is already registered as \"%s\".", fullname(  ).c_str(  ) );
    }
    parent = p;
    if( p->fullname(  ).size(  ) )
        thefullname = p->fullname(  ) + "." + n;
    else
        thefullname = n;
}

void MHAParser::base_t::rm_parent_on_remove( parser_t * )
{
    parent = NULL;
    thefullname = "";
}

/** Return the full dot-separated path name of this parser node in the \mha
 * configuration tree. */
const std::string & MHAParser::base_t::fullname(  ) const 
{
    return thefullname;
}

void MHAParser::base_t::activate_query( const std::string & n, query_t a )
{
    if( queries.find( n ) != queries.end(  ) )
        throw MHA_Error( __FILE__, __LINE__, "The query mode \"%s\" already exists.", n.c_str(  ) );
    queries[n] = a;
}

MHAParser::base_t::~base_t(  )
{
    if( parent )
        parent->remove_item( this );
}

std::string MHAParser::base_t::query_readfile( const std::string & s )
{
    throw MHA_Error( __FILE__, __LINE__,
                     "Query ?read is not implemented"
                     " for parser objects of type %s", typeid(*this).name());
}
std::string MHAParser::base_t::query_savefile( const std::string & s )
{
    throw MHA_Error( __FILE__, __LINE__,
                     "Query ?save is not implemented"
                     " for parser objects of type %s", typeid(*this).name());
}

std::string MHAParser::base_t::query_savefile_compact( const std::string & s )
{
    throw MHA_Error( __FILE__, __LINE__,
                     "Query ?saveshort is not implemented"
                     " for parser objects of type %s", typeid(*this).name());
}

std::string MHAParser::base_t::query_savemons( const std::string & s )
{
    throw MHA_Error( __FILE__, __LINE__,
                     "Query ?savemons is not implemented"
                     " for parser objects of type %s", typeid(*this).name());
}

std::string MHAParser::base_t::query_dump( const std::string & s )
{
    throw MHA_Error( __FILE__, __LINE__,
                     "Query ? is not implemented"
                     " for parser objects of type %s", typeid(*this).name());
}

std::string MHAParser::base_t::query_entries( const std::string & s )
{
    throw MHA_Error( __FILE__, __LINE__,
                     "Query ?entries is not implemented"
                     " for parser objects of type %s", typeid(*this).name());
}

std::string MHAParser::base_t::query_perm( const std::string & s )
{
    throw MHA_Error( __FILE__, __LINE__,
                     "Query ?perm is not implemented"
                     " for parser objects of type %s", typeid(*this).name());
}

std::string MHAParser::base_t::query_range( const std::string & s )
{
    throw MHA_Error( __FILE__, __LINE__,
                     "Query ?range is not implemented"
                     " for parser objects of type %s", typeid(*this).name());
}

std::string MHAParser::base_t::query_type( const std::string & s )
{
    throw MHA_Error( __FILE__, __LINE__,
                     "Query ?type is not implemented"
                     " for parser objects of type %s", typeid(*this).name());
}

std::string MHAParser::base_t::query_val( const std::string & s )
{
    throw MHA_Error( __FILE__, __LINE__,
                     "Query ?val is not implemented"
                     " for parser objects of type %s", typeid(*this).name());
}

std::string MHAParser::base_t::query_listids( const std::string & s )
{
    throw MHA_Error( __FILE__, __LINE__,
                     "Query ?listid is not implemented"
                     " for parser objects of type %s", typeid(*this).name());
}

std::string MHAParser::base_t::query_help( const std::string & s )
{
    return help;
}


std::string MHAParser::base_t::query_addsubst( const std::string & s )
{
    std::string v1, v2;
    unsigned int pos = s.find(" ");
    if( pos < s.size() ){
        v1 = s.substr(0,pos);
        v2 = s.substr(pos+1,s.size()-pos-1);
    }else{
        v1 = s;
        v2 = "";
    }
    add_replace_pair(v1,v2);
    return "";
}

std::string MHAParser::base_t::query_subst( const std::string & s )
{
    std::string r;
    repl_list_t::iterator q_it;
    for( q_it = repl_list.begin(  ); q_it != repl_list.end(  ); ++q_it )
        r += q_it->get_a(  ) + std::string( ":" ) + q_it->get_b(  ) + std::string( " " );
    if( r.rfind( " " ) < r.size(  ) )
        r.erase( r.rfind( " " ) );
    return std::string( "[" ) + r + std::string( "]" );
}

std::string MHAParser::base_t::query_cmds( const std::string & s )
{
    std::string r;
    query_map_t::iterator q_it;
    for( q_it = queries.begin(  ); q_it != queries.end(  ); ++q_it )
        r += std::string( "?" ) + q_it->first + std::string( " " );
    if( r.rfind( " " ) < r.size(  ) )
        r.erase( r.rfind( " " ) );
    return std::string( "[" ) + r + std::string( "]" );
}

std::string MHAParser::base_t::query_id( const std::string& )
{
    return id_str;
}

std::string MHAParser::base_t::query_version( const std::string& )
{
    return MHA_VERSION_STRING;
}

/** Set the identification string of this parser node.
 * The id can be queried from the configuration language using the ?id query
 * command.  Nodes can be found by id using the ?listid query command on a
 * containing parser node. 
 * @param s The new identification string. */
void MHAParser::base_t::set_node_id(const std::string& s )
{
    id_str = s;
}

/**
   \brief Set the help comment of a variable or parser.
   \param s New help comment.
 */
void MHAParser::base_t::set_help(const std::string& s)
{
    help = s;
}

/** Causes this node to process a command in the \mha configuration language.
 * @param cs The command to parse
 * @return The response to the command, if successful
 * @throw MHA_Error If the command cannot be executed successfully.
 *   The reason for failure is given in the message string of the exception. */
std::string MHAParser::base_t::parse( const std::string & cs )
{
    if( !cs.size(  ) )
        return "";
    if( nested_lock )
        throw MHA_ErrorMsg( "Nested parse() call blocked!" );
    nested_lock = true;
    try {
        std::string s( cs );
        for( repl_list_t::iterator it = repl_list.begin(  ); it != repl_list.end(  ); ++it ){
            it->replace( s );
        }
        envreplace( s );
        MHAParser::expression_t x( s, oplist(  ) );
        if( operators.find( x.op ) == operators.end(  ) )
            throw MHA_Error( __FILE__, __LINE__, "Invalid operator: \"%s\" (%s)", x.op.c_str(  ), s.c_str(  ) );
        opact_t act = operators[x.op];
        if( !act )
            throw MHA_ErrorMsg( "Invalid action!" );
        nested_lock = false;
        return ( this->*act ) ( x );
    }
    catch( MHA_Error & e ) {
        nested_lock = false;
        throw e;
    }
}

void MHAParser::base_t::parse(const std::vector<std::string>& cs,std::vector<std::string>& retv)
{
    retv.clear();
    for(unsigned int k=0;k<cs.size();k++)
        retv.push_back(parse(cs[k]));
}

/**
This function parses a command and writes the parsing result into a C
character array.  This base class implementation delegates to
#parse(const std::string &).

\param cmd  Command to be parsed
\param retv Buffer for the result
\param len      Length of buffer
*/
void MHAParser::base_t::parse( const char *cmd, char *retv, unsigned int len )
{
    if( !cmd )
        return;
    std::string r = parse( cmd );
    if( retv && len ) {
        strncpy( retv, r.c_str(  ), len );
        retv[len - 1] = 0;
    }
}

std::string MHAParser::base_t::op_query( expression_t & )
{
    throw MHA_Error( __FILE__, __LINE__, "no op_query defined." );
}

std::string MHAParser::base_t::op_setval( expression_t & )
{
    throw MHA_Error( __FILE__, __LINE__, "no op_setval defined." );
}

std::string MHAParser::base_t::op_subparse( expression_t & )
{
    throw MHA_Error( __FILE__, __LINE__, "no op_subparse defined." );
}

MHAParser::base_t::replace_t::replace_t( const std::string & ia, const std::string & ib )
    :a( ia ), b( ib )
{
}

void MHAParser::base_t::replace_t::replace( std::string & s )
{
    MHAParser::strreplace( s, "$[" + a + "]", b );
}

void MHAParser::base_t::add_replace_pair( const std::string & a, const std::string & b )
{
    if (a.size()) {
        repl_list_t::iterator it;
        for (it = repl_list.begin(); it != repl_list.end(); ++it) {
            if (a == it->get_a()) {
                if (b.size())
                    *it = replace_t(a,b);
                else
                    repl_list.erase(it);
                return;
            }
        }
        if (b.size())
            repl_list.push_back( replace_t( a, b ) );
    }
}


/**************************************************************************/
/*   parser_t                                                            **/
/**************************************************************************/
MHAParser::parser_t::parser_t(const std::string & help_text)
    : base_t(help_text), id_string("[MHAVersion " MHA_VERSION_STRING "]")
{
    activate_query( "entries", &base_t::query_entries );
    activate_query( "allvars", &base_t::query_val );
    activate_query( "read", &base_t::query_readfile );
    activate_query( "save", &base_t::query_savefile );
    activate_query( "saveshort", &base_t::query_savefile_compact );
    activate_query( "savemons", &base_t::query_savemons );
    activate_query( "listid", &base_t::query_listids );
}

std::string MHAParser::base_t::oplist(  )
{
    std::string r;
    opact_map_t::iterator op;
    for( op = operators.begin(  ); op != operators.end(  ); ++op )
        r += op->first;
    return r;
}

void MHAParser::parser_t::set_id_string( const std::string & s )
{
    id_string = s;
}


/**\brief Register a parser item into this sub-parser.
    
This function registers an item under a given name into this sub-parser
and makes it accessible to the parser interface.

\param n
    Name of the item in the configuration tree
\param e
    C++ pointer to the item instance.  e can either point to a variable, to 
    a monitor, or to another sub-parser.
*/
void MHAParser::parser_t::insert_item( const std::string & n, MHAParser::base_t * e )
{
    for( unsigned int k=0;k<n.size();k++)
        if( isspace(n.c_str()[k] ) )
            throw MHA_Error(__FILE__,__LINE__,
                            "An MHA variable name may not contain whitespace"
                            " characters (\"%s\",%u)",n.c_str(),k);
    if( e == this )
        throw MHA_Error( __FILE__, __LINE__, "Not able to insert entry into itself." );
    for( entry_map_t::iterator i = entries.begin(  ); i != entries.end(  ); ++i )
        if( i->name == n )
            throw MHA_Error( __FILE__, __LINE__, "The entry \"%s\" already exists.", n.c_str(  ) );
    entries.push_back( entry_t( n, e ) );
    e->add_parent_on_insert( this, n );
}

/**
   \brief Remove an item by name.

   If the item does not exist, an error is being reported.

   \param n Name of parser item to be removed from list.
*/
void MHAParser::parser_t::remove_item( const std::string & n )
{
    for( entry_map_t::iterator i = entries.begin(  ); i != entries.end(  ); ++i )
        if( i->name == n ) {
            i->entry->rm_parent_on_remove( this );
            entries.erase( i );
            return;
        }
    throw MHA_Error( __FILE__, __LINE__, "The entry \"%s\" does not exist.", n.c_str(  ) );
}

/**
   \brief Remove an item by name.

   Non-existing items are ignored.

   \param n Name of parser item to be removed from list.
*/
void MHAParser::parser_t::force_remove_item( const std::string & n )
{
    for( entry_map_t::iterator i = entries.begin(  ); i != entries.end(  ); ++i )
        if( i->name == n ) {
            i->entry->rm_parent_on_remove( this );
            entries.erase( i );
            return;
        }
}

MHAParser::parser_t::~parser_t(  )
{
    for( entry_map_t::iterator i = entries.begin(  ); i != entries.end(  ); ++i )
        i->entry->rm_parent_on_remove( this );
}

/**
   \brief Remove an item by address.

   The item belonging to an address is being removed from the list of items.

   \param addr Address of parser item to be removed.
*/
void MHAParser::parser_t::remove_item( const base_t * addr )
{
    entry_map_t::iterator current_iterator, next_iterator;
    for( current_iterator = entries.begin(  ); current_iterator != entries.end(  ); current_iterator = next_iterator ) {
        next_iterator = current_iterator;
        ++next_iterator;
        if( current_iterator->entry == addr ) {
            current_iterator->entry->rm_parent_on_remove( this );
            entries.erase( current_iterator );
        }
    }
}


std::string MHAParser::parser_t::op_setval( expression_t & x )
{
    if( !x.lval.size(  ) ){
        throw MHA_Error(__FILE__, __LINE__,
                        "Cannot assign value to parser node.");
    }
    for( entry_map_t::iterator i = entries.begin(  ); i != entries.end(  ); ++i )
        if( i->name == x.lval ) {
            std::string r = i->entry->parse( x.op + x.rval );
            writeaccess(  );
            writeaccess( x.rval );
            return r;
        }
    throw MHA_Error( __FILE__, __LINE__, "Invalid entry: \"%s\"", x.lval.c_str(  ) );
}

std::string MHAParser::parser_t::query_type( const std::string & s )
{
    return "parser";
}

std::string MHAParser::parser_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string retv = cfg_dump_short(this, "");
    readaccess(  );readaccess( s );
    return retv;
}

std::string MHAParser::parser_t::query_listids( const std::string & s )
{
    std::string retv = all_ids(this, "", s);
    return retv;
}

std::string MHAParser::parser_t::query_savefile( const std::string & fname )
{
    std::ofstream fh( fname.c_str(  ) );
    if( fh.fail(  ) )
        throw MHA_Error( __FILE__, __LINE__, "not able to open the file \"%s\" for writing", fname.c_str(  ) );
    fh << "#" << id_string << std::endl;
    fh << cfg_dump( this, "" );
    fh.close(  );
    return "";
}

std::string MHAParser::parser_t::query_savefile_compact( const std::string & fname )
{
    std::ofstream fh( fname.c_str(  ) );
    if( fh.fail(  ) )
        throw MHA_Error( __FILE__, __LINE__, "not able to open the file \"%s\" for writing", fname.c_str(  ) );
    fh << "#" << id_string << std::endl;
    fh << cfg_dump_short( this, "" );
    fh.close(  );
    return "";
}

std::string MHAParser::parser_t::query_savemons( const std::string & fname )
{
    std::ofstream fh( fname.c_str(  ) );
    if( fh.fail(  ) )
        throw MHA_Error( __FILE__, __LINE__, "not able to open the file \"%s\" for writing", fname.c_str(  ) );
    fh << "#" << id_string << std::endl;
    fh << mon_dump( this, "" );
    fh.close(  );
    return "";
}

std::string MHAParser::parser_t::query_readfile( const std::string & fname )
{
    srcfile = fname;
    srcline = 0;
    std::ifstream fh( fname.c_str(  ) );
    std::string  returnval;
    if( fh.fail(  ) ) {
        throw MHA_Error( __FILE__, __LINE__, "not able to open file \"%s\" for reading.", fname.c_str(  ) );
    }
    try {
        std::string s;
        std::string next_cmd("");
        do {
            s = "";
            getline( fh, s );
            srcline++;
            MHAParser::trim( s );
            /* remove comments: */
            if( s.find( "#" ) < s.size(  ) )
                s.erase( s.find( "#" ) );
            if( s.size(  ) ){
                if( s.rfind("...")+3 == s.size() ){
                    next_cmd += s.substr(0, s.rfind("..."));
                }else{
                    next_cmd += s;
                    returnval += parse( next_cmd );
                    next_cmd = "";
                }
            }
        } while( !fh.eof(  ) );
        fh.close(  );
        return returnval;

    }
    catch( MHA_Error & e ) {
        fh.close(  );
        last_errormsg = e.get_msg(  );
        throw MHA_Error( __FILE__, __LINE__,
                         "%s\n(while parsing \"%s\" line %u)", last_errormsg.c_str(  ), srcfile.c_str(  ), srcline );
    }
}

std::string MHAParser::parser_t::query_entries( const std::string & s )
{
    std::string r;
    for( entry_map_t::iterator i = entries.begin(  ); i != entries.end(  ); ++i )
        r += i->name + " ";
    if( r.rfind( " " ) < r.size(  ) )
        r.erase( r.rfind( " " ) );
    return std::string( "[" ) + r + std::string( "]" );
}

std::string MHAParser::cfg_dump( base_t * p, const std::string & pref )
{
    std::string cmds = p->parse( pref + "?cmds" );
    std::string r( "" );
    if( cmds.find( "?type" ) < cmds.size(  ) )
        if( p->parse( pref + "?type" ) == "parser" ) {
            if( pref.size(  ) )
                r += MHAParser::commentate( "parser \"" + pref + "\":\n" );
            if( cmds.find( "?help" ) < cmds.size(  ) )
                r += MHAParser::commentate( p->parse( pref + "?help" ) );
        }
    if( cmds.find( "?val" ) < cmds.size(  ) )
        if( cmds.find( "?perm" ) < cmds.size(  ) )
            if( p->parse( pref + "?perm" ) != "monitor" ) {
                if( cmds.find( "?help" ) < cmds.size(  ) )
                    r += MHAParser::commentate( p->parse( pref + "?help" ) );
                if( cmds.find( "?type" ) < cmds.size(  ) ) {
                    std::string q = p->parse( pref + "?type" );
                    if( cmds.find( "?range" ) < cmds.size(  ) ) {
                        std::string rg = p->parse( pref + "?range" );
                        if( rg.size(  ) )
                            q += ":" + rg;
                    }
                    r += MHAParser::commentate( q );
                }
                r += pref + " = " + p->parse( pref + "?val" ) + "\n\n";
            }
    if( cmds.find( "?entries" ) < cmds.size(  ) ) {
        std::vector < std::string > entries;
        MHAParser::StrCnv::str2val( p->parse( pref + "?entries" ), entries );
        for( unsigned int k = 0; k < entries.size(  ); k++ )
            if( pref.size(  ) )
                r += cfg_dump( p, pref + "." + entries[k] );
            else
                r += cfg_dump( p, entries[k] );
    }
    return r;
}


std::string MHAParser::mon_dump( base_t * p, const std::string & pref )
{
    std::string cmds = p->parse( pref + "?cmds" );
    std::string r( "" );
    if( cmds.find( "?val" ) < cmds.size(  ) )
        if( cmds.find( "?perm" ) < cmds.size(  ) )
            if( p->parse( pref + "?perm" ) == "monitor" ) {
                if( cmds.find( "?help" ) < cmds.size(  ) )
                    r += MHAParser::commentate( p->parse( pref + "?help" ) );
                if( cmds.find( "?type" ) < cmds.size(  ) ) {
                    std::string q = p->parse( pref + "?type" );
                    if( cmds.find( "?range" ) < cmds.size(  ) ) {
                        std::string rg = p->parse( pref + "?range" );
                        if( rg.size(  ) )
                            q += ":" + rg;
                    }
                    r += MHAParser::commentate( q );
                }
                r += pref + " = " + p->parse( pref + "?val" ) + "\n\n";
            }
    if( cmds.find( "?entries" ) < cmds.size(  ) ) {
        std::vector < std::string > entries;
        MHAParser::StrCnv::str2val( p->parse( pref + "?entries" ), entries );
        for( unsigned int k = 0; k < entries.size(  ); k++ )
            if( pref.size(  ) )
                r += mon_dump( p, pref + "." + entries[k] );
            else
                r += mon_dump( p, entries[k] );
    }
    return r;
}


std::string MHAParser::all_ids( base_t * p, const std::string & pref, const std::string & id )
{
    std::string cmds = p->parse( pref + "?cmds" );
    std::string r( "" );
    std::string temp_id("");
    if( cmds.find( "?id" ) < cmds.size(  ) ){
        temp_id = p->parse( pref + "?id" );
        if( temp_id.size() ){
            if( id.size() ){
                if( temp_id == id )
                    r += pref + "\n";
            }else{
                r += pref + " = " + temp_id + "\n";
            }
        }
    }
    if( cmds.find( "?entries" ) < cmds.size(  ) ) {
        std::vector < std::string > entries;
        MHAParser::StrCnv::str2val( p->parse( pref + "?entries" ), entries );
        for( unsigned int k = 0; k < entries.size(  ); k++ )
            if( pref.size(  ) )
                r += all_ids( p, pref + "." + entries[k], id );
            else
                r += all_ids( p, entries[k], id );
    }
    return r;
}

std::string MHAParser::cfg_dump_short( base_t * p, const std::string & pref )
{
    std::string cmds = p->parse( pref + "?cmds" );
    std::string r( "" );
    if( cmds.find( "?val" ) < cmds.size(  ) )
        if( cmds.find( "?perm" ) < cmds.size(  ) )
            if( p->parse( pref + "?perm" ) != "monitor" ) {
                r += pref + " = " + p->parse( pref + "?val" ) + "\n";
            }
    if( cmds.find( "?entries" ) < cmds.size(  ) ) {
        std::vector < std::string > entries;
        MHAParser::StrCnv::str2val( p->parse( pref + "?entries" ), entries );
        for( unsigned int k = 0; k < entries.size(  ); k++ )
            if( pref.size(  ) )
                r += cfg_dump_short( p, pref + "." + entries[k] );
            else
                r += cfg_dump_short( p, entries[k] );
    }
    return r;
}

std::string MHAParser::all_dump( base_t * p, const std::string & pref )
{
    std::string cmds = p->parse( pref + "?cmds" );
    std::string r( "" );
    if( pref.size(  ) )
        if( cmds.find( "?type" ) < cmds.size(  ) )
            if( p->parse( pref + "?type" ) == "parser" )
                r += MHAParser::commentate( "parser \"" + pref + "\":\n" );
    if( cmds.find( "?help" ) < cmds.size(  ) )
        r += MHAParser::commentate( p->parse( pref + "?help" ) );
    if( cmds.find( "?val" ) < cmds.size(  ) ) {
        if( cmds.find( "?type" ) < cmds.size(  ) ) {
            std::string q = p->parse( pref + "?type" );
            if( cmds.find( "?range" ) < cmds.size(  ) )
                q += ":" + p->parse( pref + "?range" );
            if( cmds.find( "?perm" ) < cmds.size(  ) )
                q += " (" + p->parse( pref + "?perm" ) + ")";
            r += MHAParser::commentate( q );
        }
        r += pref + " = " + p->parse( pref + "?val" ) + "\n\n";
    }
    if( cmds.find( "?entries" ) < cmds.size(  ) ) {
        std::vector < std::string > entries;
        MHAParser::StrCnv::str2val( p->parse( pref + "?entries" ), entries );
        for( unsigned int k = 0; k < entries.size(  ); k++ )
            if( pref.size(  ) )
                r += all_dump( p, pref + "." + entries[k] );
            else
                r += all_dump( p, entries[k] );
    }
    return r;
}

std::string MHAParser::parser_t::query_dump( const std::string & s )
{
    return all_dump( this, "" );
}

bool MHAParser::parser_t::has_entry(const std::string& s) {
    return std::find_if(entries.begin(),entries.end(),
                        [&s](const entry_t & i){return i.name==s;})!=entries.end();
}

std::string MHAParser::parser_t::op_subparse( expression_t & x )
{
    if( !x.lval.size(  ) ) {
        return parse( x.rval );
    }
    for( entry_map_t::iterator i = entries.begin(  ); i != entries.end(  ); ++i )
        if( i->name == x.lval )
            return i->entry->parse( x.rval );
    throw MHA_Error( __FILE__, __LINE__, "Invalid entry: \"%s\"", x.lval.c_str(  ) );
}

std::string MHAParser::parser_t::op_query( expression_t & x )
{
    if( !x.lval.size(  ) ) {
        expression_t q( x.rval, ":" );
        query_t query;
        if( queries.find( q.lval ) != queries.end(  ) ) {
            query = queries[q.lval];
            if( !query )
                throw MHA_ErrorMsg( "Invalid query." );
        } else
            throw MHA_Error( __FILE__, __LINE__, "Invalid query mode: \"%s\"", q.lval.c_str(  ) );
        return ( this->*query ) ( q.rval );
    }
    for( entry_map_t::iterator i = entries.begin(  ); i != entries.end(  ); ++i )
        if( i->name == x.lval )
            return i->entry->parse( x.op + x.rval );
    throw MHA_Error( __FILE__, __LINE__, "Invalid entry: \"%s\"", x.lval.c_str(  ) );
}

/**************************************************************************/
/*   variable_t                                                          **/
/**************************************************************************/

MHAParser::variable_t::variable_t( const std::string & h )
    :  monitor_t( h ), locked( false )
{
}

/**
   \brief Lock a variable against write access
   \param b Lock state
*/
void MHAParser::variable_t::setlock( const bool & b )
{
    locked = b;
}

std::string MHAParser::variable_t::op_setval( expression_t & x )
{
    if( locked )
        throw MHA_Error( __FILE__, __LINE__, "The variable is locked." );
    if( x.lval.size(  ) )
        throw MHA_Error( __FILE__, __LINE__, 
                         "Invalid (non empty) lvalue: \"%s\"",
                         x.lval.c_str() );
    return "";
}

std::string MHAParser::variable_t::query_perm( const std::string & s )
{
    return locked ? "locked" : "writable";
}

/**************************************************************************/
/*   monitor_t                                                          **/
/**************************************************************************/

MHAParser::monitor_t::monitor_t( const std::string & h )
    :base_t( h )
{
    activate_query( "val", &base_t::query_val );
    activate_query( "perm", &base_t::query_perm );
}

MHAParser::monitor_t::monitor_t( const monitor_t& src )
    :base_t( src )
{
    activate_query( "val", &base_t::query_val );
    activate_query( "perm", &base_t::query_perm );
}

std::string MHAParser::monitor_t::op_query( expression_t & x )
{
    if( x.lval.size(  ) )
        throw MHA_Error( __FILE__, __LINE__, 
                         "Invalid (non empty) lvalue: \"%s\"",
                         x.lval.c_str() );
    expression_t q( x.rval, ":" );
    if( queries.find( q.lval ) != queries.end(  ) )
        return ( this->*( queries[q.lval] ) ) ( q.rval );
    throw MHA_Error( __FILE__, __LINE__, "Invalid query mode: \"%s\"", q.lval.c_str(  ) );
}

std::string MHAParser::monitor_t::query_dump( const std::string & s )
{
    std::string cmds = parse( "?cmds" );
    std::string r = parse( "?help" ) + "\n" + parse( "?type" );
    if( cmds.find( "?range" ) < cmds.size(  ) )
        r += ":" + parse( "?range" );
    if( cmds.find( "?perm" ) < cmds.size(  ) )
        r += " (" + parse( "?perm" ) + ")";
    r = MHAParser::commentate( r );
    r += parse( "?val" ) + "\n";
    return r;
}

std::string MHAParser::monitor_t::query_perm( const std::string & s )
{
    return "monitor";
}

/**
   
   \class expression_t
   \brief Class for separating a string into a left hand value and a right hand value.

   A list of valid operators can be provided. After construction, the
   class members lval, rval and op contain the apropriate contents.

*/

/**
   \brief Constructor
   \param s String to be splitted
   \param o List of valid operators (single character only)
*/
MHAParser::expression_t::expression_t( const std::string & s, const std::string & o )
    :  lval( "" ), rval( "" ), op( "" )
{
    std::string::size_type opos = s.find_first_of( o );
    if( opos >= s.size(  ) ) {
        lval = s;
        rval = "";
        op = "";
        return;
    }
    op = s.substr( opos, 1 );
    lval = s.substr( 0, opos );
    rval = s.substr( opos + 1 );
    if( rval.size(  ) )
        if( rval[0] == op[0] )
            throw MHA_ErrorMsg( "Invalid (duplicate) operator." );
    MHAParser::trim( lval );
    MHAParser::trim( rval );
}

MHAParser::expression_t::expression_t(  )
    :  lval( "" ), rval( "" ), op( "" )
{
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/*                                                                         **/
/*   StrCnv                                                                **/
/*                                                                         **/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

// special handling of nan values on windows is no longer needed
static inline std::ostream & write_float(std::ostream & o, const float & f)
{
    return o << f;
}


/****************************************************************************/
/*  val2str                                                                **/
/****************************************************************************/

std::string MHAParser::StrCnv::val2str( const float &v )
{
    std::ostringstream tmp( "" );
    tmp.precision(MHAParser::get_precision());
    write_float(tmp,v);
    return tmp.str(  );
}

std::string MHAParser::StrCnv::val2str( const mha_complex_t & v )
{
    std::ostringstream tmp( "" );
    tmp.precision(MHAParser::get_precision());
    if( v.im == 0 ){
        write_float(tmp, v.re) ;
    }else{
        tmp << "(";
        write_float(tmp, v.re) ;
        if( v.im < 0.0f ){
            write_float(tmp, v.im);
        }else{
            tmp << "+";
            write_float(tmp, v.im);
        }
        tmp << "i)";
    }
    return tmp.str(  );
}

std::string MHAParser::StrCnv::val2str( const int &v )
{
    std::ostringstream tmp( "" );
    tmp << v;
    return tmp.str(  );
}

std::string MHAParser::StrCnv::val2str( const std::string & v )
{
    return v;
}

std::string MHAParser::StrCnv::val2str( const std::vector < int >&v )
{
    std::ostringstream tmp( "" );
    for( unsigned int k = 0; k < v.size(  ); k++ ) {
        tmp << MHAParser::StrCnv::val2str( v[k] ) << ( k < v.size(  ) - 1 ? " " : "" );
    }
    return std::string( "[" ) + tmp.str(  ) + std::string( "]" );
}

std::string MHAParser::StrCnv::val2str( const std::vector < std::string > &v )
{
    std::ostringstream tmp( "" );
    for( unsigned int k = 0; k < v.size(  ); k++ ) {
        tmp << MHAParser::StrCnv::val2str( v[k] ) << ( k < v.size(  ) - 1 ? " " : "" );
    }
    return std::string( "[" ) + tmp.str(  ) + std::string( "]" );
}

std::string MHAParser::StrCnv::val2str( const std::vector < float >&v )
{
    std::ostringstream tmp( "" );
    for( unsigned int k = 0; k < v.size(  ); k++ ) {
        tmp << MHAParser::StrCnv::val2str( v[k] ) << ( k < v.size(  ) - 1 ? " " : "" );
    }
    return std::string( "[" ) + tmp.str(  ) + std::string( "]" );
}

std::string MHAParser::StrCnv::val2str( const std::vector < mha_complex_t > &v )
{
    std::ostringstream tmp( "" );
    for( unsigned int k = 0; k < v.size(  ); k++ ) {
        tmp << MHAParser::StrCnv::val2str( v[k] ) << ( k < v.size(  ) - 1 ? " " : "" );
    }
    return std::string( "[" ) + tmp.str(  ) + std::string( "]" );
}

std::string MHAParser::StrCnv::val2str( const std::vector < std::vector < float > >&v )
{
    std::ostringstream tmp( "" );
    for( unsigned int k = 0; k < v.size(  ); k++ ) {
        tmp << MHAParser::StrCnv::val2str( v[k] ) << ( k < v.size(  ) - 1 ? ";" : "" );
    }
    return std::string( "[" ) + tmp.str(  ) + std::string( "]" );
}

std::string MHAParser::StrCnv::val2str( const std::vector < std::vector < int > >&v )
{
    std::ostringstream tmp( "" );
    for( unsigned int k = 0; k < v.size(  ); k++ ) {
        tmp << MHAParser::StrCnv::val2str( v[k] ) << ( k < v.size(  ) - 1 ? ";" : "" );
    }
    return std::string( "[" ) + tmp.str(  ) + std::string( "]" );
}

std::string MHAParser::StrCnv::val2str( const std::vector<std::vector<mha_complex_t> >&v )
{
    std::ostringstream tmp( "" );
    for( unsigned int k = 0; k < v.size(  ); k++ ) {
        tmp << MHAParser::StrCnv::val2str( v[k] ) << ( k < v.size(  ) - 1 ? ";" : "" );
    }
    return std::string( "[" ) + tmp.str(  ) + std::string( "]" );
}

std::string MHAParser::StrCnv::val2str( const bool & v )
{
    if( v )
        return "yes";
    return "no";
}

/*
 * str2val
 */

void MHAParser::StrCnv::str2val( const std::string & s, int &v )
{
    const char *s_cp = s.c_str(  );
    int val = 0;
    char *s_ptr = NULL;
    val = strtol( s_cp, &s_ptr, 0 );
    if( ( s_cp == s_ptr ) || ( strlen( s_ptr ) != 0 ) ) {
        throw MHA_Error( __FILE__, __LINE__, "\"%s\" does not contain a valid scalar value (%s)", s.c_str(  ), s_ptr );
    }
    v = val;
}

template<class arg_t> void MHAParser::StrCnv::str2val( const std::string & s, std::vector<arg_t>& v )
{
    arg_t tmpval;
    std::vector<arg_t> val;
    int nbr = MHAParser::StrCnv::num_brackets( s );
    if( nbr == 0 ){
        MHAParser::StrCnv::str2val( s, tmpval );
        val.push_back( tmpval );
        v = val;
    }else if( nbr >= 2 ){
        std::string fv;
        std::istringstream tmp(s.substr(1,s.size()-2) + std::string(" "));
        while( tmp >> fv ) {
            MHAParser::StrCnv::str2val( fv, tmpval );
            val.push_back( tmpval );
        }
        v = val;
    }else if (nbr == -1){
        v = val; // empty string without brackets creates empty vector
    }else{
        throw MHA_Error(__FILE__,__LINE__,"Invalid brackets (\"%s\", %d)",s.c_str(),nbr);
    }
}

template<class arg_t> void MHAParser::StrCnv::str2val( const std::string & s, std::vector<std::vector<arg_t> >& v )
{
    switch( MHAParser::StrCnv::num_brackets( s ) ){
    case -1 : // empty string, error
        throw MHA_Error(__FILE__,__LINE__,"Empty string \"%s\"",s.c_str());
        break;
    case 0 : // no brackets, scalar
    case 2 : // both brackets, vector
    {
        std::vector<arg_t> tmpval;
        std::vector<std::vector<arg_t> > val;
        MHAParser::StrCnv::str2val( s, tmpval );
        val.push_back( tmpval );
        v = val;
        break;
    }
    case 4 : // all brackets, matrix
    {
        std::string cs(s);
        cs.erase( 0, 1 );
        cs.erase( cs.size(  ) - 1, 1 );
        MHAParser::trim( cs );
        cs += ";";
        std::string fv;
        std::vector<arg_t> tmpval;
        std::vector<std::vector<arg_t> > val;
        std::string tmp;
        unsigned int p1;
        p1 = cs.find( ";" );
        while( p1 < cs.size(  ) ) {
            MHAParser::trim( fv = cs.substr( 0, p1 ) );
            cs.erase( 0, p1 + 1 );
            if( fv.size(  ) ) {
                MHAParser::StrCnv::str2val( fv, tmpval );
                val.push_back( tmpval );
            }
            p1 = cs.find( ";" );
        }
        if( val.size(  ) ) {
            unsigned int dim1 = val[0].size(  );
            for( unsigned int k = 1; k < val.size(  ); k++ ) {
                if( val[k].size(  ) != dim1 )
                    throw MHA_Error( __FILE__, __LINE__, "Row %u has %zu entries, expected %u.", k, val[k].size(  ), dim1 );
            }
        }
        v = val;
        break;
    }
    default :
        throw MHA_Error(__FILE__,__LINE__,"Invalid brackets: %s",s.c_str());
    }
}

#ifndef DOXY_PARSE
// doxygen does not understand this:
template void MHAParser::StrCnv::str2val<std::string>(const std::string& s,std::vector<std::string>& v);
template void MHAParser::StrCnv::str2val<int>(const std::string& s,std::vector<int>& v);
template void MHAParser::StrCnv::str2val<mha_complex_t>(const std::string&,std::vector<mha_complex_t>&);
template void MHAParser::StrCnv::str2val<int>(const std::string&,std::vector<std::vector<int> >&);
template void MHAParser::StrCnv::str2val<float>(const std::string&,std::vector<std::vector<float> >&);
template void MHAParser::StrCnv::str2val<mha_complex_t>(const std::string&,std::vector<std::vector<mha_complex_t> >&);
#endif

void MHAParser::StrCnv::str2val( const std::string & s, std::string & v )
{
    v = s;
}

void MHAParser::StrCnv::str2val( const std::string & s, MHAParser::keyword_list_t & v )
{
    v.set_value( s );
}

void MHAParser::StrCnv::str2val( const std::string & s, float &v )
{
    float val = 0;
    char *s_ptr = NULL;
    val = strtod( s.c_str(  ), &s_ptr );
    if( ( s.c_str(  ) == s_ptr ) || ( strlen( s_ptr ) != 0 ) ) {
        throw MHA_Error( __FILE__, __LINE__, "\"%s\" does not contain a valid scalar value (%s)", s.c_str(  ), s_ptr );
    }
    v = val;
}

/**
 * This internal function parses a floating point number from the beginning
 * of a string.
 * 
 * @param s The string to parse
 * @param v The float variable to fill with a value
 * @return  The rest of the string.
 */
static std::string parse_1_float( const std::string & s, mha_real_t & v )
{
    char *endptr = 0;
    v = strtod( s.c_str(  ), &endptr );
    int converted_characters = endptr - s.c_str(  );
    if( converted_characters == 0 )
        throw MHA_Error( __FILE__, __LINE__, "Number expected, found '%c'", s[0] );
    return s.substr( converted_characters );
}

/**
 * This internal function parses a complex number from the beginning of a
 * string.
 * 
 * @param s The string to parse
 * @param v The complex variable to fill with a value
 * @return  The rest of the string.
 */
static std::string parse_1_complex( const std::string & s, mha_complex_t & v )
{
    std::string rest = s.substr( s.find_first_not_of( " \t" ) );
    if( rest[0] != '(' ) {
        v.im = 0;
        return parse_1_float( rest, v.re );
    }
    rest = parse_1_float( rest.substr( 1 ), v.re );
    rest = rest.substr( rest.find_first_not_of( " \t" ) );
    int sign = 0;
    switch ( rest[0] ) {
    case '+':
        sign = 1;
        break;
    case '-':
        sign = -1;
        break;
    default:
        throw MHA_Error( __FILE__, __LINE__, "imaginary part starts with '%c' instead of '+' or '-'", rest[0] );
    }
    rest = parse_1_float( rest.substr( 1 ), v.im );
    v.im *= sign;
    rest = rest.substr( rest.find_first_not_of( " \t" ) );
    if( rest[0] != 'i' )
        throw MHA_ErrorMsg( "missing 'i' after imaginary part" );
    if (rest.size() > 1)
        rest = rest.substr( rest.find_first_not_of( " \t", 1 ) );
    if( rest[0] != ')' )
        throw MHA_ErrorMsg( "closing ')' missing in complex value" );
    return rest.substr( 1 );
}

void MHAParser::StrCnv::str2val( const std::string & s, mha_complex_t & v )
{
    std::string rest = parse_1_complex( s, v );
    if( rest.size(  ) )
        throw MHA_Error( __FILE__, __LINE__, "unrecognized content %s after parsing complex number", rest.c_str(  ) );
}

std::string MHAParser::StrCnv::val2str( const MHAParser::keyword_list_t & v )
{
    return v.get_value(  );
}

void MHAParser::StrCnv::str2val( const std::string & s, bool & v )
{
    if( (s == "yes") || (s == "1") ){
        v = true;
    }else if( (s == "no") || (s == "0") ) {
        v = false;
    }else{
        throw MHA_Error( __FILE__, __LINE__, 
                         "Syntax error in boolean value: \"%s\" (expected \"yes\" or \"no\")", s.c_str() );
    }
}

/** 
    \brief Return number of brackets according to layer depth (vector:2, matrix:4, etc)
    \param s    String
    \return Number of brackets, or -1 for empty string, or -2 for invalid brackets
*/
int MHAParser::StrCnv::num_brackets(const std::string& s)
{
    int num_b{0};
    if( s.size() == 0 )
        num_b = -1;
    else{
        int num_open{0};
        int num_close{0};
        int max_open{0};
        for (unsigned idx=0; idx<s.size(); ++idx){
            num_open += s[idx] == '[';
            num_close += s[idx] == ']';
            if (num_close > num_open)
                return -2;
            // the idea is not to count the brackets per se but to
            // get the number of bracket layers
            // for example [foo] is a vector, [[bar]] is a matrix but
            // [[foo][bar]] is still a matrix
            max_open = std::max((num_open - num_close),max_open);
        }
        if (num_open != num_close)
            num_b = -2;
        else
            num_b = max_open * 2;
    }
    return num_b;
}

int MHAParser::StrCnv::bracket_balance(const std::string& s)
{
    int r=0;
    for( std::string::const_iterator c=s.begin();c!=s.end();++c){
        r += (*c == '[');
        r -= (*c == ']');
    }
    return r;
}



/*
 *  keyword_list_t
 */


//! Select a value from keyword list.
/*!

This function selects a value from the keyword list. The index is
set to the last matching entry.

\param s      Value to be selected.
*/
void MHAParser::keyword_list_t::set_value( const std::string & s )
{
    if( (!entries.size()) && (!s.size()) )
        return;
    unsigned int nidx( entries.size(  ) );
    for( unsigned k = 0; k < entries.size(  ); k++ )
        if( entries[k] == s )
            nidx = k;
    if( nidx == entries.size(  ) ) {
        std::string en = StrCnv::val2str( entries );
        throw MHA_Error( __FILE__, __LINE__, "The keyword \"%s\" is not in the list (%s)", s.c_str(  ), en.c_str(  ) );
    }
    index = nidx;
}

void MHAParser::keyword_list_t::set_index( unsigned int idx )
{
    if( idx >= entries.size(  ) )
        throw MHA_Error( __FILE__, __LINE__, "The index %u is out of range (%zu).", idx, entries.size(  ) );
    index = idx;
}

//! Set keyword list entries.
/*!

With this function, the keyword list can be set from a space
separated string list.

\param s      Space separated entry list.
*/
void MHAParser::keyword_list_t::set_entries( const std::string & s )
{
    StrCnv::str2val( s, entries );
}

//! Return selected value.
const std::string & MHAParser::keyword_list_t::get_value(  ) const 
{
    if( !entries.size() )
        return empty_string;
    if( index < entries.size(  ) )
        return entries[index];
    throw MHA_Error( __FILE__, __LINE__, "No valid entry" );
}

//! Return keyword list.
const std::vector < std::string > &MHAParser::keyword_list_t::get_entries(  ) const 
{
    return entries;
}

//! Return index of selected value.
const MHAParser::keyword_list_t::size_t & 
MHAParser::keyword_list_t::get_index(  ) const 
{
    return index;
}

//! Check if index of selected value is valid.
void MHAParser::keyword_list_t::validate(  ) const 
{
    if( !entries.size() )
        return;
    if( index >= entries.size(  ) )
        throw MHA_Error( __FILE__, __LINE__, "Keyword is invalid." );
}

//! Constructor.
MHAParser::keyword_list_t::keyword_list_t(  )
    :index( 0 )
{
    entries.clear(  );
}

/**************************************************************************/
/*  string_t                                                            **/
/**************************************************************************/

/** Constructor of a \mha configuration variable for string values.
 * @param h A help string describing the purpose of this variable.
 * @param v The initial string value */
MHAParser::string_t::string_t( const std::string & h, const std::string & v )
    : variable_t( h ), data( v )
{
    data_is_initialized = true;
}

std::string MHAParser::string_t::op_setval( expression_t & x )
{
    variable_t::op_setval( x );
    std::string oldval( data );
    data = x.rval;
    try {
        writeaccess(  );
        writeaccess( x.rval );
        if( data_is_initialized )
            if( data != oldval )
                valuechanged();
    }
    catch( ... ) {
        data = oldval;
        throw;
    }
    return "";
}

std::string MHAParser::string_t::query_type( const std::string & s )
{
    return "string";
}

std::string MHAParser::string_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = data;
    readaccess(  );readaccess( s );
    return tmp;
}

/**************************************************************************/
/*  int_t                                                                 */
/**************************************************************************/

MHAParser::int_t::int_t(const std::string & help_text,
                        const std::string & initial_value,
                        const std::string & range)
    :  range_var_t(help_text, range), data(0)
{
    parse("=" + initial_value);
    data_is_initialized = true;
}

std::string MHAParser::int_t::op_setval( expression_t & x )
{
    variable_t::op_setval( x );
    int newval( data );
    int oldval( data );
    StrCnv::str2val( x.rval, newval );
    validate( newval );
    data = newval;
    try {
        writeaccess(  );
        writeaccess( x.rval );
        if( data_is_initialized )
            if( data != oldval )
                valuechanged();
    }
    catch( ... ) {
        data = oldval;
        throw;
    }
    return "";
}

std::string MHAParser::int_t::query_val( const std::string & s)
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::int_t::query_type( const std::string & s )
{
    return "int";
}

/**************************************************************************/
/*  kw_t                                                                **/
/**************************************************************************/

bool MHAParser::kw_t::isval(const std::string& testval) const
{
    return data.get_value() == testval;
}

/** @param r A string containing the list of valid entries.
 * The entries have to be separated by spaces.  The list of entries
 * has to be delimited by brackets "[", "]". */
void MHAParser::kw_t::set_range( const std::string & r )
{
    data.set_entries( r );
}

void MHAParser::kw_t::validate( const keyword_list_t & s )
{
    s.validate(  );
}

std::string MHAParser::kw_t::query_range( const std::string & s )
{
    return StrCnv::val2str( data.get_entries(  ) );
}

/** Constructor of a keyword list \mha configuration variable.
 * @param h A help string describing the purpose of this variable.
 * @param v The initial value, has to be a value from the list of
 *           possible values given in the last parameter.
 * @param rg A string containing the list of valid entries.
 *           The entries have to be separated by spaces.  The list of entries
 *           has to be delimited by brackets "[", "]". */
MHAParser::kw_t::kw_t( const std::string & h, const std::string & v, const std::string & rg )
    :variable_t( h )
{
    activate_query( "range", &base_t::query_range );
    set_range( rg );
    parse( "=" + v );
    data_is_initialized = true;
}

MHAParser::kw_t::kw_t( const kw_t & src )
    :variable_t( src ),
     data(src.data)
{
    activate_query( "range", &base_t::query_range );
}

std::string MHAParser::kw_t::query_val( const std::string & s)
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::kw_t::query_type( const std::string & s )
{
    return "keyword_list";
}

std::string MHAParser::kw_t::op_setval( expression_t & x )
{
    variable_t::op_setval( x );
    keyword_list_t newval( data );
    keyword_list_t oldval( data );
    StrCnv::str2val( x.rval, newval );
    validate( newval );
    data = newval;
    try {
        writeaccess(  );
        writeaccess( x.rval );
        if( data_is_initialized )
            if( data.get_index() != oldval.get_index() )
                valuechanged();
    }
    catch( ... ) {
        data = oldval;
        throw;
    }
    return "";
}


MHAParser::float_t::float_t(const std::string & help_text,
                            const std::string & initial_value,
                            const std::string & range)
    :range_var_t( help_text, range )
{
    parse("=" + initial_value);
    data_is_initialized = true;
}

std::string MHAParser::float_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::float_t::query_type( const std::string & s )
{
    return "float";
}

std::string MHAParser::float_t::op_setval( expression_t & x )
{
    variable_t::op_setval( x );
    float newval( data );
    float oldval( data );
    StrCnv::str2val( x.rval, newval );
    validate( newval );
    data = newval;
    try {
        writeaccess(  );
        writeaccess( x.rval );
        if( data_is_initialized )
            if( data != oldval )
                valuechanged();
    }
    catch( ... ) {
        data = oldval;
        throw;
    }
    return "";
}


MHAParser::complex_t::complex_t( const std::string & h, const std::string & v, const std::string & rg )
    :range_var_t( h, rg )
{
    parse( "=" + v );
    data_is_initialized = true;
}

std::string MHAParser::complex_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::complex_t::query_type( const std::string & s )
{
    return "complex";
}

std::string MHAParser::complex_t::op_setval( expression_t & x )
{
    variable_t::op_setval( x );
    mha_complex_t newval( data );
    mha_complex_t oldval( data );
    StrCnv::str2val( x.rval, newval );
    validate( newval );
    data = newval;
    try {
        writeaccess(  );
        writeaccess( x.rval );
        if( data_is_initialized )
            if( (data.re != oldval.re) || (data.im != oldval.im) )
                valuechanged();
    }
    catch( ... ) {
        data = oldval;
        throw;
    }
    return "";
}


/****************************************************************************/
/* vint                                                                    **/
/****************************************************************************/

/** Constructor.
 * @param h help string
 * @param v initial value
 * @param rg optional: range constraint for all elements */
MHAParser::vint_t::vint_t( const std::string & h, const std::string & v, const std::string & rg )
    :range_var_t( h, rg )
{
    parse( "=" + v );
    data_is_initialized = true;
}

std::string MHAParser::vint_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::vint_t::query_type( const std::string & s )
{
    return "vector<int>";
}

std::string MHAParser::vint_t::op_setval( expression_t & x )
{
    variable_t::op_setval( x );
    std::vector < int >newval( data );
    std::vector < int >oldval( data );
    StrCnv::str2val( x.rval, newval );
    validate( newval );
    data = newval;
    try {
        writeaccess(  );
        writeaccess( x.rval );
        if( data_is_initialized )
            if( data != oldval )
                valuechanged();
    }
    catch( ... ) {
        data = oldval;
        throw;
    }
    return "";
}

/****************************************************************************/
/* vfloat                                                                    **/
/****************************************************************************/

/** Create a float vector parser variable.
 * * @param h A human-readable text describing the purpose of this configuration variable.
 * @param v The initial value of the variable, as a string, in \mha configuration language:
 *  (e.g. "[0 1 2.1 3]" for a vector), described in the "Multidimensional Variables" s2.1.3 section of the \mha User Manual.
 * @param rg The numeric range to enforce on all members of the vector. */
MHAParser::vfloat_t::vfloat_t( const std::string & h, const std::string & v, const std::string & rg )
    :range_var_t( h, rg )
{
    parse( "=" + v );
    data_is_initialized = true;
}

std::string MHAParser::vfloat_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::vfloat_t::query_type( const std::string & s )
{
    return "vector<float>";
}

std::string MHAParser::vfloat_t::op_setval( expression_t & x )
{
    variable_t::op_setval( x );
    std::vector < float >newval( data );
    std::vector < float >oldval( data );
    StrCnv::str2val( x.rval, newval );
    validate( newval );
    data = newval;
    try {
        writeaccess(  );
        writeaccess( x.rval );
        if( data_is_initialized )
            if( data != oldval )
                valuechanged();
    }
    catch( ... ) {
        data = oldval;
        throw;
    }
    return "";
}

/****************************************************************************/
/* vcomplex                                                                    **/
/****************************************************************************/

MHAParser::vcomplex_t::vcomplex_t( const std::string & h, const std::string & v, const std::string & rg )
    :range_var_t( h, rg )
{
    parse( "=" + v );
    data_is_initialized = true;
}

std::string MHAParser::vcomplex_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::vcomplex_t::query_type( const std::string & s )
{
    return "vector<complex>";
}

std::string MHAParser::vcomplex_t::op_setval( expression_t & x )
{
    variable_t::op_setval( x );
    std::vector < mha_complex_t > newval( data );
    std::vector < mha_complex_t > oldval( data );
    StrCnv::str2val( x.rval, newval );
    validate( newval );
    data = newval;
    try {
        writeaccess(  );
        writeaccess( x.rval );
        if( data_is_initialized ){
            if( data.size() != oldval.size() )
                valuechanged();
            else{
                for( unsigned int k=0;k<data.size();k++ )
                    if( (data[k].re != oldval[k].re)||(data[k].im != oldval[k].im) )
                        valuechanged();
            }
        }
    }
    catch( ... ) {
        data = oldval;
        throw;
    }
    return "";
}



/** Create a int matrix parser variable.
 * @param h A human-readable text describing the purpose of this configuration variable.
 * @param v The initial value of the variable, as a string, in \mha configuration language:
 *  (e.g. "[[0 1]; [2 3]]" for a matrix), described in the "Multidimensional Variables" s2.1.3 section of the \mha User Manual.
 * @param rg The numeric range to enforce on all members of the matrix. */
MHAParser::mint_t::mint_t( const std::string & h, const std::string & v, const std::string & rg )
    :range_var_t( h, rg )
{
    parse( "=" + v );
    data_is_initialized = true;
}

std::string MHAParser::mint_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::mint_t::query_type( const std::string & s )
{
    return "matrix<int>";
}

std::string MHAParser::mint_t::op_setval( expression_t & x )
{
    variable_t::op_setval( x );
    std::vector < std::vector < int > > newval( data );
    std::vector < std::vector < int > > oldval( data );
    StrCnv::str2val( x.rval, newval );
    validate( newval );
    data = newval;
    try {
        writeaccess(  );
        writeaccess( x.rval );
        if( data_is_initialized )
            if( data != oldval )
                valuechanged();
    }
    catch( ... ) {
        data = oldval;
        throw;
    }
    return "";
}

/****************************************************************************/
/* vvfloat                                                                    **/
/****************************************************************************/

/** Create a float matrix parser variable.
 * @param h A human-readable text describing the purpose of this configuration variable.
 * @param v The initial value of the variable, as a string, in \mha configuration language:
 *  (e.g. "[[0 1]; [2 3]]" for a matrix), described in the "Multidimensional Variables" s2.1.3 section of the \mha User Manual.
 * @param rg The numeric range to enforce on all members of the matrix. */
MHAParser::mfloat_t::mfloat_t( const std::string & h, const std::string & v, const std::string & rg )
    :range_var_t( h, rg )
{
    parse( "=" + v );
    data_is_initialized = true;
}

std::string MHAParser::mfloat_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::mfloat_t::query_type( const std::string & s )
{
    return "matrix<float>";
}

std::string MHAParser::mfloat_t::op_setval( expression_t & x )
{
    variable_t::op_setval( x );
    std::vector < std::vector < float > > newval( data );
    std::vector < std::vector < float > > oldval( data );
    StrCnv::str2val( x.rval, newval );
    validate( newval );
    data = newval;
    try {
        writeaccess(  );
        writeaccess( x.rval );
        if( data_is_initialized )
            if( data != oldval )
                valuechanged();
    }
    catch( ... ) {
        data = oldval;
        throw;
    }
    return "";
}

/****************************************************************************/
/* vvcomplex                                                               **/
/****************************************************************************/

MHAParser::mcomplex_t::mcomplex_t( const std::string & h, const std::string & v, const std::string & rg )
    :range_var_t( h, rg )
{
    parse( "=" + v );
    data_is_initialized = true;
}

std::string MHAParser::mcomplex_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::mcomplex_t::query_type( const std::string & s )
{
    return "matrix<complex>";
}

std::string MHAParser::mcomplex_t::op_setval( expression_t & x )
{
    variable_t::op_setval( x );
    std::vector < std::vector < mha_complex_t > > newval( data );
    std::vector < std::vector < mha_complex_t > > oldval( data );
    StrCnv::str2val( x.rval, newval );
    validate( newval );
    data = newval;
    try {
        writeaccess(  );
        writeaccess( x.rval );
        if( data_is_initialized )
            if( data != oldval )
                valuechanged();
    }
    catch( ... ) {
        data = oldval;
        throw;
    }
    return "";
}

/****************************************************************************/
/* vstring                                                                    **/
/****************************************************************************/

MHAParser::vstring_t::vstring_t( const std::string & h, const std::string & v )
    :variable_t( h )
{
    parse( "=" + v );
    data_is_initialized = true;
}

std::string MHAParser::vstring_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::vstring_t::query_type( const std::string & s )
{
    return "vector<string>";
}

std::string MHAParser::vstring_t::op_setval( expression_t & x )
{
    variable_t::op_setval( x );
    std::vector < std::string > newval( data );
    std::vector < std::string > oldval( data );
    StrCnv::str2val( x.rval, newval );
    data = newval;
    try {
        writeaccess(  );
        writeaccess( x.rval );
        if( data_is_initialized )
            if( data != oldval )
                valuechanged();
    }
    catch( ... ) {
        data = oldval;
        throw;
    }
    return "";
}


/****************************************************************************/
/* bool                                                                   **/
/****************************************************************************/

MHAParser::bool_t::bool_t(const std::string & help_text,
                          const std::string & initial_value)
    :variable_t(help_text)
{
    parse("=" + initial_value);
    data_is_initialized = true;
}

std::string MHAParser::bool_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::bool_t::query_type( const std::string & s )
{
    return "bool";
}

std::string MHAParser::bool_t::op_setval( expression_t & x )
{
    variable_t::op_setval( x );
    bool newval( data );
    bool oldval( data );
    StrCnv::str2val( x.rval, newval );
    data = newval;
    try {
        writeaccess(  );
        writeaccess( x.rval );
        if( data_is_initialized )
            if( data != oldval )
                valuechanged();
    }
    catch( ... ) {
        data = oldval;
        throw;
    }
    return "";
}


/**************************************************************************/
/*  range_var_t                                                         **/
/**************************************************************************/

MHAParser::range_var_t::range_var_t( const std::string & h, const std::string & r )
    :variable_t( h )
{
    activate_query( "range", &base_t::query_range );
    set_range( r );
}

MHAParser::range_var_t::range_var_t( const range_var_t & src )
    :variable_t( src ),
     low_limit(src.low_limit),
     up_limit(src.up_limit),
     low_incl(src.low_incl),
     up_incl(src.up_incl),
     check_low(src.check_low),
     check_up(src.check_up),
     check_range(src.check_range)
 
{
    activate_query( "range", &base_t::query_range );
}

void MHAParser::range_var_t::set_range( const std::string & r )
{
    if( r.size(  ) == 0 ) {
        check_range = false;
        return;
    }
    std::string rng( r );
    unsigned int p1 = rng.find_first_of( "[]" );
    unsigned int p2 = rng.find( "," );
    unsigned int p3 = rng.find_last_of( "[]" );
    if( ( p1 != 0 ) || ( p3 != rng.size(  ) - 1 ) || ( p2 >= rng.size(  ) ) ) {
        throw MHA_Error( __FILE__, __LINE__, "Invalid range string: \"%s\"", r.c_str(  ) );
    }
    low_incl = ( rng[p1] == '[' );
    up_incl = ( rng[p3] == ']' );
    std::string val = rng.substr( p1 + 1, p2 - p1 - 1 );
    if( val.size(  ) ) {
        StrCnv::str2val( val, low_limit );
        check_low = true;
    } else
        check_low = false;
    val = rng.substr( p2 + 1, p3 - p2 - 1 );
    if( val.size(  ) ) {
        StrCnv::str2val( val, up_limit );
        check_up = true;
    } else
        check_up = false;
    if( check_low || check_up )
        check_range = true;
    else
        check_range = false;
}

std::string MHAParser::range_var_t::query_range( const std::string & s )
{
    if( !check_range )
        return "";
    std::string r( "" );
    if( low_incl )
        r += "[";
    else
        r += "]";
    if( check_low )
        r += StrCnv::val2str( low_limit );
    r += ",";
    if( check_up )
        r += StrCnv::val2str( up_limit );
    if( up_incl )
        r += "]";
    else
        r += "[";
    return r;
}

void MHAParser::range_var_t::validate( const int &v )
{
    if( !check_range ) {
        return;
    }
    bool ok( true );
    if( check_low ) {
        if( low_incl ) {
            if( v < low_limit )
                ok = false;
        } else {
            if( v <= low_limit )
                ok = false;
        }
    }
    if( check_up ) {
        if( up_incl ) {
            if( v > up_limit )
                ok = false;
        } else {
            if( v >= up_limit )
                ok = false;
        }
    }
    if( !ok ) {
        std::string rg = query_range( "" );
        std::string vl = StrCnv::val2str( v );
        throw MHA_Error( __FILE__, __LINE__, "The value %s is not in the range %s.", vl.c_str(  ), rg.c_str(  ) );
    }
}

void MHAParser::range_var_t::validate( const float &v )
{
    if( !check_range ) {
        return;
    }
    bool ok( true );
    if( check_low ) {
        if( low_incl ) {
            if( v < low_limit )
                ok = false;
        } else {
            if( v <= low_limit )
                ok = false;
        }
    }
    if( check_up ) {
        if( up_incl ) {
            if( v > up_limit )
                ok = false;
        } else {
            if( v >= up_limit )
                ok = false;
        }
    }
    if( !ok ) {
        std::string rg = query_range( "" );
        std::string vl = StrCnv::val2str( v );
        throw MHA_Error( __FILE__, __LINE__, "The value %s is not in the range %s.", vl.c_str(  ), rg.c_str(  ) );
    }
}

void MHAParser::range_var_t::validate( const mha_complex_t & v )
{
    if( !check_range ) {
        return;
    }
    // valid range if real and imaginary part independently are valid:
    validate( v.re );
    validate( v.im );
    // (other valid ranges imaginable, e.g. abs(v))
}

void MHAParser::range_var_t::validate( const std::vector < int >&v )
{
    if( !check_range )
        return;
    bool ok( true );
    for( unsigned int k = 0; k < v.size(  ); k++ ) {
        if( check_low ) {
            if( low_incl ) {
                if( v[k] < low_limit )
                    ok = false;
            } else {
                if( v[k] <= low_limit )
                    ok = false;
            }
        }
        if( check_up ) {
            if( up_incl ) {
                if( v[k] > up_limit )
                    ok = false;
            } else {
                if( v[k] >= up_limit )
                    ok = false;
            }
        }
        if( !ok ) {
            std::string rg = query_range( "" );
            std::string vl = StrCnv::val2str( v );
            std::string vl1 = StrCnv::val2str( v[k] );
            throw MHA_Error( __FILE__, __LINE__,
                             "The entry at index %u of %s (value: %s) is not in the range %s.",
                             k + 1, vl.c_str(  ), vl1.c_str(  ), rg.c_str(  ) );
        }
    }
}

void MHAParser::range_var_t::validate( const std::vector < float >&v )
{
    if( !check_range )
        return;
    bool ok( true );
    for( unsigned int k = 0; k < v.size(  ); k++ ) {
        if( check_low ) {
            if( low_incl ) {
                if( v[k] < low_limit )
                    ok = false;
            } else {
                if( v[k] <= low_limit )
                    ok = false;
            }
        }
        if( check_up ) {
            if( up_incl ) {
                if( v[k] > up_limit )
                    ok = false;
            } else {
                if( v[k] >= up_limit )
                    ok = false;
            }
        }
        if( !ok ) {
            std::string rg = query_range( "" );
            std::string vl = StrCnv::val2str( v );
            std::string vl1 = StrCnv::val2str( v[k] );
            throw MHA_Error( __FILE__, __LINE__,
                             "The entry at index %u of %s (value: %s) is not in the range %s.",
                             k + 1, vl.c_str(  ), vl1.c_str(  ), rg.c_str(  ) );
        }
    }
}

void MHAParser::range_var_t::validate( const std::vector < mha_complex_t > &v )
{
    if( !check_range )
        return;
    for( unsigned int k = 0; k < v.size(  ); k++ ) {
        try {
            validate( v[k] );
        }
        catch( MHA_Error & e ) {
            std::string tmp = StrCnv::val2str( v );
            throw MHA_Error( __FILE__, __LINE__, "%s\n(while scanning entry at index %u of %s)", Getmsg( e ), k, tmp.c_str(  ) );
        }
    }
}

void MHAParser::range_var_t::validate(const std::vector<std::vector<int> > &v) {
    if( !check_range )
        return;
    bool ok( true );
    for( unsigned int k = 0; k < v.size(  ); k++ ) {
        for( unsigned int k2 = 0; k2 < v[k].size(  ); k2++ ) {
            if( check_low ) {
                if( low_incl ) {
                    if( v[k][k2] < low_limit )
                        ok = false;
                } else {
                    if( v[k][k2] <= low_limit )
                        ok = false;
                }
            }
            if( check_up ) {
                if( up_incl ) {
                    if( v[k][k2] > up_limit )
                        ok = false;
                } else {
                    if( v[k][k2] >= up_limit )
                        ok = false;
                }
            }
            if( !ok ) {
                std::string rg = query_range( "" );
                std::string vl1 = StrCnv::val2str( v[k][k2] );
                throw MHA_Error( __FILE__, __LINE__,
                                 "The entry at index (%u,%u) (value: %s) is not in the range %s.",
                                 k + 1, k2 + 1, vl1.c_str(  ), rg.c_str(  ) );
            }
        }
    }
}

void MHAParser::range_var_t::validate( const std::vector < std::vector < float > > &v )
{
    if( !check_range )
        return;
    bool ok( true );
    for( unsigned int k = 0; k < v.size(  ); k++ ) {
        for( unsigned int k2 = 0; k2 < v[k].size(  ); k2++ ) {
            if( check_low ) {
                if( low_incl ) {
                    if( v[k][k2] < low_limit )
                        ok = false;
                } else {
                    if( v[k][k2] <= low_limit )
                        ok = false;
                }
            }
            if( check_up ) {
                if( up_incl ) {
                    if( v[k][k2] > up_limit )
                        ok = false;
                } else {
                    if( v[k][k2] >= up_limit )
                        ok = false;
                }
            }
            if( !ok ) {
                std::string rg = query_range( "" );
                std::string vl1 = StrCnv::val2str( v[k][k2] );
                throw MHA_Error( __FILE__, __LINE__,
                                 "The entry at index (%u,%u) (value: %s) is not in the range %s.",
                                 k + 1, k2 + 1, vl1.c_str(  ), rg.c_str(  ) );
            }
        }
    }
}

void MHAParser::range_var_t::validate( const std::vector< std::vector < mha_complex_t > > &v )
{
    if( !check_range )
        return;
    for( unsigned int k = 0; k < v.size(  ); k++ ) {
        for (unsigned k2 = 0; k2 < v[k].size(); ++k2) {
            try {
                validate( v[k][k2] );
            }
            catch( MHA_Error & e ) {
                std::string tmp = StrCnv::val2str( v );
                throw MHA_Error( __FILE__, __LINE__, "%s\n(while scanning entry at 1-based index (%u,%u)", Getmsg(e), k+1u, k2+1u );
            }
        }
    }
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/*                                                                         **/
/*   MHAParser::Monitors                                                   **/
/*                                                                         **/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/

MHAParser::int_mon_t::int_mon_t( const std::string & hlp )
    : monitor_t( hlp ), data(0)
{
    data_is_initialized = true;
}

MHAParser::float_mon_t::float_mon_t( const std::string & hlp )
    : monitor_t( hlp ),
      data(0)
{
    data_is_initialized = true;
}

MHAParser::complex_mon_t::complex_mon_t( const std::string & hlp )
    : monitor_t( hlp ), data(mha_complex(0,0))
{
    data_is_initialized = true;
}
MHAParser::vcomplex_mon_t::vcomplex_mon_t( const std::string & hlp ) : monitor_t( hlp )
{
    data_is_initialized = true;
}

MHAParser::vint_mon_t::vint_mon_t( const std::string & hlp ):monitor_t( hlp )
{
    data_is_initialized = true;
}

MHAParser::mint_mon_t::mint_mon_t( const std::string & hlp ):monitor_t( hlp )
{
    data_is_initialized = true;
}

MHAParser::vfloat_mon_t::vfloat_mon_t( const std::string & hlp ):monitor_t( hlp )
{
    data_is_initialized = true;
}

MHAParser::mfloat_mon_t::mfloat_mon_t( const std::string & hlp ):monitor_t( hlp )
{
    data_is_initialized = true;
}

MHAParser::mcomplex_mon_t::mcomplex_mon_t( const std::string & hlp ):monitor_t( hlp )
{
    data_is_initialized = true;
}

MHAParser::string_mon_t::string_mon_t( const std::string & hlp )
    : monitor_t( hlp ), data("")
{
    data_is_initialized = true;
}

MHAParser::bool_mon_t::bool_mon_t( const std::string & hlp )
    : monitor_t( hlp ), data(false)
{
    data_is_initialized = true;
}

MHAParser::vstring_mon_t::vstring_mon_t( const std::string & hlp ) 
    : monitor_t( hlp )
{
    data_is_initialized = true;
}

std::string MHAParser::int_mon_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::int_mon_t::query_type( const std::string & s )
{
    return "int";
}

std::string MHAParser::vint_mon_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::vint_mon_t::query_type( const std::string & s )
{
    return "vector<int>";
}

std::string MHAParser::mint_mon_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::mint_mon_t::query_type( const std::string & s )
{
    return "matrix<int>";
}

std::string MHAParser::float_mon_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::float_mon_t::query_type( const std::string & s )
{
    return "float";
}

std::string MHAParser::complex_mon_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::complex_mon_t::query_type( const std::string & s )
{
    return "complex";
}

std::string MHAParser::vcomplex_mon_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::vcomplex_mon_t::query_type( const std::string & s )
{
    return "vector<complex>";
}

std::string MHAParser::vfloat_mon_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::vfloat_mon_t::query_type( const std::string & s )
{
    return "vector<float>";
}

std::string MHAParser::mfloat_mon_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::mcomplex_mon_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::mfloat_mon_t::query_type( const std::string & s )
{
    return "matrix<float>";
}

std::string MHAParser::mcomplex_mon_t::query_type( const std::string & s )
{
    return "matrix<complex>";
}

std::string MHAParser::bool_mon_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::bool_mon_t::query_type( const std::string & s )
{
    return "bool";
}

std::string MHAParser::string_mon_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::string_mon_t::query_type( const std::string & s )
{
    return "string";
}

std::string MHAParser::vstring_mon_t::query_val( const std::string & s )
{
    prereadaccess(  );prereadaccess( s );
    std::string tmp = StrCnv::val2str( data );
    readaccess(  );readaccess( s );
    return tmp;
}

std::string MHAParser::vstring_mon_t::query_type( const std::string & s )
{
    return "vector<string>";
}


/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/*                                                                         **/
/*   MHAParser::c_ifc_parser                                              **/
/*                                                                         **/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/


MHAParser::c_ifc_parser_t::c_ifc_parser_t( const std::string& modulename_ )
    :  MHAParser::base_t( "variables of external parser" ),
       modulename(modulename_),
       c_parse_cmd( NULL ), 
       c_parse_err( NULL ), 
       liberr( 0 ), 
       libdata( NULL ), 
       ret_size( DEFAULT_RETSIZE ), 
       retv( NULL )
{
    retv = new char[ret_size];
}

MHAParser::c_ifc_parser_t::~c_ifc_parser_t(  )
{
    if( retv )
        delete[] retv;
}

void MHAParser::c_ifc_parser_t::set_parse_cb( MHAParser::c_parse_cmd_t cb, MHAParser::c_parse_err_t strerr, void *d )
{
    c_parse_cmd = cb;
    c_parse_err = strerr;
    libdata = d;
}

std::string MHAParser::c_ifc_parser_t::op_setval( MHAParser::expression_t & x )
{
    if( c_parse_cmd ) {
        if( retv )
            *retv = 0;
        std::string s = x.lval + x.op + x.rval;
        liberr = c_parse_cmd( libdata, s.c_str(  ), retv, ret_size );
        test_error(  );
        return retv;
    }
    throw MHA_Error( __FILE__, __LINE__, "No parse callback defined." );
}

std::string MHAParser::c_ifc_parser_t::op_query( MHAParser::expression_t & x )
{
    if( c_parse_cmd ) {
        if( retv )
            *retv = 0;
        std::string s = x.lval + x.op + x.rval;
        liberr = c_parse_cmd( libdata, s.c_str(  ), retv, ret_size );
        test_error(  );
        return retv;
    }
    throw MHA_Error( __FILE__, __LINE__, "No parse callback defined." );
}

std::string MHAParser::c_ifc_parser_t::op_subparse( MHAParser::expression_t & x )
{
    if( c_parse_cmd ) {
        if( retv )
            *retv = 0;
        std::string s = x.lval + x.op + x.rval;
        liberr = c_parse_cmd( libdata, s.c_str(  ), retv, ret_size );
        test_error(  );
        return retv;
    }
    throw MHA_Error( __FILE__, __LINE__, "No parse callback defined." );
}

void MHAParser::c_ifc_parser_t::test_error(  )
{
    if( liberr != 0 ) {
        if( c_parse_err )
            throw MHA_Error( __FILE__, __LINE__, 
                             "Parser error in %s:\n%s", 
                             modulename.c_str(), 
                             c_parse_err( libdata, liberr ) );
        else
            throw MHA_Error( __FILE__, __LINE__, "error %d", liberr );
    }
}

template<> void MHAParser::StrCnv::str2val<mha_real_t>( const std::string & s, std::vector<mha_real_t>& v )
{
    mha_real_t tmpval;
    std::vector<mha_real_t> val;
    switch( num_brackets( s ) ){
    case -1 : // empty string, error
        throw MHA_Error(__FILE__,__LINE__,"Empty string \"%s\"",s.c_str());
        // no break; needed here, the break would be unreachable.
    case 0 : // no brackets, scalar
    {
        MHAParser::StrCnv::str2val( s, tmpval );
        val.push_back( tmpval );
        v = val;
        break;
    }
    case 2 : // both brackets, vector
    {
        std::string fv;
        std::istringstream tmp(s.substr(1,s.size()-2) + std::string(" "));
        while( tmp >> fv ) {
            if( fv.find(":") < fv.size() ){
                MHAParser::expression_t exp1( fv, ":");
                MHAParser::expression_t exp2( exp1.rval, ":" );
                mha_real_t tmp_start;
                mha_real_t tmp_inc;
                mha_real_t tmp_end;
                MHAParser::StrCnv::str2val( exp1.lval, tmp_start );
                MHAParser::StrCnv::str2val( exp2.lval, tmp_inc );
                MHAParser::StrCnv::str2val( exp2.rval, tmp_end );
                if( (tmp_inc > 0) && (tmp_end > tmp_start) ){
                    for( mha_real_t tmp_value=tmp_start;tmp_value<=tmp_end;tmp_value+=tmp_inc)
                        val.push_back(tmp_value);
                }else throw MHA_Error(__FILE__,__LINE__,"Invalid start:inc:end expression (%g:%g:%g).",tmp_start,tmp_inc,tmp_end);
            }else{
                MHAParser::StrCnv::str2val( fv, tmpval );
                val.push_back( tmpval );
            }
        }
        v = val;
        break;
    }
    default :
        throw MHA_Error(__FILE__,__LINE__,"Invalid brackets: %s",s.c_str());
    }
}

MHAParser::mhaconfig_mon_t::mhaconfig_mon_t(const std::string& help)
    : MHAParser::parser_t(help),
      channels("Number of audio channels"),
      domain("Signal domain (MHA_WAVEFORM or MHA_SPECTRUM)"),
      fragsize("Fragment size of waveform data"),
      wndlen("Window length of spectral data"),
      fftlen("FFT length of spectral data"),
      srate("Sampling rate in Hz")
{
    insert_member(channels);
    insert_member(domain);
    insert_member(fragsize);
    insert_member(wndlen);
    insert_member(fftlen);
    insert_member(srate);
}

void MHAParser::mhaconfig_mon_t::update(const mhaconfig_t& cf)
{
    channels.data = cf.channels;
    switch( cf.domain ){
    case MHA_WAVEFORM: 
        domain.data = "MHA_WAVEFORM"; 
        break;
    case MHA_SPECTRUM:
        domain.data = "MHA_SPECTRUM";
        break;
    default:
        domain.data = "unknown";
    }
    fragsize.data = cf.fragsize;
    wndlen.data = cf.wndlen;
    fftlen.data = cf.fftlen;
    srate.data = cf.srate;
}

// Local Variables:
// compile-command: "make -C .."
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
