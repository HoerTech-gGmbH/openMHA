#ifndef %ProjectName:u%_H
#define %ProjectName:u%_H

#include "mha_plugin.hh"

//runtime config
class %ProjectName%_config%TypeStr% {

public:
    %ProjectName%_config%TypeStr%(algo_comm_t &ac, const mhaconfig_t in_cfg);
    ~%ProjectName%_config%TypeStr%();

    mha_%OutputType%_t* process(mha_%InputType%_t*);

    //declare data necessary for processing state here

@if "%InputType%" != "%OutputType%"
    //declare a default output variable
@if "%OutputType%" == "wave"
    MHASignal::waveform_t %OutputType%;
@elsif "%OutputType%" == "spec"
    MHASignal::spectrum_t %OutputType%;
@endif
@endif
};

class %ProjectName%%TypeStr% : public MHAPlugin::plugin_t<%ProjectName%_config%TypeStr%> {

public:
    %ProjectName%%TypeStr%(algo_comm_t & ac,const std::string & chain_name,
               const std::string & algo_name);
    ~%ProjectName%%TypeStr%();
    mha_%OutputType%_t* process(mha_%InputType%_t*);
    void prepare(mhaconfig_t&);
    void release(void) {/* Do nothing in release */}

private:
    void update_cfg();

    /* patch bay for connecting configuration parser
       events with local member functions: */
    MHAEvents::patchbay_t<%ProjectName%%TypeStr%> patchbay;

};

#endif // %ProjectName:u%_H
