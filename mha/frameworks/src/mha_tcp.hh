// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2008 2009 2011 2012 2013 2015 2016 HörTech gGmbH
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

#ifndef MHA_TCP_HH
#define MHA_TCP_HH

#include <string>
#include <set>
#include "mha_error.hh"

#ifdef _WIN32
# include <WINSOCK2.h>
namespace MHA_TCP {
    typedef HANDLE OS_EVENT_TYPE;
}
#else
# include <sys/time.h>
# include <netinet/in.h>
# include <netdb.h>
# define Sleep(x) usleep((x)*1000);
namespace MHA_TCP {
    typedef int SOCKET;
    struct OS_EVENT_TYPE {
        enum {R=0,W=1,X=2,T} mode;
        union {
            int fd;
            double timeout;
        };
    };
}
#endif

namespace MHA_TCP {
    // Error string functions
    /**
     * Portable conversion from error number to error string.
     */
    std::string STRERROR(int err);
    /**
     * Portable conversion from hostname error number to error string.
     */
    std::string HSTRERROR(int err);
    /**
     * Portable access to last network error number.
     */
    int N_ERRNO();
    /**
     * Portable access to last hostname error number.
     */
    int H_ERRNO();
    /**
     * Portable access to last non-network error number.
     */
    int G_ERRNO();
}

/**
 * A Namespace for TCP helper classes 
 */
namespace MHA_TCP {

    /**
     * Time access function for system's high resolution time,
     * retrieve current time as double.
     */
    double dtime();
    
#ifndef _WIN32
    /**
     * Time access function for unix' high resolution time,
     * converts struct timeval to double.
     */
    double dtime(const struct timeval & tv);

    /**
     * Time access function for unix' high resolution time,
     * converts time from double to struct timeval.
     */
    struct timeval stime(double d);
#endif


    /**
     * A base class for asynchronous wakeup events.
     */ 
    class Wakeup_Event {
        /**
         * A list of all Event_Watcher instances that this Wakeup_Event is
         * observed by (stored here for proper deregistering).
         */
        std::set<class Event_Watcher *> observers;
    protected:
        OS_EVENT_TYPE os_event;
        bool os_event_valid;
    public:
        /**
         * Event Constructor. The new event has invalid state.
         */
        Wakeup_Event();

        /**
         * Called by the Event_Watcher when this event is added to its list of
         * observed events.
         */
        virtual void observed_by(Event_Watcher * observer);
        /**
         * Called by the Event_Watcher when this event is removed from its
         * list of observed events.
         */
        virtual void ignored_by(Event_Watcher * observer);
        /**
         * Destructor deregisters from observers.
         */
        virtual ~Wakeup_Event();
        /**
         * Get necessary information for the Event Watcher.
         */
        virtual OS_EVENT_TYPE get_os_event();
        /**
         * For pure notification events, reset the "signalled" status
         */
        virtual void reset();
        /**
         * Query wether the event is in signalled state now.
         */
        virtual bool status();
    };

    /** 
        \brief Portable Multiplexable cross-thread notification
    */
    class Async_Notify : public Wakeup_Event {
#ifndef _WIN32
        int pipe[2];
#endif
    public:
        Async_Notify();
        virtual void reset();
        virtual void set();
        virtual ~Async_Notify();
    };
    /**
     * OS-independent event watcher, uses select on Unix and
     * WaitForMultipleObjects on Windows.
     */
    class Event_Watcher {
        /** The list of events to watch. */
        std::set<Wakeup_Event*> events;
    public:
        typedef std::set<Wakeup_Event*> Events;
        typedef std::set<Wakeup_Event*>::iterator iterator;
        /** Add an event to this observer. */
        void observe(Wakeup_Event * event);
        /** Remove an event from this observer */
        void ignore(Wakeup_Event * event);
        /**\ Wait for some event to occur. Return all events that are ready */
        std::set<Wakeup_Event *> wait();
        virtual ~Event_Watcher();
    };
    
    class Timeout_Event : public Wakeup_Event {
#ifndef _WIN32
        double end_time;
#endif
    public:
        Timeout_Event(double interval);
        virtual OS_EVENT_TYPE get_os_event();
    };

    /**
     * OS-independent event watcher with internal fixed-end-time timeout.
     */
    class Timeout_Watcher : public Event_Watcher {
        Timeout_Event timeout;
    public:
        explicit Timeout_Watcher(double interval);
        virtual ~Timeout_Watcher();
    };

    /** Watch socket for incoming data */
    class Sockread_Event : public Wakeup_Event {
    public:
        /** Set socket to watch for.
         * @param s The socket to observe incoming data on. */
        Sockread_Event(SOCKET s);
    };
    class Sockwrite_Event : public Wakeup_Event {
    public:
        Sockwrite_Event(SOCKET s);
    };

    class Sockaccept_Event : public Wakeup_Event {
    public:
        Sockaccept_Event(SOCKET);
    };

    class Server;
    /**
     * Connection handles Communication between client and server,
     * is used on both sides.
     */
    class Connection {
        std::string outbuf, inbuf;

        Sockread_Event * read_event;
        Sockwrite_Event * write_event;
        bool closed;
        struct sockaddr_in peer_addr;

        /**
         * determine peer address and port
         */
        void init_peer_data();
        /**
         * Determine wether at least 1 byte can be read without blocking.
         */
        bool can_sysread();
        /**
         * Determine wether at least 1 byte can be written without blocking.
         */
        bool can_syswrite();
        /**
         * Call the system's read function and try to read bytes.
         * This will block in a situation where can_sysread returns false.
         * @param bytes
         *   The desired number of characters.
         * @return
         *   The characters read from the socket. The result may have fewer
         *   characters than specified by bytes.
         *   If the result is an empty string, then the socket has been
         *   closed by the peer.
         */
        std::string sysread(unsigned bytes);
        /**
         * Call the system's write function and try to write all characters
         * in the string data. May write fewer characters, but will at least
         * write one character.
         * @param data
         *    A string of characters to write to the socket.
         * @return
         *    The rest of the characters that have not yet been written.
         */
        std::string syswrite(const std::string & data);
    protected:
        /**
         * The file descriptor of the TCP Socket.
         */
        SOCKET fd;

        friend class Server;
        /**
         * Create a connection instance from a socket filedescriptor.
         * @param _fd  The file descriptor of the TCP Socket. 
         *           This file descriptor is closed again in the destructor.
         * @throw MHA_Error If the file descriptor is < 0.
         */
        Connection(SOCKET _fd);
    public:
        Sockread_Event * get_read_event();
        Sockwrite_Event * get_write_event();

        /**
         * Get peer's IP Address
         */
        std::string get_peer_address();
        /**
         * Get peer's TCP port
         */
        unsigned short get_peer_port();
        /**
         * Return the (protected) file descriptor of the connection. 
         * Will be required for SSL.
         */
        SOCKET get_fd() const {return fd;};

        /**
         * Destructor closes the underlying file descriptor
         */
        virtual ~Connection();

        /**
         * Checks if the peer has closed the connection. As a side effect,
         * this method fills the internal "incoming" buffer if it was empty
         * and the socket is readable and not eof.
         */
        bool eof();
        
        /**
         * Checks if a full line of text has arrived by now. This method
         * reads data from the socket into the internal "incoming" buffer if
         * it can be done without blocking.
         * @param delim
         *   The line delimiter.
         * @return
         *   true if at least one full line of text is present in the internal
         *   buffer after this method call, false otherwise.
         */
        bool can_read_line(char delim = '\n');

        /**
         * Checks if the specified ammount of data can be read. This method
         * reads data from the socket into an internal "incoming" buffer if it
         * can be done without blocking.
         * @param howmany
         *   The number of bytes that the caller wants to have checked.
         * @return
         *   true if at least the specified ammount of data is present in the
         *   internal buffer after this method call, false otherwise
         */
        bool can_read_bytes(unsigned howmany);

        /**
         * Reads a single line of data from the socket. Blocks if necessary.
         * @param delim The line delimiter.
         * @return      The string of characters in this line, including the
         *              trailing delimiter. The delimiter may be missing if
         *              the last line before EOF does not have a delimiter.
         */
        std::string read_line(char delim = '\n');
        /**
         * Reads the specified ammount of dat from the socket.
         * Blocks if necessary.
         * @param howmany The number of bytes to read.
         * @return      The string of characters read.
         *              The string may be shorter if EOF is encountered.
         */
        std::string read_bytes(unsigned howmany);
        
        /**
         * Adds data to the internal "outgoing" buffer, and then tries to write
         * as much data from that buffer to the socket as possible without
         * blocking.
         * @param data
         *   data to send over the socket.
         */
        void try_write(const std::string & data = "");

        /**
         * Adds data to the internal "outgoing" buffer, and then writes that
         * that buffer to the socket, regardless of blocking.
         * @param data
         *   data to send over the socket.
         */
        void write(const std::string & data = "");

        /**
         * Checks if the internal "outgoing" buffer contains data.
         */
        bool needs_write();

        /**
         * Returns the number of bytes in the internal "incoming" buffer.
         */
        unsigned buffered_incoming_bytes() const;

        /**
         * Returns the number of bytes in the internal "outgoing" buffer.
         */
        unsigned buffered_outgoing_bytes() const;
    };

    class Server {
        sockaddr_in sock_addr;
        SOCKET serversocket;
        std::string iface;
        unsigned short port;
        Sockaccept_Event * accept_event;
        void initialize(const std::string & iface, unsigned short port);
    public:
        /**
         * Create a TCP server socket.
         * @param port  The TCP port to listen to.
         * @param iface The network interface to bind to.
         */
        Server(unsigned short port = 0, const std::string & iface = "0.0.0.0");
        /**
         * Create a TCP server socket.
         * @param port  The TCP port to listen to.
         * @param iface The network interface to bind to.
         */
        Server(const std::string & iface, unsigned short port = 0);
        /**
         * Close the TCP server socket.
         */
        ~Server();
        /**
         * Get the name given in the constructor for the network interface.
         */
        std::string get_interface() const;
        /**
         * Get the port that the TCP server socket currently listens to.
         */
        unsigned short get_port() const;
        /**
         * Produces an event that can be observed by an Event_Watcher. This
         * event signals incoming connections that can be accepted.
         */
        Sockaccept_Event * get_accept_event();

        /**
         * Accept an incoming connection. blocks if necessary.
         * @return The new TCP connection. The connection has to be deleted
         *         by the caller.
         */
        Connection * accept();

        /**
         * Accept an incoming connection if it can be done without blocking.
         * @return The new TCP connection or 0 if there is no immediate
         *         connection. The connection has to be deleted by the caller.
         */
        Connection * try_accept();
    };

    /**
     * A portable class for a tcp client connections.
     */
    class Client : public Connection {
    public:
        /**
         * Constructor connects to host, port via TCP.
         * @param host The hostname of the TCP Server.
         * @param port The port or the TCP Server.
         */
        Client(const std::string & host, unsigned short port);

        /**
         * Constructor connects to host, port via TCP, using a timeout.
         * @param host The hostname of the TCP Server.
         * @param port The port or the TCP Server.
         * @param timeout_watcher an Event watcher that implements a timeout.
         */
        Client(const std::string & host,
               unsigned short port,
               Timeout_Watcher & timeout_watcher);
    };

    /**
     * A very simple class for portable threads.
     */
    class Thread {
#ifdef _WIN32
        /**
         * The win32 Thread Handle. This handle reaches the "signalled"
         * state when the thread exits.
         */
        HANDLE thread_handle;
        /**
         * The win32 Thread ID. Required for Windows9x.
         */
        DWORD thread_id;
#else
        /**
         * The posix thread handle.
         */
        pthread_t thread_handle;
        /**
         * The posix thread attribute structure. Required for starting a thread
         * in detached state. Detachment is required to eliminate the need for
         * joining this thread.
         */
        pthread_attr_t thread_attr;
#endif
    protected:
        /** The argument for the client's thread function. */
        void * arg;
        /**
         * The return value from the client's thread function is stored here
         * When that function returns.
         */
        void * return_value;
        /**
         * Default constructor may only be used by derived classes that want
         * to start the thread themselves.
         */
        Thread();
    public:
        /**
         * Event will be triggered when the thread exits.
         */
        Async_Notify thread_finish_event;
        /**
         * The current state of the thread.
         */ 
        enum {PREPARED, RUNNING, FINISHED} state;
        /**
         * The thread function signature to use with this
         * class. Derive from this class and call protected standard
         * constructor to start threads differently.
         */
        typedef void* (*thr_f)(void*);
        /**
         * The thread function that the client has registered.
         */
        thr_f thread_func;
        /** The argument that the client wants to be handed through to the
         * thread function. */
        void * thread_arg;
        /** Constructor starts a new thread.
         * @param func The function to be executed by the thread.
         * @param arg The argument given to pass to the thread function.
         */
        Thread(thr_f func, void * arg = 0);
        /**
         * The destructor should only be called when the Thread is
         * finished.  There is preliminary support for forceful thread
         * cancellation in the destructor, but probably not very
         * robust or portable..
         */
        virtual ~Thread();
        /**
         * The internal method that delegated the new thread to the registered
         * Thread function.
         */
        virtual void run();
        /**
         * The MHA_Error that caused the thread to abort, if any.
         */
        MHA_Error * error;
    };
}

// Local Variables:
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
#endif
