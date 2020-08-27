// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2009 2010 2013 2014 2015 2018 2019 2020 HörTech gGmbH
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

#include "altconfig.hh"
#include "mha_utils.hh"

altconfig_t::altconfig_t(algo_comm_t iac,const char* ,const char* )
    : MHAPlugin::plugin_t<char>("Alternative configurations for a plugin",iac),
      MHAParser::mhapluginloader_t(*this,iac),
      parser_algos("List of names for plugin configurations. Assigning names to algos creates"
                   " configuration variables with the given names which can be used to store"
                   " configuration commands for the loaded plugin.","[]"),
      select_plug("Select a configuration for parsing. Assigning one of the names to select"
                  " causes execution of the configuration command stored under that name in the"
                  " context of the loaded plugin. ","(none)","[(none)]"),
      selectall("Iterate through all configuration options (for validation)","no")
{
    set_node_id( "altconfig" );
    insert_item("algos",&parser_algos);
    insert_item("select",&select_plug);
    insert_member(selectall);
    patchbay.connect(&parser_algos.writeaccess,this,&altconfig_t::on_set_algos);
    patchbay.connect(&select_plug.writeaccess,this,&altconfig_t::on_set_select);
    patchbay.connect(&selectall.writeaccess,this,&altconfig_t::event_select_all);
}

void altconfig_t::on_set_select()
{
    if(configs.count(select_plug.data.get_value())==0)
        throw MHA_Error(__FILE__,__LINE__,"Bug: Config string not found in map.");

    if(plug && plug->has_parser() &&
       configs.at(select_plug.data.get_value()).data.size()){
        plug->parse(configs.at(select_plug.data.get_value()).data);
    }
}

void altconfig_t::event_select_all()
{
    if( selectall.data )
        for(const auto & it : configs){
            auto cfg_string=it.second;
            if( plug && plug->has_parser() && cfg_string.data.size() )
                plug->parse(cfg_string.data);
        }
}

std::map<std::string,MHAParser::string_t> altconfig_t::save_state(){
    std::map<std::string,MHAParser::string_t> res;
    for(auto & elm : configs){
        res.insert({elm.first,MHAParser::string_t("configuration_command",elm.second.data)});
    }
    return res;
}

void altconfig_t::restore_state(std::map<std::string,MHAParser::string_t>& state,std::map<std::string,MHAParser::string_t>& failed_state){
    for(auto & old_entry : failed_state  ){
        if(has_entry(old_entry.first) && !state.count(old_entry.first))
            force_remove_item(old_entry.first);
    }
    for(auto & it : state){
        if(!has_entry(it.first)){
            configs.insert({it.first,MHAParser::string_t("configuration command",it.second.data)});
            insert_item(it.first,&configs.at(it.first));
        }
    }
}

void altconfig_t::on_set_algos()
{
    std::map<std::string,MHAParser::string_t> old_configs=save_state();
    // try-catch because error-handling is more involved, see above
    try{
        auto is_in_algos=[&](auto cfg_name){return std::find(std::begin(parser_algos.data),
                                                             std::end(parser_algos.data),
                                                             cfg_name)!=parser_algos.data.end();};
        // Remove all string_t's from the config map that are no longer in parser_algos.
        // Important for us: erase does not not invalidate iterators and references
        for(auto it = configs.begin(); it != configs.end();){
            if(!is_in_algos(it->first)){
                force_remove_item(it->first);
                it=configs.erase(it);
            }
            else
                ++it;
        }
        // For every element of parser_algos:
        // Check if a child parser with that name already exists.
        //If not, it is always okay to create and insert_item the string_t
        for(auto& algo : parser_algos.data){
            //Check if algo needs to be inserted...
            if(!has_entry(algo)){
                // No child parser of that name already exists, inserting is allowed
                configs.insert({algo, MHAParser::string_t("configuration command",algo.c_str() /* <-Use name as
                                                                                                  initial value for
                                                                                                  easy debugging, should never
                                                                                                  be used.*/)});
                insert_item(algo,&((*configs.find(algo)).second));
            }
            else {
                // A child parser with that name already exists, do nothing if it is a user-defined configuration command,
                // otherwise complain as we would overwrite unrelated parsers.
                if(old_configs.count(algo)==0)
                    throw MHA_Error(__FILE__,__LINE__,"%s is a reserved word in this context and may not be used as"
                                    " a configuration name",algo.c_str());
            }
        }
        select_plug.data.set_entries(MHAParser::StrCnv::val2str(parser_algos.data));
    }
    //This clause catches all MHA_Errors that may be thrown in force_remove_item() and insert_item() or during the loop itself in line 112
    //Possible fail conditions are: Bad parser name( insert_item() ), usage of reserved words (line 112)
    catch(MHA_Error & e){
        auto failed_configs=configs;
        configs=old_configs;
        restore_state(configs,failed_configs);
        // Re-throw for further error-handling
        throw;
    }
}

MHAPLUGIN_CALLBACKS(altconfig,altconfig_t,wave,wave)
MHAPLUGIN_PROC_CALLBACK(altconfig,altconfig_t,spec,spec)
MHAPLUGIN_PROC_CALLBACK(altconfig,altconfig_t,spec,wave)
MHAPLUGIN_PROC_CALLBACK(altconfig,altconfig_t,wave,spec)
MHAPLUGIN_DOCUMENTATION(altconfig,
                        "plugin-arrangement data-flow",
                        "This plugin loads another MHA plugin for signal processing when user assigns configuration"
                        " variable \"plugin\\_name\" and allows sending stored configuration commands to the loaded plugin"
                        " at run time by selecting one of the pre-configured alternative configuration commands."
                        " Users can create names for an arbitrary number of alternative configurations by assigning a"
                        " list of names to configuration variable \"algos\". New configuration variables with these "
                        "names are then created by the plugin. Users can then store arbitrary configuration commands"
                        " in these new configuration variables. Execution of these configuration commands can later"
                        " be triggered by selecting them, i.e. assigning their name to configuration variable \"select\"."
                        " The selected configuration command will be executed in the context of the loaded plugin."
                        " Empty configuration commands will be ignored."
                        )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
