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
#include "mha_os.h"
#include <gmock/gmock.h>
#include <asio/connect.hpp>
#include <asio/read.hpp>
#include <asio/write.hpp>
#include <thread>

using mha_tcp::server_t;

const uint16_t any_port = 0U;

TEST(server_test, os_finds_free_port_when_port_is_0) {
  server_t server_1("127.0.0.1", any_port);
  server_t server_2("127.0.0.1", any_port);

  // The OS should have assigned us a port between 1024 and 65535
  EXPECT_NE(0U, server_1.get_port());
  EXPECT_GE(server_1.get_port(), 1024U);
  EXPECT_LE(server_1.get_port(), 65535U);

  EXPECT_NE(0U, server_2.get_port());
  EXPECT_GE(server_2.get_port(), 1024U);
  EXPECT_LE(server_2.get_port(), 65535U);

  // two open sockets should listen on different port numbers
  EXPECT_NE(server_1.get_port(), server_2.get_port());
}

TEST(server_test, invalid_interface_name_throws) {
  // A name that cannot be resolved to an IP will throw a system error
  EXPECT_THROW(server_t server("invalid.42", any_port),
               std::system_error);

  // A name that resolves to an IP but which we cannot listen on will also
  // throw a system error
  EXPECT_THROW(server_t server("google.com", any_port),
               std::system_error);
}

TEST(server_test, two_identical_ports_throw) {
  // We cannot listen on a port that is already in use. Either the first
  // constructor will throw because the port is already opened by another
  // process, or the second constructor will throw because the first object
  // already has the port.

  // special case: on windows, no exception is thrown, skip test
#ifndef __WIN32
  // Also on windows subsystem for linux, no exception is thrown
  if (system("uname -r | grep -qi microsoft")) {
    // if we are here, we are neither on windows directly nor in its subsystem
    // for linux.  We expect an exception when opening the same port twice
    EXPECT_THROW(server_t server1("0.0.0.0", 2222);
                 server_t server2("0.0.0.0", 2222),
                 std::system_error);
  }
#endif // __WIN32
}

TEST(server_test, can_find_free_port_of_choice_w_enough_tries)
{
  uint16_t port = 1024U;
  while (port < 10000U) {
    // range 1024-9999 should be more than enough to find a free port
    try {
      server_t server("0.0.0.0", port);
      break; // success, break out of while loop
    } catch (std::system_error &) {
      port++; // We could not use this port, try next
    }
  }
  EXPECT_LT(port, 10000U); // should have found a free port below 10000
}

TEST(server_test, canonical_ipv4_interface_choices_work)
{
  const std::string interfaces[] = {
    "localhost", // localhost by name
    "127.0.0.1", // ipv4 localhost
    "0.0.0.0",   // ipv4 any
  };
  for (auto interface : interfaces) {
    EXPECT_NO_THROW(server_t server(interface,any_port))
      << "Allocating port on interface " << interface;
  }
}

#include <unistd.h>
// This function tests if we run in docker.  This is specific to our build farm:
// The host name of docker build agents is set to something containing "docker".
inline bool inside_docker(void) {
#ifdef __WIN32
  return false; // if this is windows, then this is not docker.
#else
  const size_t s = 256U; // maximum host name length is 255, says man page
  char hostname[s] = "";
  gethostname(hostname, s);
  hostname[s-1] = 0; // ensure C string termination
  return strstr(hostname, "docker");
#endif
}

TEST(server_test, canonical_ipv6_interface_choices_work)
{
  const std::string interfaces[] = {
    "::1",       // ipv6 localhost
    "::",        // ipv6 any
  };
  for (auto interface : interfaces) {
    if (not inside_docker()) {// docker containers are not ipv6-enabled
      EXPECT_NO_THROW(server_t server(interface,any_port))
        << "Allocating port on interface " << interface;
    }
  }
}

TEST(server_test, client_can_connect)
{
  server_t server("127.0.0.1",any_port);
  asio::ip::tcp::socket client(server.get_context());
  bool client_connected = false; // set to true in connect handler
  asio::async_connect(client,
                      asio::ip::tcp::resolver(server.get_context()).
                      resolve("127.0.0.1", std::to_string(server.get_port())),
                      [&](const asio::error_code & ec,
                          const asio::ip::tcp::endpoint & endpoint) {
                        // googletest assertions would throw from this handler
                        // through the run function and abort it
                        ASSERT_FALSE(ec); // assert no error: successful connect
                        client_connected = true;

                        // Check that we have connected to the expected server
                        ASSERT_EQ(asio::ip::make_address("127.0.0.1"),
                                  endpoint.address());
                        ASSERT_EQ(server.get_port(), endpoint.port());
                      });
  // We cannot be connected before the event loop executes
  ASSERT_FALSE(client_connected);
  // limit the time spent in the event loop to 1 second (it should exit sooner)
  server.get_context().run_for(std::chrono::seconds(1));
  // check that we had a successful connection
  ASSERT_TRUE(client_connected);
}

TEST(server_test, server_accepts_client)
{
  server_t server("127.0.0.1",any_port);
  asio::ip::tcp::socket client(server.get_context());

  // trigger the client connection
  asio::async_connect(client, asio::ip::tcp::resolver(server.get_context()).
                      resolve("127.0.0.1", std::to_string(server.get_port())),
                      [](const asio::error_code &,
                         const asio::ip::tcp::endpoint &) {});

  // We have not accepted a connection before the event loop executes
  ASSERT_EQ(0U, server.get_num_accepted_connections());

  // limit the time spent in the event loop to 1 second
  asio::steady_timer t(server.get_context(), std::chrono::seconds(1));
  t.async_wait([&server](const asio::error_code&)
               {server.get_context().stop();});

  // Triggers the acceptance of incoming connections and starts event loop
  server.run();

  // The event loop should have handled the connection and the acceptance by now
  ASSERT_EQ(1U, server.get_num_accepted_connections());
}

TEST(server_test, disallows_multiple_calls_to_run)
{
  // The program should call run once and let the event handler run
  // its course.  It is most likely an error if the event loop is
  // terminated and started a second time.
  server_t server("127.0.0.1",any_port);

  // limit the time spent in the event loop to 1 millisecond (the application
  // should not do something like this)
  asio::steady_timer t(server.get_context(), std::chrono::milliseconds(1));
  t.async_wait([&server](const asio::error_code&)
               {server.get_context().stop();});
  // Triggers the acceptance of incoming connections and starts event loop
  server.run();

  // limit the time spent in the event loop again to 1 millisecond
  t.async_wait([&server](const asio::error_code&)
               {server.get_context().stop();});

  // starting the event loop a second time is most likely an error.
  EXPECT_THROW(server.run(), std::logic_error);
}

TEST(server_test, on_received_line_default_impl_returns_false)
{
  // The default reaction to receiving a line of text would be to not register
  // for another line of text by returning false from the content handler.
  EXPECT_FALSE(server_t("127.0.0.1",any_port).
               on_received_line(nullptr, ""));
}

TEST(server_test, server_reads_client)
{
  class mock_server_t : public server_t {
  public:
    using server_t::server_t; // constructor is unmodified
    MOCK_METHOD2(on_received_line,
                 bool(std::shared_ptr<mha_tcp::buffered_socket_t>,
                      const std::string &));
  };

  mock_server_t server("127.0.0.1",any_port);
  asio::ip::tcp::socket client(server.get_context());

  // trigger the client connection, send three commands (last one incomplete)
  asio::async_connect(client, asio::ip::tcp::resolver(server.get_context()).
                      resolve("127.0.0.1", std::to_string(server.get_port())),
                      [&client](const asio::error_code & ec,
                         const asio::ip::tcp::endpoint &) {
                        ASSERT_FALSE(ec);
                        // We send the first line in two fragments...
                        std::string s = "cm";
                        asio::write(client,asio::buffer(s));
                        // With the final fragment of the first line with unix
                        // line-ending we also send a complete second line
                        // with dos line-ending and an incomplete third line.
                        s = "d1\n" "cmd2 \r\n" "cmd...";
                        asio::write(client,asio::buffer(s));
                      });

  // limit the time spent in the event loop to 1 second
  asio::steady_timer t(server.get_context(), std::chrono::seconds(1));
  t.async_wait([&server](const asio::error_code&)
               {server.get_context().stop();});

  // set expectations to receive separate "cmd1" and "cmd2 " as complete lines
  // in sequence without the line-endings. The trailing "cmd..." has no line
  // ending, server continues to wait for its line-ending before invoking
  // on_received_line for the third line, which does not happen during this
  // test.
  { ::testing::InSequence check_order;
    using ::testing::_; using ::testing::StrEq; using ::testing::Return;
    EXPECT_CALL(server, on_received_line(_,StrEq("cmd1"))).
      WillOnce(Return(true));
    EXPECT_CALL(server, on_received_line(_,StrEq("cmd2 "))).
      WillOnce(Return(true));
  }

  // Triggers the acceptance of incoming connections and starts event loop
  server.run();

  // Destructor of server will check that both expected messages were received
}

TEST(server_test, server_answers)
{
  class answering_server_t : public server_t {
  public:
    using server_t::server_t; // constructor is unmodified
    bool on_received_line(std::shared_ptr<mha_tcp::buffered_socket_t> c,
                          const std::string & line) override {
      c->queue_write("Thanks for your message '" + line + "'\r\n");
      return true;
    }
  };

  answering_server_t server("127.0.0.1",any_port);
  asio::ip::tcp::socket client(server.get_context());

  // connect and send two commands
  asio::async_connect(client, asio::ip::tcp::resolver(server.get_context()).
                      resolve("127.0.0.1", std::to_string(server.get_port())),
                      [&client](const asio::error_code & ec,
                         const asio::ip::tcp::endpoint &) {
                        ASSERT_FALSE(ec);
                        // We send two lines at once.  Receiver separates them.
                        std::string s = "cmd1\n" "cmd2 \r\n";
                        client.send(asio::const_buffer(s.data(), s.size()));
                      });

  // limit the time spent in the event loop to 1 second and run the event loop
  asio::steady_timer t(server.get_context(), std::chrono::seconds(1));
  t.async_wait([&server](const asio::error_code&)
               {server.get_context().stop();});
  server.run();

  const std::string expected_responses =
    "Thanks for your message 'cmd1'\r\n"
    "Thanks for your message 'cmd2 '\r\n";
  // Inspect the responses
  std::string actual_responses(expected_responses.size(), '\0');
  asio::read(client, asio::buffer(actual_responses));
  EXPECT_EQ(expected_responses, actual_responses);
}

TEST(server_test, server_shutdown_refuses_connections)
{
  server_t server("127.0.0.1",any_port);
  asio::ip::tcp::socket client(server.get_context());

  auto endpoints = asio::ip::tcp::resolver(server.get_context()).
    resolve("127.0.0.1", std::to_string(server.get_port()));

  server.shutdown();

  // cannot connect anymore
  asio::async_connect(client, endpoints,
                      [&client](const asio::error_code & ec,
                         const asio::ip::tcp::endpoint &) {
                        ASSERT_TRUE(ec); // non-zero error code
                      });

  // limit the time spent in the event loop to 1 second and run the event loop
  asio::steady_timer t(server.get_context(), std::chrono::seconds(1));
  t.async_wait([&server](const asio::error_code&)
               {server.get_context().stop();});
  server.run();
}

TEST(server_test, server_shutdown_closes_connection_for_reading)
{
  class count_server_t : public server_t {
  public:
    size_t received_msg = 0;
    using server_t::server_t; // constructor is unmodified
    bool on_received_line(std::shared_ptr<mha_tcp::buffered_socket_t>,
                          const std::string & line) override
    {return ++received_msg;}
  };

  count_server_t server("127.0.0.1",any_port);
  auto endpoints = asio::ip::tcp::resolver(server.get_context()).
    resolve("127.0.0.1", std::to_string(server.get_port()));
  asio::ip::tcp::socket client(server.get_context());
  asio::connect(client, endpoints);
  std::thread thread([&server](){server.run();});
  ASSERT_EQ(0U, server.received_msg);
  client.send(asio::const_buffer("\n", 1U));
  mha_msleep(50); // synchronize by sleeping, only ok for a test like this.
  ASSERT_EQ(1U, server.received_msg);
  // execute shutdown in the event loop thread to avoid synchronization issues
  asio::post(server.get_context(),[&server](){server.shutdown();});
  mha_msleep(50);
  client.send(asio::const_buffer("\n", 1U));
  mha_msleep(50);
  ASSERT_EQ(1U, server.received_msg);
  thread.join();
}

// Local Variables:
// compile-command: "make -C .. unit-tests"
// coding: utf-8-unix
// c-basic-offset: 2
// indent-tabs-mode: nil
// End:
