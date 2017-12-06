// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2007 2008 2009 2010 2011 2012 2013 HörTech gGmbH
// Copyright © 2014 2016 2017 HörTech gGmbH
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

#ifdef _WIN32
# include <WINSOCK2.h>
#endif
#include "mhafw_lib.h"
#include "mha_parser.hh"
#include <getopt.h>
#include <stdarg.h>
#include <fstream>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <string>
#include "mha_tcp.hh"

#define MAX_LINE_LENGTH 0x100000

#ifdef _WIN32 
#define getpid(x) _getpid(x) // FIXME: use ACE_OS::getpid()
#endif

/// MHA Framework listening on TCP port for commands
class mhaserver_t : public fw_t {
    MHA_TCP::Server * tcpserver;
public:
    /** @param ao Acknowledgement string at end of successful command responses
        @param af Achknoledgement string at end of failed command responses
        @param lf File system path of file to use as log file. MHA appends.
    */
    mhaserver_t(const std::string &ao, const std::string &af,
                const std::string &lf);
    ~mhaserver_t();
    /** A line of text was received from network client */
    virtual std::string received_group(const std::string & line);
    /** Notification: "TCP port is open" */
    virtual void acceptor_started(int status);
    /** If set to nonzero, the spawning process has asked to be notified
        of the TCP port used by this process. */
    virtual void set_announce_port(unsigned short announce_port);
    /** Log a message to log file */
    inline void logstring(const std::string&);
    /** Accept network connections and act on commands.
        Calls #acceptor_started() when the TCP port is opened.
        Calls received_group for every line received.
        @return exit code that can be used as process exit code */
    int run(unsigned short port, const std::string & _interface);
private:
    std::string ack_ok;
    std::string ack_fail;
    std::string logfile;
    unsigned short announce_port;
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

void mhaserver_t::acceptor_started(int status)
{
    using MHAParser::StrCnv::val2str;
    port.data = tcpserver->get_port();
    if (status == 0 && announce_port != 0) {
        // timeout (0.5 seconds) for connection and sending.
        MHA_TCP::Timeout_Watcher timeout_watcher(0.5);
        MHA_TCP::Client announcer("127.0.0.1", announce_port, timeout_watcher);
        logstring("Announcing TCP port to process creator at port " 
                  + val2str(int(announce_port)) + "\n");
        timeout_watcher.observe(announcer.get_write_event());
        announcer.try_write("pid=" + val2str(int(getpid())) + "\n" +
                            "port=" + val2str(int(port.data)) + "\n");
        while (announcer.needs_write()) {
            std::set<MHA_TCP::Wakeup_Event *> wake_set = timeout_watcher.wait();
            if (wake_set.find(announcer.get_write_event()) == wake_set.end()) {
                throw MHA_Error(__FILE__, __LINE__,
                                "Cannot announce MHA port to port %hu: Timeout",
                                announce_port);
            }
            announcer.try_write();
        }
        logstring("Announced\n");
    }
}

int mhaserver_t::run(unsigned short port, const std::string & _interface)
{
    using MHAParser::StrCnv::val2str;
    if (tcpserver)
        throw MHA_Error(__FILE__, __LINE__,
                        "BUG: Nested invocation of mhaserver_t::run");
    tcpserver = new MHA_TCP::Server(port, _interface);
    logstring("Listening on " + _interface + ":" + val2str(int(port)) + "\n");
    acceptor_started(0);
    while (!exit_request()) {
        MHA_TCP::Connection * connection = tcpserver->accept();
        logstring("new connection from " + connection->get_peer_address() +
                  ":" +  val2str(int(connection->get_peer_port())) + "\n");
        try {
            while ((!exit_request()) && (!connection->eof())) {
                connection->write(received_group(connection->read_line()));
            }
        } catch (MHA_Error & e) {
            // If the connection is force-closed by the client, an Exception may occur here.
            logstring(Getmsg(e)+std::string("\n"));
        }
        try {
        delete connection;
        } catch(...) {logstring("Delete threw!\n");}
        logstring("Closing Connection\n");

    }
    logstring("exit request, closing server.\n");
    delete tcpserver;
    tcpserver = 0;
    logstring("exit request, server closed.\n");
    // TODO: Replace with Error/Success exit code
    return 0;
}

void mhaserver_t::set_announce_port(unsigned short announce_port)
{ this->announce_port = announce_port; }

mhaserver_t::mhaserver_t(const std::string& ao,const std::string& af,const std::string& lf)
    : tcpserver(0),
      ack_ok(ao),
      ack_fail(af),
      logfile(lf),
      announce_port(0),
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

std::string mhaserver_t::received_group(const std::string& cmd)
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
" --interface=str | -i str  set the server interface to 'str'\n"\
" --daemon | -d             start in daemon mode, i.e., restart after exit\n"\
" --ok-ack=str | -o str     set ok acknowledgement string\n"\
" --fail-ack=str | -f str   set failure acknowledgement string\n"\
" --log=logfile             activate logging to logfile\n"\
" --help | -h               show this help screen\n"\
" --lockstr=str | -l str    create a port lockfile with content 'str'\n"\

#define GREETING_TEXT \
"The Open Master Hearing Aid (openMHA) server\n"\
"Copyright (c) 2005-2017 HoerTech gGmbH, D-26129 Oldenburg, Germany"\
"\n\n"\
"This program comes with ABSOLUTELY NO WARRANTY; "\
"for details see file COPYING.\n"\
"This is free software, and you are welcome to redistribute it \n"\
"under the terms of the GNU AFFERO GENERAL PUBLIC LICENSE, Version 3; \n"\
"for details see file COPYING.\n\n"

void create_lock(unsigned int p,std::string s)
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
    bool b_quiet(false);
    bool b_daemon(false);
    std::string ack_ok("(MHA:success)\n");
    std::string ack_fail("(MHA:failure)\n");
    unsigned short port(33337), announce_port(0);
    std::string interface_("127.0.0.1");
    std::string lock_str("");
    std::string logfile("");
    bool b_create_lock(false);
    srand(time(0) + 1481490587);
    try{
        // command line interface...
        int option;
        static struct option long_options[] = {
            {"quiet",    0, NULL, 'q'},
            {"help",     0, NULL, 'h'},
            {"port",     1, NULL, 'p'},
            {"announce", 1, NULL, 'a'},
            {"interface",1, NULL, 'i'},
            {"ok-ack",   1, NULL, 'o'},
            {"fail-ack", 1, NULL, 'f'},
            {"lockstr",  1, NULL, 'l'},
            {"log",      1, NULL, 'm'},
            {"daemon",   0, NULL, 'd'},
            {NULL,       0, NULL, 0  }
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
            };
        }
        if(!b_quiet)
            printf(GREETING_TEXT);
        else{
            close(1);
            close(2);
        }
        mhaserver_t* server;
        int rval = 0;
        do{
            server = new mhaserver_t(ack_ok,ack_fail,logfile);
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
