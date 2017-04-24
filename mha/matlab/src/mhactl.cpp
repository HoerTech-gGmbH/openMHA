#include "mexhelperfunctions.h"
#include <string>
#include <map>
#include "mha_error.hh"
#include "mha_os.h"
#include "text_client.hh"
#include <unistd.h>
#include <ace/Init_ACE.h>

#define BUFSIZE 0x100000

#ifdef DEBUG_MHACTL
#define DEBUGLOG(fmt,a,b,c,d) \
do { \
  FILE * debuglogfile = fopen("mhactl.dbg.log","a"); \
  if (debuglogfile) { \
    fprintf(debuglogfile, fmt, a,b,c,d); \
    fclose(debuglogfile); \
  } \
} while(0)
#else
#define DEBUGLOG(fmt,a,b,c,d) do {} while(0)
#endif

static const double HANDLE_FACTOR = 33.714159265358979;
static inline
double HANDLe(void * pointer) {
  return reinterpret_cast<long>(pointer) / HANDLE_FACTOR;
}
typedef std::map<double, Text_Client *> Conn_Map;

static Conn_Map * connections = 0;

static void destroy()
{
  if (connections != 0) {
    for (Conn_Map::iterator i = connections->begin();
         i != connections->end();
         ++i) {
        delete i->second;
        i->second = 0;
    }
    if (connections->size() > 0)
        mexPrintf("mhactl: unlinked mex-dll. "\
                  "Destroyed %u open connections\n",
                  unsigned(connections->size()));
    delete connections;
    connections = 0;
  }
}

static
void initialize()
{
  ACE::init();
  if (connections == 0) {
    connections = new Conn_Map;
    mexAtExit(&destroy);
  }
}

std::string rm_newline(const std::string& s)
{
    std::string r(s);
    while( r.size() && ((r[r.size()-1] == '\n')||(r[r.size()-1] == '\r')) )
	r.erase(r.size()-1,1);
    return r;
}

class handle_struct_t {
    const mxArray * matlab_struct;
    unsigned short port;
    std::string host;
    double timeout;
    Text_Client * socket;
public:
    explicit handle_struct_t(const mxArray *);
    virtual ~handle_struct_t() {}
    virtual unsigned short get_port() const {return port;}
    virtual const std::string & get_host() const {return host;}
    virtual double get_timeout() const {return timeout;}
    virtual Text_Client * get_socket() const {return socket;}
    virtual mxArray * as_mxArray() const;
    virtual void open();
    virtual void close();
    virtual void eval(const std::vector<std::string> & cmds,
                      std::vector<std::string> & retval,
                      std::vector<int> & status) const;
};

class Socket_Manager {
    Text_Client * socket;
    bool manage_socket;
public:
    Socket_Manager(const handle_struct_t * handle);
    virtual ~Socket_Manager();
    virtual Text_Client * get_socket() const { return socket; }
};

Socket_Manager::Socket_Manager(const handle_struct_t * handle)
    : socket( handle->get_socket() ), manage_socket( socket == 0 )
{
    if( manage_socket ) {
        const char * promptv[] = {"(MHA:success)", "(MHA:failure)", 0};
        socket = new Text_Client( promptv, BUFSIZE );
        /*DEBUGLOG("(%p) Managing socket to %s:%hu (Timeout: %f)\n",
                 this, handle->get_host(),
                 handle->get_port(), handle->get_timeout());*/
        socket->connect( handle->get_host(),
                         handle->get_port(),
                         handle->get_timeout() );
    }
}
Socket_Manager::~Socket_Manager()
{
    if( manage_socket ) {
        /*DEBUGLOG("(%p) Closing managed socket\n",
          this, 0,0,0);*/
        delete socket;
        manage_socket = false;
    }
    socket = 0;
}

handle_struct_t::handle_struct_t(const mxArray * m)
    : matlab_struct(m), port(33337), host("localhost"), timeout(5.0), socket(0)
{
    mxArray * mport = mxGetField(m,0,"port");
    if( mport != 0 ) {
        int iport;
        MHAMex::mx_convert(mport, iport);
        port = iport;
        if( int(port) != iport )
            throw MHA_Error(__FILE__,__LINE__,"Invalid port number %d", iport);
    }

    mxArray * mhost = mxGetField(m,0,"host");
    if( mhost != 0 ) MHAMex::mx_convert(mhost, host);

    mxArray * mtimeout = mxGetField(m,0,"timeout");
    if( mtimeout != 0 ) MHAMex::mx_convert(mtimeout, timeout);

    mxArray * msocket = mxGetField(m,0,"socket");
    if( msocket != 0 ) {
        double handle;
        MHAMex::mx_convert(msocket, handle);
        if( connections->find(handle) == connections->end() )
            throw MHA_ErrorMsg("Field 'socket' does not"
                             " identify a valid socket.");
        socket = connections->find(handle)->second;
    }
}

mxArray * handle_struct_t::as_mxArray() const
{
    mxArray * m = mxDuplicateArray(matlab_struct);
    const int field_count = 4;
    const char * fields[field_count] = {"port","host","timeout","socket"};
    mxArray * field_values[field_count];
    field_values[0] = mxCreateDoubleScalar(port);
    field_values[1] = MHAMex::mx_create(host);
    field_values[2] = mxCreateDoubleScalar(timeout);
    field_values[3] = mxCreateDoubleScalar(HANDLe(socket));
    for (int field_index = 0; field_index < field_count; ++field_index) {
        int field_number = mxGetFieldNumber(m, fields[field_index]);
        if (field_number == -1)
            field_number = mxAddField(m, fields[field_index]);
        mxSetFieldByNumber(m, 0, field_number, field_values[field_index]);
    }
    if (socket == 0) {
        mxRemoveField(m, mxGetFieldNumber(m, "socket"));
    }
    return m;
}

void handle_struct_t::open()
{
    if (socket != 0)
        throw MHA_ErrorMsg("Close existing connection before opening a new one");
    const char * promptv[] = {"(MHA:success)", "(MHA:failure)", 0};
    socket = new Text_Client(promptv, BUFSIZE);
    socket->connect(host, port, timeout);
    (*connections)[HANDLe(socket)] = socket;
}

void handle_struct_t::close()
{
    if (socket == 0)
        throw MHA_ErrorMsg("Cannot close closed connection");
    connections->erase(HANDLe(socket));
    delete socket;
    socket = 0;
}

void handle_struct_t::eval(const std::vector<std::string> & cmds,
                  std::vector<std::string> & retval,
                  std::vector<int> & status) const
{
    std::string full_cmd("");
    unsigned int k;
    DEBUGLOG("sending %u commands to the MHA:\n",unsigned(cmds.size()),0,0,0);
    //mexPrintf("mhactl: sending %u commands to the MHA:\n",
    //          unsigned(cmds.size()));
    for(k=0;k<cmds.size();k++){
	if( cmds[k].find("\n") < cmds[k].size())
	    throw MHA_Error(__FILE__,__LINE__,
			    "MHA command no. %d contains one or more line breaks.",k);
	full_cmd += cmds[k] + "\n";
        DEBUGLOG("->%u) %s\n",k,cmds[k].c_str(),0,0);
    }

    std::vector<response_t> r = 
        Socket_Manager(this).get_socket()->cmd(full_cmd,timeout, cmds.size());
    retval.clear();
    status.clear();
    DEBUGLOG("received %u responses:\n",unsigned(r.size()),0,0,0);
    if (r.size() != cmds.size())
        DEBUGLOG("ERROR: %u requests, %u answers!\n",
                 unsigned(cmds.size()),unsigned(r.size()),0,0);
    for(k=0;k<r.size();k++){
        retval.push_back(rm_newline(r[k].content));
        status.push_back(r[k].prompt);
        DEBUGLOG("=>%u) (%d)%s\n",k,status[k],retval[k].c_str(),0);
    }
}

enum method_enum_t {METHOD_EVAL, METHOD_OPEN, METHOD_CLOSE};

static void usage();
static method_enum_t get_method(const mxArray * mxName, int nrhs);

extern "C"
void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[] )
{
    if (connections == 0)
        initialize();
    if( nrhs < 2 )
        usage();
    try{
        handle_struct_t handle_struct(prhs[0]);
        fflush(stdout);
        switch (get_method(prhs[1], nrhs)) {
        case METHOD_OPEN:
            handle_struct.open();
            plhs[0] = handle_struct.as_mxArray();
            break;
        case METHOD_CLOSE:
            handle_struct.close();
            plhs[0] = handle_struct.as_mxArray();
            break;
        case METHOD_EVAL: {
            std::vector<std::string> cmds;
            MHAMex::mx_convert(prhs[2],cmds);
	    std::vector<std::string> retval;
	    std::vector<int> status;
            handle_struct.eval(cmds,retval,status);
            if( (retval.size() != cmds.size())
                || (status.size() != cmds.size()) )
		throw MHA_Error(__FILE__,__LINE__,
				"Sent %d queries, but received only %d"
                                " responses.",cmds.size(),retval.size());
            plhs[0] = MHAMex::mx_create( retval );
            if( nlhs > 1 )
                plhs[1] = MHAMex::mx_create( status );
        }
            break;
        };
    }        
    catch(std::exception& e) {
        mexErrMsgTxt(e.what());
    }
}

static void usage()
{
    mexErrMsgTxt
        ("Usage:\n\n"
         "mhactl is a tool for sending a configuration command over a network connection:\n"
         "It uses matlab structures as handles, containing the following fields:\n"
         "  port: Connect to this tcp port.      (Default: 33337)\n"
         "  host: The hostname to connect to.    (Default: localhost)\n"
         "  timeout: number of seconds for connection initiation and data exchange.\n"
         "                                       (Default: 5)\n"
         "  socket: socket handle installed by the 'open' command\n\n"
         "   handle = mhactl(handle, 'open')\n"
         "      On input, handle may contain the fields port, host, and timeout, but not\n"
         "      socket.  A connection to the mha is initiated and the socket handle is\n"
         "      installed in the returned handle structure.\n\n"
         "   handle = mhactl(handle, 'close')\n"
         "      On input, handle must contain the socket field. The corresponding network\n"
         "      connection is closed and the socket handle fiels is removed from the\n"
         "      handle structure.\n\n"
         "   [retv, status] = mhactl(handle, 'eval', cmds)\n"
         "      The eval method behaves differently depending on presence of a socket\n"
         "      field in the input handle structure.  If if is present, then that\n"
         "      connection is used for data transfer.  If it is not present, then \n"
         "      .  Returns the Framework answer.\n\n"
         "\n"
         "\n(c) 2005 by HoerTech gGmbH, Marie-Curie-Str. 2, D-26129 Oldenburg"
         );
}

static method_enum_t get_method(const mxArray * mxName, int nrhs)
{
    if( nrhs > 3 )
        mexErrMsgTxt("Expected not more then 3 arguments.");
    if( nrhs < 2 )
        mexErrMsgTxt("Need at least 2 arguments.");
    if( (!mxIsChar(mxName)) || (mxGetM(mxName) != 1) )
        mexErrMsgTxt("2nd argument has to be a string");
    std::string name;
    MHAMex::mx_convert(mxName, name);
    if( name == "eval" ) {
        if( nrhs != 3 )
            mexErrMsgTxt("Method 'eval' needs a cell array of"
                         " configuration language commands");
        return METHOD_EVAL;
    }

    method_enum_t method;
    if( name == "open" )
        method = METHOD_OPEN;
    else if( name == "close" )
        method = METHOD_CLOSE;
    else 
        throw MHA_Error(__FILE__,__LINE__,
                        "Unknown method '%s'",
                        name.c_str());
    if( nrhs != 2 )
        throw MHA_Error(__FILE__,__LINE__,
                        "Too many arguments for method '%s'",
                        name.c_str());
    return method;
}



// Local Variables:
// c-basic-offset: 4
// indent-tabs-mode: nil
// compile-command: "make -C ../.."
// End:
