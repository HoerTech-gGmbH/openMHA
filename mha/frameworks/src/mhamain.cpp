// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2007 2008 2009 2010 2011 2012 2013 HörTech gGmbH
// Copyright © 2014 2016 2017 2018 2019 2020 HörTech gGmbH
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
#include "mhafw_lib.h"
#include "mha_utils.hh"
#include <getopt.h>
#include <fstream>
#include <unistd.h>
#include <asio/connect.hpp>
#include <asio/write.hpp>
#include <asio/read_until.hpp>
#include <thread>

#ifdef _WIN32
#define getpid(x) _getpid(x)
#endif

/// MHA Framework listening on TCP port for commands
class mhaserver_t : public fw_t {
    class tcp_server_t : public mha_tcp::server_t {
        mhaserver_t * mha;
    public:
        tcp_server_t(const std::string & interface, uint16_t port,
                     mhaserver_t * mha)
            : mha_tcp::server_t(interface, port),   mha(mha)
        {}

        /** This method is invoked when a line of text is received on one of
         * the accepted connections. Override this method to process the
         * communication with the client.
         * @param c the connection that has received this line
         * @param l the line that has been received, without the line ending
         * @return client should return true when client wants to read another
         *         line of text, else false.        */
        virtual bool
        on_received_line(std::shared_ptr <mha_tcp::buffered_socket_t> c,
                         const std::string & l) override
        {
            bool old_exit_request = mha->exit_request();
            c->queue_write(mha->on_received_line(l));
            bool new_exit_request = mha->exit_request();
            if (new_exit_request && !old_exit_request) {
                shutdown();
            }
            return !new_exit_request;
        }
    };
    std::shared_ptr<tcp_server_t> tcpserver;
public:
    /** @param ao Acknowledgement string at end of successful command responses
        @param af Acknowledgement string at end of failed command responses
        @param lf File system path of file to use as log file. MHA appends.
    */
    mhaserver_t(const std::string &ao, const std::string &af,
                const std::string &lf, bool b_interactive_);
    ~mhaserver_t();
    /** A line of text was received from network client */
    virtual std::string on_received_line(const std::string & line);
    /** Notification: "TCP port is open" */
    virtual void acceptor_started();
    /** sends an announcement which port this MHA is listening on to the creator of the
        process. See command line option --announce */
    virtual void send_port_announcement();
    /** Starts a separate thread that reads lines from stdin and forwards these lines over
        TCP to the MHA configuration thread which multiplexes multiple TCP connections.
        Enables users to type mha configuration language commands directly into the terminal
        where MHA was started, without the need to use third-party tools like nc or putty. */
    virtual void start_stdin_thread();
    /** If set to nonzero, the spawning process has asked to be notified
        of the TCP port used by this process. */
    virtual void set_announce_port(unsigned short announce_port);
    /** Log a message to log file */
    inline void logstring(const std::string&);
    /** Accept network connections and act on commands.
        Calls #acceptor_started() when the TCP port is opened.
        Calls on_received_line for every line received.
        @return exit code that can be used as process exit code */
    int run(unsigned short port, const std::string & _interface);
private:
    std::string ack_ok;
    std::string ack_fail;
    std::string logfile;
    unsigned short announce_port;
    bool b_interactive;
    MHAParser::int_mon_t pid_mon;
public:
    MHAParser::int_t port;
};

inline void mhaserver_t::logstring(const std::string& s)
{
    if( logfile.size() ){
        std::ofstream lf(logfile.c_str(),std::ios_base::app);
        if( lf.fail() )
            throw MHA_Error(__FILE__,__LINE__,
                            "Unable to open log file \"%s\".",
                            logfile.c_str());
        time_t now = time(NULL);
        char timestr[1024];
        memset(timestr,0,1024);
        strftime(timestr,1023,"%Y-%m-%d %H:%M:%S",localtime(&now));
        lf << timestr << " " << s;
    }
}

void mhaserver_t::acceptor_started()
{
    port.data = tcpserver->get_port();
    if (announce_port != 0)
        send_port_announcement();
    if(b_interactive)
        start_stdin_thread();
}


void mhaserver_t::start_stdin_thread()
{
    using MHAUtils::strip;
    using MHAUtils::remove;
    std::thread stdin_thread([this](){
            std::string stdin_line;
            asio::io_context terminal_context;
            asio::ip::tcp::socket terminal_connection(terminal_context);
            try{
                asio::connect(terminal_connection, std::vector<asio::ip::
                              tcp::endpoint>{tcpserver->get_endpoint()});
            }
            catch(std::exception& e){
                std::cerr<<"Caught exception during connection to stdin:\n"<<e.what()<<'\n';
                logstring("Caught exception during connection to stdin:\n"+std::string(e.what())+'\n');
                exit(1);
            }
            catch(...){
                std::cerr<<"Caught unknown exception during connection to stdin.\n";
                logstring("Caught unknown exception during connection to stdin.\n");
                exit(1);
            }
            asio::streambuf streambuf;
            unsigned nlines=1U;
            auto read_until_prompt = [&](){
                std::string response_line;
                while (strip(response_line) != strip(ack_fail) && strip(response_line) != strip(ack_ok)) {
                    asio::read_until(terminal_connection, streambuf, '\n');
                    std::istream istream(&streambuf);
                    std::getline(istream, response_line);
                    response_line=strip(response_line);
                    printf("%s\n", response_line.c_str());
                }
            };

            std::cout<<"mha ["<<nlines++<<"] ";
            while (std::getline(std::cin, stdin_line).good()) {
                if(stdin_line.size() and stdin_line[0]=='#'){
                    continue;
                }
                stdin_line=strip(stdin_line);
                // special case: If we do not handle this, we'd try to
                // send an exit request twice, but mha might have already
                // closed the connection
                if(remove(stdin_line,' ')=="cmd=quit"){
                    break;
                }
                stdin_line+='\n';
                asio::write(terminal_connection, asio::buffer(stdin_line));
                read_until_prompt();
                if(remove(stdin_line,' ')!="cmd=quit"){
                    std::cout<<"mha ["<<nlines++<<"] ";
                }
            }
            asio::write(terminal_connection, asio::buffer("cmd=quit\n"));
            read_until_prompt();
        });
    stdin_thread.detach();
}

void mhaserver_t::send_port_announcement()
{
    if (announce_port == 0) {
        // nothing to announce
        return;
    }
    std::shared_ptr<asio::ip::tcp::socket> announcer = std::
        make_shared<asio::ip::tcp::socket>(tcpserver->get_context());
    auto announce_endpoints = asio::ip::tcp::resolver(tcpserver->get_context()).
        resolve("127.0.0.1", std::to_string(announce_port));
    std::shared_ptr<const std::string> announcement =
        std::make_shared<const std::string>
        ("pid=" + std::to_string(getpid()) + "\n"
         "port=" + std::to_string(tcpserver->get_port()) + "\n");
    logstring("Announcing TCP port to process creator at port "
              + std::to_string(announce_port) + "...\n");
    using asio::async_connect;
    async_connect(*announcer, announce_endpoints,
                  [announcer,this,announcement]
                  (const asio::error_code & ec,
                   const asio::ip::tcp::endpoint & endpoint) {
                      if (ec) {
                          logstring("announcement failed: "+ec.message()+"\n");
                          announcer->close();
                          return;
                      }
                      logstring("connected to announcement port,"
                                " sending announcement...\n");
                      using asio::async_write;
                      async_write(*announcer, asio::buffer(*announcement),
                                  [announcer,this,announcement]
                                  (const asio::error_code & ec, std::size_t) {
                                      if (ec) {
                                          logstring("announcement failed: " +
                                                    ec.message() + "\n");
                                      } else {
                                          logstring("announcement sent, closing"
                                                    " connection.\n");
                                      }
                                      announcer->close();
                                  });
                  });
}

int mhaserver_t::run(unsigned short port, const std::string & _interface)
{
    if (tcpserver)
        throw MHA_Error(__FILE__, __LINE__,
                        "BUG: Nested invocation of mhaserver_t::run");
    logstring("Opening TCP server on " +
              _interface + ":" + std::to_string(port) + "\n");
    tcpserver = std::make_shared<tcp_server_t>(_interface, port, this);
    logstring("TCP server listens on " +
              _interface + ":" + std::to_string(tcpserver->get_port()) + "\n");
    acceptor_started();

    tcpserver->run();

    logstring("exit request, closing server.\n");
    tcpserver = nullptr; // closes server
    logstring("exit request, server closed.\n");
    // TODO: Replace with Error/Success exit code
    return 0;
}

void mhaserver_t::set_announce_port(unsigned short announce_port)
{ this->announce_port = announce_port; }

mhaserver_t::mhaserver_t(const std::string& ao,const std::string& af,const std::string& lf, bool b_interactive_)
    : tcpserver(0),
      ack_ok(ao),
      ack_fail(af),
      logfile(lf),
      announce_port(0),
      b_interactive(b_interactive_),
      pid_mon("PID of mha server"),
      port("Port number of MHA server (0 = use command line settings).","0","[0,]")
{
    if( logfile.size() ){
        std::ofstream lfh(logfile.c_str(),std::ios_base::app);
        if( lfh.fail() )
            throw MHA_Error(__FILE__,__LINE__,
                            "Unable to open log file \"%s\".",
                            logfile.c_str());
        lfh.close();
    }
    pid_mon.data = getpid();
    insert_item("pid",&pid_mon);
    insert_member(port);
    logstring("MHA server initialized.\n");
}

mhaserver_t::~mhaserver_t()
{
    logstring("MHA server closed successfully.\n");
}

std::string mhaserver_t::on_received_line(const std::string& cmd)
{
    using MHAParser::StrCnv::val2str;
    std::string retv("");
    std::string lcmd(cmd);
    // remove \n and \r from line ends:
    if( lcmd.size() && ((lcmd[lcmd.size()-1]=='\r')||(lcmd[lcmd.size()-1]=='\n'))) {
        lcmd.erase(lcmd.size()-1,1u);
    }

    logstring("received: \""+lcmd+"\"\n");
    try{
        if( lcmd.size() ){
            retv += parse(lcmd);
            if( retv.size() && (retv[retv.size()-1] != '\n') )
                retv += '\n';
        }
        retv += ack_ok;
    }
    catch(std::exception& e){
        retv += e.what();
        if( retv.size() && (retv[retv.size()-1] != '\n') )
            retv += '\n';
        retv += ack_fail;
    }
    logstring(retv);
    return retv;
}

#define HELP_TEXT \
"\n"\
"Usage:\n"\
"mha [options] [commands]\n"\
"\n"\
"Possible options are:\n"\
" --quiet | -q              suppress all output\n"\
" --port=portno | -p portno set port number (default: 33337)\n"\
" --announce=port | -a port announce pid & portno to 127.0.0.1:port \n"\
" --interactive             start in interactive mode\n"\
" --interface=str | -i str  set the server interface to 'str'\n"\
" --daemon | -d             start in daemon mode, i.e., restart after exit\n"\
" --ok-ack=str | -o str     set ok acknowledgement string\n"\
" --fail-ack=str | -f str   set failure acknowledgement string\n"\
" --log=logfile             activate logging to logfile\n"\
" --help | -h               show this help screen\n"\
" --lockstr=str | -l str    create a port lockfile with content 'str'\n"\

#ifndef NORELEASE_WARNING // This is not a release build. Add warning to output.
#define NORELEASE_WARNING "\n" \
    "##############################################################\n" \
    "# ATTENTION: THIS VERSION OF OPENMHA IS A PRERELEASE VERSION #\n" \
    "##############################################################\n" \
    "\n"
#define VERSION_EXTENSION "+"
#endif

#define GREETING_TEXT \
NORELEASE_WARNING \
"The Open Master Hearing Aid (openMHA) server version " \
MHA_RELEASE_VERSION_STRING    VERSION_EXTENSION    "\n" \
"Copyright (c) 2005-2020 HoerTech gGmbH, D-26129 Oldenburg, Germany"\
"\n\n"\
"This program comes with ABSOLUTELY NO WARRANTY; "\
"for details see file COPYING.\n"\
"This is free software, and you are welcome to redistribute it \n"\
"under the terms of the GNU AFFERO GENERAL PUBLIC LICENSE, Version 3; \n"\
"for details see file COPYING.\n\n"

void create_lock(unsigned int p,const std::string& s)
{
    std::string fname("locks/"+MHAParser::StrCnv::val2str((int)p));
    std::ofstream lockfile(fname.c_str());
    if( lockfile.fail() )
        throw MHA_Error(__FILE__,__LINE__,
                        "Unable to create lock file \"%s\".",fname.c_str());
    lockfile << s;
}

void remove_lock(unsigned int p)
{
    std::string fname("locks/"+MHAParser::StrCnv::val2str((int)p));
    remove(fname.c_str());
}

extern "C" int mhamain(int argc, char* argv[])
{
    unsigned short port(33337);
    srand(time(0) + 1481490587);
    try{
        bool b_quiet(false);
        bool b_daemon(false);
        bool b_interactive(false);
        std::string ack_ok("(MHA:success)\n");
        std::string ack_fail("(MHA:failure)\n");
        unsigned short announce_port(0);
        std::string interface_("127.0.0.1");
        std::string lock_str("");
        std::string logfile("");
        bool b_create_lock(false);
        // command line interface...
        int option;
        static struct option long_options[] = {
            {"quiet",      0, NULL, 'q'},
            {"help",       0, NULL, 'h'},
            {"port",       1, NULL, 'p'},
            {"announce",   1, NULL, 'a'},
            {"interactive",0, NULL, 't'},
            {"interface",  1, NULL, 'i'},
            {"ok-ack",     1, NULL, 'o'},
            {"fail-ack",   1, NULL, 'f'},
            {"lockstr",    1, NULL, 'l'},
            {"log",        1, NULL, 'm'},
            {"daemon",     0, NULL, 'd'},
            {NULL,         0, NULL, 0  }
        };
        static char short_options[] = "qhp:a:o:f:i:dm:";
        while( (option = getopt_long(argc,argv,short_options,long_options,NULL)) > -1 ){
            switch( option ){
            case 'h' :
                printf("%s", GREETING_TEXT);
                printf("%s", HELP_TEXT);
                exit(0);
                // break not needed, exit does not return
            case 'p' :
                port = atoi(optarg);
                break;
            case 'a' :
                announce_port = atoi(optarg);
                break;
            case 'i' :
                interface_ = optarg;
                break;
            case 'o' :
                ack_ok = optarg;
                break;
            case 'f' :
                ack_fail = optarg;
                break;
            case 'm' :
                logfile = optarg;
#ifndef _WIN32    //if linux, make finding the console easier
        if ( logfile.compare("CON") == 0 ) {
            //replace with linux equivalent: /proc/self/fd/2
            //standard error of the current process
            logfile = "/proc/self/fd/2";
        }
#endif
                break;
            case 'l' :
                lock_str = optarg;
                b_create_lock = true;
                break;
            case 'q' :
                b_quiet = true;
                break;
            case 'd' :
                b_daemon = true;
                break;
            case 't':
                b_interactive = true;
                break;
            };
        }
        if(!b_quiet) {
            printf(GREETING_TEXT);
            fflush(stdout);
        }
        else{
            close(1);
            close(2);
        }
        mhaserver_t* server;
        int rval = 0;
        do{
            server = new mhaserver_t(ack_ok,ack_fail,logfile,b_interactive);
            server->set_announce_port(announce_port);
            if( b_create_lock )
                create_lock(port,lock_str);
            if( !b_quiet )
                for(int k=optind;k<argc;k++){
                    server->logstring("Parsing command line argument \""+std::string(argv[k])+"\", please wait.\n");
                    std::cout << server->parse(argv[k]) << std::endl;
                }
            else
                for(int k=optind;k<argc;k++){
                    server->logstring("Parsing command line argument \""+std::string(argv[k])+"\", please wait.\n");
                    server->parse(argv[k]);
                }

            server->logstring("Command line arguments parsed, accepting network control input.\n");
            if( server->port.data > 0 ){
                server->logstring("Overwriting port number by configuration value.\n");
                port = server->port.data;
            }
            if( !server->exit_request() )
                rval = server->run(port, interface_);
            else
                b_daemon = false;
            if( b_create_lock )
                remove_lock(port);
            delete server;
        }while( b_daemon );
        return rval;
    }
    catch(std::exception& e){
        remove_lock(port);
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 1;
}

/*
 * Local Variables:
 * compile-command: "make -C .."
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * coding: utf-8-unix
 * End:
 */
