// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2005 2006 2007 2008 2009 2011 2012 2013 HörTech gGmbH
// Copyright © 2014 2015 2016 2017 2018 2019 2020 HörTech gGmbH
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

#include "mha.hh"
#include "mha_error.hh"
#include "mha_errno.h"
#include "mha_parser.hh"
#include "mha_algo_comm.h"
#include "mha_events.h"
#include "mha_git_commit_hash.hh"
#include <atomic>

#if _WIN32
#include <windows.h>
#else
// define dummy __declspec(dllexport) and others for non-windows OS
#define __declspec(p)
#define WINAPI
#define HINSTANCE int
#endif

class Test_mha_plugin_rtcfg_t; // Forward declaration

/**
   \brief Namespace for \mha plugin class templates and thread-safe runtime configurations
*/
namespace MHAPlugin {

    /** A node class for storing MHA plugin runtime configurations as a singly
     * linked list, where the nodes store besides the usual "next" and "data"
     * pointers also a flag that indicates whether this node can be deleted.
     *
     * The singly linked list is designed for a single producer thread and a
     * single consumer thread, where the producer is also responsible for
     * destroying objects when they are no longer needed because the consumer
     * is the signal processing thread that cannot afford memory allocation or
     * deallocation operations. */
    template < class runtime_cfg_t > class cfg_node_t {
    public:
        /** Constructor for a singly linked list node.
         * @param runtime_cfg
         *   Pointer to a runtime configuration object that was just created
         *   on the heap.  The newly constructed cfg_node_t object takes over
         *   object ownership of the pointed-to runtime configuration and will
         *   call delete on it in its destructor. */
        explicit cfg_node_t( runtime_cfg_t * runtime_cfg );

        /** Destructor of the singly linked list node.  Will also delete the
         * pointed-to data object (the runtime configuration) */
        ~cfg_node_t(  );

        /** A pointer to the next node in the singly linked list.
         * On construction, this will be a NULL pointer.
         * New objects can be appended to the singly linked list by writing
         * the address to the next node into this data member.
         * Since this pointer is std::atomic, writing to it is a release
         * operation which means all threads seeing the new value of the next
         * pointer will also see all other writes to memory that the thread
         * doing this write has performed, which includes anything the
         * constructor of the new runtime config has written and the assignment
         * of the address of the new runtime config to the data pointer of the
         * next node. This prevents making half-constructed runtime
         * configuration objects visible to the consumer thread. */
        std::atomic<cfg_node_t<runtime_cfg_t>*> next;

        /** Initially this data member is set to false by the constructor.
         * It is set to true by the consumer thread when it no longer needs the
         * run time configuration pointed to by this node's data member.
         * This bool is atomic because this node and the runtime object it
         * points to can be deleted by the configuration thread as soon as value
         * true is stored in this bool, which happens right after the processing
         * thread acquires a pointer to the next node in the singly linked list.
         * The atomic ensures all threads agree on this "right after" ordering.
         */
        std::atomic<bool> not_in_use;

        /** A native pointer to the runtime configuration managed by this
         * node.  The runtime configuration lives on the heap and is owned by
         * this node. It is deleted in this node's destructor.  The runtime
         * configuration object must be created by client code with
         * operator new before ownership is transferred to this node
         * by passing a pointer to it as the constructor's parameter.
         * This pointer does not need to be atomic, memory access ordering is
         * ensured by the atomic next pointer and placing accesses to "data"
         * in the correct places relative to accesses to "next". */
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

       To ensure lock-free thread safety, we use C++11 atomics and rely on the
       C++11 memory model. We only use store-release and load-acquire operations
       by using C++11 atomics with the default memory ordering.  The semantics
       of these are:

       The store-release operation atomically writes to an atomic variable,
       while the load-acquire operation atomically reads from an atomic
       variable.

       The C++11 memory model guarantees that all previous writes to memory
       performed by the thread doing the store-release are visible to other
       threads when they see the new value in the shared atomic variable when
       that value is read by the other thread with a load-acquire operation.

       An important precondition of this synchronization scheme is that there is only ever one audio thread
       and one configuration thread per plugin, i.e. there is only one thread doing
       the push_config and one thread doing the poll_config for each instance of config_t.

       For more details on atomics, refer to the C++11 or later documentation, or to these
       conference talks by Sutter:
       - Atomic Weapons, 2012
       - Lock-Free Programming, 2014
    */
    template < class runtime_cfg_t > class config_t {
      friend class ::Test_mha_plugin_rtcfg_t;
    public:
        config_t(  );
        ~config_t(  );
    protected:
        /// Pointer to the runtime configuration currently used by the signal
        /// processing thread.  Should be used to access the current runtime
        /// configuration during signal processing.  This pointer is updated
        /// as a side effect of calling poll_config() on this object.
        runtime_cfg_t* cfg;
        runtime_cfg_t* poll_config();
        runtime_cfg_t* peek_config() const;
        void push_config( runtime_cfg_t * ncfg );
        /// To be called by the push_config() for housekeeping.  Will delete
        /// any no longer used runtime configuration objects.
        void cleanup_unused_cfg();
        /// To be called on Plugin destruction, will delete all runtime
        /// configuration list nodes and objects regardless of their
        /// in_use flag.
        void remove_all_cfg();
    private:
        /// Start of a singly linked list of runtime configuration objects.
        /// cfg_root points to the oldest still existing node of that list.
        /// After object creation this pointer is updated by the
        /// configuration thread and then read by the signal processing thread.
        /// To ensure proper order of memory accesses for this
        /// transfer between threads, it needs to be atomic, this ensures that
        /// the start of the singly linked list of runtime configurations 
        /// will be properly visible to the signal processing on startup.
        std::atomic< MHAPlugin::cfg_node_t<runtime_cfg_t> *> cfg_root;
        /// Pointer to the currently used plugin runtime configurations.
        /// Used as a hint for poll_config where to start looking for the 
        /// newest node. This optimization allows poll_config to scale better
        /// with the number of nodes not yet cleaned up by push_config.
        /// Does not need to be atomic because
        /// it is only used within the signal processing thread.
        MHAPlugin::cfg_node_t<runtime_cfg_t> *cfg_node_current;
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

template < class runtime_cfg_t > MHAPlugin::cfg_node_t < runtime_cfg_t >::
cfg_node_t( runtime_cfg_t * runtime_cfg )
    :  next( nullptr ), not_in_use( false ), data( runtime_cfg )
{
    // Next line serves as a memory release operation of the configuration
    // thread.  It ensures that all previous stores by this thread, such as
    // the construction of the runtime configuration object and the assignment
    // of that object's address to the data pointer, will be visible to the
    // signal processing thread after it checks the value of the next pointer
    // with memory acquire semantics.  The corresponding acquire is executed
    // by the signal processing thread in poll_config.
    next.store(nullptr);
}

template < class runtime_cfg_t > MHAPlugin::cfg_node_t < runtime_cfg_t >::~cfg_node_t(  )
{
    if( data )
        delete data;
}

template < class runtime_cfg_t > MHAPlugin::config_t < runtime_cfg_t >::config_t(  ):
    cfg(nullptr),
    // The following performs object initialization of the atomic object but
    // is itself not an atomic operation
    cfg_root(nullptr),
    cfg_node_current(nullptr)
{
    // The following line contains a memory release operation of the
    // configuration thread.  It initializes the cfg_root atomic pointer
    // with a valid start node even though that start node does not contain
    // a valid runtime configuration.  A later call to push_config() in
    // configuration thread with a valid configuration object pointer is
    // required if and before the signal processing thread uses poll_config().
    // The corresponding acquire of the signal processing thread
    // is in poll_config().  At most one release/acquire interaction between
    // these threads through this pointer is performed during object lifetime.
    cfg_root.store(new MHAPlugin::cfg_node_t<runtime_cfg_t>(nullptr));
}

template < class runtime_cfg_t > MHAPlugin::config_t < runtime_cfg_t >::~config_t(  )
{
    try {
        remove_all_cfg(  );
        cfg_node_current = nullptr;
    }
    catch( MHA_Error & e ) {
        mha_debug( "%s", Getmsg( e ) );
    }
}

/** \brief Receive the latest run time configuration without changing the configuration pointer

    This function retrieves the latest run time configuration. 
    Returns a pointer to the latest runtime configuration without
    updating the data member cfg.  For use in the configuration thread
    when creation of a new runtime configuration object needs access
    to the previously created runtime configuration object.
    Should normally not be used because it introduces synchronization
    requirements between configuration thread and signal processing thread.
*/
template < class runtime_cfg_t > runtime_cfg_t* MHAPlugin::config_t < runtime_cfg_t >::peek_config(  ) const
{
    // peek_config() is called in configuration thread.  We cannot use
    // cfg_root_current to start later in the chain and thereby skip some
    // entries because cfg_node_current is not atomic and may only be used in
    // the signal processing thread.  Improving performance of peek_config() by
    // making cfg_node_current atomic would worsen the performance of
    // poll_config() which is used much more often and time critical.
    cfg_node_t<runtime_cfg_t> *
        res = cfg_root;
    while( res && res->next ) {
        res = res->next;
    }
    return res ? res->data : nullptr;
}

/** \brief Receive the latest run time configuration
            
    This function stores the latest run time configuration into
    the protected class member variable `cfg'. If no configuration
    exists, then an exception will be thrown. If no changes
    occured, then the value of `cfg' will be untouched. 

    This function should be only called from the <i>processing</i> thread.

    Should be called at the start of each process() callback to get
    the latest runtime configuration.  
    
    When this function finds newer run time configurations,
    it returns the newest and ensures the older run time configurations have their not_in_use flag set to true.

    @return Pointer to the latest runtime configuration object (same pointer
            as stored by this function in data member `cfg').

    @throw MHA_Error if the resulting runtime configuration is NULL.  This
           usually means that no push_config has occured.
*/
template < class runtime_cfg_t > runtime_cfg_t* MHAPlugin::config_t < runtime_cfg_t >::poll_config(  )
{
    if (cfg_node_current == nullptr) {
        // The next line contains a memory acquire operation of the
        // signal processing thread.  This is executed only once in
        // the plugin instance's lifetime.  The corresponding release of the
        // configuration thread executes in the config_t constructor.
        cfg_node_current = cfg_root.load();
    }
    // The next line contains a memory acquire operation of the signal
    // processing thread.  By aquiring the current value of the next pointer,
    // we ensure that any memory stores by the configuration thread like
    // the store to the data pointer are visible to this thread.  The
    // corresponding release that pairs with this acquire is executed by the
    // configuration thread in the cfg_node_t constructor.
    while( cfg_node_current->next.load() ) {
        // temporarily store for marking
        auto prev=cfg_node_current;
        cfg_node_current = cfg_node_current->next.load();
        // there is a next runtime config, mark this one as no longer needed.
        // This is a store-release operation. The corresponding load-acquire is
        // in cleanup_unused_cfg
        prev->not_in_use.store(true);
    }
    cfg=cfg_node_current->data;
    if( !cfg )
        throw MHA_ErrorMsg( "No valid configuration available!" );
    return cfg;
}

/** \brief Push a new run time configuration into the configuration fifo

    Should be called only by the configuration thread when a new runtime
    configuration object has been constructed in response to
    configuration changes, or during execution of the prepare() method
    to ensure that there is a valid runtime configuration for the
    signal processing which can start after prepare() returns.

    For housekeeping, this method will also delete any runtime
    configuration objects that have previously been passed to
    push_config() if they are no longer needed.

    @param ncfg
      A pointer to the new runtime configuration object.
      This object must have been created on the heap by the
      configuration thread with operator new.  By passing the pointer
      to this method, client code gives up ownership.  The object
      will be deleted in a future invocation of push_config, or on
      destruction of this config_t instance.
*/
template < class runtime_cfg_t > void MHAPlugin::config_t < runtime_cfg_t >::push_config( runtime_cfg_t * ncfg )
{
    MHAPlugin::cfg_node_t < runtime_cfg_t > *lcfg_root = cfg_root.load();
    while( lcfg_root->next.load() )
        lcfg_root = lcfg_root->next.load();
    // This is a store-release operation. Corresponding loads are scattered over
    // most member functions
    lcfg_root->next.store(new MHAPlugin::cfg_node_t < runtime_cfg_t > ( ncfg ));
    cleanup_unused_cfg(  );
}

template < class runtime_cfg_t > void MHAPlugin::config_t < runtime_cfg_t >::cleanup_unused_cfg(  )
{
    MHAPlugin::cfg_node_t < runtime_cfg_t > *lcfg;
    // Next line contains three memory acquire operations of the configuration
    // thread. Only the last one, on not_in_use, is required for cross-thread
    // memory ordering here because a corresponding release was done in a the
    // signal processing thread, in poll_config().
    while( (lcfg = cfg_root.load()) && lcfg->next.load() && lcfg->not_in_use.load() ) {
        // Operations on next line have no cross-thread memory ordering function
        cfg_root.store(lcfg->next.load());
        delete lcfg;
    }
}

template < class runtime_cfg_t > void MHAPlugin::config_t < runtime_cfg_t >::remove_all_cfg(  )
{
    if( !cfg_root )
        return;
    while( auto next = cfg_root.load()->next.load() ) {
        cfg_node_t<runtime_cfg_t> * lcfg_root = cfg_root.load();
        cfg_root = next;
        delete lcfg_root;
    }
    delete cfg_root;
    cfg_root = nullptr;
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

#ifdef __clang__
// We cannot export a function with extern "C" linkage with clang when it
// returns a C++ object
#define MHAPLUGIN_SETCPP_CALLBACK_PREFIX(prefix,classname)
#else
// We can export a function with extern "C" linkage when it
// returns a C++ object on Linux and Windows with GCC
#define MHAPLUGIN_SETCPP_CALLBACK_PREFIX(prefix,classname)              \
        __declspec(dllexport) std::string prefix ## MHASetcpp(void* handle, const std::string & command) \
        {                                                               \
            if( handle ){                                               \
                return ((classname*)handle)->parse(command);            \
            }                                                           \
            throw MHA_Error(__FILE__, __LINE__,"Invalid plugin handle");\
        }
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
        MHAPLUGIN_SETCPP_CALLBACK_PREFIX(prefix,classname)              \
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
