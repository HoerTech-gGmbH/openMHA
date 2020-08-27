// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2008 2009 2011 2012 2013 2015 2016 2017 2018 HörTech gGmbH
// Copyright © 2020 HörTech gGmbH
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

#include <string.h>
#include <sstream>
#include "mha_tcp.hh"

#ifdef _WIN32
# include <WINSOCK2.H>
  typedef int socklen_t;
# define ASYNC_CONNECT_STARTED WSAEWOULDBLOCK
#else
# include <sys/select.h>
# include <sys/socket.h>
# include <unistd.h>
# include <signal.h>
# include <fcntl.h>
# define INVALID_SOCKET (-1)
# define SOCKET_ERROR (-1)
# define closesocket(fd) (close((fd)))
  typedef int SOCKET;
# define ASYNC_CONNECT_STARTED EINPROGRESS
#endif

#include <stdarg.h>
#include <errno.h>
#include <algorithm>
#include <vector>

namespace MHA_TCP {

// need to initialize socket library:
class sock_init_t {
public:
  sock_init_t()
  {
#ifndef _WIN32
    // On Unix, we need to ignore sigpipe
    struct sigaction ignore{{SIG_IGN}};
    sigaction(SIGPIPE, &ignore, NULL);
#else
    // On windows, we need to initialize and deinitialize the socket library
    WORD winsock_version = MAKEWORD(2,2);
    error_code = WSAStartup(winsock_version, &winsock_data);
    if (error_code != 0) {
        std::string err = STRERROR(error_code);
        MessageBox(NULL,
                   err.c_str(),
                   "Winsock initialization failed",
                   MB_ICONERROR);
        throw error_code;
    }
  }
  int error_code;
  WSADATA winsock_data;
  ~sock_init_t()
  {
    if (WSACleanup() != 0)
        MessageBox(NULL,
                   STRERROR(N_ERRNO()).c_str(),
                   "Unloading winsock dll failed",
                   MB_ICONERROR);
#endif
  }
} static sock_initializer;


std::string STRERROR(int err)
{
#ifdef _WIN32
    char * lpMsgBuf;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | 
                  FORMAT_MESSAGE_FROM_SYSTEM | 
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,
                  err,
                  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
                  reinterpret_cast<LPSTR>(&lpMsgBuf),
                  0,
                  NULL);
    std::string msg(lpMsgBuf);
    // Free the buffer.
    LocalFree(lpMsgBuf);
    return msg;
#else
    return strerror(err);
#endif
}

std::string HSTRERROR(int err)
{
#ifdef _WIN32
    return STRERROR(err);
#else
    return hstrerror(err);
#endif
}

int N_ERRNO()
{
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}
int H_ERRNO()
{
#ifdef _WIN32
    return WSAGetLastError();
#else
    return h_errno;
#endif
}
int G_ERRNO()
{
#ifdef _WIN32
    return GetLastError();
#else
    return errno;
#endif
}

}

using namespace MHA_TCP;

Wakeup_Event::Wakeup_Event()
{ os_event_valid = false; }
void Wakeup_Event::observed_by(Event_Watcher * observer)
{ observers.insert(observer); }
void Wakeup_Event::ignored_by(Event_Watcher * observer)
{ observers.erase(observer); }
Wakeup_Event::~Wakeup_Event()
{
    while (observers.begin() != observers.end())
        (*observers.begin())->ignore(this);
#ifdef _WIN32
    if (os_event_valid)
        CloseHandle(os_event);
#endif
    os_event_valid = false;
}
OS_EVENT_TYPE Wakeup_Event::get_os_event()
{
    if (!os_event_valid)
        throw MHA_ErrorMsg("Cannot serve request for invalid os_event");
    return os_event;
}
void Wakeup_Event::reset()
{
    if (!os_event_valid) 
        throw MHA_ErrorMsg("Cannot reset invalid os_event");
    // Default: Do nothing.
}
bool Wakeup_Event::status()
{
    if (os_event_valid == false)
        throw MHA_ErrorMsg("Cannot investigate status of invalid event");
    Timeout_Watcher tmp_watcher(0);
    tmp_watcher.observe(this);
    Event_Watcher::Events events = tmp_watcher.wait();
    return events.find(this) != events.end();
}

void Event_Watcher::observe(Wakeup_Event * event)
{
    if (event == 0)
        throw MHA_ErrorMsg("Event_Watcher::observe: event must not be 0");
    events.insert(event);
    event->observed_by(this);
}
void Event_Watcher::ignore(Wakeup_Event * event)
{
    if (event == 0)
        throw MHA_ErrorMsg("Event_Watcher::ignore: event must not be 0");
    iterator it = events.find(event);
    if (it == events.end())
        throw MHA_ErrorMsg("Event_Watcher::ignore: event must have been "\
                         "observed prior to being ignored");
    events.erase(it);
    event->ignored_by(this);
}
Event_Watcher::~Event_Watcher()
{
    while (events.begin() != events.end())
        ignore(*events.begin());
}

std::set<Wakeup_Event *> Event_Watcher::wait()
{
    Events signaled_events;
    Events::iterator in = events.begin();
#ifdef _WIN32
    std::vector<OS_EVENT_TYPE> os_events(events.size());
    std::vector<OS_EVENT_TYPE>::iterator out = os_events.begin();
    for (; in != events.end(); ++in, ++out)
        *out = (*in)->get_os_event();
    unsigned long offset = 0;
    in = events.begin();
    unsigned itimeout = INFINITE;
    unsigned long wfmo_retval;
    while (offset < os_events.size()) {
        MSG msg;
        if (PeekMessage(&msg, 0,0,0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            //... and break loop on WM_QUIT
            if (msg.message == WM_QUIT) {
                throw MHA_ErrorMsg("Received WM_QUIT Message");
                break;
            }
            // Check if there are other messages pending
            continue;
        }
            
        wfmo_retval =
            MsgWaitForMultipleObjects(os_events.size() - offset,
                                      &os_events[offset],
                                      false,
                                      itimeout,
                                      QS_ALLINPUT);
        if (wfmo_retval >= WAIT_OBJECT_0 &&
            wfmo_retval < (WAIT_OBJECT_0 + os_events.size() - offset)) {
            // There was a hit, store corresponding event in result set
            for (unsigned index = offset;
                 (index - offset) < (wfmo_retval - WAIT_OBJECT_0);
                 ++index, ++in)
                {}
            signaled_events.insert(*in);

            // Check if any of the other events is also signalled
            offset += (wfmo_retval - WAIT_OBJECT_0) + 1;
            ++in;
            itimeout = 0;
        }
        else if (wfmo_retval == WAIT_TIMEOUT && itimeout == 0) {
            // no more hits, return
            break;
        }
        else if (wfmo_retval == 0xFFFFFFFF) {
            throw MHA_Error(__FILE__, __LINE__,
                            "MsgWaitForMultipleObjects failed: %s",
                            STRERROR(G_ERRNO()).c_str());
        }
        else if (wfmo_retval == (WAIT_OBJECT_0 + os_events.size() - offset)) {
            // inspect this message in the next iteration.
        }
        else {
            throw MHA_Error(__FILE__, __LINE__,
                            "Unexpected return value of WaitForMultipleObjects"
                            ": %lu", wfmo_retval);
        }
    }
#else
    OS_EVENT_TYPE event;
    fd_set fds[3]; // R, W, X
    int max_fd = 0;
    double timeout = -1;
    Wakeup_Event * timeout_event = 0;
    FD_ZERO(&fds[0]);FD_ZERO(&fds[1]);FD_ZERO(&fds[2]);
    for (; in != events.end(); ++in){
        event = (*in)->get_os_event();
        if (int(event.mode) < 3) {
            max_fd = std::max(max_fd, event.fd);
            FD_SET(event.fd, &fds[int(event.mode)]);
        }
        else {
            if (timeout_event == 0) {
                timeout_event = *in;
                timeout = event.timeout;
            } else if (event.timeout < timeout) {
                timeout = event.timeout;
                timeout_event = *in;
            }
        }
    }
    struct timeval tv;
    tv.tv_sec = static_cast<long>(timeout);
    tv.tv_usec = static_cast<long>((timeout - tv.tv_sec) * 1e6);
    int n = select(max_fd+1, &fds[0], &fds[1], &fds[2],
                   timeout_event ? &tv : 0);
    if (n == 0) signaled_events.insert(timeout_event);
    if (n < 0)
        throw MHA_Error(__FILE__,__LINE__,
                        "select failed: %s", strerror(errno));
    for (in = events.begin(); in != events.end(); ++in) {
        event = (*in)->get_os_event();
        if (int(event.mode) < 3 && FD_ISSET(event.fd, &fds[int(event.mode)]))
            signaled_events.insert(*in);
    }
#endif
    return signaled_events;
}

#ifdef _WIN32
double MHA_TCP::dtime() {
    return GetTickCount() * 1e-3;
}
#else
double MHA_TCP::dtime(const struct timeval & tv) {
  return tv.tv_sec + tv.tv_usec * 1e-6;
}
double MHA_TCP::dtime() {
  struct timeval tv;
  if (gettimeofday(&tv,NULL))
    throw MHA_ErrorMsg("gettimeofday() returned with error. "\
                       "This must not happen.");
  return dtime(tv);
}
struct timeval MHA_TCP::stime(double d)
{
  struct timeval tv;
  tv.tv_sec = time_t(d);
  tv.tv_usec = /*suseconds_t*/ long((d - tv.tv_sec) * 1e6);
  return tv;
}
#endif

Timeout_Event::Timeout_Event(double interval)
{
#ifdef _WIN32
    os_event = CreateWaitableTimer(0, true, 0);
    if (os_event == NULL)
        throw MHA_Error(__FILE__,__LINE__,
                        "Error during CreateWaitableTimer: %s",
                        STRERROR(G_ERRNO()).c_str());
    LARGE_INTEGER i;
    i.QuadPart = -LONGLONG(interval * 1e7);
    SetWaitableTimer(os_event, &i, 0, 0,0,0);
#else
    os_event.mode = OS_EVENT_TYPE::T;
    os_event.timeout = interval;
    end_time = interval + dtime();
#endif
    os_event_valid = true;
}

OS_EVENT_TYPE Timeout_Event::get_os_event()
{
#ifndef _WIN32
    os_event.timeout = std::max(0.0, end_time - dtime());
#endif
    return Wakeup_Event::get_os_event();
}

Timeout_Watcher::Timeout_Watcher(double interval)
    : timeout(interval)
{
    observe(&timeout);
}
Timeout_Watcher::~Timeout_Watcher()
{}


Async_Notify::Async_Notify()
{
#ifdef _WIN32
    os_event = CreateEvent(0,1,0,0);
    if (os_event == NULL)
        throw MHA_Error(__FILE__, __LINE__,
                        "CreateEvent failed: %s", 
                        STRERROR(G_ERRNO()).c_str());
#else
    if (::pipe(pipe))
        throw MHA_Error(__FILE__, __LINE__,
                        "Systemcall \"pipe\" failed: %s",
                        STRERROR(G_ERRNO()).c_str());
    os_event.mode = OS_EVENT_TYPE::R;
    os_event.fd = pipe[0];
#endif
    os_event_valid = true;
}
Async_Notify::~Async_Notify()
{
#ifndef _WIN32
    if (os_event_valid) {
        close(pipe[0]);
        close(pipe[1]);
    }
#endif
}

void Async_Notify::reset()
{
    Wakeup_Event::reset();
#ifdef _WIN32
    if (ResetEvent(get_os_event()) == 0)
        throw MHA_Error(__FILE__, __LINE__,
                        "ResetEvent failed: %s",
                        STRERROR(G_ERRNO()).c_str());
#else
    while(status()) {
        char buf[1024];
        if (read(get_os_event().fd, buf, 1024) < 0)
            throw MHA_Error(__FILE__, __LINE__,
                            "Reading from pipe failed: %s",
                            STRERROR(G_ERRNO()).c_str());
    }
#endif
}
void Async_Notify::set()
{
    if (os_event_valid == false)
        throw MHA_ErrorMsg("Cannot set an invalid Async_Notify event");
#ifdef _WIN32
    if (SetEvent(get_os_event()) == 0)
        throw MHA_Error(__FILE__, __LINE__,
                        "SetEvent failed: %s",
                        STRERROR(G_ERRNO()).c_str());
#else
    char c = ' ';
    if (write(pipe[1],&c,1) != 1)
        throw MHA_Error(__FILE__, __LINE__,
                        "Writing to pipe failed: %s",
                        STRERROR(G_ERRNO()).c_str());
#endif
}

MHA_TCP::Sockread_Event::Sockread_Event(SOCKET s)
{
#ifdef _WIN32
    os_event = (void*)WSACreateEvent();
    if (os_event == WSA_INVALID_EVENT)
        throw MHA_Error(__FILE__, __LINE__,
                        "Creating Winsock event object failed: %s",
                        STRERROR(N_ERRNO()).c_str());
    int ev_sel = WSAEventSelect(s, 
                                (WSAEVENT/*unsigned long*/
)os_event,
                                FD_READ | FD_CLOSE);
    if (ev_sel == SOCKET_ERROR)
        throw MHA_Error(__FILE__, __LINE__,
                        "Associating Sockread_Event with socket failed: %s",
                        STRERROR(N_ERRNO()).c_str());
#else
    os_event.fd = s;
    os_event.mode = OS_EVENT_TYPE::R;
#endif
    os_event_valid = true;
}

MHA_TCP::Sockwrite_Event::Sockwrite_Event(SOCKET s)
{
#ifdef _WIN32
    os_event = CreateEvent(0,TRUE,TRUE,0);
#else
    os_event.fd = s;
    os_event.mode = OS_EVENT_TYPE::W;
#endif
    os_event_valid = true;
}

MHA_TCP::Sockaccept_Event::Sockaccept_Event(SOCKET s)
{
#ifdef _WIN32
    os_event = CreateEvent(0,FALSE,FALSE,0);
    if (os_event == WSA_INVALID_EVENT)
        throw MHA_Error(__FILE__, __LINE__,
                        "Creating Winsock event object failed: %s",
                        STRERROR(N_ERRNO()).c_str());
    int ev_sel = WSAEventSelect(s, (WSAEVENT/*unsigned long*/)os_event, FD_ACCEPT);
    if (ev_sel == SOCKET_ERROR)
        throw MHA_Error(__FILE__, __LINE__,
                        "Associating SockacceptEvent with socket failed: %s",
                        STRERROR(N_ERRNO()).c_str());
#else
    os_event.fd = s;
    os_event.mode = OS_EVENT_TYPE::R;
#endif
    os_event_valid = true;
}

static sockaddr_in
host_port_to_sock_addr(const std::string & host, unsigned short port)
{
    sockaddr_in sock_addr;

    // Clear memory garbage
    memset(&sock_addr, 0, sizeof(sock_addr));

    // Fill in port and address family
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);

    // resolve IP
    struct hostent * hostent = gethostbyname(host.c_str());
    if (hostent == 0)
        throw MHA_Error(__FILE__, __LINE__,
                        "Resolving IP address from \"%s\": %s",
                        host.c_str(), HSTRERROR(H_ERRNO()).c_str());
    if (sock_addr.sin_family != hostent->h_addrtype)
        throw MHA_Error(__FILE__,__LINE__,
                        "Address \"%s\" is not an internet address",
                        host.c_str());
    if (hostent->h_length > (int)sizeof(sock_addr.sin_addr)) { 
        hostent->h_length = sizeof(sock_addr.sin_addr);
    }

    // fill in IP
    memcpy(&sock_addr.sin_addr, hostent->h_addr, hostent->h_length);

    return sock_addr;
}    
    

Server::Server(unsigned short port, const std::string & iface)
{ initialize(iface, port); }
Server::Server(const std::string & iface, unsigned short port)
{ initialize(iface, port); }
void Server::initialize(const std::string & iface, unsigned short port)
{
    // Clear memory garbage
    serversocket = INVALID_SOCKET;
    memset(&sock_addr, 0, sizeof(sock_addr));
    this->iface = "";
    this->port = 0;
    accept_event = 0;

    // Fill sock_addr with port and IP
    sock_addr = host_port_to_sock_addr(iface, port);
    this->iface = iface;
    
    // create fresh socket fd
    serversocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serversocket == INVALID_SOCKET)
        throw MHA_Error(__FILE__,__LINE__,
                        "Unable to open tcp server socket: %s",
                        STRERROR(N_ERRNO()).c_str());

    // bind serversocket to iface and port
    socklen_t socklen = sizeof(sock_addr);
    if (port) {
        int so_reuseaddr = true;
#ifdef _WIN32
        int r = setsockopt(serversocket, SOL_SOCKET, SO_REUSEADDR,
                           reinterpret_cast<char*>(&so_reuseaddr), 
                           sizeof(so_reuseaddr));
#else
        int r = setsockopt(serversocket, SOL_SOCKET, SO_REUSEADDR,
                           &so_reuseaddr, sizeof(so_reuseaddr));
#endif
        if (r) {
            std::string err = STRERROR(N_ERRNO());
            closesocket(serversocket);
            serversocket = INVALID_SOCKET;
            throw MHA_Error(__FILE__,__LINE__,
                            "Setting server socket option REUSEADDR: %s",
                            err.c_str());
        } 
    }
    if (bind(serversocket,
             reinterpret_cast<struct sockaddr *>(&sock_addr),
             socklen                                       ) == SOCKET_ERROR) {
        std::string err = STRERROR(N_ERRNO());
        closesocket(serversocket);
        serversocket = INVALID_SOCKET;
        throw MHA_Error(__FILE__,__LINE__,
                        "Binding the server socket: %s", err.c_str());
    }

    // if port was 0, then the OS chooses a free port. Find out which.
    if (getsockname(serversocket,
                    reinterpret_cast<struct sockaddr *>(&sock_addr),
                    &socklen                              ) == SOCKET_ERROR) { 
        std::string err = STRERROR(N_ERRNO());
        closesocket(serversocket);
        serversocket = INVALID_SOCKET;
        throw MHA_Error(__FILE__, __LINE__,
                        "Checking the server socket port: %s", err.c_str());
    }
    this->port = ntohs(sock_addr.sin_port);
    if (port != 0 && this->port != port) {
        closesocket(serversocket);
        serversocket = INVALID_SOCKET;
        throw MHA_Error(__FILE__, __LINE__,
                        "getsockname returned port number %hu instead of %hu",
                        this->port, port);
    }
    if (port == 0 && this->port == 0) {
        closesocket(serversocket);
        serversocket = INVALID_SOCKET;
        throw MHA_ErrorMsg("The did not properly replace port number 0");
    }

    // limit backlog to 1 pending connection
    if (listen(serversocket, 1) == SOCKET_ERROR) {
        std::string err = STRERROR(N_ERRNO());
        closesocket(serversocket);
        serversocket = INVALID_SOCKET;
        throw MHA_Error(__FILE__,__LINE__,
                        "Server socket: listen() failed: %s", err.c_str());
    }

    // set serversocket to nonblocking (accept() must not block)
#ifdef _WIN32
    // In windows, setting the socket to nonblocking is implied when creating
    // the socket event.
#else
    int serversocket_flags = fcntl(serversocket, F_GETFL);
    if (serversocket_flags == -1) {
        std::string err = STRERROR(N_ERRNO());
        closesocket(serversocket);
        serversocket = INVALID_SOCKET;
        throw MHA_Error(__FILE__,__LINE__,
                        "Server socket: fcntl() could not read "\
                        "file descriptor flags: %s", err.c_str());
    }
    serversocket_flags |= O_NONBLOCK;
    if (fcntl(serversocket, F_SETFL, serversocket_flags)) {
        std::string err = STRERROR(N_ERRNO());
        closesocket(serversocket);
        serversocket = INVALID_SOCKET;
        throw MHA_Error(__FILE__,__LINE__,
                        "Server socket: fcntl() could not set file "\
                        "descriptor flags to nonblocking: %s", err.c_str());
    }
#endif
    // create the Wakeup_Event to wait for incoming connections
    try {
        accept_event = new Sockaccept_Event(serversocket);
    } catch(...) {
        closesocket(serversocket);
        serversocket = INVALID_SOCKET;
        throw;
    }
}
Server::~Server()
{
    delete accept_event;
    accept_event = 0;
    if (serversocket != INVALID_SOCKET) {
        closesocket(serversocket);
        serversocket = INVALID_SOCKET;
    }
}
std::string Server::get_interface() const {return iface;}
unsigned short Server::get_port() const {return port;}
Sockaccept_Event * Server::get_accept_event() {return accept_event;}

Connection * Server::accept()
{
    Event_Watcher event_watcher;
    event_watcher.observe(get_accept_event());
    Connection * connection = 0;
    while (connection == 0) {
        event_watcher.wait();
        connection = try_accept();
    }
    return connection;
}
Connection * Server::try_accept()
{
    SOCKET conn = ::accept(serversocket,0,0);
    if (conn == INVALID_SOCKET) {
        int n_err = N_ERRNO();
#ifdef _WIN32
        if (n_err == WSAEWOULDBLOCK) return 0;
#else
        if (n_err == EWOULDBLOCK) return 0;
#endif
        throw MHA_Error(__FILE__, __LINE__,
                        "accepting incoming TCP connection: %s",
                        STRERROR(n_err).c_str());
    }
    return new Connection(conn);
}

static SOCKET tcp_connect_to(const std::string & host, unsigned short port)
{
    sockaddr_in sock_addr = host_port_to_sock_addr(host, port);
    SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == INVALID_SOCKET)
         throw MHA_Error(__FILE__, __LINE__,
                         "Cannot allocate TCP socket: %s",
                         STRERROR(N_ERRNO()).c_str());
    if (connect(fd,
                reinterpret_cast<struct sockaddr *>(&sock_addr),
                sizeof(sock_addr)) == SOCKET_ERROR) {
        closesocket(fd);
        throw MHA_Error(__FILE__, __LINE__,
                        "Cannot connect to remote host \"%s\" port %hu: %s",
                        host.c_str(), port, STRERROR(N_ERRNO()).c_str());
    }
    return fd;
}

static SOCKET tcp_connect_to_with_timeout(const std::string & host,
                                          unsigned short port,
                                          Timeout_Watcher & timeout_watcher)
{
    sockaddr_in sock_addr = host_port_to_sock_addr(host, port);
    SOCKET fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == INVALID_SOCKET)
         throw MHA_Error(__FILE__, __LINE__,
                         "Cannot allocate TCP socket: %s",
                         STRERROR(N_ERRNO()).c_str());

    Sockwrite_Event sockwrite_event(fd);

    // Set socket to non-blocking
#ifdef _WIN32
    // In windows, setting the socket to nonblocking is implied when creating
    // the socket write event (WSAEventSelect).
#else
    int socket_flags = fcntl(fd, F_GETFL);
    if (socket_flags == -1) {
        std::string err = STRERROR(N_ERRNO());
        closesocket(fd);
        fd = INVALID_SOCKET;
        throw MHA_Error(__FILE__,__LINE__,
                        "TCP client: fcntl() could not read "\
                        "file descriptor flags of socket: %s", err.c_str());
    }
    socket_flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, socket_flags)) {
        std::string err = STRERROR(N_ERRNO());
        closesocket(fd);
        fd = INVALID_SOCKET;
        throw MHA_Error(__FILE__,__LINE__,
                        "TCP client: fcntl() could not set file "\
                        "descriptor flags to nonblocking: %s", err.c_str());
    }
#endif    
    // TODO: Check for asynchroneous connection
    int connect_return = 
        connect(fd,
                reinterpret_cast<struct sockaddr *>(&sock_addr),
                sizeof(sock_addr));
    if (connect_return == SOCKET_ERROR) {
        int e = N_ERRNO();
        if (e != ASYNC_CONNECT_STARTED)  {
            closesocket(fd);
            throw MHA_Error(__FILE__, __LINE__,
                            "Cannot connect to remote host \"%s\" port %hu: %s",
                            host.c_str(), port, STRERROR(e).c_str());
        }
        timeout_watcher.observe(&sockwrite_event);
        std::set<Wakeup_Event *> wake_set = timeout_watcher.wait();
        if (wake_set.find(&sockwrite_event) == wake_set.end()) {
            closesocket(fd);
            throw MHA_Error(__FILE__, __LINE__,
                            "Cannot connect to remote host \"%s\" port %hu:"
                            " Timeout.",
                            host.c_str(), port);
        }
    }
    return fd;
}

Client::Client(const std::string & host, unsigned short port)
    : Connection(tcp_connect_to(host, port))
{}

Client::Client(const std::string & host, unsigned short port,
               Timeout_Watcher & timeout_watcher)
    : Connection(tcp_connect_to_with_timeout(host, port, timeout_watcher))
{}

std::string Connection::get_peer_address()
{
    std::ostringstream o;
    const unsigned int n_ip = peer_addr.sin_addr.s_addr;
    o << ( n_ip & 0x000000FFU)        << "."
      << ((n_ip & 0x0000FF00U) >>  8) << "."
      << ((n_ip & 0x00FF0000U) >> 16) << "."
      << ((n_ip & 0xFF000000U) >> 24);
    return o.str();
}
unsigned short Connection::get_peer_port()
{
    return ntohs(peer_addr.sin_port);
}

MHA_TCP::Connection::Connection(SOCKET _fd)
    : read_event(0),
      write_event(0),
      closed(false),
      fd(_fd)
{
    if (fd == INVALID_SOCKET)
        throw MHA_Error(__FILE__, __LINE__,
                        "Cannot create TCP Connection "
                        "with invalid file descriptor "
#ifdef _WIN32
                        "%llu",
#else
                        "%d",
#endif
                        fd);
    read_event = new Sockread_Event(fd);
    write_event = new Sockwrite_Event(fd);

    socklen_t socklen = sizeof(peer_addr);
    memset(&peer_addr, 0, socklen);
    if (getpeername(fd,
                    reinterpret_cast<sockaddr*>(&peer_addr),
                    & socklen)                  == SOCKET_ERROR)
        throw MHA_Error(__FILE__, __LINE__,
                        "Cannot get address of remote tcp peer for SOCKET %d",
                        int(fd));
}

Sockread_Event * Connection::get_read_event() {return read_event;}
Sockwrite_Event * Connection::get_write_event() {return write_event;}
bool Connection::can_sysread() {return get_read_event()->status();}
bool Connection::can_syswrite() {return get_write_event()->status();}

std::string Connection::sysread(unsigned bytes)
{
    if (closed) return "";
    std::string s(bytes, '\0');
    int bytes_read = 0;
#ifdef _WIN32
    Event_Watcher w;
    w.observe(get_read_event());
    w.wait();
    u_long readable = (u_long)(-1);
    if (ioctlsocket(fd, FIONREAD, &readable) == SOCKET_ERROR)
        throw MHA_Error(__FILE__, __LINE__, "ioctlsocket failed: %s",
                        STRERROR(N_ERRNO()).c_str());
    if (readable == 0) {
        closed = true;
        return "";
    }
    ResetEvent(get_read_event()->get_os_event());
    bytes_read = recv(fd, &s[0], bytes, 0);
#else
    bytes_read = read(fd, &s[0], bytes);
    if ((bytes_read == 0) && (bytes > 0))
        closed = true;
#endif
    if (bytes_read == SOCKET_ERROR)
        throw MHA_Error(__FILE__,__LINE__,
                        "Reading %u bytes from socket "
#ifdef _WIN32
                        "%llu"
#else
                        "%d"
#endif
                        " raised an error: %s",
                        bytes, fd,
                        STRERROR(N_ERRNO()).c_str());
    s.resize(bytes_read);
    return s;
}
std::string Connection::syswrite(const std::string & data)
{
    unsigned bytes = data.length();
    int bytes_written = 0;
#ifdef _WIN32
    bytes_written = send(fd, &data[0], bytes, 0);
#else
    bytes_written = ::write(fd, &data[0], bytes);
#endif
    if (bytes_written == SOCKET_ERROR)
        throw MHA_Error(__FILE__,__LINE__,
                        "Writing %u bytes to socket "
#ifdef _WIN32
                        "%llu"
#else
                        "%d"
#endif
                        " raised an error: %s",
                        bytes,
                        fd,
                        STRERROR(N_ERRNO()).c_str());
    return data.substr(bytes_written);
}

Connection::~Connection()
{
    delete read_event;
    read_event = 0;
    delete write_event;
    write_event = 0;
    closesocket(fd);
    fd = INVALID_SOCKET;
}

bool Connection::eof()
{
    if (inbuf.size() == 0 && can_sysread()) {
        std::string s = sysread(16384);
        inbuf += s;
        return inbuf.size() == 0;
    }
    return closed;
}

bool Connection::can_read_line(char delim)
{
    for(;;) {
        if (inbuf.find(delim) < buffered_incoming_bytes()) {
            return true;
        }
        if (can_sysread() == false) return false;
        std::string s = sysread(16384);
        if (s.size() == 0)
            return false;
        inbuf += s;
    }
}
bool Connection::can_read_bytes(unsigned howmany)
{
    for(;;) {
        if (buffered_incoming_bytes() >= howmany) return true;
        if (can_sysread() == false) {
            return false;
        }
        std::string s = sysread(howmany - buffered_incoming_bytes());
        if (s.size() == 0) return false;
        inbuf += s;
    }
}
void Connection::try_write(const std::string & data)
{
    outbuf += data;
    while (needs_write() && can_syswrite()) {
        outbuf = syswrite(outbuf);
    }
}
void Connection::write(const std::string & data)
{
    outbuf += data;
    while (needs_write()) {
        outbuf = syswrite(outbuf);
    }
}
bool Connection::needs_write()
{ return buffered_outgoing_bytes() != 0; }
unsigned Connection::buffered_incoming_bytes() const
{ return inbuf.size(); }
unsigned Connection::buffered_outgoing_bytes() const
{ return outbuf.size(); }

std::string Connection::read_line(char delim)
{
    Event_Watcher w;
    w.observe(get_read_event());
    while (!can_read_line(delim)) {
        w.wait();
    }
    std::string s =
        inbuf.substr(0, std::min(inbuf.find(delim), inbuf.size()) + 1);
    inbuf.erase(0, s.length());
    return s;
}
std::string Connection::read_bytes(unsigned howmany)
{
    Event_Watcher w;
    w.observe(get_read_event());
    while (!can_read_bytes(howmany) && !closed) {
        w.wait();
    }
    std::string s = inbuf.substr(0, howmany);
    inbuf.erase(0, howmany);
    return s;
}

static
#ifdef _WIN32
DWORD 
#else
void *
#endif
thread_start_func(void * thread)
{ static_cast<Thread*>(thread)->run(); return 0; }

Thread::Thread(Thread::thr_f func, void * arg)
    : thread_handle(0),
      return_value(0),
      state(Thread::PREPARED),
      thread_func(func),
      thread_arg(arg),
      error(0)
{
#ifdef _WIN32
    thread_id = 0;
    thread_handle =
        CreateThread(0,
                     0,
                     (LPTHREAD_START_ROUTINE)(&thread_start_func),
                     this,
                     0, &thread_id);
    if (thread_handle == NULL)
        throw MHA_Error(__FILE__,__LINE__,
                        "CreateThread failed: %s",
                        STRERROR(G_ERRNO()).c_str());
#else
    int err;
    err = pthread_attr_init(&thread_attr);
    if (err)
        throw MHA_Error(__FILE__,__LINE__,
                        "pthread_attr_init failed: %s",
                        STRERROR(err).c_str());
    err = pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);
    if (err)
        throw MHA_Error(__FILE__,__LINE__,
                        "pthread_attr_setdetachstate failed: %s",
                        STRERROR(err).c_str());
    err =
        pthread_create(&thread_handle, &thread_attr, &thread_start_func, this);
    if (err)
        throw MHA_Error(__FILE__,__LINE__,
                        "pthread_create failed: %s",
                        STRERROR(err).c_str());
#endif
}

void Thread::run()
{
    try {
#ifndef _WIN32
        pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
#endif
        if (state != PREPARED)
            throw MHA_ErrorMsg("Entering Thread::run(): state is not PREPARED");
        state = RUNNING;
        return_value = thread_func(thread_arg);
        if (state != RUNNING)
            throw MHA_ErrorMsg("Thread::run(): state is not RUNNING");
        state = FINISHED;
        thread_finish_event.set();
    } catch (MHA_Error & e) {
        error = new MHA_Error(e);
        state = FINISHED;
        thread_finish_event.set();
    } catch (...) {
        error = new MHA_ErrorMsg("Uncaught Exception of unknown type");
        state = FINISHED;
        thread_finish_event.set();
    }
 }

Thread::~Thread()
{
    if (state != FINISHED) {
        mha_debug("Warning: Destroying wrapper of running thread. "
                  "Expect Bad things to happen soon.");
#ifndef _WIN32
        pthread_cancel(thread_handle);
#endif
    }
    delete error;
    error = 0;
    arg = 0;
    return_value = 0;
}

// Local Variables:
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
