// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2008 2009 2013 2014 2015 2016 2017 HörTech gGmbH
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

#ifdef MHA_STATIC_PLUGINS
#include "../../../frameworks/src/mha_tcp.hh"
#else
#include "../../../frameworks/src/mha_tcp.cpp"
#endif

#include <cassert>
#include <cstdio>
#include "mha_io_ifc.h"
#include "mha_toolbox.h"
#include "mha_signal.hh"

#define ERR_SUCCESS 0
#define ERR_IHANDLE -1
#define ERR_USER -1000

#define MAX_USER_ERR 0x2000 // 8192
static char user_err_msg[MAX_USER_ERR];

#define MHA_ErrorMsg2(x,y) MHA_Error(__FILE__,__LINE__,(x),(y))
#define MHA_ErrorMsg3(x,y,z) MHA_Error(__FILE__,__LINE__,(x),(y),(z))

#define MIN_TCP_PORT          0
#define MIN_TCP_PORT_STR     "0"
#define MAX_TCP_PORT      65535
#define MAX_TCP_PORT_STR "65535"



/* ======================================================================== */
/** 
 * The parser interface of the IOTCP library. */
class io_tcp_parser_t : public MHAParser::parser_t
{
    /** Lets the user set the local network interface to listen on. */
    MHAParser::string_t local_address;
    
    /** Lets the user choose the local tcp port to listen on. */
    MHAParser::int_t local_port;

    /** Indicates wether the TCP server socket is currently open. */
    MHAParser::int_mon_t server_port_open;

    /** Indicator if there currently is a sound data connection over TCP. */
    MHAParser::int_mon_t connected;

    /** Display the ip address of the currently connected sound data client.*/
    MHAParser::string_mon_t peer_address;

    /** Display the tcp port used by the current sound data client. */
    MHAParser::int_mon_t peer_port;

    /** filename to write debugging info to (if non-empty) */
    MHAParser::string_t debug_filename;
    
    /** file handle to write debugging info to */
    FILE * debug_file;

public:
    /** Read parser variable local_address, this is the address of the
     * network interface that should listen for incoming connections. 
     * @return A string containing the address of the local interface as
     *         it was set by the user. */
    virtual const std::string & get_local_address() const
    { return local_address.data;  }
    
    /** Read parser variable local_port, this is the TCP port that should be
     * used for incoming connections.
     * @return The local tcp port to listen on as it was chosen by the
     *         user.  The port number is between MIN_TCP_PORT and
     *         MAX_TCP_PORT. */
    virtual unsigned short get_local_port() const;

    /** Set parser variable local_port.  This is needed when it was set to 0
     * before:  In this case, the OS chooses a free port for the TCP server
     * socket, and the port that it chose has to be published to the user
     * via the parser interface.
     * @param port The TCP port number that is currently used. In the range
     *             [MIN_TCP_PORT, MAX_TCP_PORT], excluding 0. 
     * @pre get_local_port() currently returns 0. */
    virtual void set_local_port(unsigned short port);

    /** Return the status of the server port as it is known to the parser.
     * @return false after initialization, or the value most recently set 
     * via @see set_server_port_open. */
    virtual bool get_server_port_open() const;

    /** Inform the parser of the current status of the server socket.
     * @param open Indicates wether the server socket has just been opened
     *             or closed.
     * @pre open may only have the value true if get_server_port_open()
     *      currently returns false. 
     * @post @see get_server_port_open returns the value of open. */
    virtual void set_server_port_open(bool open);

    /** Return the parser's knowledge concerning wether there currently exists
     * an established sound data TCP connection or not.
     * @return false after initialization, or the value most recently set 
     * via @see set_connected. */
    virtual bool get_connected() const;

    /** Inform the parser about the existance of a sound data connection.
     * @param connected Indicates wether there currently is a connection or
     *                  not.
     * @pre connected must not have the same value that is currently returned
     *      by @see get_connected.
     * @post @see get_connected returns the value of open. */
    virtual void set_connected(bool connected);

    /** Set parser monitor variables peer_port and peer_address, and calls
     * set_connected(true).  This method should be called when a new
     * connection is established.
     * @param port The TCP port number used by the peer.
     * @param host The Internet host where the peer is located.
     * @pre @see get_connected currently returns false.
     * @post @see get_connected returns true. */
    virtual void set_new_peer(unsigned short port,
                              const std::string & host);
public:
    /** Constructor initializes parser variables. */
    io_tcp_parser_t();

    /** Do-nothing destructor. */
    virtual ~io_tcp_parser_t() {}

    virtual void debug(const std::string & message) {
        if ((debug_file == 0) && (debug_filename.data.length() > 0U))
                debug_file = fopen(debug_filename.data.c_str(),"a");
        if (debug_file) {
            fprintf(debug_file, "%s\n",message.c_str());
            fflush(debug_file);
        }
    }
};

unsigned short io_tcp_parser_t::get_local_port() const {
    if (local_port.data < MIN_TCP_PORT)
        throw MHA_ErrorMsg2("Value of parser variable \"local_port\" (%d) " \
                          "is too small!", local_port.data);
    if (local_port.data > MAX_TCP_PORT)
        throw MHA_ErrorMsg2("Value of parser variable \"local_port\" (%d) " \
                          "is too large!", local_port.data);
    if (static_cast<unsigned short>(local_port.data) != local_port.data)
        throw MHA_ErrorMsg2("BUG: Value of parser variable \"local_port\" " \
                          "(%d) does not fit into data type \"unsigned " \
                          "short\" which is used to hold TCP port numbers",
                          local_port.data);
    return static_cast<unsigned short>(local_port.data);
}

void io_tcp_parser_t::set_local_port(unsigned short port) {
    if (get_local_port() != 0) 
        throw MHA_ErrorMsg3("Attempt to set Value of parser variable  " \
                          "\"local_port\"  to %hu while it had the non-zero " \
                          "value %hu", port, get_local_port());
    local_port.data = port;
}

bool io_tcp_parser_t::get_server_port_open() const {
    if (server_port_open.data == 1) return true;
    if (server_port_open.data == 0) return false;
    throw MHA_ErrorMsg2("io_tcp_parser_t::get_server_port_open():Bug: " \
                        "value of parser variable server_port_open is " \
                        "neither 0 nor 1, but %d", server_port_open.data);
}

void io_tcp_parser_t::set_server_port_open(bool open) {
    if (open && get_server_port_open())
        throw MHA_ErrorMsg("io_tcp_parser_t::set_server_port_open(true)" \
                           ":Bug: Attempt to set server_port_open to true " \
                           "while the server port was already registered " \
                           "as being currently open.");
    server_port_open.data = open;
    if (get_server_port_open() != open)
        throw MHA_ErrorMsg("io_tcp_parser_t::set_server_port_open():Bug: " \
                           "value of parser variable server_port_open just " \
                           "set is not equal to the value it has been set to.");
}

bool io_tcp_parser_t::get_connected() const {
    if (connected.data == 1) return true;
    if (connected.data == 0) return false;
    throw MHA_ErrorMsg2("io_tcp_parser_t::get_connected():Bug: " \
                        "value of parser variable connected is "        \
                        "neither 0 nor 1, but %d", connected.data);
}

void io_tcp_parser_t::set_connected(bool connected) {
    if (connected == get_connected())
        throw MHA_ErrorMsg3("io_tcp_parser_t::set_connected(%s)" \
                            ":Bug: Attempt to set connected to the same value " \
                            "that it currently has (%s).",
                            connected ? "true" : "false",
                            get_connected() ? "true" : "false");
    this->connected.data = connected;
    if (get_connected() != connected)
        throw MHA_ErrorMsg("io_tcp_parser_t::set_connected():Bug: " \
                           "value of parser variable \"connected\" just " \
                           "set is not equal to the value it has been set to.");
}

void io_tcp_parser_t::set_new_peer(unsigned short port,
                                   const std::string & host)
{
    set_connected(true);
    peer_port.data = port;
    peer_address.data = host;
}

io_tcp_parser_t::io_tcp_parser_t()
    : MHAParser::parser_t("TCP IO-lib exchanges sound samples as "      \
                          "interleaved binary float32 data in network-byte-" \
                          "order (big endian) over a TCP connection"),
      local_address("Local address, determines interfaces",
                    "0.0.0.0"),
      local_port("TCP Server Port for sound data exchange",
                 "33338", "[" MIN_TCP_PORT_STR "," MAX_TCP_PORT_STR "]"),
      server_port_open("Status of local server port"),
      connected("Status of tcp connection"),
      peer_address("IP address of remote computer"),
      peer_port("Remote tcp port of connection"),
      debug_filename("debug messages of MHAIOTCP will be written to this file if non-empty",""),
      debug_file(NULL)
{
    server_port_open.data = 0;
    insert_item("server_port_open", &server_port_open);
    connected.data = 0;
    insert_item("connected", &connected);
    peer_address.data = "(none)";
    insert_item("peer_address", &peer_address);
    peer_port.data = 0;
    insert_item("peer_port", &peer_port);
    insert_item("address", &local_address);
    insert_item("port", &local_port);
    insert_member(debug_filename);
}

/* ========================================================================= */
/**
 * Sound data handling of io tcp library.
 */
class io_tcp_sound_t {
private:
    /** Number of sound samples in each channel expected and returned 
     * from processing callback. */
    int fragsize;

    /** Sampling rate.  Number of samples per second in each channel. */
    float samplerate;

    /** Number of input channels.  Number of channels expected from and 
     * returned by signal processing callback. */
    int num_inchannels, num_outchannels;

    /** Storage for input signal. */
    MHASignal::waveform_t* s_in;

    /** This union helps in conversion of floats from host byte
     * order to network byte order and back again. */
    union float_union {
        float f;
        unsigned int i;
        char c[4];
    };

    /** Check if mha_real_t is a usable 32-bit floating point type.
     * @throw MHA_Error if mha_real_t is not compatible to 32-bit float. */
    static void check_sound_data_type();

public:
    /** Initialize sound data handling.  Checks sound data type by calling
     * @see check_sound_data_type.
     * @param fragsize
     *    Number of sound samples in each channel expected and returned 
     *    from processing callback.
     * @param samplerate
     *    Number of samples per second in each channel. */
    io_tcp_sound_t(int fragsize, float samplerate);

    /** Do-nothing destructor */
    virtual ~io_tcp_sound_t() {}

    /** Called during prepare, sets number of audio channels and allocates
     * sound data storage.
     * @param num_inchannels  Number of input audio channels.
     * @param num_outchannels Number of output audio channels. */
    virtual void prepare(int num_inchannels, int num_outchannels);

    /** Called during release.  Deletes sound data storage. */
    virtual void release();

    /** Number of bytes that constitute one input sound chunk. 
     * @return Number of bytes to read from TCP connection before invoking
     *          signal processing. */
    virtual int chunkbytes_in() const;

    /** Create the tcp sound header lines. */
    virtual std::string header() const;

    /** Copy data received from tcp into mha_wave_t structure.  
     * Doing network-to-host byte order swapping in the process.
     * @param data One chunk (@see chunkbytes_in) of sound data to process.
     * @return Pointer to the sound data storage. */
    virtual mha_wave_t * ntoh(const std::string & data);

    /** Copy sound data from the output sound structure to a string.  
     * Doing host-to-network byte order swapping while at it.
     * @param s_out Pointer to the storage of the sound to put out.
     * @return The sound data in network byte order. */
    virtual std::string hton(const mha_wave_t * s_out);
};

io_tcp_sound_t::io_tcp_sound_t(int _fragsize, float _samplerate)
    : fragsize(_fragsize),
      samplerate(_samplerate),
      num_inchannels(0),
      num_outchannels(0),
      s_in(0)
{
    if (fragsize <= 0)
        throw MHA_Error(__FILE__, __LINE__,
                        "Chunk size has to be positive, was %d", fragsize);
    check_sound_data_type();
}

void io_tcp_sound_t::check_sound_data_type()
{
    float_union data;
    if (sizeof(float) != sizeof(unsigned int))
        throw MHA_ErrorMsg("Cannot use this host: sizeof(float) != "\
                         "sizeof(unsigned int)");
    if (sizeof(data.c) != sizeof(unsigned int))
        throw MHA_ErrorMsg("Cannot use this host: sizeof(char[4]) != "\
                         "sizeof(unsigned int)");
    data.f = 1.539989614e-36; // bits in float are 0x04030201 for this value
    data.i = htonl(data.i);
    if (strncmp("\004\003\002\001", data.c, 4) != 0)
        throw MHA_ErrorMsg("Cannot use this host: hton<float>(1e-38) != "\
                         "\"\\004\\003\\002\\001\"");
}

void io_tcp_sound_t::prepare(int num_inchannels, int num_outchannels)
{
    if (s_in != 0)
        throw MHA_ErrorMsg("Prepare called although "\
                         "waveform structure is already allocated");
    this->num_inchannels = num_inchannels;
    if (num_inchannels <= 0)
        throw MHA_ErrorMsg2("Number of input channels has to be positive, "\
                          "but was %d", num_inchannels);
    this->num_outchannels = num_outchannels;
    if (num_outchannels < 0)
        throw MHA_ErrorMsg2("Number of output channels has to be positive, "\
                          "but was %d", num_outchannels);
    s_in = new MHASignal::waveform_t(fragsize,num_inchannels);
}

void io_tcp_sound_t::release()
{
    delete s_in;
    s_in = 0;
    num_inchannels = num_outchannels = 0;
}

int io_tcp_sound_t::chunkbytes_in() const
{
    return num_inchannels * fragsize * sizeof(mha_real_t);
}

std::string io_tcp_sound_t::header() const
{
    // tcp sound metadata, followed by 1 blank line
    std::ostringstream o;
    o << "[MHAVersion 4.2 TCP Audio Stream]" << std::endl
      << "nchannels_in=" << num_inchannels << std::endl
      << "nchannels_out=" << num_outchannels << std::endl
      << "fragsize=" << fragsize << std::endl
      << "srate=" << samplerate << std::endl
      << std::endl;
    return o.str();
}

mha_wave_t * io_tcp_sound_t::ntoh(const std::string & data)
{
    assert(chunkbytes_in() >= 0 &&
           data.length() == unsigned(chunkbytes_in()));
    assert(fragsize >= 0 &&
           s_in->num_frames == unsigned(fragsize));
    assert(num_inchannels >= 0 &&
           s_in->num_channels == unsigned(num_inchannels));
    const float_union * src = 
        reinterpret_cast<const float_union *>(data.data());
    float_union * dst =  reinterpret_cast<float_union*>(s_in->buf);
    // byteorder swap if needed
    for (unsigned k = 0; k < size(s_in); ++k)
        dst[k].i = ntohl(src[k].i);
    return s_in;
}
    

std::string io_tcp_sound_t::hton(const mha_wave_t * s_out)
{
    std::vector<char> output_data(s_out->num_frames * s_out->num_channels *
                                  sizeof(mha_real_t));
    float_union * dst = reinterpret_cast<float_union *>(&output_data[0]);
    const float_union * src =
        reinterpret_cast<const float_union *>(s_out->buf);
    for (unsigned k = 0; k < size(s_out); ++k)
        dst[k].i = htonl(src[k].i);
    return std::string(&output_data[0], size(s_out) * sizeof(mha_real_t));
}

/* ========================================================================= */
/**
 * TCP sound-io library's interface to the framework callbacks. */
class io_tcp_fwcb_t {
private:
    /** Pointer to signal processing callback function. */
    IOProcessEvent_t proc_event;

    /** Pointer to start notification callback function.  Called when
     * a new TCP connection is established or the user issues the start
     * command while there is a connection. */
    IOStartedEvent_t start_event;


    /** Pointer to stop notification callback function. Called when the
     * connection is closed. */
    IOStoppedEvent_t stop_event;

    /** Handles belonging to framework. */
    void *proc_handle, *start_handle, *stop_handle;

    /** Errors from the processing callback and from the TCP IO itself
     * are stored here before closing Network handles.  MHAIOTCP is
     * notified by the server when the connection has been taken
     * down, and calls @see stop from that callback.  Within stop,
     * these error numbers are read again and transmitted to the
     * framework. */
    int proc_err, io_err;
public:
    /** Constructor stores framework handles and initializes error
     * numbers to 0. */
    io_tcp_fwcb_t(IOProcessEvent_t proc_event, void* proc_handle,
                  IOStartedEvent_t start_event, void* start_handle,
                  IOStoppedEvent_t stop_event, void* stop_handle);

    /** Do-nothing destructor. */
    virtual ~io_tcp_fwcb_t() {}

    /** Call the framework's start callback. */
    virtual void start();

    /** Call the frameworks processing callback. 
     * @param sIn The input sound data just received from TCP.
     * @param sOut A pointer to output sound data. 
     *        Will point to the output sound data storage when the
     *        callback finishes.
     * @return Status, an error number from the signal processing callback.
     *         If this is != 0, then the connection should be closed. */
    virtual int process(mha_wave_t * sIn, mha_wave_t *& sOut);

    /** Save error numbers to use during @see stop 
     * @param proc_err The error number from the @see process callback. 
     * @param io_err   The error number from the io library itself. */
    virtual void set_errnos(int proc_err, int io_err);

    /** Call the frameworks stop callback.  Uses the error numbers set
     * previously with @see set_errnos. */
    virtual void stop();
};

io_tcp_fwcb_t::io_tcp_fwcb_t(IOProcessEvent_t _proc_event, void* _proc_handle,
                             IOStartedEvent_t _start_event,void* _start_handle,
                             IOStoppedEvent_t _stop_event, void* _stop_handle)
    : proc_event(_proc_event),
      start_event(_start_event),
      stop_event(_stop_event),
      proc_handle(_proc_handle),
      start_handle(_start_handle),
      stop_handle(_stop_handle),
      proc_err(0),
      io_err(0)
{
    assert(proc_event != 0);
    assert(stop_event != 0);
    assert(proc_handle != 0);
    assert(stop_handle != 0);
}

void io_tcp_fwcb_t::start() 
{
    if (start_event)
        start_event(start_handle);
}

int io_tcp_fwcb_t::process(mha_wave_t * sIn, mha_wave_t *& sOut)
{ 
    return proc_event(proc_handle, sIn, &sOut);
}

void io_tcp_fwcb_t::stop() 
{
    if (stop_event)
        stop_event(stop_handle, proc_err, io_err);
}

void io_tcp_fwcb_t::set_errnos(int proc_err, int io_err)
{
    this->proc_err = proc_err;
    this->io_err = io_err;
}

/* ========================================================================= */
/** The tcp sound io library. */
class io_tcp_t
{
public:
    io_tcp_t(int fragsize, float samplerate,
             IOProcessEvent_t proc_event, void* proc_handle,
             IOStartedEvent_t start_event, void* start_handle,
             IOStoppedEvent_t stop_event, void* stop_handle);

    /** Allocate server socket and start thread waiting for sound data 
     * exchange */
    void prepare(int num_inchannels, int num_outchannels);

    /** Call frameworks start callback if there is a sound data connection
     * at the moment. */
    void start();

    /** Close the current connection if there is one. */
    void stop();

    /** Close the current connection and close the server socket. */
    void release();

    /** IO thread executes this method. */
    virtual void accept_loop();

    /** IO thread executes this method for each connection.
     * @param c pointer to connection. connection_loop deletes 
     * connection before exiting. */
    virtual void connection_loop(MHA_TCP::Connection * c);

    /** Parser interface. */
    virtual void parse(const char* cmd, char* retval, unsigned int len)
    { parser.parse(cmd, retval, len); }

    virtual ~io_tcp_t(){}
private:
    io_tcp_parser_t parser;
    io_tcp_sound_t sound;
    io_tcp_fwcb_t fwcb;
    MHA_TCP::Server * server;
    MHA_TCP::Thread * thread;
    MHA_TCP::Async_Notify notify_start, notify_stop, notify_release;
};

static int copy_error(MHA_Error& e) {
    strncpy(user_err_msg, Getmsg(e), MAX_USER_ERR-1);
    user_err_msg[MAX_USER_ERR-1] = '\0';
    return ERR_USER;
}

static void * thread_startup_function(void * parameter) {
    io_tcp_t * io_tcp = static_cast<io_tcp_t *>(parameter);
    io_tcp->accept_loop();
    return 0;
}

io_tcp_t::io_tcp_t(int _fragsize, float _samplerate,
                   IOProcessEvent_t proc_event, void* proc_handle,
                   IOStartedEvent_t start_event, void* start_handle,
                   IOStoppedEvent_t stop_event, void* stop_handle)
    : sound(_fragsize, _samplerate),
      fwcb(proc_event, proc_handle, start_event, start_handle,
           stop_event, stop_handle),
      server(0), thread(0)
{}

/** prepare opens the tcp server socket and starts the io thread that
 *  listens for audio data on the tcp socket after doing some sanity
 *  checks */
void io_tcp_t::prepare(int num_inchannels, int num_outchannels)
{
    if (parser.get_server_port_open())
        throw MHA_ErrorMsg("Prepare called although "\
                           "server port is already open");
    if (parser.get_connected())
        throw MHA_ErrorMsg("Prepare called although "\
                           "status of TCP io connection is \"connected\"");
    if (server != 0)
        throw MHA_ErrorMsg("Prepare called although "\
                           "TCP Server is already allocated");
    if (thread != 0)
        throw MHA_ErrorMsg("Prepare called although "\
                           "IO thread is already allocated");
    sound.prepare(num_inchannels, num_outchannels);
    parser.debug("opening server");
    server = new MHA_TCP::Server(parser.get_local_port(), parser.get_local_address());
    parser.set_server_port_open(true);
    if (parser.get_local_port() != server->get_port())
        parser.set_local_port(server->get_port());
    parser.debug("starting thread");
    thread = new MHA_TCP::Thread(thread_startup_function, this);
}

using namespace MHA_TCP;

void io_tcp_t::accept_loop() {
    parser.debug("accept_loop()");
    for(;;) {
        Event_Watcher w;
        w.observe(server->get_accept_event());
        w.observe(&notify_start);
        w.observe(&notify_stop);
        w.observe(&notify_release);
        parser.debug("accept_loop waits");
        std::set<Wakeup_Event *> s = w.wait();
        parser.debug("accept_loop woke up");
        if (s.find(&notify_release) != s.end()) {
            parser.debug("accept_loop received notify_release");
            delete server;
            server = 0;
            parser.set_server_port_open(false);
            notify_release.reset();
            parser.debug("accept_loop returns");
            return;
        }
        if (s.find(&notify_start) != s.end()) {
            // ignore start command when there is no connection
            parser.debug("accept_loop ignores notify_start");
            notify_start.reset();
        }
        if (s.find(&notify_stop) != s.end()) {
            // ignore stop command when there is no connection
            parser.debug("accept_loop ignores notify_stop");
            notify_stop.reset();
        }
        if (s.find(server->get_accept_event()) != s.end()) {
            parser.debug("accept_loop starts connection_loop");
            connection_loop(server->try_accept());
        }
    }
}
void io_tcp_t::connection_loop(MHA_TCP::Connection * c) {
    parser.debug("connection_loop()");
    parser.set_new_peer(c->get_peer_port(), c->get_peer_address());
    fwcb.start();
    parser.debug("connection_loop writes header");
    c->try_write(sound.header());
    for(;;) {
        Event_Watcher w;
        w.observe(&notify_start);
        w.observe(&notify_stop);
        w.observe(&notify_release);
        w.observe(c->get_read_event());
        parser.debug("connection_loop checks if writes are necessary before waiting");
        if (c->needs_write()) {
            parser.debug("connection_loop: yes, writes are necessary, observe write event");

            w.observe(c->get_write_event());
        }
        else {
            parser.debug("connection_loop: currently no further writes necessary");
        }
        parser.debug("connection_loop waits with buffered_incoming_bytes="+MHAParser::StrCnv::val2str((int)c->buffered_incoming_bytes()));
        std::set<Wakeup_Event *> s = w.wait();
        if (s.find(&notify_release) != s.end()
            || s.find(&notify_stop) != s.end()) {
            // close connection and return to accept loop.
            // don't reset release event,this will be done in accept loop
            parser.debug("connection_loop received stop or release");
            notify_stop.reset();
            goto terminate_connection_cleanly;
        }
        if (s.find(&notify_start) != s.end()) {
            parser.debug("connection_loop received start");
            notify_start.reset();
            fwcb.start();
        }
        if (s.find(c->get_read_event()) != s.end()) {
            parser.debug("connection_loop got read event");
            while (c->eof() || c->can_read_bytes(sound.chunkbytes_in())) {
                /* Also enter this loop on EOF, because:
                 * - I need an EOF check somewhere and act on it, obviously,
                 * - Whenever I check for EOF, the Connection class might
                 *   already read the next chunk of data.
                 * - If checking for EOF read in the next chunk, it needs to be
                 *   processed. I cannot fall back to another wait(), as that
                 *   might not wake up again. This is also the reason why eof() 
                                 *   is checked first in while argument.
                 * - Therefore, the correct location to check for EOF and act
                 *   on it is within this loop, which processes chunks of data.
                 * - This loop therefore also needs to be entered in a clear
                 *   EOF case when there is no chunk of audio data pending.
                 */
                if (!(c->eof())) { // means, we have another chunk
                    parser.debug("connection_loop can read another chunk");
                    mha_wave_t * s_out = 0;
                    parser.debug("connection_loop processes chunk");
                    int status = fwcb.process(sound.ntoh(c->read_bytes(sound.chunkbytes_in())), s_out);
                    if (status != 0) {
                        fwcb.set_errnos(status, 0);
                        goto terminate_connection;
                    }
                    parser.debug("connection_loop tries to write chunk result");
                    c->try_write(sound.hton(s_out));
                }
                parser.debug("connection_loop checks for eof");
                if (c->eof()) {
                    parser.debug("connection_loop got eof");
                    goto terminate_connection_cleanly;
                }
                                parser.debug("After EOF check, buffered_incoming_bytes="+MHAParser::StrCnv::val2str((int)c->buffered_incoming_bytes()));
            }
        }
        if (s.find(c->get_write_event()) != s.end()) {
            parser.debug("connection_loop got write event and tries to write");
            c->try_write();
        }
    }
 terminate_connection_cleanly:
    parser.debug("connection_loop resets errnos to clean");
    fwcb.set_errnos(0, 0);
 terminate_connection:
    parser.debug("connection_loop closes connection");
    delete c;
    parser.set_connected(false);
    fwcb.stop();
    parser.debug("connection_loop returns");
}

void io_tcp_t::start()
{
    if (parser.get_server_port_open() == false)
        throw MHA_ErrorMsg("Start called although "\
                           "server port is closed");
    notify_start.set();
}

/** stop IO thread */
void io_tcp_t::stop()
{
    // send stop request to io_thread
    notify_stop.set();
}

/** Stop IO thread and close server socket */
void io_tcp_t::release()
{
    parser.debug("release()");
    notify_release.set();
    MHA_TCP::Event_Watcher w;
    w.observe(&thread->thread_finish_event);
    parser.debug("release waits for thread termination");
    w.wait();
    delete thread; 
    thread=0;
    sound.release();
    parser.debug("release returns");
}

/* ========================================================================= */
/* IOLIB interface */
#ifdef MHA_STATIC_PLUGINS
#define IOInit               MHA_STATIC_MHAIOTCP_IOInit
#define IOPrepare            MHA_STATIC_MHAIOTCP_IOPrepare
#define IOStart              MHA_STATIC_MHAIOTCP_IOStart
#define IOStop               MHA_STATIC_MHAIOTCP_IOStop
#define IORelease            MHA_STATIC_MHAIOTCP_IORelease
#define IOSetVar             MHA_STATIC_MHAIOTCP_IOSetVar
#define IOStrError           MHA_STATIC_MHAIOTCP_IOStrError
#define IODestroy            MHA_STATIC_MHAIOTCP_IODestroy
#define dummy_interface_test MHA_STATIC_MHAIOTCP_dummy_interface_test
#else
#define IOInit               MHA_DYNAMIC_IOInit
#define IOPrepare            MHA_DYNAMIC_IOPrepare
#define IOStart              MHA_DYNAMIC_IOStart
#define IOStop               MHA_DYNAMIC_IOStop
#define IORelease            MHA_DYNAMIC_IORelease
#define IOSetVar             MHA_DYNAMIC_IOSetVar
#define IOStrError           MHA_DYNAMIC_IOStrError
#define IODestroy            MHA_DYNAMIC_IODestroy
#define dummy_interface_test MHA_DYNAMIC_dummy_interface_test
#endif
extern "C"
int IOInit(int fragsize,
           float samplerate,
           IOProcessEvent_t proc_event,
           void* proc_handle,
           IOStartedEvent_t start_event,
           void* start_handle,
           IOStoppedEvent_t stop_event,
           void* stop_handle,
           void** handle)
{
    try {
        assert(handle != 0); // Would be a framework bug
        *handle = new io_tcp_t(fragsize, samplerate,
                               proc_event, proc_handle,
                               start_event, start_handle,
                               stop_event, stop_handle);
    }
    catch(MHA_Error& e){
        return copy_error(e);
    }
    return ERR_SUCCESS;
}

extern "C"
int IOPrepare(void* handle,
              int num_inchannels,
              int num_outchannels)
{
    if (handle == 0) return ERR_IHANDLE;
    try {
        io_tcp_t * io_tcp = static_cast<io_tcp_t *>(handle);
        io_tcp->prepare(num_inchannels, num_outchannels);
    } 
    catch(MHA_Error& e){
        return copy_error(e);
    }
    return ERR_SUCCESS;
}

extern "C"
int IOStart(void * handle){
    if (handle == 0) return ERR_IHANDLE;
    try {
        io_tcp_t * io_tcp = static_cast<io_tcp_t *>(handle);
        io_tcp->start();
    } 
    catch(MHA_Error& e){
        return copy_error(e);
    }
    return ERR_SUCCESS;
}

extern "C"
int IOStop(void * handle)
{
    if (handle == 0) return ERR_IHANDLE;
    try {
        io_tcp_t * io_tcp = static_cast<io_tcp_t *>(handle);
        io_tcp->stop();
    } 
    catch(MHA_Error& e){
        return copy_error(e);
    }
    return ERR_SUCCESS;
}

extern "C"
int IORelease(void* handle)
{
    if (handle == 0) return ERR_IHANDLE;
    try {
        io_tcp_t * io_tcp = static_cast<io_tcp_t *>(handle);
        io_tcp->release();
    } 
    catch(MHA_Error& e){
        return copy_error(e);
    }
    return ERR_SUCCESS;
}

extern "C"
int IOSetVar(void* handle, const char* cmd, char* retval, unsigned int len)
{
    if (handle == 0) return ERR_IHANDLE;
    try {
        io_tcp_t * io_tcp = static_cast<io_tcp_t *>(handle);
        io_tcp->parse(cmd, retval, len);
    } 
    catch(MHA_Error& e){
        return copy_error(e);
    }
    return ERR_SUCCESS;
}

extern "C"
const char* IOStrError(void* handle, int err)
{
    (void)handle;
    switch( err ){
    case ERR_SUCCESS :
        return "Success";
    case ERR_IHANDLE :
        return "Invalid handle.";
    case ERR_USER :
        return user_err_msg;
    default :
        return "Unknown error.";
    }
}

extern "C"
void IODestroy(void* handle){
    delete static_cast<io_tcp_t *>(handle);
}

extern "C"
void dummy_interface_test(void){
#ifdef MHA_STATIC_PLUGINS
    MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOTCP_,IOInit);
    MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOTCP_,IOPrepare);
    MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOTCP_,IOStart);
    MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOTCP_,IOStop);
    MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOTCP_,IORelease);
    MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOTCP_,IOSetVar);
    MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOTCP_,IOStrError);
    MHA_CALLBACK_TEST_PREFIX(MHA_STATIC_MHAIOTCP_,IODestroy);
#else
    MHA_CALLBACK_TEST(IOInit);
    MHA_CALLBACK_TEST(IOPrepare);
    MHA_CALLBACK_TEST(IOStart);
    MHA_CALLBACK_TEST(IOStop);
    MHA_CALLBACK_TEST(IORelease);
    MHA_CALLBACK_TEST(IOSetVar);
    MHA_CALLBACK_TEST(IOStrError);
    MHA_CALLBACK_TEST(IODestroy);
#endif
}

/*
 * Local Variables:
 * compile-command: "make -C .."
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * coding: utf-8-unix
 * End:
 */
