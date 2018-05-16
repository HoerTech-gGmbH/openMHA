// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2007 2008 2009 2010 2012 2014 2015 2016 2018 HörTech gGmbH
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

/** @internal @file split.cpp Source code for the split plugin.  The
 * split plugin splits the audio signal by channel.  The splitted paths
 * execute in parallel.
 */

/** This define modifies the definition of MHAPLUGIN_CALLBACKS and friends.
 * The output signal is transferred through a second parameter to the process
 * method, enabling all four domain transformations in a single plugin. */
#define MHAPLUGIN_OVERLOAD_OUTDOMAIN

#include <typeinfo>
#include "mha_algo_comm.hh"
#include "mha_multisrc.h"
#include "mhapluginloader.h"

#ifdef _WIN32
#define win32threads 1
#define default_thread_platform_string "win32"
#define default_thread_platform_type win32_threads_t
#else
#define posixthreads 1
#include <pthread.h>
#include <sched.h>
#define default_thread_platform_string "posix"
#define default_thread_platform_type posix_threads_t
#endif

// forward declaration of test classes
class Test_splitted_part_t;

/** \internal
 * A namespace for the split plugin.  Helps testability and documentation. */
namespace MHAPlugin_Split {

    /// Invalid thread priority
    enum {INVALID_THREAD_PRIORITY = 999999999};

    /** An interface to a class that sports a process method with no
     * parameters and no return value.  No signal transfer occurs
     * through this interface, because the signal transfer is
     * performed in another thread than the processing. */
    class uni_processor_t {
    public:
        /** This method uses some input signal, performs processing
         * and stores the output signal somewhere.  This method also
         * has to dispatch the process call based on the configured
         * domains.
         *
         * Signal transfer and domain configuration have to be done in
         * derived class in different methods. */
        virtual void process() = 0;
        /// Classes containing virtual methods need virtual destructors.
        virtual ~uni_processor_t() {}
    };

    /** Basic interface for encapsulating thread creation, thread
     * priority setting, and synchronization on any threading platform
     * (i.e., pthreads or win32threads).
     * Derived classes specialize in the actual thread platform.
     */
    class thread_platform_t {
    private:
        /// Disallow copy constructor
        thread_platform_t(const thread_platform_t &);
        /// Disallow assignment operator
        thread_platform_t & operator=(const thread_platform_t &);
    protected:
        /// A pointer to the plugin loader that processes the sound
        /// data in the channels for which this thread was created.
        /// Using the MHAPlugin_Split::uni_processor_t interface
        /// instead of the mhapluginloader class directly for
        /// testability (no need to load real plugins for testing the
        /// thread platform).
        uni_processor_t * processor;
    public:
        /// Constructor. Derived classes create the thread in the constructor.
        /// @param proc  Pointer to the associated plugin loader.
        ///   This plugin loader has to live at least as long as this
        ///   instance.  This instance does not take possession of the
        ///   plugin loader.  In production code, this thread platform
        ///   and the plugin loader are both created and destroyed by
        ///   the MHAPlugin_Split::splitted_part_t instance.
        thread_platform_t(uni_processor_t * proc)
            : processor(proc)
        {}

        /// Make derived classes destructable via pointer to this base
        /// class.  Derived classes' destructors notify the thread
        /// that it should terminate itself, and wait for the
        /// termination to occur.
        virtual ~thread_platform_t(){}
        
        /// Derived classes notify their processing thread that it should call
        /// processor->process().
        virtual void kick_thread() = 0;

        /// Derived classes wait for their signal processing thread to
        /// return from the call to part->process().
        virtual void catch_thread() = 0;
    };

    /** Dummy specification of a thread platform: This class
     * implements everything in a single thread. */
    class dummy_threads_t : public thread_platform_t {
    public:
        /// perform signal processing immediately (no multiple threads
        /// in this dummy class)
        void kick_thread() {processor->process();}
        /// No implementation needed: Processing has been completed
        /// during ummy_threads_t::kick_thread.
        void catch_thread() {}
        /// Constructor.
        /// @param proc  Pointer to the associated plugin loader
        /// @param thread_scheduler
        ///   Unused in dummy thread platform
        /// @param thread_priority
        ///   Unused in dummy thread platform
        dummy_threads_t(uni_processor_t * proc,
                        const std::string & thread_scheduler,
                        int thread_priority)
            : thread_platform_t(proc)
        {
            (void) thread_scheduler;
            (void) thread_priority;
        }
    };

#ifdef posixthreads
    /** Posix threads specification of thread platform */
    class posix_threads_t : public thread_platform_t {
        /// The mutex.
        pthread_mutex_t mutex;
        /// The condition for signalling the kicking and termination.
        pthread_cond_t kick_condition;
        /// The condition for signalling the processing is finished.
        pthread_cond_t catch_condition;
        /// Thread attributes
        pthread_attr_t attr;
        /// Thread scheduling priority
        struct sched_param priority;
        int scheduler;
        /// The thread object
        pthread_t thread;
        /** A flag that is set to true by kick_thread and to false by
         * the thread after it has woken up from the kicking. */
        bool kicked;
        /** A flag that is set to true by the thread when it returns
         * from processing and to false by catch_thread after it has
         * waited for that return. */
        bool processing_done;
        /** Set to true by the destructor. */
        bool termination_request;
    public:
        /// Start signal processing in separate thread.
        void kick_thread() {
            if (kicked != false) throw MHA_ErrorMsg("synchronization error");
            pthread_mutex_lock(&mutex);
            kicked = true;
            pthread_cond_signal(&kick_condition);
            pthread_mutex_unlock(&mutex);
        }
        /// Wait for signal processing to finish.
        void catch_thread() {
            pthread_mutex_lock(&mutex);
            while ( ! processing_done )
                pthread_cond_wait(&catch_condition, &mutex);
            processing_done = false;
            pthread_mutex_unlock(&mutex);
        }
        /** Constructor.
         * @param proc
         *   Pointer to the associated signal processor instance
         * @param thread_scheduler
         *   A string describing the posix thread scheduler. Possible values:
         *   "SCHED_OTHER", "SCHED_RR", "SCHED_FIFO".
         * @param thread_priority
         *   The scheduling priority of the new thread.
         */
        posix_threads_t(uni_processor_t * proc,
                        const std::string & thread_scheduler,
                        int thread_priority)
            : thread_platform_t(proc)
        {
            pthread_mutex_init(&mutex, 0);
            pthread_cond_init(&kick_condition, 0);
            pthread_cond_init(&catch_condition, 0);
            kicked = processing_done = termination_request = false;
            bool setting_attributes =
                thread_priority != INVALID_THREAD_PRIORITY;
            if (setting_attributes) {
                pthread_attr_init(&attr);
                if (thread_scheduler == "SCHED_OTHER")
                    pthread_attr_setschedpolicy(&attr, scheduler = SCHED_OTHER);
                else if (thread_scheduler == "SCHED_RR")
                    pthread_attr_setschedpolicy(&attr, scheduler = SCHED_RR);
                else if (thread_scheduler == "SCHED_FIFO")
                    pthread_attr_setschedpolicy(&attr, scheduler = SCHED_FIFO);
                else
                    setting_attributes = false;
            }
            if (setting_attributes) {
                priority.sched_priority = thread_priority;
                pthread_attr_setschedparam(&attr, &priority);
            }
            pthread_create(&thread,
                           (setting_attributes ? &attr : NULL),
                           &posix_threads_t::thread_start,
                           this);
            if (setting_attributes) {
                pthread_setschedparam(thread, scheduler, &priority);
            }
            catch_thread(); // wait for thread to become ready.
        }
        /// Terminate thread
        ~posix_threads_t() {
            pthread_mutex_lock(&mutex);
            termination_request = true;
            pthread_cond_signal(&kick_condition);
            pthread_mutex_unlock(&mutex);
            pthread_join(thread, 0);
        }
        /// Thread start function
        static void * thread_start(void * thr) {
            static_cast<posix_threads_t *>(thr)->main();
            return 0;
        };
        /// Thread main loop.  Wait for process/termination trigger, then act.
        void main() {
            for(;;) {
                pthread_mutex_lock(&mutex);
                processing_done = true;
                pthread_cond_signal(&catch_condition);
                while (!kicked && !termination_request)
                    pthread_cond_wait(&kick_condition, &mutex);
                kicked = false;
                pthread_mutex_unlock(&mutex);
                if (termination_request) return;
                processor->process();
            }
        }
        static std::string current_thread_scheduler()
        {
            struct sched_param priority;
            int policy;
            pthread_getschedparam(pthread_self(), &policy, &priority);
            if (policy == SCHED_RR)
                return "SCHED_RR";
            if (policy == SCHED_FIFO)
                return "SCHED_FIFO";
            return "SCHED_OTHER";
        }
            
        static int current_thread_priority()
        {
            struct sched_param priority;
            int policy;
            pthread_getschedparam(pthread_self(), &policy, &priority);
            return priority.sched_priority;
        }
    };
#endif

#ifdef win32threads
    /** Windows threads implementation of thread platform */
    class win32_threads_t : public thread_platform_t {
        /// The Event for signalling the kicking
        HANDLE kick_event;
        /// The condition for signalling the processing is finished.
        HANDLE catch_event;
        /// The Event for signalling termination.
        HANDLE termination_event;
        /// win32 thread priority
        long priority;
        /// The thread object
        HANDLE thread;
    public:
        /// Start signal processing in separate thread.
        void kick_thread() {
            SetEvent(kick_event);
        }
        /// Wait for signal processing to finish.
        void catch_thread() {
            WaitForSingleObject(catch_event, INFINITE);
        }
        /// Constructor.
        /// @param proc  Pointer to the associated signal processor
        /// @param thread_scheduler not used for win32 threads
        /// @param thread_priority Thread priority for worker thread as
        ///                        specified by MHA configuration.
        win32_threads_t(uni_processor_t * proc,
                        const std::string & thread_scheduler,
                        int thread_priority)
            : thread_platform_t(proc)
        {
            (void)thread_scheduler;
            kick_event = CreateEvent(0, false, false, 0);
            if (kick_event == 0)
                throw MHA_ErrorMsg("Cannot create win32 Event (kick_event)");
            catch_event = CreateEvent(0, false, false, 0);
            if (catch_event == 0) {
                CloseHandle(kick_event);
                kick_event = 0;
                throw MHA_ErrorMsg("Cannot create win32 Event (catch_event)");
            }
            termination_event = CreateEvent(0, false, false, 0);
            if (termination_event == 0) {
                CloseHandle(kick_event);
                kick_event = 0;
                CloseHandle(catch_event);
                catch_event = 0;
                throw MHA_ErrorMsg("Cannot create win32 Event"
                                   " (termination_event)");
            }
            bool setting_priority =
                thread_priority != INVALID_THREAD_PRIORITY;
            thread = CreateThread(0,0,
                                  win32_threads_t::thread_start, this,
                                  0,0);
            if (thread == 0) {
                CloseHandle(kick_event);
                kick_event = 0;
                CloseHandle(catch_event);
                catch_event = 0;
                CloseHandle(termination_event);
                termination_event = 0;
                throw MHA_ErrorMsg("Cannot create win32 thread");
            }
            if (setting_priority) {
                if (SetThreadPriority(thread, thread_priority)
                    == ((BOOL)0)) {
                    SetEvent(termination_event);
                    WaitForSingleObject(thread,100);
                    CloseHandle(kick_event);
                    kick_event = 0;
                    CloseHandle(catch_event);
                    catch_event = 0;
                    CloseHandle(termination_event);
                    termination_event = 0;
                    CloseHandle(thread);
                    thread = 0;
                    throw MHA_ErrorMsg("Cannot set priority of win32 thread");
                }
            }
            catch_thread(); // wait for thread to become ready.
        }
        /// Terminate thread
        ~win32_threads_t() {
            if (termination_event)
                SetEvent(termination_event);
            if (thread)
                WaitForSingleObject(thread,100);
            if (kick_event)
                CloseHandle(kick_event);
            kick_event = 0;
            if (catch_event)
                CloseHandle(catch_event);
            catch_event = 0;
            if (termination_event)
                CloseHandle(termination_event);
            termination_event = 0;
            if (thread)
                CloseHandle(thread);
            thread = 0;
         }
        /// Thread start function
        static DWORD WINAPI thread_start(void * thr) {
            static_cast<win32_threads_t *>(thr)->main();
            return 0;
        };
        /// Thread main loop.  Wait for process/termination trigger, then act.
        void main() {
            HANDLE events[2];
            events[0] = kick_event;
            events[1] = termination_event;
            for(;;) {
                SetEvent(catch_event);
                DWORD wait_result = 
                    WaitForMultipleObjects(2,events,false,INFINITE);
                if (wait_result == WAIT_OBJECT_0)
                    processor->process();
                else
                    return;
            }
        }
        static std::string current_thread_scheduler()
        {
            return "SCHED_OTHER";
        }
        
        static int current_thread_priority()
        {
            return GetThreadPriority(GetCurrentThread());
        }
    };
#endif

    /// Handles domain-specific partial input and output signal.
    class domain_handler_t : public uni_processor_t {
    private:
        /// Disallow copy constructor
        domain_handler_t(const domain_handler_t &);
        /// Disallow assignment operator
        domain_handler_t & operator=(const domain_handler_t &);
    public:
        /// Partial wave input signal
        MHASignal::waveform_t * wave_in;
        /// Partial wave output signal
        mha_wave_t ** wave_out;
        /// Partial spec input signal
        MHASignal::spectrum_t * spec_in;
        /// Partial spec input signal
        mha_spec_t ** spec_out;
        /// The domain-specific signal processing methods are implemented here.
        PluginLoader::fourway_processor_t * processor;

        /// Set parameters of input signal
        /// @param settings_in domain and dimensions of partial input signal
        void set_input_domain(const mhaconfig_t & settings_in)
        {
            if (wave_in != 0 || spec_in != 0)
                throw MHA_ErrorMsg("Attempt to set input domain when it is"
                                 " already set.");
            if (settings_in.domain == MHA_WAVEFORM)
                wave_in = new MHASignal::waveform_t(settings_in.fragsize,
                                                    settings_in.channels);
            else
                spec_in = new MHASignal::spectrum_t(settings_in.fftlen/2+1,
                                                    settings_in.channels);
        }            
        /// Set output signal parameters
        /// @param settings_out domain and dimensions of partial output signal
        void set_output_domain(const mhaconfig_t & settings_out)
        {
            if (wave_out != 0 || spec_out != 0)
                throw MHA_ErrorMsg("Attempt to set output domain when it is"
                                 " already set.");
            if (settings_out.domain == MHA_WAVEFORM) {
                wave_out = new mha_wave_t *;
                *wave_out = 0;
            }
            else {
                spec_out = new mha_spec_t *;
                *spec_out = 0;
            }
        }
        /// Deallocate domain indicators and signal holders
        void deallocate_domains()
        {
            delete wave_in; wave_in = 0;
            delete spec_in; spec_in = 0;
            delete wave_out; wave_out = 0;
            delete spec_out; spec_out = 0;
        }
        /** Construct a new domain handler once the domains and
         * dimensions of input and output signal of one of the child
         * plugins of split are known. */
        domain_handler_t(const mhaconfig_t & settings_in,
                         const mhaconfig_t & settings_out,
                         PluginLoader::fourway_processor_t * processor)
            : wave_in(0), wave_out(0), spec_in(0), spec_out(0)
        {
            set_input_domain(settings_in);
            set_output_domain(settings_out);
            this->processor = processor;
        }
        /// Deallocation of signal holders
        virtual ~domain_handler_t()
        {
            deallocate_domains();
            processor = 0;
        }

        /** Store the relevant channels from the input signal for
         * processing.  The number of channels to store is taken from
         * the dimensions of the partial input signal holder #wave_in.
         * @param s_in
         *    The combined waveform input signal.
         * @param start_channel
         *    The index (0-based) of the first channel in s_in to be
         *    copied to the partial input signal. 
         * @return
         *   The number of channels that were copied from the input signal */
        unsigned put_signal(mha_wave_t * s_in, unsigned start_channel)
        {
            if (wave_in == 0)
                throw MHA_ErrorMsg("Input signal domain was not configured as"
                                 " waveform");
            if ((start_channel + wave_in->num_channels) > s_in->num_channels)
                throw MHA_Error(__FILE__,__LINE__,
                                "Channel number mismatch: %u overall channels,"
                                " %u partial channels, start-index %u"
                                " (0-based)",
                                s_in->num_channels, wave_in->num_channels, 
                                start_channel);
            if (wave_in->num_frames != s_in->num_frames)
                throw MHA_ErrorMsg("Mismatch in number of frames between partial"
                                 " and combined input signal");
            for (unsigned ch = 0; ch < wave_in->num_channels; ++ch)
                wave_in->copy_channel(*s_in, ch+start_channel, ch);
            return wave_in->num_channels;
        }

        /** Store the relevant channels from the input signal for
         * processing.  The number of channels to store is taken from
         * the dimensions of the partial input signal holder #spec_in.
         * @param s_in
         *    The combined spectrum input signal.
         * @param start_channel
         *    The index (0-based) of the first channel in s_in to be
         *    copied to the partial input signal.
         * @return
         *   The number of channels that were copied from the input signal */
        unsigned put_signal(mha_spec_t * s_in, unsigned start_channel)
        {
            if (spec_in == 0)
                throw MHA_ErrorMsg("Input signal domain was not configured as"
                                 " spectrum");
            if ((start_channel + spec_in->num_channels) > s_in->num_channels)
                throw MHA_Error(__FILE__,__LINE__,
                                "Channel number mismatch: %u overall channels,"
                                " %u partial channels, start-index %u"
                                " (0-based)",
                                s_in->num_channels, spec_in->num_channels,
                                start_channel);
            if (spec_in->num_frames != s_in->num_frames)
                throw MHA_ErrorMsg("Mismatch in number of frames between partial"
                                 " and combined input signal");
            for (unsigned ch = 0; ch < spec_in->num_channels; ++ch)
                spec_in->copy_channel(*s_in, ch+start_channel, ch);
            return spec_in->num_channels;
        }

        /** Store all partial signal output channels in the combined
         * waveform signal with the given channel offset.  All
         * channels present in #wave_out will be copied.  Caller may
         * use (*wave_out)->num_channels to check the number of
         * channels in advance.
         * @param s_out
         *    The combined waveform output signal.
         * @param start_channel
         *    The channel offset (0-based) in s_out.
         * @return
         *   The number of channels that were copied to the output signal */
        unsigned get_signal(MHASignal::waveform_t * s_out,
                            unsigned start_channel)
        {
            if (wave_out == 0)
                throw MHA_ErrorMsg("Output signal domain was not configured as"
                                 " waveform");
            if (*wave_out == 0)
                throw MHA_ErrorMsg("No partial waveform output signal present");
            if ((start_channel + (*wave_out)->num_channels)
                > s_out->num_channels)
                throw MHA_Error(__FILE__,__LINE__,
                                "Channel number mismatch: %u overall channels,"
                                " %u partial channels, start-index %u"
                                " (0-based)",
                                s_out->num_channels, (*wave_out)->num_channels,
                                start_channel);
            if ((*wave_out)->num_frames != s_out->num_frames)
                throw MHA_ErrorMsg("Mismatch in number of frames between partial"
                                 " and combined output signal");
            for (unsigned ch = 0; ch < (*wave_out)->num_channels; ++ch)
                s_out->copy_channel(**wave_out, ch, ch+start_channel);
            return (*wave_out)->num_channels;
        }

        /** Store all partial signal output channels in the combined
         * spectrum signal with the given channel offset.  All
         * channels present in #spec_out will be copied.  Caller may
         * use (*spec_out)->num_channels to check the number of
         * channels in advance.
         * @param s_out
         *    The combined spectrum output signal.
         * @param start_channel
         *    The channel offset (0-based) in s_out.
         * @return
         *   The number of channels that were copied to the output signal */
        unsigned get_signal(MHASignal::spectrum_t * s_out, 
                            unsigned start_channel)
        {
            if (spec_out == 0)
                throw MHA_ErrorMsg("Output signal domain was not configured as"
                                 " spectrum");
            if (*spec_out == 0)
                throw MHA_ErrorMsg("No partial spectrum output signal present");
            if ((start_channel + (*spec_out)->num_channels)
                > s_out->num_channels)
                throw MHA_Error(__FILE__,__LINE__,
                                "Channel number mismatch: %u overall channels,"
                                " %u partial channels, start-index %u"
                                " (0-based)",
                                s_out->num_channels, (*spec_out)->num_channels,
                                start_channel);
            if ((*spec_out)->num_frames != s_out->num_frames)
                throw MHA_ErrorMsg("Mismatch in number of frames between partial"
                                 " and combined output signal");
            for (unsigned ch = 0; ch < (*spec_out)->num_channels; ++ch)
                s_out->copy_channel(**spec_out, ch, ch+start_channel);
            return (*spec_out)->num_channels;
        }
        /** Call the processing method of the processor with
         * configured input/output signal domains.  The input signal
         * has to be stored using #put_signal before this method may
         * be called. */
        void process()
        {
            if (wave_in && !spec_in && wave_out && !spec_out)
                processor->process(wave_in, wave_out);
            else if (wave_in && !spec_in && !wave_out && spec_out)
                processor->process(wave_in, spec_out);
            else if (!wave_in && spec_in && !wave_out && spec_out)
                processor->process(spec_in, spec_out);
            else if (!wave_in && spec_in && wave_out && !spec_out)
                processor->process(spec_in, wave_out);
            else
                throw MHA_ErrorMsg("Invalid combination (multiple or none) of"
                                 " input and/or output domains");
        }
    };

    /**
     * The splitted_part_t instance manages the plugin that performs
     * processing on the reduced set of channels.  The signal is split
     * by channels by this instance, but the signal is combined again
     * by the calling class. */
    class splitted_part_t : public MHAKernel::algo_comm_class_t {
        // Allow test class access to private fields.
        friend class Test_splitted_part_t;
        /// Disallow copy constructor
        splitted_part_t(const splitted_part_t &);
        /// Disallow assignment operator
        splitted_part_t & operator=(const splitted_part_t &);

        /** The plugin that performs the signal processing on the prepared 
         * channels. */
        PluginLoader::fourway_processor_t * plug;
        /** The domain specific signal handler, allocated from prepare
         * when input and output domains and signal parameters are
         * known. */
        domain_handler_t * domain;
        /** The platform-dependent thread synchronization implementation. */
        thread_platform_t * thread;
    public:
        splitted_part_t(const std::string & plugname,
                        MHAParser::parser_t * parent);
        splitted_part_t(PluginLoader::fourway_processor_t * plugin);
        ~splitted_part_t() throw();
        
        /** Delegates the prepare method to the plugin and allocates a
         * suitable MHAPlugin_Split::domain_handler_t instance. */
        void prepare(mhaconfig_t & signal_parameters,
                     const std::string & thread_platform,
                     const std::string & thread_scheduler,
                     int thread_priority);

        /** Delegates the release method to the plugin and deletes the
         * MHAPlugin_Split::domain_handler_t instance. */
        void release();

        /** Delegates parser incovation to plugin */
        std::string parse(const std::string & str) {return plug->parse(str);}

        /** The domain handler copies the input signal channels.  Then,
         * processing is initiated.
         * @param s_in
         *    The combined input signal.
         * @param start_channel
         *    The index (0-based) of the first channel in s_in to be
         *    copied to the partial input signal. 
         * @return
         *   The number of channels that were copied from the input signal */
        template <class SigType>
        unsigned trigger_processing(SigType * s_in, unsigned start_channel)
        {
            if (domain == 0 || thread == 0)
                throw MHA_ErrorMsg("Bug: Contained plugin is not prepared.");
            unsigned channels = domain->put_signal(s_in, start_channel);
            thread->kick_thread();
            return channels;
        }

        /** Wait until processing is finished, then copy the output data.
         * @param s_out
         *    The combined waveform output signal.
         * @param start_channel
         *    The channel offset (0-based) in s_out.
         * @return
         *   The number of channels that were copied to the output signal */
        template <class SigType>
        unsigned collect_result(SigType * s_out, unsigned start_channel)
        {
            if (domain == 0 || thread == 0)
                throw MHA_ErrorMsg("Bug: Contained plugin is not prepared.");
            thread->catch_thread();
            return domain->get_signal(s_out, start_channel);
        }
    };

    /// Load the plugin for this partial signal path.
    /** Loads the MHA plugin for a signal path of these audio channels.
     * @param plugname 
     *   The name of the MHA plugin, optionally followed by a colon
     *   and the algorithm name.
     * @param parent
     *   The parser node where the configuration of the new plugin is inserted.
     *   The plugin's parser name is the configured name (colon syntax).
     */
    splitted_part_t::splitted_part_t(const std::string & plugname,
                                     MHAParser::parser_t * parent)
        : plug(0), domain(0), thread(0)
    {
        PluginLoader::mhapluginloader_t * plugloader =
            new PluginLoader::mhapluginloader_t(MHAKernel::algo_comm_class_t::
                                                get_c_handle(),
                                                plugname);
        try {
            if (plugloader->has_parser() && parent != 0) {
                parent->insert_item(plugloader->get_configname(), plugloader);
            }
        } catch (MHA_Error & e) {
            delete plugloader;
            throw;
        }
        plug = plugloader;
    }
    /** Create the handler for the partial signal.  The plugin is
     * loaded by the caller, but it will be deleted by the destructor
     * of this class.  This constructor exists solely for testing purposes.
     * @param plugin The plugin used for processing the signal. The new
     * #splitted_part_t instance will take ownership of this instance and
     * release it in the destructor. */
    splitted_part_t::splitted_part_t(PluginLoader::fourway_processor_t * plugin)
        : plug(plugin), domain(0), thread(0)
    {}
    
    /// Destructor.  Deletes the plugin #plug.
    splitted_part_t::~splitted_part_t() throw()
    {
        delete plug; plug = 0; 
    }

    /// Prepare the loaded plugin.
    /** Plugin preparation. 
     * @param signal_parameters
     *   The signal description parameters for this path. 
     * @param thread_platform 
     *   The name of the thread platform to use. Possible values:
     *   "posix", "win32", "dummy". 
     * @param thread_scheduler
     *   The name of the scheduler to use. Posix threads support 
     *   "SCHED_OTHER", "SCHED_RR", "SCHED_FIFO".  The other thread
     *   platforms do not support different thread schedulers.
     *   This value is not used for platforms other than "posix".
     * @param thread_priority
     *   The new thread priority. Interpretation and permitted range
     *   depend on the thread platform and possibly on the scheduler. */
    void splitted_part_t::prepare(mhaconfig_t& signal_parameters,
                                  const std::string & thread_platform,
                                  const std::string & thread_scheduler,
                                  int thread_priority)
    {
        mhaconfig_t settings_in = signal_parameters;
        plug->prepare(signal_parameters);
        mhaconfig_t settings_out = signal_parameters;
        domain = new domain_handler_t(settings_in, settings_out, plug);
        if (false) {}
#if posixthreads
        else if (thread_platform == "posix")
            thread =
                new posix_threads_t(domain, thread_scheduler, thread_priority);
#else
#if win32threads
        else if (thread_platform == "win32")
            thread =
                new win32_threads_t(domain, thread_scheduler, thread_priority);
#endif // win32threads
#endif // !posixthreads
        else
            thread =
                new dummy_threads_t(domain, thread_scheduler, thread_priority);
    }

    /// Release the loaded plugin.
    /** Plugin release. */
    void splitted_part_t::release()
    {
        delete thread; thread = 0;
        delete domain; domain = 0;
        plug->release();
    }

    /// Implements split plugin
    /** An instance of class split_t implements the split plugin
     * functionality: The audio channels are splitted and groups of
     * audio channels are processed by different plugins in parallel.
     */
    class split_t : public MHAParser::parser_t
    {
    public:
        split_t(algo_comm_t iac,
                const std::string & chain_name,
                const std::string & algo_name);
        ~split_t();
        void prepare_(mhaconfig_t&);
        void release_();
        template <class SigTypeIn, class SigTypeOut>
        void process(SigTypeIn *, SigTypeOut **);
    private:
        void update();
        void clear_chains();
        mha_wave_t* copy_output_wave();
        mha_spec_t* copy_output_spec();

        /// Split the argument input signal to groups of channels for
        /// the plugins and initiate signal processing
        template <class SigType>
        void trigger_processing(SigType * s_in);
        /// Combine the output signal from the plugins
        template <class SigType>
        void collect_result(SigType * s_out);

        /// Reload plugins when the algos variable changes.
        MHAEvents::patchbay_t<split_t> patchbay;
        /// Vector of plugins to load in parallel
        MHAParser::vstring_t algos;
        /// Number of channels to route through each plugin
        MHAParser::vint_t channels;
        /// Thread platform chooser
        MHAParser::kw_t thread_platform;
        /// Scheduler used for worker threads
        MHAParser::kw_t worker_thread_scheduler;
        /// Priority of worker threads
        MHAParser::int_t worker_thread_priority;
        /// Scheduler of the signal processing thread
        MHAParser::string_mon_t framework_thread_scheduler;
        /// Priority of signal processing thread
        MHAParser::int_mon_t framework_thread_priority;
        /// Switch to activate parallel processing of plugins at the cost
        /// of one block of additional delay
        MHAParser::bool_t delay;
        /// Interfaces to parallel plugins
        std::vector<splitted_part_t*> chains;
        /// Combined output waveforms structure
        MHASignal::waveform_t* wave_out;
        /// Combined output spectra structure
        MHASignal::spectrum_t* spec_out;

        /// Waveform domain output signal structure accessor.
        /** Parameter is only for domain disambiguation and is ignored. */
        MHASignal::waveform_t * signal_out(mha_wave_t **)
        {return wave_out;}
        /// Spectrum domain output signal structure.  Parameter is ignored.
        MHASignal::spectrum_t * signal_out(mha_spec_t **)
        {return spec_out;}
    };

    /// Plugin constructor.
    /* @param iac
     *   Space for algorithm communication variables.  Currently,
     *   These varialbes are not used by the split plugin, and the splitted
     *   pathways get their own, fresh AC namespaces.
     * @param chain_name
     *   Historic parameter.
     * @param algo_name
     *   Configured name of the plugin. Accessible by this name in the 
     *   configuration language tree. */
    split_t::split_t(algo_comm_t iac,
                     const std::string & chain_name,
                     const std::string & algo_name)
        : MHAParser::parser_t("Split audio signal into channel groups and have"
                              " them processed by different plugins in"
                              " parallel"),
          algos("List of plugins which process the different groups of audio"
                " channels.","[]"),
          channels("Number of channels processed by the respective plugins.",
                   "[]",
                   "[0,["),
          thread_platform("Thread platform to use.\n"
                          "posix is the native linux thread platform,\n"
                          "win32 is the native thread platform on windows,\n"
                          "dummy means that all processing is performed in a"
                          " single thread.",
                          default_thread_platform_string,
                          "[posix win32 dummy]"),
          worker_thread_scheduler("Scheduler used for worker threads."
                                  " Only used for posix threads.\n"
                                  "Suggested setting is: The same as present"
                                  " in framework_thread_scheduler\n"
                                  "after prepare.",
                                  "SCHED_OTHER",
                                  "[SCHED_OTHER SCHED_RR SCHED_FIFO]"),
          worker_thread_priority("Priority assigned to worker threads. "
                                 " Suggested setting is: The same as\n"
                                 "present in framework_thread_priority after"
                                 "preparing the MHA.\n"
                                 "The default thread priority given here is"
                                 " invalid. No attempt will be made to\n"
                                 "set the priority of the threads"
                                 " if this value remains unchanged.",
                                 MHAParser::StrCnv::
                                 val2str(INVALID_THREAD_PRIORITY),
                                 "],["),
          framework_thread_scheduler("Scheduler used by the framework's"
                                     " processing thread.\n"
                                     "Only valid after first signal processing"
                                     " callback."),
          framework_thread_priority("Priority of the frameworks processing"
                                    " thread.\n"
                                    "Only valid after first signal processing"
                                    " callback."),
          delay("activates parallel processing of plugins at the cost of one block of additional delay","no"),
          wave_out(NULL),
          spec_out(NULL)
    {
        set_node_id( "split" );
        insert_item("algos", &algos);
        insert_item("channels", &channels);
        insert_item("thread_platform", &thread_platform);
        insert_item("worker_thread_scheduler", &worker_thread_scheduler);
        insert_item("worker_thread_priority", &worker_thread_priority);
        insert_item("framework_thread_scheduler", &framework_thread_scheduler);
        insert_item("framework_thread_priority", &framework_thread_priority);
        insert_member(delay);
        framework_thread_scheduler.data =
            worker_thread_scheduler.data.get_value();
        framework_thread_priority.data = worker_thread_priority.data;
        patchbay.connect(&algos.writeaccess,this,&split_t::update);
    }
    
    /// Plugin destructor. Unloads nested plugins.
    split_t::~split_t()
    {
        clear_chains();
    }

    /// Check signal parameters, prepare chains, and allocate output
    /// signal holders.
    void split_t::prepare_(mhaconfig_t & signal_parameters)
    {
        // Begin check parameters
        if (chains.size() == 0)
            throw MHA_ErrorMsg("No chain configured");
        if (chains.size() != algos.data.size())
            throw MHA_Error(__FILE__, __LINE__,
                            "Bug: chains.size(%u)!=algos.data.size(%u)",
                            unsigned(chains.size()), 
                            unsigned(algos.data.size()));
        if (algos.data.size() != channels.data.size())
            throw MHA_Error(__FILE__, __LINE__,
                            "Configuration variables algos (size %u) and"
                            " channels (size %u) need to have the same size"
                            " when the plugin is prepared",
                            unsigned(algos.data.size()), 
                            unsigned(channels.data.size()));
        
        unsigned total_channels_in = 0;
        for (unsigned i = 0; i < chains.size(); ++i) {
            if (channels.data[i] < 0)
                throw MHA_ErrorMsg("Bug: channels vector has negative entry");
            total_channels_in += channels.data[i];
        }
        if (total_channels_in != signal_parameters.channels)
            throw MHA_Error(__FILE__,__LINE__,
                            "Sum of channels in configration variable"
                            " \"channels\" (%u) does not match the number of"
                            " channels in the input signal (%u)",
                            total_channels_in,
                            signal_parameters.channels);
        // End check parameters
        
        // Begin prepare plugins
        mhaconfig_t chain_signal_parameters;
        mhaconfig_t signal_parameters_in = signal_parameters;
        mhaconfig_t signal_parameters_out;
        unsigned total_channels_out = 0;
        unsigned prepared_plugins = 0;
        try{
            for (unsigned i = 0; i < chains.size(); ++i) {
                chain_signal_parameters = signal_parameters;
                chain_signal_parameters.channels = channels.data[i];
                chains[i]->prepare(chain_signal_parameters,
                                   thread_platform.data.get_value(),
                                   worker_thread_scheduler.data.get_value(),
                                   worker_thread_priority.data);
                prepared_plugins =  i + 1;
                unsigned channels_out = chain_signal_parameters.channels;
                total_channels_out += channels_out;
                if (i == 0) {
                    signal_parameters_out = chain_signal_parameters;
                } 
                else {
                    signal_parameters_out.channels = channels_out;
                    PluginLoader::mhaconfig_compare(signal_parameters_out,
                                      chain_signal_parameters,
                                      "Parameters of the output signal from"
                                      " plugin " + 
                                      algos.data[i] + 
                                      " do not match those of the previously"
                                      " prepared plugin");
                }
            }
        }
        catch(...) {
            for(unsigned int k=0; k < prepared_plugins;k++){
                try{
                    chains[k]->release();
                }
                catch(...){
                }
            }
            throw;
        }
        // End prepare plugins

        signal_parameters = signal_parameters_out;
        signal_parameters.channels = total_channels_out;

        // Allocate combined output signal holder
        if (signal_parameters.domain == MHA_WAVEFORM) {
            wave_out = new MHASignal::waveform_t(signal_parameters.fragsize,
                                                 signal_parameters.channels);
        }
        else {
            spec_out = new MHASignal::spectrum_t(signal_parameters.fftlen/2+1,
                                                 signal_parameters.channels);
        }
        // End allocate signal holders

        algos.setlock(true);
        channels.setlock(true);
        delay.setlock(true);

        if (delay.data) {
            // perform the initial kick with a zero signal
            if (signal_parameters_in.domain == MHA_WAVEFORM) {
                MHASignal::waveform_t silent_wave(signal_parameters_in.fragsize,
                                                  signal_parameters_in.channels);
                trigger_processing(&silent_wave);
            } else {
                MHASignal::spectrum_t silent_spec(signal_parameters_in.fragsize,
                                                  signal_parameters_in.channels);
                trigger_processing(&silent_spec);
            }
        }
    }



    /// Delete output signal holder and release chains.
    void split_t::release_()
    {
        if (delay.data) {
            // perform the final catch and discard signal
            if (wave_out)
                collect_result(wave_out);
            else 
                collect_result(spec_out);
        }

        delay.setlock(false);
        channels.setlock(false);
        algos.setlock(false);

        // Begin release plugins and delete signal holders/pointers
        MHA_Error * lasterr = 0;
        for (unsigned i=0; i < chains.size(); i++) {
            try{
                chains[i]->release();
            }
            catch(const MHA_Error& e){
                // mhapluginloader#release can throw exceptions.
                if (lasterr == 0)
                    lasterr = new MHA_Error(e);
                else {
                    MHA_Error tmp(__FILE__,__LINE__,
                                  "Errors from multiple parallel paths:\n"
                                  "%s\nEarlier Error(s):\n%s",
                                  e.get_longmsg(), lasterr->get_longmsg());
                    delete lasterr;
                    lasterr = 0;
                    lasterr = new MHA_Error(tmp); 
                }
            }
        }
        delete wave_out;
        wave_out = 0;
        delete spec_out;
        spec_out = 0;
        // End release plugins and delete signal holders/pointers

        if (lasterr) {
            MHA_Error tmp_err(*lasterr);
            delete lasterr;
            throw tmp_err;
        }
    }

    template <class SigType>
    void split_t::trigger_processing(SigType * s_in)
    {
        unsigned ch_global, chain;
        for (ch_global = chain = 0; chain < chains.size(); ++chain) {
            ch_global += chains[chain]->trigger_processing(s_in, ch_global);
        }
        if (ch_global != s_in->num_channels)
            throw MHA_ErrorMsg("Input signal has too many channels");
    }

    template <class SigType>
    void split_t::collect_result(SigType * s_out)
    {
        if (s_out == 0)
            throw MHA_Error(__FILE__,__LINE__,
                            "This plugin is not configured to return"
                            " output signal of domain %s", 
                            typeid(SigType).name());
        unsigned ch_global, chain;
        for (ch_global = chain = 0; chain < chains.size(); ++chain) {
            ch_global += chains[chain]->collect_result(s_out, ch_global);
        }
        if (ch_global != s_out->num_channels)
            throw MHA_ErrorMsg("Too few output channels from plugins");
    }

    /// Let the parallel plugins process channel groups of the input signal.
    template <class SigTypeIn, class SigTypeOut>
    void split_t::process(SigTypeIn * s_in, SigTypeOut ** s_out)
    {
        if (s_out == 0)
            throw MHA_ErrorMsg("Bug: invalid output signal handle");
        if (framework_thread_priority.data == INVALID_THREAD_PRIORITY) {
            framework_thread_priority.data =
                default_thread_platform_type::current_thread_priority();
            framework_thread_scheduler.data =
                default_thread_platform_type::current_thread_scheduler();
        }
        if (delay.data) {
            collect_result(signal_out(s_out));
            trigger_processing(s_in);
        } else {
            trigger_processing(s_in);
            collect_result(signal_out(s_out));
        }
        *s_out = signal_out(s_out);
    }
    
    /// Unload the plugins.
    void split_t::clear_chains()
    {
        for(unsigned int k=0;k<chains.size();k++) {
            delete chains[k];
            chains[k] = 0;
        }
        chains.clear();
    }

    /// Load plugins in response to a value change in the algos variable
    void split_t::update()
    {
        clear_chains();
        try{
            for (unsigned chain = 0; chain < algos.data.size(); chain++) {
                chains.push_back(new splitted_part_t(algos.data[chain], this));
            }
        }
        catch(...){
            clear_chains();
            throw;
        }
    }
} // namespace MHAPlugin_Split

MHAPLUGIN_CALLBACKS(split,MHAPlugin_Split::split_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(split,MHAPlugin_Split::split_t,spec,spec)
MHAPLUGIN_PROC_CALLBACK(split,MHAPlugin_Split::split_t,spec,wave)
MHAPLUGIN_PROC_CALLBACK(split,MHAPlugin_Split::split_t,wave,spec)
MHAPLUGIN_DOCUMENTATION(split,"signalflow",
"The plugin 'split' takes a multi-channel input signal and splits it up\n"
"into separate chains of groups of channels. After processing of each\n"
"chain, the output channels are merged into a multi-channel output\n"
"signal.\n"
"The order of the audio channels is left unchanged.\n"
)

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
