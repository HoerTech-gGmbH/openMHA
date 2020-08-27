// This file is part of the HörTech Master Hearing Aid (MHA)
// Copyright © 2012 2013 2014 2015 2018 2019 2020 HörTech gGmbH
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
#include <algorithm>
#include <memory>
#include <stdio.h>
#include <lo/lo.h>
#include "mha_plugin.hh"

/** Class for converting messages received at a single osc address to a single
    AC variable. OSC variables are received asynchronously in a network thread
    and must not modify their AC variables directly, because MHA plugins may
    only access their AC variables while executing their prepare, release, or
    process callbacks.

    One osc2ac plugin uses multiple instances of osc_variable_t, one for each
    mapping of an OSC address to an AC variable.
 **/
class osc_variable_t {
public:
    /** An instance of this class cannot safely be copied. */
    osc_variable_t(const osc_variable_t &) = delete;
    /** Constructor. Allocates memory.
        @param name The name of the AC variable that stores the latest value.
        @param size Number of elements to copy from OSC message to AC variable.
        @param hAC Handle of Algorithm Communication Variable space.
        @param lost libLO Server Thread. */
    osc_variable_t(const std::string& name, unsigned int size,
                   algo_comm_t hAC, lo_server_thread lost);
    /** Copies the latest OSC data from the OSC storage to the AC storage.
     * To be executed during process callback of osc2ac plugin.
     * @todo Data is copied from array osc_data while osc_data may be changed
     *       simultaneously in another thread in method handler. */
    void sync_osc2ac() {ac_data.copy(osc_data);};
    /** Insert/Re-insert the AC variable into AC space. Should be done in each
     * process callback. */
    void ac_insert() {ac_data.insert();}
    /** Callback function called by network thread managed by liblo when a new
     * OSC message has been received. This static method forwards to the
     * instance method by casting user_data to osc_variable_t*.
     * @param path  Unused.
     * @param types The OSC data type indicator of the received message.
     * @param argv  Array of received OSC data.
     * @param argc  Number of elements in array of received OSC data.
     * @param msg   Unused.
     * @param user_data Pointer to osc_variable_t instance.
     * @return 1 if the message was accepted, 0 if not. */
    static int handler(const char *path, const char *types, lo_arg **argv,int argc, lo_message msg, void *user_data);
    /** Callback function called by network thread managed by liblo when a new
     * OSC message has been received. This instance method checks if the
     * received data is of expected length and contains only floats, and if yes,
     * copies the data into the buffer osc_data where the latest received data
     * for this osc address is stored until it is either overwritten by the
     * next data for the same osc address or copied to an AC variable.
     * @todo The data is copied to osc_data without thread synchronization.
     * @param types The OSC data type indicator of the received message.
     * @param argv  Array of received OSC data.
     * @param argc  Number of elements in array of received OSC data.
     * @return 1 if the message had correct length and contained only floats,
     *         0 if not. */
    int handler(const char *types, lo_arg **argv,int argc);
private:
    /** AC variable storage */
    MHA_AC::waveform_t ac_data;
    /** OSC variable storage */
    MHASignal::waveform_t osc_data;
    /** Name of AC variable and OSC address without the initial slash */
    std::string name_;
};

osc_variable_t::osc_variable_t(const std::string& name, unsigned int size,
                               algo_comm_t hAC, lo_server_thread lost)
    : ac_data(hAC,name,size,1,false),
      osc_data(size,1),
      name_("/"+name)
{
    std::string fmt;
    for(unsigned int k=0;k<size;k++)
        fmt += "f";
    lo_server_thread_add_method(lost, name_.c_str(), fmt.c_str(), handler,
                                this);
}

int osc_variable_t::handler(const char *path, const char *types,
                            lo_arg **argv,int argc, lo_message msg,
                            void *user_data)
{
    return static_cast<osc_variable_t*>(user_data)->handler(types,argv,argc);
}

int osc_variable_t::handler(const char *types, lo_arg **argv,int argc)
{
    bool valid_fmt(false);
    if ((argc>=0) && (argc == static_cast<int>(size(osc_data)) ) ) {
        valid_fmt = true;
        for(int k=0;k<argc;k++)
            if( types[k] != 'f' )
                valid_fmt = false;
        if( valid_fmt )
            for(int k=0;k<argc;k++)
                osc_data.buf[k] = argv[k]->f;
    }
    return valid_fmt;
}

/** OSC receiver implemented using liblo. */
class osc_server_t {
public:
    osc_server_t(const std::string& multicast_addr, const std::string& port);
    ~osc_server_t();
    void server_stop();
    void server_start();
    void insert_variable(const std::string& name, unsigned int size, algo_comm_t hAC);
    void sync_osc2ac();
    void ac_insert();
    static void error_h(int num, const char *msg, const char *path);
private:
    std::vector<std::unique_ptr<osc_variable_t>> pVars;
    lo_server_thread lost;
    bool is_running;
};

void osc_server_t::sync_osc2ac()
{
    for(auto& pVar : pVars){
        pVar->sync_osc2ac();
    }
}

void osc_server_t::ac_insert()
{
    for(auto& pVar : pVars){
        pVar->ac_insert();
    }
}


void osc_server_t::error_h(int num, const char *msg, const char *path)
{
    mha_debug("liblo server error %d in path %s: %s\n",num,path,msg);
}

void osc_server_t::server_stop()
{
    if( is_running )
        lo_server_thread_stop(lost);
    is_running = false;
}

void osc_server_t::server_start()
{
    if( !is_running )
        lo_server_thread_start(lost);
    is_running = true;
}

osc_server_t::osc_server_t(const std::string& multicastgroup, const std::string& serverport)
    : is_running(false)
{
    if( multicastgroup.size() )
        lost = lo_server_thread_new_multicast( multicastgroup.c_str(),
                                               serverport.c_str(),
                                               error_h );
    else
        lost = lo_server_thread_new( serverport.c_str(), error_h );
    if( !lost )
        throw MHA_Error(__FILE__,__LINE__,"liblo server error.");
}

void osc_server_t::insert_variable(const std::string& name, unsigned int size, algo_comm_t hAC)
{
    pVars.push_back(std::make_unique<osc_variable_t>(name, size, hAC, lost));
}

osc_server_t::~osc_server_t()
{
    server_stop();
    lo_server_thread_free(lost);
}

class osc2ac_t : public MHAPlugin::plugin_t<int>
{
public:
    osc2ac_t(algo_comm_t iac,const char* chain, const char* algo);
    void prepare(mhaconfig_t&);
    void release();
    mha_wave_t* process(mha_wave_t* s) {process();return s;};
    mha_spec_t* process(mha_spec_t* s) {process();return s;};
    void process();
private:
    void setlock(bool b);
    MHAParser::string_t host;
    MHAParser::string_t port;
    MHAParser::vstring_t vars;
    MHAParser::vint_t size;
    MHAEvents::patchbay_t<osc2ac_t> patchbay;
    std::unique_ptr<osc_server_t> srv;
};

void osc2ac_t::setlock(bool b)
{
    host.setlock(b);
    port.setlock(b);
    vars.setlock(b);
    size.setlock(b);
}

osc2ac_t::osc2ac_t(algo_comm_t iac,const char* chain, const char* algo)
    : MHAPlugin::plugin_t<int>("Receive OSC messages and convert them to AC variables.\n"
                               "Only data type float can be received."
                               ,iac),
      host("multicast adress (empty for unicast)",""),
      port("server port","7777"),
      vars("List of AC variables to provide as receivers of OSC messages.\n"
           "Each AC variable will mirror the latest received OSC message with address\n"
           "/NAME\n"
           "where each NAME is the name of the mirroring AC variable as given here.  For\n"
           "more details, please refer to the detailed description subsection in the\n"
           "manual.","[]"),
      size("Number of floats to receive with the AC variables from OSC.\n"
           "Each entry here corresponds to the entry in vars with the same index\n"
           "and determines the length of the float vector that will be allocated\n"
           "to receive the OSC messages with the corresponding address.","[1]","[1,]"),
      srv(nullptr)
{
    insert_member(host);
    insert_member(port);
    insert_member(vars);
    insert_member(size);
}

void osc2ac_t::prepare(mhaconfig_t& cf)
{
    setlock(true);
    try{
        srv = std::make_unique<osc_server_t>(host.data,port.data);
        while( vars.data.size() > size.data.size() )
            size.data.push_back(1);
        for(unsigned int k=0;k<vars.data.size();k++)
            srv->insert_variable(vars.data[k],size.data[k],ac);
        srv->server_start();
        srv->ac_insert();
    }
    catch(...){
        srv.reset(nullptr);
        setlock(false);
        throw;
    }
}

void osc2ac_t::release()
{
    srv.reset(nullptr);
    setlock(false);
}

void osc2ac_t::process()
{
    srv->sync_osc2ac();
}

MHAPLUGIN_CALLBACKS(osc2ac,osc2ac_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(osc2ac,osc2ac_t,spec,spec)
MHAPLUGIN_DOCUMENTATION\
(osc2ac,
 "data-import network-communication open-sound-control",
 "Receive open sound control (OSC) messages and mirror their data "
 "in AC variables. "
 "This plugin can receive OSC messages provided that they contain only "
 "numbers (scalars or vectors). "
 "\n\n"
 "The configuration variable \\texttt{vars} can be used to control which "
 " OSC messages to receive, and to define the names of the AC variables "
 " in which the received data is mirrored inside the \\mha{}, e.g.:"
 "\\begin{verbatim}\n"
 "# Example 1:\n"
 "osc2ac.vars = [spatial/source/0/sphericalCoords spatial/setParam/headRadius]\n"
 "osc2ac.size = [3 1]\n"
 "\\end{verbatim}\n"
 "Example 1 shows an {\\em osc2ac} configuration that receives OSC messages "
 " with addresses \\texttt{/spatial/source/0/sphericalCoords} and "
 " \\texttt{/spatial/setParam/headRadius}, the first of which is expected "
 " to contain 3  floating point values (a vector), "
 " while messages with the latter address  are expected to contain only 1 "
 " floating point value (a scalar) in each received message.\n"
 " The data received at these addresses will be mirrored in the AC variables "
 " \\texttt{spatial/source/0/sphericalCoords} and "
 " \\texttt{spatial/setParam/headRadius}, respectively. "
 " Note that the \\mha{} configuration and the AC variable name do not contain "
 " the leading slash \\texttt{/}. "
 " This leading slash is prepended by the {\\em osc2ac} "
 " plugin when constructing the OSC address for which messages are received "
 " from the \\texttt{vars} entry."
 "\n\n"
 "It is also possible to give AC variable name and OSC address separately by "
 " separating them with a colon, e.g.:"
 " colon delimiter, e.g.:"
 "\\begin{verbatim}\n"
 "# Example 2:\n"
 "vars = [level:/mhalevels]\n"
 "\\end{verbatim}\n"
 "In example 2, data received in OSC messages with address "
 " \\texttt{/mhalevels} is mirrored in the AC variable  \\texttt{level}. "
 " When \\texttt{size} is not set in this example, the default value 1 "
 " for scalars is used for all AC variables and OSC messages.\n"
 )


/*
 * Local variables:
 * c-basic-offset: 4
 * compile-command: "make"
 * indent-tabs-mode: nil
 * End:
 */
