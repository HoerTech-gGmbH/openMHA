// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2007 2009 2010 2011 2014 2015 2016 2017 2018 HörTech gGmbH
// Copyright © 2019 2020 HörTech gGmbH
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

#include "mha_parser.hh"
#include "mha_plugin.hh"
#include "mha_events.h"
#include "mha_signal.hh"
#include "mha_filter.hh"
#include "mhapluginloader.h"
#include "speechnoise.h"

/*
 * SOFTCLIP
 */

class softclipper_variables_t : public MHAParser::parser_t {
public:
    softclipper_variables_t();
    MHAParser::float_t tau_attack;
    MHAParser::float_t tau_decay;
    MHAParser::float_t tau_clip;
    MHAParser::float_t threshold;
    MHAParser::float_t hardlimit;
    MHAParser::float_t slope;
    MHAParser::bool_t linear;
    MHAParser::float_mon_t clipped;
    MHAParser::float_t max_clipped;
};

class softclipper_t {
public:
    softclipper_t(const softclipper_variables_t& v,const mhaconfig_t&);
    mha_real_t process(mha_wave_t*);
private:
    MHAFilter::o1flt_lowpass_t attack;
    MHAFilter::o1flt_maxtrack_t decay;
    MHAFilter::o1flt_lowpass_t clipmeter;
    mha_real_t threshold;
    mha_real_t hardlimit;
    mha_real_t slope;
    bool linear;
};

softclipper_variables_t::softclipper_variables_t()
    : MHAParser::parser_t("'Hardware' softclipper"),
      tau_attack("attack filter time constant / s","0.002","[0,]"),
      tau_decay("decay filter time constant / s","0.005","[0,]"),
      tau_clip("clipping meter time constant / s","1","[0,]"),
      threshold("start point on linear scale (hard clipping at 1.0)","0.6","[0,]"),
      hardlimit("hard limit","1","]0,]"),
      slope("compression factor","0.5","[0,1]"),
      linear("input/output function is linear on linear (yes) or logarithmic (no) scale","no"),
      clipped("clipped ratio"),
      max_clipped("maximum allowed clipped ratio", "1", "[0, 1]")
{
    insert_item("tau_attack",&tau_attack);
    insert_item("tau_decay",&tau_decay);
    insert_item("threshold",&threshold);
    insert_member(hardlimit);
    insert_item("slope",&slope);
    insert_item("linear",&linear);
    insert_item("tau_clip",&tau_clip);
    insert_item("clipped",&clipped);
    insert_item("max_clipped", &max_clipped);
}

softclipper_t::softclipper_t(const softclipper_variables_t& v,const mhaconfig_t& tf)
    : attack(std::vector<mha_real_t>(tf.channels,v.tau_attack.data),tf.srate),
      decay(std::vector<mha_real_t>(tf.channels,v.tau_decay.data),tf.srate),
      clipmeter(std::vector<mha_real_t>(1,v.tau_clip.data),tf.srate),
      threshold(v.threshold.data),
      hardlimit(v.hardlimit.data),
      slope(v.slope.data),
      linear(v.linear.data)
{
}

mha_real_t softclipper_t::process(mha_wave_t* s)
{
    mha_real_t clipped;
    mha_real_t rclipped = 0;
    unsigned int k, ch;
    mha_real_t val;
    for( k=0;k<s->num_frames;k++){
        clipped = 0;
        for( ch=0;ch<s->num_channels;ch++){
            val = decay( ch, attack( ch, fabsf(value(s,k,ch))));
            if( val > threshold ){
                if( linear )
                    value(s,k,ch)*=((val-threshold)*slope+threshold)/val;
                else
                    value(s,k,ch)*=pow(val/threshold,slope-1.0);
                clipped = 1;
            }
        }
        rclipped = clipmeter( 0, clipped );
    }
    MHASignal::limit(*s,-hardlimit,hardlimit);
    return rclipped;
}

/*
 * CALIBRATOR
 */

class calibrator_variables_t
{
public:
    calibrator_variables_t(bool is_input,MHAParser::parser_t& parent);
    MHAParser::vfloat_t peaklevel;
    MHAParser::mfloat_t fir;
    MHAParser::int_t nbits;
    MHAParser::float_t tau_level;
    MHAParser::kw_t spnoise_mode;
    MHAParser::vint_t spnoise_channels;
    MHAParser::float_t spnoise_level;
    MHAParser::vfloat_mon_t rmslevel;
    MHAParser::parser_t spnoise_parser;
    MHAParser::float_mon_t srate;
    MHAParser::int_mon_t fragsize;
    MHAParser::int_mon_t num_channels;
    MHAParser::parser_t config_parser;
    softclipper_variables_t softclip;
    MHAParser::bool_t do_clipping;
};

speechnoise_t::noise_type_t kw_index2type( unsigned int idx )
{
    switch( idx ){
    case 0 : // off, use default (ignored anyway)
        return speechnoise_t::mha;
    case 1 : // original MHA
        return speechnoise_t::mha;
    case 2 : // olnoise
        return speechnoise_t::olnoise;
    case 3 : // LTASS_combined
        return speechnoise_t::LTASS_combined;
    case 4 : // LTASS_female
        return speechnoise_t::LTASS_female;
    case 5 : // LTASS_male
        return speechnoise_t::LTASS_male;
    case 6 : // white
        return speechnoise_t::white;
    case 7 : // pink
        return speechnoise_t::pink;
    case 8 : // brown
        return speechnoise_t::brown;
    case 9 : // TEN_SPL
        return speechnoise_t::TEN_SPL;
    case 10 : // TEN_SPL_250_8k
        return speechnoise_t::TEN_SPL_250_8k;
    case 11 : // TEN_SPL_50_16k
        return speechnoise_t::TEN_SPL_50_16k;
    case 12 : // sin125
        return speechnoise_t::sin125;
    case 13 : // sin250
        return speechnoise_t::sin250;
    case 14 : // sin500
        return speechnoise_t::sin500;
    case 15 : // sin1k
        return speechnoise_t::sin1k;
    case 16 : // sin2k
        return speechnoise_t::sin2k;
    case 17 : // sin4k
        return speechnoise_t::sin4k;
    case 18 : // sin8k
        return speechnoise_t::sin8k;
    }
    return speechnoise_t::mha;
}

calibrator_variables_t::calibrator_variables_t(bool is_input,MHAParser::parser_t& parent)
    : peaklevel("Reference peak level in dB (0 dB FS corresponds to this SPL level)","[]"),
      fir("FIR filter coefficients, one row for each channel","[[]]"),
      nbits("Number of bits to simulate, or zero for limiting only","0","[0,32]"),
      tau_level("Time constant in seconds for RMS level meter","0.125","]0,10]"),
      spnoise_mode("Playback mode and level of speech shaped noise","off","[off on olnoise LTASS_combined LTASS_female LTASS_male white pink brown TEN_SPL TEN_SPL_250_8k TEN_SPL_50_16k sin125 sin250 sin500 sin1k sin2k sin4k sin8k]"),
      spnoise_channels("Channels where to playb speech noise signal","[]","[0,]"),
      spnoise_level("Test signal level in dB SPL","80","[0,120]"),
      rmslevel(is_input?"RMS level in dB at input (after calibration or addition of noise)":"RMS level in dB at output (before calibration or addition of noise)"),
      srate("Actual sampling rate / Hz"),
      fragsize("Actual fragment size / samples"),
      num_channels("Actual number of channels"),
      do_clipping("Will the soft/ hard clipping be executed", "no")
{
    spnoise_parser.insert_item("mode",&spnoise_mode);
    spnoise_parser.insert_item("level",&spnoise_level);
    spnoise_parser.insert_item("channels",&spnoise_channels);
    config_parser.insert_item("srate",&srate);
    config_parser.insert_item("fragsize",&fragsize);
    config_parser.insert_item("channels",&num_channels);
    if( is_input ){
        parent.insert_item("nbits",&nbits);
        parent.insert_item("fir",&fir);
        parent.insert_item("peaklevel",&peaklevel);
        parent.insert_item("speechnoise",&spnoise_parser);
    }else{
        parent.insert_item("speechnoise",&spnoise_parser);
        parent.insert_item("peaklevel",&peaklevel);
        parent.insert_item("fir",&fir);
        parent.insert_item("softclip",&softclip);
        parent.insert_item("nbits",&nbits);
        parent.insert_item("do_clipping", &do_clipping);
    }
    parent.insert_item("tau_level",&tau_level);
    parent.insert_item("rmslevel",&rmslevel);
    parent.insert_item("config",&config_parser);
}

class calibrator_runtime_layer_t {
public:
    calibrator_runtime_layer_t(bool is_input,
                               const mhaconfig_t& tf,
                               calibrator_variables_t& vars);
    mha_real_t process(mha_wave_t**);
private:
    static unsigned int firfirlen(const std::vector<std::vector<float> >&);
    static unsigned int firfir2fftlen(unsigned int,const std::vector<std::vector<float> >&);
    MHAFilter::fftfilter_t fir;
    MHASignal::quantizer_t quant;
    MHASignal::waveform_t gain;
    softclipper_t softclip;
    bool b_is_input;
    bool b_use_fir;
    bool b_use_clipping;
    MHASignal::loop_wavefragment_t speechnoise;
    MHASignal::loop_wavefragment_t::playback_mode_t pmode;
};

mha_real_t calibrator_runtime_layer_t::process(mha_wave_t** s)
{
    mha_real_t retv(0);
    if( b_is_input ){
        quant(**s);
        if( b_use_fir )
            fir.filter( *s, s );
        **s *= gain;
        speechnoise.playback(*s,pmode);
    }else{
        speechnoise.playback(*s,pmode);
        **s *= gain;
        if( b_use_fir )
            fir.filter( *s, s );
        if( b_use_clipping )
            retv = softclip.process( *s );
        else
            retv = 0;
        quant(**s);
    }
    return retv;
}

unsigned int calibrator_runtime_layer_t::firfirlen(const std::vector<std::vector<float> >& fir)
{
    unsigned int irslen = 0;
    for(unsigned int ch=0;ch<fir.size();ch++)
        if( fir[ch].size() > irslen )
            irslen = fir[ch].size();
    return irslen;
}

unsigned int calibrator_runtime_layer_t::firfir2fftlen(unsigned int fragsize,
                                                       const std::vector<std::vector<float> >& fir)
{
    unsigned int irslen = std::max(1u,firfirlen(fir)) - 1 + fragsize;
    unsigned int fftlen = 1;
    while( fftlen < irslen )
        fftlen *= 2;
    return fftlen;
}

std::vector<int> vint_0123n1(unsigned int n)
{
    std::vector<int> retv;
    retv.resize(n);
    for(unsigned int k=0;k<n;k++)
        retv[k] = k;
    return retv;
}

calibrator_runtime_layer_t::calibrator_runtime_layer_t(bool is_input,
                                                       const mhaconfig_t& tf,
                                                       calibrator_variables_t& vars)
    : fir(tf.fragsize,tf.channels,firfir2fftlen(tf.fragsize,vars.fir.data)),
      quant(vars.nbits.data),
      gain(tf.fragsize,tf.channels),
      softclip(vars.softclip,tf),
      b_is_input(is_input),
      b_use_fir(false),
      b_use_clipping(vars.do_clipping.data),
      speechnoise(speechnoise_t(2.0f,tf.srate,1,kw_index2type( vars.spnoise_mode.data.get_index())),true,MHASignal::loop_wavefragment_t::rms,vars.spnoise_channels.data),
      pmode(vars.spnoise_mode.data.get_index()==0?MHASignal::loop_wavefragment_t::input:MHASignal::loop_wavefragment_t::replace)
{
    speechnoise.set_level_db(vars.spnoise_level.data);
    unsigned int k, ch;
    if( (vars.peaklevel.data.size() != tf.channels) && (vars.peaklevel.data.size() != 1) )
        throw MHA_Error(__FILE__,__LINE__,
                        "The number of entries in the peaklevel vector must be either %u (one per channel) or 1"
                        " (same peaklevel for all channels)", tf.channels);
    if( vars.peaklevel.data.size() == 1 ){
        for(k=0;k<tf.fragsize;k++)
            for(ch=0;ch<tf.channels;ch++)
                gain(k,ch) = 2e-5*pow(10.0,0.05*vars.peaklevel.data[0]);
    }else{
        for(k=0;k<tf.fragsize;k++)
            for(ch=0;ch<tf.channels;ch++)
                gain(k,ch) = 2e-5*pow(10.0,0.05*vars.peaklevel.data[ch]);
    }
    if( !b_is_input )
        for(k=0;k<tf.fragsize;k++)
            for(ch=0;ch<tf.channels;ch++)
                gain(k,ch) = 1.0f/gain(k,ch);
    if( firfirlen( vars.fir.data ) ){
        b_use_fir = true;
        MHASignal::waveform_t fir_c(firfirlen(vars.fir.data),tf.channels);
        if( vars.fir.data.size() == tf.channels ){
            for(ch=0;ch<tf.channels;ch++)
                for(k=0;k<vars.fir.data[ch].size();k++)
                    fir_c(k,ch) = vars.fir.data[ch][k];
        }else if( vars.fir.data.size() == 1 ){
            for(ch=0;ch<tf.channels;ch++)
                for(k=0;k<vars.fir.data[0].size();k++)
                    fir_c(k,ch) = vars.fir.data[0][k];
        }else{
            throw MHA_Error(__FILE__,__LINE__,
                            "Invalid dimension of filter coefficients (%zu rows), expected either %u"
                            " (one row for each channel) or 1 (same coefficients for all channels).",
                            vars.fir.data.size(),tf.channels);
        }
        fir.update_coeffs(&fir_c);
    }
    vars.srate.data = tf.srate;
    vars.fragsize.data = tf.fragsize;
    vars.num_channels.data = tf.channels;
}

typedef MHAPlugin::config_t<MHASignal::async_rmslevel_t> rmslevelmeter;
typedef MHAPlugin::plugin_t<calibrator_runtime_layer_t> rtcalibrator;

class calibrator_t : public rtcalibrator,
                     private rmslevelmeter
{
public:
    calibrator_t(algo_comm_t,bool is_input);
    void prepare(mhaconfig_t& tf){tftype=tf;prepared=true;update();update_tau_level();};
    void release(){prepared=false;};
    mha_wave_t* process(mha_wave_t* s);
private:
    void update();
    void update_tau_level();
    void read_levels();
    bool b_is_input;
    MHAEvents::patchbay_t<calibrator_t> patchbay;
    calibrator_variables_t vars;
    bool prepared;
};

void calibrator_t::update_tau_level()
{
    rmslevelmeter::push_config(new MHASignal::async_rmslevel_t(std::max(1u,(unsigned int)(tftype.srate*vars.tau_level.data)),
                                                               tftype.channels));
}

void calibrator_t::read_levels()
{
    if( rmslevelmeter::cfg )
        vars.rmslevel.data = rmslevelmeter::cfg->rmslevel();
    else
        vars.rmslevel.data = std::vector<float>(tftype.channels,-200.0f);
}

mha_wave_t* calibrator_t::process(mha_wave_t* s)
{
    bool peaklevel_set = true;

    for(unsigned int i = 0; i < vars.peaklevel.data.size(); i++) {
        if (std::isnan(vars.peaklevel.data[i]))
            peaklevel_set = false;
    }

    if (!peaklevel_set)
        throw MHA_Error(__FILE__, __LINE__, "Peaklevel has not been set");

    if( b_is_input ){
        vars.softclip.clipped.data = rtcalibrator::poll_config()->process(&s);
        rmslevelmeter::poll_config()->process(s);
    }else{
        rmslevelmeter::poll_config()->process(s);
        vars.softclip.clipped.data = rtcalibrator::poll_config()->process(&s);

        if (vars.softclip.clipped.data > vars.softclip.max_clipped.data)
            throw MHA_Error(__FILE__, __LINE__, "Fatal error: Too much clipped");
    }
    return s;
}

calibrator_t::calibrator_t(algo_comm_t iac,bool is_input)
    : MHAPlugin::plugin_t<calibrator_runtime_layer_t>("calibration module",iac),b_is_input(is_input),vars(is_input,*this),prepared(false)
{
    patchbay.connect(&vars.spnoise_parser.writeaccess,this,&calibrator_t::update);
    patchbay.connect(&vars.peaklevel.writeaccess,this,&calibrator_t::update);
    patchbay.connect(&vars.fir.writeaccess,this,&calibrator_t::update);
    patchbay.connect(&vars.nbits.writeaccess,this,&calibrator_t::update);
    patchbay.connect(&vars.tau_level.writeaccess,this,&calibrator_t::update_tau_level);
    patchbay.connect(&vars.rmslevel.prereadaccess,this,&calibrator_t::read_levels);
}

void calibrator_t::update()
{
    if( prepared )
        rtcalibrator::push_config(new calibrator_runtime_layer_t(b_is_input,tftype,vars));
}

/*
 * PLUGIN INTERFACE
 */

class bbcalib_interface_t : public MHAPlugin::plugin_t<int> 
{
public:
    bbcalib_interface_t(const algo_comm_t&,const std::string&,const std::string&);
    ~bbcalib_interface_t();
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
    void release();
private:
    calibrator_t calib_in;
    calibrator_t calib_out;
    MHAParser::mhapluginloader_t plugloader;
};

bbcalib_interface_t::bbcalib_interface_t(const algo_comm_t& iac,
                                         const std::string&,
                                         const std::string&)
    : MHAPlugin::plugin_t<int>(
        "Signal level calibration plugin.",iac),
      calib_in(iac,true),calib_out(iac,false),
      plugloader(*this,iac)
{
    set_node_id("transducers");
    insert_member(calib_in);
    insert_member(calib_out);
}

bbcalib_interface_t::~bbcalib_interface_t()
{
}

void bbcalib_interface_t::prepare(mhaconfig_t& tf)
{
    if( tf.domain != MHA_WAVEFORM )
        throw MHA_Error(__FILE__,__LINE__,"This plugin operates in the waveform domain. Please use frequency analysis within calibrated signal path.");
    tftype = tf;
    calib_in.prepare(tf);
    plugloader.prepare(tf);
    if( tf.domain != MHA_WAVEFORM )
        throw MHA_Error(__FILE__,__LINE__,"The processing plugin does not return waveform data.");
    calib_out.prepare(tf);
}

mha_wave_t* bbcalib_interface_t::process(mha_wave_t* s)
{
    s = calib_in.process(s);
    plugloader.process(s,&s);
    s = calib_out.process(s);
    return s;
}

void bbcalib_interface_t::release()
{
    plugloader.release();
}

MHAPLUGIN_CALLBACKS(transducers,bbcalib_interface_t,wave,wave)
MHAPLUGIN_DOCUMENTATION\
(transducers,
 "filter limiter calibration level-meter",
 "Some plugins in the MHA expect the input signal to be calibrated to\n"
 "sound pressure level in Pascal. This plugin converts AD and DA converter\n"
 "levels to SPL in Pa and also allows for a FIR filters for mircophone and\n"
 "receiver equalization.\n"
 "\n"
 "\\subsection*{A schematic calibration rule for the MHA}\n"
 "\\begin{enumerate}\n"
 "\\item Measure frequency response of hearing aid microphones and receiver.\n"
 "\\item Create FIR filter coefficients for frequency response equalization\n"
 "      for microphones and receiver, configure the FIR coefficients of"
 "      this plugin correspondingly.\n"
 "\\item Play an acoustic reference signal of a known SPL level to the"
 " microphone, adjust the 'calib\\_in.peaklevel' variable until the internal"
 " level meter (e.g. rmslevel, p. \\pageref{plug:rmslevel}) shows the same level.\n"
 "\\item Create a test tone in the MHA (e.g. with 'noise',"
 " p. \\pageref{plug:noise}, or 'sine', p. \\pageref{plug:sine})"
 " of a given level, and adjust the variable 'calib\\_out.peaklevel' until"
 " the same acoustic level is measured at the receiver."
 "\\end{enumerate}\n"
 "\n"
 "Besides the signal calibration, this plugin also contains a soft-limiter\n"
 "in the output path, and a quantization module. The soft-limiter acts as a\n"
 "fast broadband compressor, and can be configured correspondingly.\n"
 "The quantisation module limits the signal to the interval $[-1,1]$\n"
 "and optionally reduces the resolution, by this quantization rule:\n"
 "\\begin{equation}\n"
 "y = {\\textrm{floor}}(2^{(N-1)} x) 2^{-(N-1)}\n"
 "\\end{equation}\n"
 "$N$ is the number of bits, $x$ the input signal and $y$ the output signal.\n"
 )

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
