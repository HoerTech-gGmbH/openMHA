// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2018 2019 HörTech gGmbH
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

#include "mha_tcp_server.hh"
#include <asio/read_until.hpp>
#include <asio/write.hpp>

namespace mha_tcp {
    server_t::server_t(const std::string & interface,
                       uint16_t port)
    {
        // A single hostname + port may resolve to multiple ip addresses
        const auto endpoints = asio::ip::tcp::resolver(io_context).
            resolve(interface, std::to_string(port));
        acceptor = std::
            make_shared<asio::ip::tcp::acceptor>(io_context,
                                                 *endpoints.begin(), // use 1st
                                                 true); // reuse-port early
        acceptor->listen();
    }

    asio::ip::tcp::endpoint server_t::get_endpoint() const {
        return acceptor->local_endpoint();
    }

    uint16_t server_t::get_port() const {
        return acceptor->local_endpoint().port();
    }

    asio::ip::address server_t::get_address() const {
        return acceptor->local_endpoint().address();
    }

    size_t server_t::get_num_accepted_connections() const {
        // Total number of completed accept operations. Only used for tests.
        return num_accepted_connections;
    }

    void server_t::run() {
        trigger_accept();
        get_context().run();
    }

    bool server_t::on_received_line(std::shared_ptr<buffered_socket_t> c,
                                    const std::string & l)
    {
        // default implementation. To be overriden by derived classes.
        return false;
        // return false means: read no more lines. Will cause this connection
        // to be closed because the shared pointer holding it in memory goes
        // out of scope and no new shared_ptr is registered anywhere.
        // Derived classes will usually return true.
    }

    void server_t::trigger_accept() {
        if (async_accept_has_been_triggered) {
            // we can only accept one connection at a time.
            throw std::logic_error("network acceptance cannot be triggered"
                                   " while another acceptance is still in"
                                   " progress");
        }
        // We need to allocate the socket to be used for the future accepted
        // connection in advance
        std::shared_ptr<buffered_socket_t> connection =
            std::make_shared<buffered_socket_t>(get_context());

        // Set flag that we have a pending acceptance job in asio
        async_accept_has_been_triggered = true;
        acceptor->async_accept(*connection,
                               // The acceptance handler (lambda) keeps the
                               // connection (socket) object in memory by
                               // capturing the shared_ptr by value.
                               [this,connection](const asio::error_code& ec) {
                                   // When this handler is called, then asio
                                   // has finished the async_accept request
                                   async_accept_has_been_triggered = false;
                                   if (!ec) {
                                       // in the absence of errors we have a new
                                       // connection
                                       trigger_read_line(connection);
                                       ++num_accepted_connections;
                                       add_connection(connection);
                                   }
                                   // Trigger acceptance of the next connection
                                   trigger_accept();
                               });
    }

    void server_t::post_trigger_read_line(std::shared_ptr<buffered_socket_t> c)
    {
        // Call trigger_read_line with a detour through asio's scheduler
        // to avoid starving other connections when one connection floods us
        asio::post(get_context(),[this,c](){trigger_read_line(c);});
    }

    void server_t::trigger_read_line(std::shared_ptr<buffered_socket_t> c)
    {
        using asio::async_read_until;
        async_read_until(*c, c->get_buffer(), '\n',
                         // The read handler (lambda) keeps the connection
                         // object (socket + buffers) in memory by
                         // capturing the shared_ptr by value.
                         [this,c](const asio::error_code& ec, std::size_t) {
                             // When this handler is called, then asio has
                             // received a line of text in the input buffer
                             if (ec) {
                                 // There was an error, probably end-of-input.
                                 // We will not reregister for reading.
                                 // Already queued write requests will still be
                                 // served unless they also cause errors.
                                 // Not re-registering the shared_ptr
                                 // will cause it to be closed and destructed,
                                 // either now or after any pending write
                                 // requests.
                             } else {
                                 // get the received line from the input buffer
                                 std::string line;
                                 std::istream istream(&c->get_buffer());
                                 std::getline(istream, line);
                                 // remove end-of-line
                                 while (line.size() && (line.back() == '\r' ||
                                                        line.back() == '\n')) {
                                     line.resize(line.size()-1);
                                 }
                                 // let client handle the received line
                                 if (on_received_line(c, line)) {
                                     // client returned true, reregister for
                                     // reading next line
                                     post_trigger_read_line(c);
                                 }
                             }
                         });
    }

    asio::io_context & server_t::get_context()
    {
        return io_context;
    }

    void server_t::shutdown()
    {
        // refuse any new connections
        acceptor->close();
        // close the input data side on all connections.  No new commands
        // will be received.
        for (auto weak_connection : connections) {
            if (weak_connection.use_count()) {
                auto connection = weak_connection.lock();
                try {
                    connection->shutdown(asio::ip::tcp::socket::shutdown_receive);
                }
                catch(asio::system_error& e){
                    if(e.what()==std::string("shutdown: Socket is not connected")){}
                    else throw;
                }
            }
        }
        // in 1 second, terminate the event loop. This should give us enough
        // time to respond to the cmd=quit command with a "success" message.
        std::shared_ptr<asio::steady_timer> t =
            std::make_shared<asio::steady_timer>(get_context(),
                                                 std::chrono::seconds(1));
        t->async_wait([this,t](std::error_code){get_context().stop();});
    }

    void buffered_socket_t::queue_write(const std::string & message) {
        std::shared_ptr<buffered_socket_t> self = this->shared_from_this();
        next_message += message;
        if (next_message.size() && current_message.empty()) {
            current_message.swap(next_message);
            using asio::async_write;
            async_write(*this, asio::buffer(current_message),
                        // The lambda keeps this socket and the buffers
                        // in memory through copy-capturing the shared_ptr.
                        [self,this](const asio::error_code & ec, std::size_t) {
                            if (ec) {
                                // There was an error sending the last message.
                                // Connection closed?  Do not re-register for
                                // writing.
                            } else {
                                current_message.clear();
                                if (next_message.size()) {
                                    // We already have a next message, send it
                                    queue_write("");
                                }
                            }
                        });
        }
    }
}

/*
 * Local Variables:
 * compile-command: "make -C .."
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * coding: utf-8-unix
 * End:
 */
