// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2009 2010 2012 2013 2014 2015 2018 2020 HörTech gGmbH
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

#include "mha_plugin.hh"
#include "complex_filter.h"
#include "mha_fftfb.hh"
#include "speechnoise.h"
#include "mhapluginloader.h"

/** Runtime configuration class of gtfb_simple_bridge plugin */
class gtfb_simple_rt_t {
public:
    /* Constructor of runtime configuration.  Allocates all needed
     * signal storage containers and AC variables.
     * @param ac
     *   Algorithm Communication Variable space.  The imaginary part of the
     *   filtered signal is stored in a waveform container in AC space.
     *   The new object is the owner of the memory and needs to register it.
     * @param name
     *   Name of the AC variable where the imaginary part of the filtered
     *   signal is stored.
     * @param chcfg
     *   Input signal dimensions  
     * @param cf
     *   Center frequencies of gammatone filter bands in Hz
     * @param bw
     *   Equivalent rectangular bandwidths of gammatone filter bands in Hz
     * @param order
     *   Gammatone filter order
     * @param pre_stages
     *   Number of gammatone filter orders applied before the signal is
     *   processed by the loaded plugin.  The remaining filter-orders
     *   are applied after the loaded plugin has modified the filtered signal.
     * @param desired_delay
     *   Desired group delay of complete analysis-synthesis processing, in
     *   audio samples.
     * @param vcltass
     *   Vector of band-specific level correction values for the long-term
     *   average speech spectrum, in dB per band.  Published as AC variable.
     * @param resynthesis_gain
     *   Linear factors to apply to the signal in the different bands before
     *   resynthesis
     * @param element_gain_name
     *   Either an empty string, or the name of an AC variable from which 
     *   element-wise linear factors are read.
     */
    gtfb_simple_rt_t(algo_comm_t ac,
                     const std::string& name,
                     mhaconfig_t& chcfg,
                     std::vector<mha_real_t> cf,
                     std::vector<mha_real_t> bw,
                     unsigned int order,
                     unsigned int pre_stages,
                     unsigned int desired_delay,
                     std::vector<mha_real_t>& vcltass,
                     std::vector<mha_real_t>& resynthesis_gain,
                     const std::string& element_gain_name);

    /** Filter real input signal s with the pre_stages filter orders in each
     * gammatone filter band.  The real part of the complex output is returned
     * in the return value of the method, the imaginary part is stored into
     * the AC variable.
     * @param s real-valued, broad-band input signal
     * @return  real part of complex-valued output signal after pre_stages
     *          gammatone filter orders have been applied in each band.
     *          Order of output bands in real and imaginary output are:
     *          (channel0,band0), (channel0,band1), ..., (cannel1,band0), ... */
    mha_wave_t* pre_plugin(mha_wave_t* s);

    /** Post-filter the complex-valued filter-bank signal s after it has been
     * processed by the loaded plugin.  The remaining gammatone filter orders
     * are applied to restrict the loaded plugin's output signal to the 
     * respective bands.  After
     * @param s complex-valued, filter-bank signal.  This signal is produced 
     *          by letting the loaded plugin process the output signal of the
     *          pre_plugin method.
     * @return  real part of complex-valued output signal after pre_stages
     *          gammatone filter orders have been applied in each band.
     *          Order of output bands in real and imaginary output are:
     *          (channel0,band0), (channel0,band1), ..., (cannel1,band0), ... */
    mha_wave_t* post_plugin(mha_wave_t* s);

    /** Const-accessor to contained gammatone filterbank object. */
    const MHAFilter::gamma_flt_t & get_gf() const {return gf;}
private:
    /** Helper function to repeat the elements in a vector.
     * @param src       vector to repeat
     * @param nchannels number of times to repeat input vector
     * @return a vector that returns nchannels repetitions of input vector. */
    static std::vector<mha_real_t>
    duplicate_vector(const std::vector<mha_real_t>& src,unsigned int nchannels);

    /** Total number of gammatone filter orders */
    unsigned int _order;

    /** Number of filter orders applied before the loaded plugin is invoked */
    unsigned int _pre_stages;

    /** Number of frequency bands to produce = number of gammatone filters */
    unsigned int nbands;

    /** Storage for the imaginary part of the filterbank signal.  It is used
     * as the imaginary input signal for the loaded plugin.  Furthermore, it
     * is expected that the loaded plugin processes the imaginary part of
     * the data in place. */
    MHA_AC::waveform_t imag;

    /** AC variable to publish the center frequencies of the gammatone filters*/
    MHA_AC::waveform_t accf;

    /** AC variable to publish the bandwiths of the gammatone filters */
    MHA_AC::waveform_t acbw;

    /** Real part of the filterbank signal.  It is used as the real input signal
     * to the loaded plugin. */
    MHASignal::waveform_t input;

    /** Resynthesized broadband signal, used as the output signal of this plugin
     */
    MHASignal::waveform_t output;

    /** The gammatone filter bank implementation. */
    MHAFilter::gamma_flt_t gf;

    /** AC variable to publish band-specific LTASS level correction values */
    MHA_AC::waveform_t cLTASS;

    /** AC variable to publish the configured per-frequency resyntesis gains */
    MHA_AC::waveform_t ac_resynthesis_gain;

    /** Either an empty string, or the name of an AC variable from which 
     *   element-wise linear factors are read. */
    std::string element_gain_name_;

    /** Algorithm Communication Variable space. */
    algo_comm_t _ac;
};

std::vector<mha_real_t> gtfb_simple_rt_t::duplicate_vector(const std::vector<mha_real_t>& src,unsigned int nchannels)
{
    std::vector<mha_real_t> ret(src);
    for(unsigned int ch=1;ch<nchannels;ch++)
        ret.insert(ret.end(),src.begin(),src.end());
    return ret;
}

gtfb_simple_rt_t::gtfb_simple_rt_t(algo_comm_t ac,
                                   const std::string& name,
                                   mhaconfig_t& chcfg,
                                   std::vector<mha_real_t> cf,
                                   std::vector<mha_real_t> bw,
                                   unsigned int order,
                                   unsigned int pre_stages,
                                   unsigned int desired_delay,
                                   std::vector<mha_real_t>& vcltass,
                                   std::vector<mha_real_t>& resynthesis_gain,
                                   const std::string& element_gain_name)
    : _order(order),
      _pre_stages(pre_stages),
      nbands(cf.size()),
      imag(ac,name+"_imag",chcfg.fragsize,chcfg.channels*nbands,true),
      accf(ac,name+"_cf",1,cf.size(),true),
      acbw(ac,name+"_bw",1,bw.size(),true),
      input(chcfg.fragsize,chcfg.channels*nbands),
      output(chcfg.fragsize,chcfg.channels),
      gf(duplicate_vector(cf,chcfg.channels),
         duplicate_vector(bw,chcfg.channels),chcfg.srate,order),
      cLTASS(ac,name+"_cLTASS",1,chcfg.channels*nbands,true),
      ac_resynthesis_gain(ac,name+"_resyngain",1,chcfg.channels*nbands,true),
      element_gain_name_(element_gain_name),
      _ac(ac)
{
    accf.copy(cf);
    acbw.copy(bw);
    gf.phase_correction(desired_delay,chcfg.channels);
    chcfg.channels *= nbands;
    speechnoise_t ltass(1.0f,
                        chcfg.srate,
                        imag.num_channels,
                        speechnoise_t::LTASS_combined);
    ltass *= 2e-5/MHASignal::rmslevel(ltass,0);
    MHASignal::waveform_t ltass_real(ltass.num_frames,ltass.num_channels);
    MHASignal::waveform_t ltass_imag(ltass.num_frames,ltass.num_channels);

    gf(ltass,ltass_real,ltass_imag);
    gf(ltass,ltass_real,ltass_imag);
    gf.reset_state();
    vcltass.clear();
    for(unsigned int ch=0;ch<cLTASS.num_channels;ch++){
        cLTASS.buf[ch] =
            20*log10(std::max(1e-10,MHASignal::rmslevel(ltass_real,ch)/2e-5));
        vcltass.push_back(cLTASS.buf[ch]);
    }

    if( pre_stages < order ){
        float corr = pow(2.0,(order-pre_stages)/(float)(order));
        std::vector<mha_complex_t> w;
        w = gf.get_weights(0);
        for(unsigned int k=0;k<w.size();k++)
            w[k] *= corr;
        gf.set_weights(0,w);
        w = gf.get_weights(order-1);
        for(unsigned int k=0;k<w.size();k++)
            w[k] /= corr;
        gf.set_weights(order-1,w);
    }
    resynthesis_gain = gf.get_resynthesis_gain();
    ac_resynthesis_gain.copy(resynthesis_gain);
}

mha_wave_t* gtfb_simple_rt_t::pre_plugin(mha_wave_t* s)
{
    for(unsigned int k=0;k<s->num_frames;k++)
        for(unsigned int ch=0;ch<s->num_channels;ch++)
            for(unsigned int kband=0;kband<nbands;kband++)
                input.value(k,ch*nbands+kband) = value(s,k,ch);
    for(unsigned int stage=0;stage<_pre_stages;stage++)
        gf(input,imag,stage);
    return &input;
}


mha_wave_t* gtfb_simple_rt_t::post_plugin(mha_wave_t* s)
{
    MHA_assert_equal(output.num_frames,s->num_frames);
    MHA_assert_equal(output.num_channels*nbands,s->num_channels);
    if( element_gain_name_.size() ){
        mha_wave_t elem_gain = MHA_AC::get_var_waveform(_ac,element_gain_name_);
        *s *= elem_gain;
        imag *= elem_gain;
    }
    for(unsigned int stage=_pre_stages;stage<_order;stage++)
        gf(*s,imag,stage);
    clear(output);
    for(unsigned int k=0;k<output.num_frames;k++)
        for(unsigned int ch=0;ch<output.num_channels;ch++)
            for(unsigned int kband=0;kband<nbands;kband++)
                output.value(k,ch) += value(s,k,ch*nbands+kband) *
                    ac_resynthesis_gain.buf[ch*nbands+kband];
    return &output;
}


// *****************************************************************
//
// Interface
//
// *****************************************************************

/** Interface class of gtfb_simple_bridge plugin */
class gtfb_simple_t : public MHAPlugin::plugin_t<gtfb_simple_rt_t> {
public:
    /** Constructor.  Registers parser variables.
     * @param ac    Algorithm Communication Variable space
     * @param chain chain name
     * @param algo  configured name of this plugin instance */
    gtfb_simple_t(algo_comm_t ac,
                  const std::string& chain,
                  const std::string& algo);

    /** Prepare contained plugin for signal processing.  Allocates the runtime
     * configuration instance.  Locks all variables. */
    void prepare(mhaconfig_t& chcfg);

    /** Releases contained plugin.  Unlocks all variables. */
    void release();

    /** Process the input signal:  Computes the filterbank with the pre-stages,
     * calls the loaded plugin for processing of the filterbank signal,
     * and resynthesizes the result to a modified broadband signal. */
    mha_wave_t* process(mha_wave_t* sIn);

    /** Locks / unlocks all configuration variables before / after signal
     * processing */
    void setlock(bool b) {
        order.setlock(b);
        prestages.setlock(b);
        desired_delay.setlock(b);
        element_gain_name.setlock(b);
    }
private:
    /** Handle to the loaded plugin. */
    MHAParser::mhapluginloader_t plug;

    /** Object determines the filterbank frequencies. */
    MHAOvlFilter::fscale_bw_t fscale;

    /** total order of gammatone filter */ 
    MHAParser::int_t order;

    /** Number of gammatone filter order to process before the loaded plugin
     * processes the filterbank signal. */
    MHAParser::int_t prestages;

    /** Desired group delay in audio samples */
    MHAParser::int_t desired_delay;

    /** Number of AC variable to take element-wise gain factors from for 
     * resynthesis. */
    MHAParser::string_t element_gain_name;

    /** Monitoring of LTASS correction values / dB */
    MHAParser::vfloat_mon_t cLTASS;

    /** configured frequency-specific resynthesis gains */
    MHAParser::vfloat_mon_t resynthesis_gain;

    /** For tests and debugging:
     * a serialization of the gammatone filter internals */
    MHAParser::string_mon_t gf_internals;

    /** Configured algorithm name, used to name the AC variables. */
    std::string name_;
};

gtfb_simple_t::gtfb_simple_t(algo_comm_t ac,const std::string& chain,const std::string& algo)
    : MHAPlugin::plugin_t<gtfb_simple_rt_t>("Simple gammatone filterbank",ac),
      plug(*this,ac),
      fscale(*this),
      order("Filterbank order","4","[1,]"),
      prestages("Number of stages to be processed before the plugin","3","[0,]"),
      desired_delay("Desired delay in samples","0","[0,]"),
      element_gain_name("Name of element wise gain AC variable (looked up during waveform process, can be empty)",""),
      cLTASS
      ("Vector of band-specific negative LTASS level correction values in dB:\n"
       "Of a broadband LTASS signal with level X dB, X+cLTASS dB will fall\n"
       "into each band of the gammatone filterbank"),
      resynthesis_gain("Linear gains for resynthesis."),
      gf_internals("internal coefficients of the gammatone filterbank"),
      name_(algo)
{
    insert_member(order);
    insert_member(prestages);
    insert_member(desired_delay);
    insert_member(element_gain_name);
    insert_member(cLTASS);
    insert_member(resynthesis_gain);
    insert_member(gf_internals);
}

void gtfb_simple_t::prepare(mhaconfig_t& chcfg)
{
    if( chcfg.domain != MHA_WAVEFORM )
        throw MHA_Error(__FILE__,__LINE__,
                        "Only waveform domain input is supported.");
    gtfb_simple_rt_t * next_cfg =
        new gtfb_simple_rt_t(ac,name_,chcfg,fscale.get_f_hz(),
                             fscale.get_bw_hz(),order.data,prestages.data,
                             desired_delay.data,cLTASS.data,
                             resynthesis_gain.data,element_gain_name.data);
    push_config(next_cfg);
    gf_internals.data = next_cfg->get_gf().inspect();
    mhaconfig_t chcfg_expected(chcfg);
    plug.prepare(chcfg);
    PluginLoader::mhaconfig_compare(chcfg_expected,chcfg);
    chcfg = input_cfg();
    setlock(true);
}

void gtfb_simple_t::release()
{
    plug.release();
    setlock(false);
}

mha_wave_t* gtfb_simple_t::process(mha_wave_t* sIn)
{
    poll_config();
    sIn = cfg->pre_plugin(sIn);
    mha_wave_t* sOut;
    plug.process(sIn,&sOut);
    return cfg->post_plugin(sOut);
}

MHAPLUGIN_CALLBACKS(gtfb_simple_bridge,gtfb_simple_t,wave,wave)
MHAPLUGIN_DOCUMENTATION
(gtfb_simple_bridge,
 "filterbank",
 "Simple gammatone filterbank plugin.  Computes complex-valued gammatone"
 " filterbank signal from the real-valued broad-band signal, processes"
 " the filterbank signal with the plugin loaded via plugin\\_name, and"
 " resynthesizes the output signal again to a real-valued broadband output"
 " signal.\n\n"
 "The signal in each band can be restricted to the respective frequency"
 " band by applying additional gammatone filter stages to the output"
 " signal of the loaded plugin.\n\n"
 "Gammatone filterbank is implemented after Hohmann 2002 and produces"
 " complex-valued analytic output in each frequency band."
 " Frequency bands are presented as audio channels to the loaded plugin."
 " The order of bands is:  All bands created from the first input channel form"
 " the first nbands audio channel, followed by all bands created from the"
 " second input channel, etc.\n\n"
 "Real and imaginary signal are presented separately to the loaded plugin:"
 " The real part is transferred as the regular MHA audio input signal,"
 " while the imaginary part is transferred through an AC variable with the"
 " same name as the configured name of this filterbank plugin.\n\n"
 "This plugin does not support changing the configuration at run time.\n\n"
 "This plugin creates the following AC variables during preparation:\n"
 "\\begin{description}\n"
 "\\item[gtfb\\_simple\\_bridge\\_imag] waveform matrix, contains the imaginary"
 "                                      part of the filtered signal to be"
 "                                      processed by the loaded plugin"
 "                                      in-place. Size:"
 "                                      (fragsize  x channels*bands)\n"
 "\\item[gtfb\\_simple\\_bridge\\_cf] vector containing the center frequencies"
 "                                    of the gammatone filter bands in Hz."
 "                                    Size: (1 x bands)\n"
 "\\item[gtfb\\_simple\\_bridge\\_bw] vector containing the bandwidths"
 "                                    of the gammatone filter bands in Hz."
 "                                    Size: (1 x bands)\n"
 "\\item[gtfb\\_simple\\_bridge\\_cLTASS] vector containing negative LTASS"
 "                                        correction values in dB for the"
 "                                        gammatone filter bands."
 "                                        Size: (1 x channels*bands)\n"
 "\\item[gtfb\\_simple\\_bridge\\_resyncgain] vector containing the per-band"
 "                                            resynthesis gains computed by the"
 "                                            Hohmann 2002 method (linear"
 "                                            factors, applied during"
 "                                            resynthesis)."
 "                                            Size: (1 x channels*bands)\n"
 "\\end{description}\n"
 "If the plugin is assigned a different name than gtfb\\_simple\\_bridge,"
 " then the first parts of the above AC variable names change accordingly.\n\n"
 "This plugin can make use of an AC variables with the name given to"
 " configutation variable element\\_gain\\_name: It expects a real matrix"
 " with size (fragsize x channels*bands).  If this name is given, then"
 " the values given in this matrix are multiplied element-wise with the"
 " real and imaginary output signals of the loaded plugin before the filterbank"
 " resynthesis is performed.\n"
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
