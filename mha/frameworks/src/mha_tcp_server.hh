// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2018 HörTech gGmbH
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

#ifndef MHA_TCP_SERVER_HH
#define MHA_TCP_SERVER_HH

#include <string>
#include <memory>
#include <asio/ip/tcp.hpp>
#include <asio/streambuf.hpp>

/// namespace for network communication classes of MHA
namespace mha_tcp {
    /** An asio TCP socket with an associated streambuf buffer for receiving
     * lines of text, as well as string buffers for sending responses. 
     * Used for communicating with MHA TCP clients.  The life time of the
     * connection objects is managed with shared pointers registered together
     * with callbacks in the asio event loop.  This is a common idiom in asio.
     * To support this, we inherit from enable_shared_from_this which is also
     * common in code that uses asio. */
    class buffered_socket_t :
        public asio::ip::tcp::socket,
        public std::enable_shared_from_this<buffered_socket_t>
    {
        using asio::ip::tcp::socket::socket; // reuse base class constructors
        /** associated streambuf object to collect received pieces into lines */
        asio::streambuf streambuf;
        /** The message that is currently sent back to the client. */
        std::string current_message;
        /** A buffer for the next  message(s) that must be sent back to
         * the client after the sending of current_message has completed. */
        std::string next_message;
    public:
        /** Access to associated streambuf.  Needed to invoke async_read.
         * @return associated streambuf object by reference */
        asio::streambuf & get_buffer() {return streambuf;}

        /** Send the given message through this connection to the client
         * asynchronously.
         * @param message The text to send. Method copies the message before
         *                returning. */
        void queue_write(const std::string & message);
    };

    /** Class for accepting TCP connections from clients */
    class server_t {
        /// The io context used to run event loops
        asio::io_context io_context;
        /// The underlying asio object used to accept incoming TCP connections
        std::shared_ptr<asio::ip::tcp::acceptor> acceptor;
        /// Set to true when async_acceptance is triggered in trigger_accept
        /// (Only one accept can be in process at any time).
        bool async_accept_has_been_triggered = false;
        /// Number of accepted connections (not necessarily still existing)
        size_t num_accepted_connections = 0U;
        /// Weak pointers to the existing connections. Needed to shutdown
        /// the active connections for incoming data when server shuts down.
        std::vector<std::weak_ptr<buffered_socket_t> > connections;
    public:
        /** Allocates a TCP server.
         * @param interface Host name of the network interface to listen on.
         *                  Can be "localhost" or "127.0.0.1" for localhost,
         *                  "0.0.0.0" for any ipv4 interface, ...
         * @param port      TCP port to open for incoming connections.  If
         *                  port==0, then the operating system will select a
         *                  free port.
         * @throw system_error if the name given in interface cannot be resolved
         * @throw system_error if we cannot bind to the requested interface */
        server_t(const std::string & interface, uint16_t port);

        /** @return The port number of the TCP port that has been opened.
         *          If the port specified in the constructor was 0, this will
         *          return the port that the operating system has selected. */
        uint16_t get_port() const;

        /** @return The local endpoint of the acceptor. */
        asio::ip::tcp::endpoint get_endpoint() const;

        /** @return The ip adress that the server is bound to. */
        asio::ip::address get_address() const;

        /** @return the number of TCP connections that ha
            ve been accepted */
        size_t get_num_accepted_connections() const;

        /** Accepts connections on the TCP port and serves them.  Triggers
         * the acceptance of the next connection to start things off. */
        void run();

        /** This method is invoked when a line of text is received on one of
         * the accepted connections. Override this method to process the
         * communication with the client.
         * @param c the connection that has received this line
         * @param l the line that has been received, without the line ending
         * @return client should return true when client wants to read another
         *         line of text, else false.        */
        virtual bool on_received_line(std::shared_ptr<buffered_socket_t> c,
                                      const std::string & l);

        /** Shuts down the server: Close the acceptor (no new connections),
         * shuts down the receiving direction of all accepted connections
         * (no new commands, but responses can still be finished), registers
         * a timer event that will cause event loop termination and return from
         * the run() method 1 second in the future, giving us reasonably
         * enough time for the pending responses to be sent out. */
        virtual void shutdown();

        /** Make destructor virtual */
        virtual ~server_t() = default;

        /** @return the asio io context used to run the event loop */
        asio::io_context & get_context();

    private:
        /** Triggers the acceptance of the next connection. Called from
         * run to accept the first connection and from the accept handler to
         * accept each next connection. Once a connection from a client is
         * accepted, the accept handler will register it with the event loop 
         * for receiving a line of text. */
        void trigger_accept();

        /** Call trigger_read_line with a single detour through asio's
         * event loop to avoid starving other connections when one
         * connection floods us. */
        void post_trigger_read_line(std::shared_ptr<buffered_socket_t> c);

        /** Triggers the reading of the next line from a connection.
         * @param c The connection where the incoming data is expected from. */
        void trigger_read_line(std::shared_ptr<buffered_socket_t> c);

        /** Add new connection to the list of connections, retire stale pointers
         */
        void add_connection(std::shared_ptr<buffered_socket_t> connection) {
            connections.push_back(connection);
            // remove pointers whose connection has expired
            connections.
                erase(remove_if(connections.begin(), connections.end(),
                                [](const std::weak_ptr<buffered_socket_t> & p)
                                { return p.expired(); }),
                      connections.end());
        }
    };
}
#endif

/*
 * Local Variables:
 * compile-command: "make -C .."
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * coding: utf-8-unix
 * End:
 */
