// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2014 2015 2017 HörTech gGmbH
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
// Also observe the algorithm copyright statement below.
#include "mha_plugin.hh"
/*

  Copyright (c) 2011
  Author: Timo Gerkmann and Richard Hendriks
  Royal Institute of Technology (KTH)
  Delft university of Technology

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

  * Redistributions of source code must retain the above copyright
    notice and this list of conditions.

  * Redistributions in binary form must reproduce the above copyright
    notice and this list of conditions in the documentation and/or
    other materials provided with the distribution


 */

namespace noisePowProposedScale {

class noisePowProposed {
public:
    noisePowProposed(const mhaconfig_t& cf,algo_comm_t ac,const std::string& name,float alphaPH1mean,float alphaPSD,float q,float xiOptDb);
    void process(mha_spec_t* noisyDftFrame);
    void insert(){
        noisePow.insert();
        inputPow.insert();
        snrPost1Debug.insert();
        GLRDebug.insert();
        PH1Debug.insert();
        estimateDebug.insert();
        inputSpec.insert();
    }
private:
    MHASignal::waveform_t noisyPer;
    MHASignal::waveform_t PH1mean;
    MHA_AC::waveform_t noisePow;

    //gkc debug:
    MHA_AC::waveform_t inputPow;
    MHA_AC::waveform_t snrPost1Debug;
    MHA_AC::waveform_t GLRDebug;
    MHA_AC::waveform_t PH1Debug;
    MHA_AC::waveform_t estimateDebug;
    MHA_AC::spectrum_t inputSpec;

    // parameters:
    float alphaPH1mean_;
    float alphaPSD_;
    // a priori probability of speech presence:
    float priorFact;
    float xiOpt;
    // optimal fixed a priori SNR for SPP estimation:
    float logGLRFact;
    float GLRexp;
    //count frames for initial estimate
    int frameno;
};

noisePowProposed::noisePowProposed(const mhaconfig_t& cf,
				   algo_comm_t ac,
				   const std::string& name,
				   float alphaPH1mean,
				   float alphaPSD,
				   float q,
				   float xiOptDb)
    : noisyPer(cf.fftlen/2+1,cf.channels),
      PH1mean(cf.fftlen/2+1,cf.channels),
      noisePow(ac,name,cf.fftlen/2+1,cf.channels,false),
      inputPow(ac,"inputPow",cf.fftlen/2+1,cf.channels,false),
      snrPost1Debug(ac,"snrPost1Debug",cf.fftlen/2+1,cf.channels,false),
      GLRDebug(ac,"GLRDebug",cf.fftlen/2+1,cf.channels,false),
      PH1Debug(ac,"PH1Debug",cf.fftlen/2+1,cf.channels,false),
      estimateDebug(ac,"estimateDebug",cf.fftlen/2+1,cf.channels,false),
      inputSpec(ac,"inputSpec",cf.fftlen/2+1,cf.channels,false),
      alphaPH1mean_(alphaPH1mean),
      alphaPSD_(alphaPSD),
      priorFact(q/(1.0f-q)),
      xiOpt(powf(10.0f,(xiOptDb / 10.0f))),
      logGLRFact(log(1.0f/(1.0f+xiOpt))),
      GLRexp(xiOpt/(1.0f+xiOpt)),
      frameno(0)
{
    PH1mean = 0.5;
    // this is different to original "noisePowProposed"
    // (estimation during the first five frames instead):
    noisePow.assign(0.5);
    // better solution could be: LTASS noise at x dB
}

#define POWSPEC_FACTOR 0.0025

void noisePowProposed::process(mha_spec_t* noisyDftFrame)
{
    insert();

    inputSpec.copy(*noisyDftFrame);
    noisyPer.powspec(*noisyDftFrame);

    for( unsigned int k=0;k<size(noisyPer);k++) {
        noisyPer[k] = noisyPer[k] * ( 5.2429e+007 );
    }

    if ( frameno < 5 ) {

        for( unsigned int k=0;k<size(noisyPer);k++){

            noisePow[k] += noisyPer[k] / 5.0;
        }
    }

    // noise power estimation
    for( unsigned int k=0;k<size(noisyPer);k++){

        // a posteriori SNR based on old noise power estimate:
        double snrPost1 = noisyPer[k] / noisePow[k];
        double GLR = priorFact * exp(std::min(logGLRFact + GLRexp*snrPost1,200.0));
        // a posteriori speech presence probability:
        double PH1 = GLR/(1.0f+GLR);
        PH1mean[k]  = alphaPH1mean_ * PH1mean[k] + (1.0f-alphaPH1mean_) * PH1;
        if( PH1mean[k] > 0.99 )
            PH1 = std::min(PH1,0.99);
        double estimate =  PH1 * noisePow[k] + (1.0f-PH1) * noisyPer[k];
        noisePow[k] = alphaPSD_ * noisePow[k] + (1.0f-alphaPSD_)*estimate;

        //gkc: just saving the spectrum to compare inputs
        inputPow[k] = noisyPer[k];
        snrPost1Debug[k] = snrPost1;
        GLRDebug[k] = GLR;
        PH1Debug[k] = PH1;
        estimateDebug[k] = estimate;
    }

    frameno++;
}

class interface_t : public MHAPlugin::plugin_t<noisePowProposed> {
public:
    interface_t(const algo_comm_t&,const std::string&,const std::string&);
    mha_spec_t* process(mha_spec_t*);
    void prepare(mhaconfig_t&);
private:
    void update_cfg();
    /* integer variable of MHA-parser: */
    MHAParser::float_t alphaPH1mean;
    MHAParser::float_t alphaPSD;
    MHAParser::float_t q;
    MHAParser::float_t xiOptDb;
    std::string name;
    MHAEvents::patchbay_t<interface_t> patchbay;
};

interface_t::interface_t(const algo_comm_t & iac, const std::string &, const std::string & iname)
    : MHAPlugin::plugin_t<noisePowProposed>("noise power estimator after Gerkmann (2012)\n\n"
                                            " Copyright (c) 2011\n"
                                            " Author: Timo Gerkmann and Richard Hendriks\n"
                                            " Royal Institute of Technology (KTH)\n"
                                            " Delft university of Technology\n",iac),
      alphaPH1mean("low pass filter coefficient for PH1mean","0.9","[0,1["),
      alphaPSD("low pass filter coefficient for PSD","0.8","[0,1["),
      q("a priori probability of speech presence","0.5","[0,1]"),
      xiOptDb("optimal fixed a priori SNR for SPP estimation","15"),
      name(iname)
{
    insert_member(alphaPH1mean);
    insert_member(alphaPSD);
    insert_member(q);
    insert_member(xiOptDb);
    patchbay.connect(&writeaccess,this,&interface_t::update_cfg);
}

void interface_t::update_cfg()
{
    if( is_prepared() )
        push_config(new noisePowProposed(input_cfg(),ac,name,alphaPH1mean.data,alphaPSD.data,q.data,xiOptDb.data));
}

mha_spec_t* interface_t::process(mha_spec_t* s)
{
    poll_config()->process(s);
    return s;
}

void interface_t::prepare(mhaconfig_t& cf)
{
    if( cf.domain != MHA_SPECTRUM )
	throw MHA_Error(__FILE__,__LINE__,"Only spectral processing is supported");
    update_cfg();
    poll_config()->insert();
}

}

MHAPLUGIN_CALLBACKS(noisePowProposedScale,noisePowProposedScale::interface_t,spec,spec)
