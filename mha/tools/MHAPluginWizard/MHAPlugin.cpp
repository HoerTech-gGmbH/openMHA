/*
 * %PluginDocumentSource%
 */

#include "%ProjectName%.h"

#define PATCH_VAR(var) patchbay.connect(&var.valuechanged, this, &%ProjectName%%TypeStr%::update_cfg)
#define INSERT_PATCH(var) insert_member(var); PATCH_VAR(var)

%ProjectName%_config%TypeStr%::%ProjectName%_config%TypeStr%(algo_comm_t &ac, const mhaconfig_t in_cfg)
@if "%InputType%" != "%OutputType%"
  : %OutputType%(in_cfg.fragsize, in_cfg.channels)
@endif
{
    //initialize plugin state for a new configuration
}

%ProjectName%_config%TypeStr%::~%ProjectName%_config%TypeStr%() {}

//the actual processing implementation
mha_%OutputType%_t *%ProjectName%_config%TypeStr%::process(mha_%InputType%_t *%InputType%)
{
    //do actual processing here using configuration state

@if "%InputType%" == "%OutputType%"
    //return current fragment
    return %InputType%;
@else
    //return default output
    return &%OutputType%;
@endif
}

/** Constructs our plugin. */
%ProjectName%%TypeStr%::%ProjectName%%TypeStr%(algo_comm_t & ac,
                 const std::string & chain_name,
                 const std::string & algo_name)
    : MHAPlugin::plugin_t<%ProjectName%_config%TypeStr%>("%PluginNameDesc%",ac)

{
    //add parser variables and connect them to methods here
    //INSERT_PATCH(foo_parser);
}

%ProjectName%%TypeStr%::~%ProjectName%%TypeStr%() {}

/** Plugin preparation.
 *  An opportunity to validate configuration parameters before instantiating a configuration.
 * @param signal_info
 *   Structure containing a description of the form of the signal (domain,
 *   number of channels, frames per block, sampling rate.
 */
void %ProjectName%%TypeStr%::prepare(mhaconfig_t & signal_info)
{
    //good idea: restrict input type and dimension
    /*
    if (signal_info.channels != 2)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin must have 2 input channels: (%d found)\n"
                        "[Left, Right].", signal_info.channels);

    if (signal_info.domain != MHA_SPECTRUM)
        throw MHA_Error(__FILE__, __LINE__,
                        "This plugin can only process spectrum signals.");
                        */

    /* make sure that a valid runtime configuration exists: */
    update_cfg();
}

void %ProjectName%%TypeStr%::update_cfg()
{
    if ( is_prepared() ) {

        //when necessary, make a new configuration instance
        //possibly based on changes in parser variables
        %ProjectName%_config%TypeStr% *config;
        config = new %ProjectName%_config%TypeStr%( ac, input_cfg() );
        push_config( config );
    }
}

/**
 * Checks for the most recent configuration and defers processing to it.
 */
mha_%OutputType%_t * %ProjectName%%TypeStr%::process(mha_%InputType%_t * signal)
{
    //this stub method defers processing to the configuration class
    return poll_config()->process( signal );
}

/*
 * This macro connects the plugin1_t class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the 
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(%ProjectName%,%ProjectName%%TypeStr%,
                    %InputType%,%OutputType%)

/*
 * This macro creates code classification of the plugin and for
 * automatic documentation.
 *
 * The first argument to the macro is a space separated list of
 * categories, starting with the most relevant category. The second
 * argument is a LaTeX-compatible character array with some detailed
 * documentation of the plugin.
 */
MHAPLUGIN_DOCUMENTATION(%ProjectName%%TypeStr%,"%PluginCategory%",
    "%PluginDocument%"
    )
