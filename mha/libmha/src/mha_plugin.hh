// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2005 2006 2007 2008 2009 2011 2012 2013 HörTech gGmbH
// Copyright © 2014 2015 2016 2017 HörTech gGmbH
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

#ifndef MHA_EXTERN_IFC_HH
#define MHA_EXTERN_IFC_HH

#include "mha.h"
#include "mha_error.hh"
#include "mha_errno.h"
#include "mha_parser.hh"
#include "mha_algo_comm.h"
#include "mha_events.h"

#if _WIN32
#include <windows.h>
#else
// define dummy __declspec(dllexport) and others for non-windows OS
#define __declspec(p)
#define WINAPI
#define HINSTANCE int
#endif

#ifndef GITCOMMITHASH
#define GITCOMMITHASH "independent-plugin-build"
#endif

/// store git commit hash in every binary plgin to support reproducible research
__attribute__((unused)) static const char* mha_git_commit_hash =
  "MHA_GIT_COMMIT_HASH=" GITCOMMITHASH;

class Test_mha_plugin_rtcfg_t; // Forward declaration

/**
   \brief Namespace for \mha plugin class templates and thread-safe runtime configurations
*/
namespace MHAPlugin {

    template < class runtime_cfg_t > class cfg_chain_t {
    public:
        cfg_chain_t( runtime_cfg_t * id );
        ~cfg_chain_t(  );
        cfg_chain_t < runtime_cfg_t > *next;
        bool not_in_use;
        runtime_cfg_t *data;
    };


    /**
       \brief Template class for thread safe configuration

       This template class provides a mechanism for the handling of
       thread safe configuration which is required for run time
       configuration changes of the \mha plugins.

       The template parameter runtime_cfg_t is the run time
       configuration class of the \mha plugin. The constructor of that
       class should transform the MHAParser variables into derived
       runtime configuration. The constructor should fail if the
       configuration is invalid by any reason.

       A new runtime configuration is provided by the function
       push_config(). In the processing thread, the actual
       configuration can be received by a call of poll_config().

       \image html config_fifo_2.png "Schematic drawing of runtime configuration update: configuration updated, but not used yet."
       \image latex config_fifo_2.pdf "Schematic drawing of runtime configuration update: configuration updated, but not used yet." width=0.8\linewidth
       \image html config_fifo_3.png "Schematic drawing of runtime configuration update: configuration in use."
       \image latex config_fifo_3.pdf "Schematic drawing of runtime configuration update: configuration in use." width=0.8\linewidth

    */
    template < class runtime_cfg_t > class config_t {
      friend class ::Test_mha_plugin_rtcfg_t;
    public:
        config_t(  );
        ~config_t(  );
    protected:
        runtime_cfg_t* cfg;
        runtime_cfg_t* poll_config();
        runtime_cfg_t* last_config();
        void push_config( runtime_cfg_t * ncfg );
        void cleanup_unused_cfg();
    private:
        // root of config chain
        MHAPlugin::cfg_chain_t<runtime_cfg_t> *cfg_chain;
        // current element of config chain
        MHAPlugin::cfg_chain_t<runtime_cfg_t> *cfg_chain_current;
        void remove_all_cfg();
    };


    /** 
        \ingroup plugif
        \brief The template class for C++ \mha plugins
        \tparam runtime_cfg_t run-time configuration.

        \todo Describe all services provided by this class, so that the reason
        why it is recommended that all plugins use this class as their base is
        evident. Document all relevant methods and fields.

        This template class provides thread safe configuration handling
        and standard methods to be compatible to the C++ \mha plugin
        wrapper macro \ref MHAPLUGIN_CALLBACKS.

        The template parameter runtime_cfg_t should be the runtime
        configuration of the plugin.

        See \ref MHAPlugin::config_t for details on the thread safe
        communication update mechanism.
       
    */
    template < class runtime_cfg_t> class plugin_t:
        public MHAParser::parser_t, public MHAPlugin::config_t < runtime_cfg_t > {
    public:
        plugin_t( const std::string &, const algo_comm_t & );
        virtual ~plugin_t();
        virtual void prepare( mhaconfig_t & ) = 0;
        virtual void release();
        void prepare_( mhaconfig_t& );
        void release_();
        /**
           \brief Flag, if the prepare method is successfully called (or currently evaluated)
        */
        bool is_prepared() const { return is_prepared_; };
        /**
           \brief Current input channel configuration.
        */
        mhaconfig_t input_cfg() const { return input_cfg_; };
        /**
           \brief Current output channel configuration.
        */
        mhaconfig_t output_cfg() const { return output_cfg_; };
    protected:
        /**
           \brief Member for storage of plugin interface configuration.

           This member is defined for convenience of the
           developer. Typically, the actual contents of mhaconfig_t
           are stored in this member in the prepare() method.

           \note This member is likely to be removed in later versions, use input_cfg() and output_cfg() instead.
        */
        mhaconfig_t tftype;
        /**
           \brief AC handle of the chain.

           This variable is initialized in the constructor and can be
           used by derived plugins to access the AC space. Its
           contents should not be modified.
        */
        algo_comm_t ac;
    private:
        bool is_prepared_;
        mhaconfig_t input_cfg_;
        mhaconfig_t output_cfg_;
        MHAParser::mhaconfig_mon_t mhaconfig_in;
        MHAParser::mhaconfig_mon_t mhaconfig_out;
    };

}

template < class runtime_cfg_t > MHAPlugin::cfg_chain_t < runtime_cfg_t >::cfg_chain_t( runtime_cfg_t * id )
    :  next( NULL ), not_in_use( false ), data( id )
{
}

template < class runtime_cfg_t > MHAPlugin::cfg_chain_t < runtime_cfg_t >::~cfg_chain_t(  )
{
    if( data )
        delete data;
}

template < class runtime_cfg_t > MHAPlugin::config_t < runtime_cfg_t >::config_t(  ):
    cfg( NULL ), 
    cfg_chain( new MHAPlugin::cfg_chain_t < runtime_cfg_t > ( NULL ) ),
    cfg_chain_current(NULL)
{
}

template < class runtime_cfg_t > MHAPlugin::config_t < runtime_cfg_t >::~config_t(  )
{
    try {
        remove_all_cfg(  );
        cfg_chain_current = NULL;
    }
    catch( MHA_Error & e ) {
        mha_debug( "%s", Getmsg( e ) );
    }
}

/** \brief Receive the latest run time configuration
            
    This function stores the latest run time configuration into
    the protected class member variable `cfg'. If no configuration
    exists, then an exception will be thrown. If no changes
    occured, then the value of `cfg' will be untouched. This
    function may be called instead of poll_config.

    The difference between poll_config and last_config is that
    poll_config marks previous configurations as ready for deletion,
    while this function does not.  Therefore, memory usage of all
    runtime configurations will accumulate if only this function is
    called, but it enables safe access to previous runtime
    configurations.

    Also, last_config does not raise an Exception when the latest run
    time configuration is NULL.
*/
template < class runtime_cfg_t > runtime_cfg_t* MHAPlugin::config_t < runtime_cfg_t >::last_config(  )
{
    if (cfg_chain_current == 0)
      cfg_chain_current = cfg_chain;

    while( cfg_chain_current->next ) {
        cfg_chain_current = cfg_chain_current->next;
    }
    cfg = cfg_chain_current->data;
    return cfg;
}

/** \brief Receive the latest run time configuration
            
    This function stores the latest run time configuration into
    the protected class member variable `cfg'. If no configuration
    exists, then an exception will be thrown. If no changes
    occured, then the value of `cfg' will be untouched. This
    function should be called before any access to the `cfg'
    variable, typically once in each signal processing call.

    This function should be only called from the <i>processing</i> thread.

    @throw MHA_Error if the resulting runtime configuration is NULL.  This
           usually means that no push_config has occured.
*/
template < class runtime_cfg_t > runtime_cfg_t* MHAPlugin::config_t < runtime_cfg_t >::poll_config(  )
{
    if (cfg_chain_current == 0)
      cfg_chain_current = cfg_chain;
    while( cfg_chain_current->next ) {
        volatile MHAPlugin::cfg_chain_t<runtime_cfg_t> * volatile former =
          cfg_chain_current;
        cfg_chain_current = cfg_chain_current->next;
        former->not_in_use = true;
    }
    cfg = cfg_chain_current->data;
    if( !cfg )
        throw MHA_ErrorMsg( "No valid configuration available!" );
    return cfg;
}

/** \brief Push a new run time configuration into the configuration fifo

    This function adds a new run time configuration. The next time
    \ref poll_config is called, this configuration will be
    available. Configurations which are not in use or are outdated
    will be removed.

    This function should be only called from the <i>configuration</i> thread.
        
    \param ncfg pointer on a new configuration

    \warning The runtime configuration passed to this function will be removed by the internal garbage collector. Do not free manually.

*/
template < class runtime_cfg_t > void MHAPlugin::config_t < runtime_cfg_t >::push_config( runtime_cfg_t * ncfg )
{
    MHAPlugin::cfg_chain_t < runtime_cfg_t > *lcfg_chain = cfg_chain;
    while( lcfg_chain->next )
        lcfg_chain = lcfg_chain->next;
    lcfg_chain->next = new MHAPlugin::cfg_chain_t < runtime_cfg_t > ( ncfg );
    cleanup_unused_cfg(  );
}

template < class runtime_cfg_t > void MHAPlugin::config_t < runtime_cfg_t >::cleanup_unused_cfg(  )
{
    if( !cfg_chain )
        return;
    MHAPlugin::cfg_chain_t < runtime_cfg_t > *lcfg;
    while( cfg_chain->next && cfg_chain->not_in_use ) {
        lcfg = cfg_chain;
        cfg_chain = cfg_chain->next;
        delete lcfg;
    }
}

template < class runtime_cfg_t > void MHAPlugin::config_t < runtime_cfg_t >::remove_all_cfg(  )
{
    if( !cfg_chain )
        return;
    MHAPlugin::cfg_chain_t < runtime_cfg_t > *lcfg_chain;
    while( cfg_chain->next ) {
        lcfg_chain = cfg_chain;
        cfg_chain = cfg_chain->next;
        delete lcfg_chain;
    }
    delete cfg_chain;
    cfg_chain = 0;
}

/**
   \brief Constructor of plugin template

   \param help Help comment to provide some general information about the plugin.
   \param iac AC space handle (will be stored into the member variable ac).
*/
template < class runtime_cfg_t > MHAPlugin::plugin_t < runtime_cfg_t >::plugin_t( const std::string & help,
                                                                                  const algo_comm_t & iac )
    : MHAParser::parser_t( help ), ac( iac ), is_prepared_(false),
      mhaconfig_in("Input configuration"),
      mhaconfig_out("Output configuration")
{
    memset(&tftype,0,sizeof(tftype));
    memset(&input_cfg_,0,sizeof(input_cfg_));
    memset(&output_cfg_,0,sizeof(output_cfg_));
    insert_member(mhaconfig_in);
    insert_member(mhaconfig_out);
}

template < class runtime_cfg_t > MHAPlugin::plugin_t < runtime_cfg_t >::~plugin_t(  )
{
}

template < class runtime_cfg_t > void MHAPlugin::plugin_t < runtime_cfg_t >::release()
{
}

template < class runtime_cfg_t > void MHAPlugin::plugin_t < runtime_cfg_t >::release_()
{
    is_prepared_ = false;
    release();
    MHAPlugin::config_t < runtime_cfg_t >::cleanup_unused_cfg();
}

template < class runtime_cfg_t > void MHAPlugin::plugin_t < runtime_cfg_t >::prepare_(mhaconfig_t& cf)
{
    is_prepared_ = true;
    try{
        input_cfg_ = cf; //initialize input and output configurations
        output_cfg_ = cf;
        prepare( output_cfg_ ); //allow output config modifications
        cf = output_cfg_; //reflect config changes to calling prepare

        mhaconfig_in.update(input_cfg_);
        mhaconfig_out.update(output_cfg_);
    }
    catch(...){
        is_prepared_ = false;
        memset(&input_cfg_,0,sizeof(input_cfg_));
        memset(&output_cfg_,0,sizeof(output_cfg_));
        throw;
    }
}

#ifndef MHAPLUGIN_OVERLOAD_OUTDOMAIN
#define MHAPLUGIN_PROC_CALLBACK_PREFIX(prefix,classname,indom,outdom)   \
    extern "C" {                                                        \
        __declspec(dllexport) int prefix ## MHAProc_ ## indom ## 2 ## outdom (void* handle,mha_ ## indom ## _t* s,mha_ ## outdom ## _t** out) \
        {                                                               \
            if(handle){                                                 \
                try{                                                    \
                    *out = ((classname*)handle)->process(s);            \
                    return MHA_ERR_SUCCESS;                             \
                }                                                       \
                catch(MHA_Error& e){                                    \
                    mha_set_user_error(Getmsg(e));                      \
                    return MHA_ERR_USER;                                \
                }                                                       \
            }                                                           \
            return MHA_ERR_INVALID_HANDLE;                              \
        }}
#else
#define MHAPLUGIN_PROC_CALLBACK_PREFIX(prefix,classname,indom,outdom)           \
    extern "C" {                                                        \
        __declspec(dllexport) int prefix ## MHAProc_ ## indom ## 2 ## outdom (void* handle,mha_ ## indom ## _t* s,mha_ ## outdom ## _t** out) \
        {                                                               \
            if(handle){                                                 \
                try{                                                    \
                    ((classname*)handle)->process(s,out);               \
                    return MHA_ERR_SUCCESS;                             \
                }                                                       \
                catch(MHA_Error& e){                                    \
                    mha_set_user_error(Getmsg(e));                      \
                    return MHA_ERR_USER;                                \
                }                                                       \
            }                                                           \
            return MHA_ERR_INVALID_HANDLE;                              \
        }}
#endif

#define MHAPLUGIN_INIT_CALLBACKS_PREFIX(prefix,classname)               \
    extern "C" {                                                        \
        __declspec(dllexport) int prefix ## MHAInit(algo_comm_t algo_comm,const char* chain, const char*algo,void** handle) \
        {                                                               \
            try{                                                        \
                *handle = new classname(algo_comm,chain,algo);          \
                return MHA_ERR_SUCCESS;                                 \
            }                                                           \
            catch(MHA_Error& e){                                        \
                mha_set_user_error(Getmsg(e));                          \
                return MHA_ERR_USER;                                    \
            }                                                           \
        }                                                               \
                                                                        \
        __declspec(dllexport) void prefix ## MHADestroy(void* data)             \
        {                                                               \
            if( data ){                                                 \
                classname* plug=(classname*)data;                       \
                delete plug;                                            \
            }                                                           \
        }                                                               \
        __declspec(dllexport) const char* prefix ## MHAStrError(void* data,int err) \
        {                                                               \
            (void) data;                                                \
            return mha_strerror( err );                                 \
        }                                                               \
                                                                        \
        __declspec(dllexport) unsigned int prefix ## MHAGetVersion(void)                \
        {                                                               \
            return MHA_VERSION;                                         \
        }                                                               \
                                                                        \
        __declspec(dllexport) const char* prefix ## MHAGetCommitHash(void)              \
        {                                                               \
            return mha_git_commit_hash;                                 \
        }                                                               \
                                                                        \
        __declspec(dllexport) int prefix ## MHASet(void* handle,const char *command,char *retval,unsigned int maxretlen) \
        {                                                               \
            if( handle ){                                               \
                try{                                                    \
                    ((classname*)handle)->parse(command,retval,maxretlen); \
                    return MHA_ERR_SUCCESS;                             \
                }                                                       \
                catch(MHA_Error& e){                                    \
                    mha_set_user_error(Getmsg(e));                      \
                    if( retval && maxretlen )                           \
                        strncpy(retval,"",maxretlen);                   \
                    return MHA_ERR_USER;                                \
                }                                                       \
            }                                                           \
            return MHA_ERR_INVALID_HANDLE;                              \
        }                                                               \
                                                                        \
        __declspec(dllexport) int prefix ## MHAPrepare(void* handle,mhaconfig_t* cfg) \
        {                                                               \
            if( handle ){                                               \
                try{                                                    \
                    ((classname*)handle)->prepare_(*cfg);               \
                    return MHA_ERR_SUCCESS;                             \
                }                                                       \
                catch(MHA_Error& e){                                    \
                    mha_set_user_error(Getmsg(e));                      \
                    return MHA_ERR_USER;                                \
                }                                                       \
            }                                                           \
            return MHA_ERR_INVALID_HANDLE;                              \
        }                                                               \
        __declspec(dllexport) int prefix ## MHARelease(void* handle)            \
        {                                                               \
            if( handle ){                                               \
                try{                                                    \
                    ((classname*)handle)->release_();                   \
                    return MHA_ERR_SUCCESS;                             \
                }                                                       \
                catch(MHA_Error& e){                                    \
                    mha_set_user_error(Getmsg(e));                      \
                    return MHA_ERR_USER;                                \
                }                                                       \
            }                                                           \
            return MHA_ERR_INVALID_HANDLE;                              \
        }}

/** \ingroup plugif

    \brief C++ wrapper macro for the plugin interface

    \param classname The name of the plugin class
    \param indom Input domain (\c wave or \c spec)
    \param outdom Output domain (\c wave or \c spec)

    This macro defines all required \mha Plugin interface functions
    and passes calls of these functions to the corresponding member
    functions of the class \c `classname'.  The parameters \c `indom'
    and \c `outdom' specify the input and output domain of the
    processing method.  The \c MHAInit() and \c MHADestroy() functions
    will create or destroy an instance of the class.  The approriate
    member functions have to be defined in the class.  It is suggested
    to make usage of the MHAPlugin::plugin_t template class.
    Exceptions of type \ref MHA_Error are caught and transformed into
    apropriate error codes with their corresponding error messages.

*/
#define MHAPLUGIN_CALLBACKS_PREFIX(prefix,classname,indom,outdom)       \
                                                                \
    MHAPLUGIN_INIT_CALLBACKS_PREFIX(prefix,classname)           \
    MHAPLUGIN_PROC_CALLBACK_PREFIX(prefix,classname,indom,outdom)       \
                                                                \
    void prefix ## dummy_interface_test(void){                          \
        MHA_CALLBACK_TEST_PREFIX(prefix,MHAGetVersion);         \
        MHA_CALLBACK_TEST_PREFIX(prefix,MHAInit);               \
        MHA_CALLBACK_TEST_PREFIX(prefix,MHAPrepare);            \
        MHA_CALLBACK_TEST_PREFIX(prefix,MHARelease);            \
        MHA_CALLBACK_TEST_PREFIX(prefix,MHASet);                \
        MHA_CALLBACK_TEST_PREFIX(prefix,MHADestroy);            \
    }                                                           \
                                                                \
    int WINAPI prefix ## DllEntryPoint(HINSTANCE,unsigned long,void*)   \
    {                                                           \
        return 1;                                               \
    }                                                           \
                                                                \
/** \ingroup plugif

    \brief Wrapper macro for the plugin documentation interface

    \param cat Space separated list of categories to which belong the plugin (as const char*)
    \param doc Documentation of the plugin (as const char*)

    This macro defines the \mha Plugin interface function for the
    documentation. The categories can be any space seperated list of
    category names. An empty string will categorize the plugin in the
    category 'other'. 

    The documentation should contain a description of the plugin including
    a description of the underlying models, and a paragraph containing
    hints for usage. The text should be LaTeX compatible (e.g., avoid or
    quote underscores in the text part); equations should be formatted as
    LaTeX.

*/
#define MHAPLUGIN_DOCUMENTATION_PREFIX(prefix,cat,doc)                  \
  extern "C" __declspec(dllexport) const char* prefix ## MHAPluginDocumentation(){MHA_CALLBACK_TEST_PREFIX(prefix,MHAPluginDocumentation);return doc;} \
  extern "C" __declspec(dllexport) const char* prefix ## MHAPluginCategory(){MHA_CALLBACK_TEST_PREFIX(prefix,MHAPluginCategory);return cat;} 


#ifdef MHA_STATIC_PLUGINS
#define MHAPLUGIN_PROC_CALLBACK(plugname,classname,indom,outdom)        \
  MHAPLUGIN_PROC_CALLBACK_PREFIX(MHA_STATIC_ ## plugname ## _,classname,indom,outdom)
#define MHAPLUGIN_INIT_CALLBACKS(plugname,classname)            \
  MHAPLUGIN_INIT_CALLBACKS_PREFIX(MHA_STATIC_ ## plugname ## _,classname)
/** \ingroup plugif

    \brief C++ wrapper macro for the plugin interface

    \param plugname The file name of the plugin without the .so or .dll extension
    \param classname The name of the plugin class
    \param indom Input domain (\c wave or \c spec)
    \param outdom Output domain (\c wave or \c spec)

    This macro defines all required \mha Plugin interface functions and
    passes calls of these functions to the corresponding member functions
    of the class `classname'. The parameters `indom' and `outdom' specify
    the input and output domain of the processing method. The MHAInit()
    and MHADestroy() functions will create or destroy an instance of
    the class. The approriate member functions have to be defined in the
    class. It is suggested to make usage of the MHAPlugin::plugin_t
    template class. Exceptions of type \ref MHA_Error are caught and
    transformed into apropriate error codes with their corresponding error
    messages.

*/
#define MHAPLUGIN_CALLBACKS(plugname,classname,indom,outdom)    \
  MHAPLUGIN_CALLBACKS_PREFIX(MHA_STATIC_ ## plugname ## _,classname,indom,outdom)    
/** \ingroup plugif

    \brief Wrapper macro for the plugin documentation interface

    \param plugin The file name of the plugin without the .so or .dll extension
    \param cat Space separated list of categories to which belong the plugin (as const char*)
    \param doc Documentation of the plugin (as const char*)

    This macro defines the \mha Plugin interface function for the
    documentation. The categories can be any space seperated list of
    category names. An empty string will categorize the plugin in the
    category 'other'. 

    The documentation should contain a description of the plugin including
    a description of the underlying models, and a paragraph containing
    hints for usage. The text should be LaTeX compatible (e.g., avoid or
    quote underscores in the text part); equations should be formatted as
    LaTeX.

*/
#define MHAPLUGIN_DOCUMENTATION(plugname,cat,doc)                       \
  MHAPLUGIN_DOCUMENTATION_PREFIX(MHA_STATIC_ ## plugname ## _,cat,doc)

#else // MHA_STATIC_PLUGINS  		 		

#define MHAPLUGIN_PROC_CALLBACK(plugname,classname,indom,outdom)        \
  MHAPLUGIN_PROC_CALLBACK_PREFIX(MHA_DYNAMIC_,classname,indom,outdom)
#define MHAPLUGIN_INIT_CALLBACKS(plugname,classname)            \
  MHAPLUGIN_INIT_CALLBACKS_PREFIX(MHA_DYNAMIC_,classname)
#define MHAPLUGIN_CALLBACKS(plugname,classname,indom,outdom)    \
  MHAPLUGIN_CALLBACKS_PREFIX(MHA_DYNAMIC_,classname,indom,outdom)    
#define MHAPLUGIN_DOCUMENTATION(plugname,cat,doc)                       \
  MHAPLUGIN_DOCUMENTATION_PREFIX(MHA_DYNAMIC_,cat,doc)

#endif // MHA_STATIC_PLUGINS   		  		

#endif

// Local Variables:
// coding: utf-8-unix
// c-basic-offset: 4
// indent-tabs-mode: nil
// compile-command: "make -C .."
// End:
