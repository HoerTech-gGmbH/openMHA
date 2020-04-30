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

#ifndef _MHA_PARSER_HH_
#define _MHA_PARSER_HH_

#include <string>
#include <list>
#include <vector>
#include <map>
#include "mha.hh"
#include "mha_events.h"
#include <typeinfo>

// A buffer of this size is allocated for every hierarchy level of
// every parser request.  As hierarchy levels are traveled through,
// string contents is copied from one buffer to the next.
#define DEFAULT_RETSIZE 0x100000 // 1 MegaByte

namespace MHAParser {

    class keyword_list_t;
    class base_t;
    class parser_t;

    std::string commentate(const std::string& s);
    void trim(std::string& s);
    std::string cfg_dump(base_t*,const std::string&);
    std::string cfg_dump_short(base_t*,const std::string&);
    std::string all_dump(base_t*,const std::string&);
    std::string mon_dump(base_t*,const std::string&);
    std::string all_ids(base_t*,const std::string&,const std::string& = "");
    void strreplace(std::string&,const std::string&,const std::string&);
    void envreplace( std::string & s );

    /** 
        
    \brief String converter namespace

    The functions defined in this namespace manage the conversions
    from C++ variables to strings and back. It was tried to keep a
    matlab compatible string format for vectors and vectors of
    vectors.
    */
    namespace StrCnv {
        void str2val(const std::string&,bool&);///< \brief Convert from string
        void str2val(const std::string&,float&);///< \brief Convert from string
        void str2val(const std::string&,mha_complex_t&);///< \brief Convert from string
        void str2val(const std::string&,int&);///< \brief Convert from string
        void str2val(const std::string&,keyword_list_t&);///< \brief Convert from string
        void str2val(const std::string&,std::string&);///< \brief Convert from string
        template<class arg_t> void str2val(const std::string& s,std::vector<arg_t>& val);///< \brief Converter for vector types
        template<> void str2val<mha_real_t>( const std::string & s, std::vector<mha_real_t>& v );///< \brief Converter for vector<mha_real_t> with Matlab-style expansion
        template<class arg_t> void str2val(const std::string& s,std::vector<std::vector<arg_t> >& val);///< \brief Converter for matrix types


        std::string val2str(const bool&);///< \brief Convert to string
        std::string val2str(const float&);///< \brief Convert to string
        std::string val2str(const mha_complex_t&);///< \brief Convert to string
        std::string val2str(const int&);///< \brief Convert to string
        std::string val2str(const keyword_list_t&);///< \brief Convert to string
        std::string val2str(const std::string&);///< \brief Convert to string
        std::string val2str(const std::vector<float>&);///< \brief Convert to string
        std::string val2str(const std::vector<mha_complex_t>&);///< \brief Convert to string
        std::string val2str(const std::vector<int>&);///< \brief Convert to string
        std::string val2str(const std::vector<std::vector<int> >&);///< \brief Convert to string
        std::string val2str(const std::vector<std::string>&);///< \brief Convert to string
        std::string val2str(const std::vector<std::vector<float> >&);///< \brief Convert to string
        std::string val2str(const std::vector<std::vector<mha_complex_t> >&);///< \brief Convert to string

        int num_brackets(const std::string& s); ///< \brief count number of brackets
    }

    //! Keyword list class.
    /*!
      
    The stucture keyword_list_t defines a keyword list (vector of
    strings) with an index into the list. Used as MHAParser::kw_t, it
    can be used to access a set of valid keywords through the parser
    (i.e. one of "pear apple banana").
    
    */
    class keyword_list_t {
    public:
        typedef std::vector<std::string>::size_type size_t;

        void set_value(const std::string&);
        void set_entries(const std::string&);
        const std::string& get_value() const;
        const std::vector<std::string>& get_entries() const;
        const size_t& get_index() const;
        void set_index(unsigned int);
        void validate() const;
        void add_entry(const std::string& en){entries.push_back(en);};
        keyword_list_t();
    private:
        size_t index;     //!< Index into list.
        std::vector<std::string> entries; //!< List of valid entries.
        std::string empty_string;
    };

    class expression_t {
    public:
        expression_t(const std::string&,const std::string&);
        expression_t();
        std::string lval;
        std::string rval;
        std::string op;
    };

    class entry_t {
    public:
        entry_t(const std::string&,base_t*);
        std::string name;
        base_t* entry;
    };

    typedef std::string (base_t::*opact_t)(expression_t&);
    typedef std::string (base_t::*query_t)(const std::string&);
    typedef std::map<std::string,opact_t> opact_map_t;
    typedef std::map<std::string,query_t> query_map_t;
    typedef std::list<entry_t> entry_map_t;


    /** \brief Base class for all parser items

    The key method of the parser base class is the std::string
    parse(const std::string&) method. Parser proxy derivatives which
    overwrite any of the other parse() methods to be the key method
    must make sure that the original parse() method utilizes the new
    key method.

     */
    class base_t {
    public:
        base_t(const std::string&);
        base_t(const base_t&);
        virtual ~base_t();
        virtual std::string parse(const std::string&);
        virtual void parse(const char*,char*,unsigned int);
        virtual void parse(const std::vector<std::string>&,std::vector<std::string>&);
        virtual std::string op_subparse(expression_t&);
        virtual std::string op_setval(expression_t&);
        virtual std::string op_query(expression_t&);
        virtual std::string query_dump(const std::string&);
        virtual std::string query_entries(const std::string&);
        virtual std::string query_perm(const std::string&);
        virtual std::string query_range(const std::string&);
        virtual std::string query_type(const std::string&);
        virtual std::string query_val(const std::string&);
        virtual std::string query_readfile(const std::string&);
        virtual std::string query_savefile(const std::string&);
        virtual std::string query_savefile_compact(const std::string&);
        virtual std::string query_savemons(const std::string&);
        virtual std::string query_listids(const std::string&);
        std::string query_version(const std::string&);
        std::string query_id(const std::string&);
        std::string query_subst(const std::string&);
        std::string query_addsubst(const std::string&);
        std::string query_help(const std::string&);
        std::string query_cmds(const std::string&);
    public:
        void set_node_id(const std::string&);
        void set_help(const std::string&);
        void add_parent_on_insert(parser_t*,std::string);
        void rm_parent_on_remove(parser_t*);
        const std::string& fullname() const;

        /// \brief Event emitted on write access.
        /// 
        /// To connect a callback that is invoked on write access to this 
        /// parser variable, use MHAEvents::patchbay_t<receiver_t> method
        /// connect(&writeaccess,&receiver_t::callback)
        /// where callback is a method that expects no parameters and returns
        /// void.
        MHAEvents::emitter_t writeaccess; 

        /// \brief Event emitted if the value has changed.
        /// 
        /// To connect a callback that is invoked when write access to this 
        /// parser variable actually changes its value, use 
        /// MHAEvents::patchbay_t<receiver_t> method 
        /// connect(&valuechanged,&receiver_t::callback)
        /// where callback is a method that expects no parameters and returns
        /// void.
        MHAEvents::emitter_t valuechanged;

        /// \brief Event emitted on read access.
        /// 
        /// To connect a callback that is invoked after the value of this 
        /// variable has been read through the configuration interface, use 
        /// MHAEvents::patchbay_t<receiver_t> method 
        /// connect(&readaccess,&receiver_t::callback)
        /// where callback is a method that expects no parameters and returns
        /// void.
        MHAEvents::emitter_t readaccess; 

        /// \brief Event emitted on read access,
        ///        before the data field is accessed.
        /// 
        /// To connect a callback that is invoked when the value of this 
        /// variable is about to be read through the configuration interface,
        /// so that the callback can influence the value that is reported,
        /// use MHAEvents::patchbay_t<receiver_t> method 
        /// connect(&prereadaccess,&receiver_t::callback)
        /// where callback is a method that expects no parameters and returns
        /// void.
        MHAEvents::emitter_t prereadaccess;
    protected:
        void activate_query(const std::string&,query_t);
        void notify();
        query_map_t queries;
        bool data_is_initialized;
    private:
        void add_replace_pair(const std::string&,const std::string&);
        class replace_t {
        public:
            replace_t(const std::string&,const std::string&);
            void replace(std::string&);
            const std::string& get_a() const {return a;};
            const std::string& get_b() const {return b;};
        private:
            std::string a,b;
        };
        std::string oplist();
        std::string help;
        std::string id_str;
        opact_map_t operators;
        typedef std::vector<replace_t> repl_list_t;
        repl_list_t repl_list;
        bool nested_lock;
        parser_t* parent;
        std::string thefullname;
    };

    /** \brief Parser node class
     *
     * A parser_t instance is a node in the configuration tree.  A
     * parser node can contain any number of other parser_t instances
     * or configuration language variables.  These items are inserted
     * into a parser node using the parser_t::insert_item method.
     */
    class parser_t : public base_t {
    public:
        /** Construct detached node to be used in the configuration tree.
         * \param help_text
         *   A text describing this node.  E.g. if this node lives at
         *   the root of some \mha plugin, then the help text should
         *   describe the functionality of the plugin.
         */
        parser_t(const std::string & help_text = "");
        ~parser_t();
        void insert_item(const std::string&,base_t*);
        void remove_item(const std::string&);
        void force_remove_item(const std::string&);
        void remove_item(const base_t*);
    protected:
        std::string op_subparse(expression_t&);
        std::string op_setval(expression_t&);
        std::string op_query(expression_t&);
        std::string query_type(const std::string&);
        std::string query_dump(const std::string&);
        std::string query_entries(const std::string&);
        std::string query_readfile(const std::string&);
        std::string query_savefile(const std::string&);
        std::string query_savefile_compact(const std::string&);
        std::string query_savemons(const std::string&);
        std::string query_val(const std::string&);
        std::string query_listids(const std::string&);
        void set_id_string(const std::string&);
        bool has_entry(const std::string&);
    private:
        entry_map_t entries;
        /** identification string */
        std::string id_string;
        std::string srcfile;
        unsigned int srcline;
        std::string last_errormsg;
    };

    typedef int (*c_parse_cmd_t)(void*,const char*,char*,unsigned int);
    typedef const char* (*c_parse_err_t)(void*,int);

    class c_ifc_parser_t : public base_t {
    public:
        c_ifc_parser_t(const std::string& modulename_);
        ~c_ifc_parser_t();
        void set_parse_cb(c_parse_cmd_t,c_parse_err_t,void*);
    protected:
        std::string op_subparse(MHAParser::expression_t&);
        std::string op_setval(MHAParser::expression_t&);
        std::string op_query(MHAParser::expression_t&);
    private:
        void test_error();
        std::string modulename;
        c_parse_cmd_t c_parse_cmd;
        c_parse_err_t c_parse_err;
        int liberr;
        void* libdata;
        unsigned int ret_size;
        char* retv;
    };

    /** 
        \brief Base class for monitors and variable nodes
    */
    class monitor_t : public base_t {
    public:
        monitor_t(const std::string&);
        monitor_t(const monitor_t&);
        std::string op_query(expression_t&);
        std::string query_dump(const std::string&);
        std::string query_perm(const std::string&);
    };

    /** 
        \brief Base class for variable nodes
    */
    class variable_t : public monitor_t {
    public:
        variable_t(const std::string&);
        std::string op_setval(expression_t&);
        std::string query_perm(const std::string&);
        void setlock(const bool&);
    private:
        bool locked;
    };

    /** \brief Base class for all variables with a numeric value range */
    class range_var_t : public variable_t {
    public:
        range_var_t(const std::string&,const std::string& ="");
        range_var_t(const range_var_t&);
        std::string query_range(const std::string&);
        /** \brief Change the valid range of a variable 
            \param r New range of the variable (string representation)
         */
        void set_range(const std::string& r);
        void validate(const int&);
        void validate(const float&);
        void validate(const mha_complex_t&);
        void validate(const std::vector<int>&);
        void validate(const std::vector<float>&);
        void validate(const std::vector<mha_complex_t>&);
        void validate(const std::vector<std::vector<int> >&);
        void validate(const std::vector<std::vector<float> >&);
        void validate(const std::vector<std::vector<mha_complex_t> >&);
    protected:
        float low_limit;//!< Lower limit of range.                          
        float up_limit; //!< Upper limit of range.                          
        bool low_incl;  //!< Lower limit is included (or excluded) in range.
        bool up_incl;   //!< Upper limit is included (or excluded) in range.
        bool check_low;  //!< Check lower limit
        bool check_up;   //!< Check upper limit
        bool check_range; //!< Range checking is active.
    };

    /** \brief Variable with keyword list value */
    class kw_t : public variable_t {
    public:
        kw_t(const std::string&,const std::string&,const std::string&);
        kw_t(const kw_t&); //!< Copy constructor.
        void set_range(const std::string&); //!< Set/change the list of valid entries
        bool isval(const std::string&) const; //!< Test if the given value is selected.
        keyword_list_t data; //!< Variable data in its native type.
    protected:
        void validate(const keyword_list_t&);
        std::string op_setval(expression_t&);
        std::string query_range(const std::string&);
        std::string query_val(const std::string&);
        std::string query_type(const std::string&);
    };

    /**\brief Variable with a string value*/
    class string_t : public variable_t {
    public:
        string_t(const std::string&,const std::string&);
        std::string data;//!< Data field
    protected:
        std::string op_setval(expression_t&);
        std::string query_type(const std::string&);
        std::string query_val(const std::string&);
    };

    /**\brief Vector variable with string values*/
    class vstring_t : public variable_t
    {
    public:
        vstring_t(const std::string&,const std::string&);
        std::vector<std::string> data;//!< Data field
    protected:
        std::string op_setval(expression_t&);
        std::string query_type(const std::string&);
        std::string query_val(const std::string&);
    };

    /**\brief Variable with a boolean value ("yes"/"no")*/
    class bool_t : public variable_t
    {
    public:
        /** Constructor for a configuration language variable for
         * boolean values.
         * \param help_text 
         *   A human-readable text describing the purpose of this
         *   configuration variable.
         * \param initial_value
         *   The initial value for this variable as a string.
         *   The string representation of 'true' is either "yes" or "1".
         *   The string representation of 'false' is either "no" or "0".
         */
        bool_t(const std::string & help_text,
               const std::string & initial_value);
        bool data;//!< Data field
    protected:
        std::string op_setval(expression_t&);
        std::string query_type(const std::string&);
        std::string query_val(const std::string&);
    };

    /** \brief Variable with integer value */
    class int_t : public range_var_t {
    public:
        /** Constructor for a configuration language variable for
         * integral  values.
         * \param help_text 
         *   A human-readable text describing the purpose of this
         *   configuration variable.
         * \param initial_value
         *   The initial value for this variable as a string 
         *   (decimal representation of the integer variable).
         *   If a range is given in the third parameter, then 
         *   the initial value has to be within the range.
         * \param range
         *   The range of values that this variable can hold can 
         *   be restricted.
         *   A range is a string of the form "[a,b]", where a and b
         *   are decimal representations of the integral inclusive boundaries
         *   of the range. a<=b.
         *   In a range of the form "]a,b[", both boundaries are excluded.
         *   Mixed forms are permitted.
         *   a or b can also be omitted if there is no lower or upper limit.
         *   The range of values is always restricted by the representable
         *   range of the underlying C data type (usually 32 bits,
         *   [-2147483648,2147483647]).
         */
        int_t(const std::string & help_text,
              const std::string & initial_value, 
              const std::string & range = "");
        int data;//!< Data field
    protected:
        std::string op_setval(expression_t&);
        std::string query_type(const std::string&);
        std::string query_val(const std::string&);
    };

    /** \brief Variable with float value */
    class float_t : public range_var_t
    {
    public:
        /** Constructor for a configuration language variable for
         * 32bit ieee floating-point values.
         * \param help_text 
         *   A human-readable text describing the purpose of this
         *   configuration variable.
         * \param initial_value
         *   The initial value for this variable as a string 
         *   (decimal representation of the floating-point variable).
         *   If a range is given in the third parameter, then 
         *   the initial value has to be within the range.
         *   A human-readable text describing the purpose of this
         *   configuration variable.
         * \param range
         *   The range of values that this variable can hold can 
         *   be restricted.
         *   A range is a string of the form "[a,b]", where a and b
         *   are decimal representations of the inclusive boundaries
         *   of the range. a<=b.
         *   In a range of the form "]a,b[", both boundaries are excluded.
         *   Mixed forms are permitted.
         *   a or b can also be omitted if there is no lower or upper limit.
         *   The range of values is always restricted by the representable
         *   range of the underlying C data type.
         */
        float_t(const std::string & help_text,
                const std::string & initial_value,
                const std::string & range = "");
        float data;//!< Data field
    protected:
        std::string op_setval(expression_t&);
        std::string query_type(const std::string&);
        std::string query_val(const std::string&);
    };

    /** \brief Variable with complex value */
    class complex_t : public range_var_t
    {
    public:
        complex_t(const std::string&,const std::string&,const std::string& ="");
        mha_complex_t data;//!< Data field
    protected:
        std::string op_setval(expression_t&);
        std::string query_type(const std::string&);
        std::string query_val(const std::string&);
    };

    /** \brief Variable with vector<int> value */
    class vint_t : public range_var_t
    {
    public:
        vint_t(const std::string&,const std::string&,const std::string& ="");
        std::vector<int> data;//!< Data field
    protected:
        std::string op_setval(expression_t&);
        std::string query_type(const std::string&);
        std::string query_val(const std::string&);
    };

    /** \brief Vector variable with float value */
    class vfloat_t : public range_var_t
    {
    public:
        vfloat_t(const std::string&,const std::string&,const std::string& ="");
        std::vector<float> data;//!< Data field
    protected:
        std::string op_setval(expression_t&);
        std::string query_type(const std::string&);
        std::string query_val(const std::string&);
    };

    /** \brief Vector variable with complex value */
    class vcomplex_t : public range_var_t
    {
    public:
        vcomplex_t(const std::string&,const std::string&,const std::string& ="");
        std::vector<mha_complex_t> data;//!< Data field
    protected:
        std::string op_setval(expression_t&);
        std::string query_type(const std::string&);
        std::string query_val(const std::string&);
    };

    /** \brief Matrix variable with int value */
    class mint_t : public range_var_t
    {
    public:
        mint_t(const std::string&,const std::string&,const std::string& ="");
        std::vector<std::vector<int> > data;//!< Data field
    protected:
        std::string op_setval(expression_t&);
        std::string query_type(const std::string&);
        std::string query_val(const std::string&);
    };

    /** \brief Matrix variable with float value */
    class mfloat_t : public range_var_t
    {
    public:
        mfloat_t(const std::string&,const std::string&,const std::string& ="");
        std::vector<std::vector<float> > data;//!< Data field
    protected:
        std::string op_setval(expression_t&);
        std::string query_type(const std::string&);
        std::string query_val(const std::string&);
    };

    /** \brief Matrix variable with complex value */
    class mcomplex_t : public range_var_t
    {
    public:
        mcomplex_t(const std::string&,const std::string&,const std::string& ="");
        std::vector<std::vector<mha_complex_t> > data;//!< Data field
    protected:
        std::string op_setval(expression_t&);
        std::string query_type(const std::string&);
        std::string query_val(const std::string&);
    };

    /** \brief Monitor variable with int value
        
    Monitor variables can be of many types. These variables can be
    queried through the parser. The public data element contains the
    monitored state. Write access is only possible from the C++ code by
    direct access to the data field.
    */
    class int_mon_t : public monitor_t
    {
    public:
        /** Create a monitor variable for integral values.
         * @param hlp A help text describing this monitor variable. */
        int_mon_t(const std::string & hlp);
        int data;//!< Data field
    protected:
        std::string query_val(const std::string&);
        std::string query_type(const std::string&);
    };

    /**\brief Monitor with string value */
    class bool_mon_t : public monitor_t
    {
    public:
        /** Create a monitor variable for string values.
         * @param hlp A help text describing this monitor variable. */
        bool_mon_t(const std::string & hlp);
        bool data;//!< Data field
    protected:
        std::string query_val(const std::string&);
        std::string query_type(const std::string&);
    };

    /**\brief Monitor with string value */
    class string_mon_t : public monitor_t
    {
    public:
        /** Create a monitor variable for string values.
         * @param hlp A help text describing this monitor variable. */
        string_mon_t(const std::string & hlp);
        std::string data;//!< Data field
    protected:
        std::string query_val(const std::string&);
        std::string query_type(const std::string&);
    };

    /**\brief Vector of monitors with string value */
    class vstring_mon_t : public monitor_t
    {
    public:
        /** Create a vector of string monitor values.
         * @param hlp A help text describing this monitor variable. */
        vstring_mon_t(const std::string & hlp);
        std::vector<std::string> data;//!< Data field
    protected:
        std::string query_val(const std::string&);
        std::string query_type(const std::string&);
    };

    /**\brief Vector of ints monitor*/
    class vint_mon_t : public monitor_t
    {
    public:
        /** Create a vector of integer monitor values.
         * @param hlp A help text describing this monitor variable. */
        vint_mon_t(const std::string & hlp);
        std::vector<int> data;//!< Data field
    protected:
        std::string query_val(const std::string&);
        std::string query_type(const std::string&);
    };

    /**\brief Matrix of ints monitor*/
    class mint_mon_t : public monitor_t
    {
    public:
        /** Create a matrix of integer monitor values.
         * @param hlp A help text describing this monitor variable. */
        mint_mon_t(const std::string & hlp);
        std::vector< std::vector<int> > data;//!< Data field
    protected:
        std::string query_val(const std::string&);
        std::string query_type(const std::string&);
    };

    /**\brief Vector of floats monitor*/
    class vfloat_mon_t : public monitor_t
    {
    public:
        /** Create a vector of floating point monitor values.
         * @param hlp A help text describing this monitor variable. */
        vfloat_mon_t(const std::string & hlp);
        std::vector<float> data;//!< Data field
    protected:
        std::string query_val(const std::string&);
        std::string query_type(const std::string&);
    };

    /**\brief Matrix of floats monitor*/
    class mfloat_mon_t : public monitor_t
    {
    public:
        /** Create a matrix of floating point monitor values.
         * @param hlp A help text describing this monitor variable. */
        mfloat_mon_t(const std::string & hlp);
        std::vector< std::vector<float> > data;//!< Data field
    protected:
        std::string query_val(const std::string&);
        std::string query_type(const std::string&);
    };

    /**\brief Monitor with float value*/
    class float_mon_t : public monitor_t
    {
    public:
        /** Initialize a floating point (32 bits) monitor variable.
         * @param hlp A help text describing this monitor variable. */
        float_mon_t(const std::string & hlp);
        float data;//!< Data field
    protected:
        std::string query_val(const std::string&);
        std::string query_type(const std::string&);
    };
    
    /**\brief Monitor with complex value*/
    class complex_mon_t : public monitor_t
    {
    public:
        /** Create a complex monitor variable.
         * @param hlp A help text describing this monitor variable. */
        complex_mon_t(const std::string & hlp);
        mha_complex_t data;//!< Data field
    protected:
        std::string query_val(const std::string&);
        std::string query_type(const std::string&);
    };
    
    /**\brief Monitor with vector of complex values */
    class vcomplex_mon_t : public monitor_t
    {
    public:
        /** Create a vector of complex monitor values.
         * @param hlp A help text describing this monitor variable. */
        vcomplex_mon_t(const std::string & hlp);
        std::vector<mha_complex_t> data;//!< Data field
    protected:
        std::string query_val(const std::string&);
        std::string query_type(const std::string&);
    };
    
    /**\brief Matrix of complex numbers monitor*/
    class mcomplex_mon_t : public monitor_t
    {
    public:
        /** Create a matrix of complex floating point monitor values.
         * @param hlp A help text describing this monitor variable. */
        mcomplex_mon_t(const std::string & hlp);
        std::vector< std::vector<mha_complex_t> > data;//!< Data field
    protected:
        std::string query_val(const std::string&);
        std::string query_type(const std::string&);
    };

    template <class receiver_t> class commit_t : public MHAParser::kw_t {
    public:
        commit_t(receiver_t*,void (receiver_t::*)(),const std::string& help="Variable changes action");
    private:
        MHAEvents::connector_t<receiver_t> extern_connector;
    };
    
    template<class receiver_t> commit_t<receiver_t>::commit_t(receiver_t* r,void (receiver_t::*rfun)(),const std::string& help)
        : MHAParser::kw_t(help,"commit","[commit]"),
          extern_connector(&writeaccess,r,rfun)
    {
    }

    class mhaconfig_mon_t : public MHAParser::parser_t {
    public:
        mhaconfig_mon_t(const std::string& help="");
        void update(const mhaconfig_t& cf);
    private:
        MHAParser::int_mon_t channels; /**< \brief Number of audio channels */
        MHAParser::string_mon_t domain; /**< \brief Signal domain (MHA_WAVEFORM or MHA_SPECTRUM) */
        MHAParser::int_mon_t fragsize; /**< \brief Fragment size of waveform data */
        MHAParser::int_mon_t wndlen; /**< \brief Window length of spectral data */
        MHAParser::int_mon_t fftlen; /**< \brief FFT length of spectral data */
        MHAParser::float_mon_t srate;    /**< \brief Sampling rate in Hz */
    };

}

/**
   \brief Macro to insert a member variable into a parser.

   \param x Member variable to be inserted. Name of member variable will be used as configuration name.

   See also MHAParser::parser_t::insert_item().
 */
#define insert_member(x) insert_item(#x,&x)

#endif

// Local Variables:
// compile-command: "make -C .."
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
