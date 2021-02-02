// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2017 2018 2019 2020 HörTech gGmbH
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
//
// Armin Kohlrausch, Ralf Fassel, Marcel Heijden, Reinier Kortekaas,  Steven van de Par, Andrew Oxenham, and Dirk Püschel.
// "Detection of Tones in Low-noise Noise: Further Evidence for the Role of Envelope Fluctuations".
// Acta Acustica united with Acustica. 83. 659-669, 1997.


#include "mha_plugin.hh"
#include "mha_filter.hh"
#include "mha_signal_fft.h"

#define DEBUG(x) std::cerr << __FILE__ << ":" << __LINE__ << " " #x "=" << x << std::endl

namespace audiometerbackend {

static inline unsigned int gcd(unsigned int a, unsigned int b)
{
    return (b == 0) ? a : gcd(b, a % b);
}

class lnn3rdoct_t : public MHASignal::waveform_t
{
public:
    lnn3rdoct_t(unsigned int fs, unsigned int f,float bw,unsigned int niter);
private:
    void iterate_lnn();
    void bandpass();
    unsigned int _fmin,_fmax;
};

lnn3rdoct_t::lnn3rdoct_t(unsigned int fs, unsigned int f,float bw,unsigned int niter)
    : MHASignal::waveform_t(fs,1),
      _fmin(std::min(fs/2+1,(unsigned int)(f*pow(2.0,-0.5*bw)))),
      _fmax(std::min(fs/2+1,(unsigned int)(f*pow(2.0,0.5*bw))))
{
    //DEBUG(fs);
    //DEBUG(f);
    unsigned int nrand_it = 2;
    for(unsigned int krand_it=0;krand_it<nrand_it;krand_it++)
        for(unsigned int k=0;k<num_frames;k++){
            buf[k] += (float)rand()/RAND_MAX;
            buf[k] -= (float)rand()/RAND_MAX;
        }

    *this *= 1.0f/nrand_it;
    bandpass();
    for(unsigned int k=0;k<niter;k++)
        iterate_lnn();
    //DEBUG(_fmin);
    //DEBUG(_fmax);
    //DEBUG(MHASignal::rmslevel(*this,0));
}

void lnn3rdoct_t::bandpass()
{
    MHASignal::fft_t fft(num_frames);
    MHASignal::spectrum_t spec(num_frames/2+1,1);
    fft.wave2spec(this,&spec,false);
    for(unsigned int k=0;k<_fmin;k++)
        spec.buf[k].re = spec.buf[k].im = 0.0f;
    for(unsigned int k=_fmax;k<spec.num_frames;k++)
        spec.buf[k].re = spec.buf[k].im = 0.0f;
    fft.spec2wave(&spec,this);
}

void lnn3rdoct_t::iterate_lnn()
{
    MHASignal::waveform_t ximag(num_frames,1);
    MHASignal::hilbert_t hilbert(num_frames);
    hilbert(this,&ximag);
    for(unsigned int k=0;k<num_frames;k++)
        buf[k] /= sqrt(buf[k]*buf[k] + ximag.buf[k]*ximag.buf[k]);
    bandpass();
}

class sine_t : public MHASignal::waveform_t
{
public:
    sine_t(unsigned int fs, unsigned int f);
};

sine_t::sine_t(unsigned int fs,unsigned int f)
    : MHASignal::waveform_t(fs/gcd(fs,f),1)
{
    unsigned int k, ch;
    for(k=0;k<num_frames;k++)
        for(ch=0;ch<num_channels;ch++)
            value(k,ch) = sin(2.0*M_PI*f*k/(double)fs);
}

class signal_gen_t : public MHASignal::loop_wavefragment_t
{
public:
    signal_gen_t(int f,int fs,unsigned int sigtype);
};

MHASignal::waveform_t return_sig(unsigned int sigtype,unsigned int fs,unsigned int f)
{
    switch( sigtype ){
    case 0 :
        return sine_t(fs,f);
    case 1:
        return lnn3rdoct_t(fs,f,1.0/3.0,2);
    case 2:
        return lnn3rdoct_t(fs,f,1.0/3.0,0);
    case 3:
        return lnn3rdoct_t(fs,f,1.0,2);
    case 4:
        return lnn3rdoct_t(fs,f,1.0,0);
    }
    return MHASignal::waveform_t(fs,1);
}

signal_gen_t::signal_gen_t(int f, int fs, unsigned int sigtype)
    : MHASignal::loop_wavefragment_t(
          return_sig(sigtype,fs,f),
          true,rms,std::vector<int>(1,0))
{
}

class level_adapt_t : public MHASignal::waveform_t
{
public:
    level_adapt_t(mhaconfig_t cf,mha_real_t adapt_len,mha_real_t l_new_,mha_real_t l_old_);
    void update_frame();
    mha_real_t get_level() const {return l_new;};
    bool can_update() const {return pos==0;};
private:
    unsigned int ilen;
    unsigned int pos;
    MHAWindow::fun_t wnd;
    mha_real_t l_new;
    mha_real_t l_old;
};

level_adapt_t::level_adapt_t(mhaconfig_t cf,mha_real_t adapt_len,mha_real_t l_new_,mha_real_t l_old_)
    : MHASignal::waveform_t(cf.fragsize,1),
      ilen(std::max(1u,(unsigned int)(cf.srate*adapt_len))),
      pos(ilen-1),
      wnd(ilen,MHAWindow::hanning,0,1),
      l_new(l_new_),
      l_old(l_old_)
{
    //DEBUG(l_old);
    //DEBUG(l_new);
}

void level_adapt_t::update_frame()
{
    
    for(unsigned int k=0;k<num_frames;k++){
        value(k,0) = wnd.buf[pos]*l_new+(1.0f-wnd.buf[pos])*l_old;
        if( pos )
            pos--;
    }
}

typedef MHAPlugin::config_t<level_adapt_t> level_adaptor;
typedef MHAPlugin::plugin_t<signal_gen_t> generator;

class audiometer_if_t : public generator, private level_adaptor
{
public:
    audiometer_if_t(algo_comm_t,const char*,const char*);
    mha_wave_t* process(mha_wave_t*);
    void prepare(mhaconfig_t&);
private:
    void update();
    void change_mode();
    void set_level();
    MHAParser::int_t freq;
    MHAParser::kw_t sigtype;
    //MHAParser::bool_t loop;
    MHAParser::float_t level;
    //MHAParser::kw_t levelmode;
    MHAParser::kw_t mode;
    MHAParser::float_t ramplen;
    //unsigned int uint_mode;
    MHASignal::loop_wavefragment_t::playback_mode_t pmode;
    std::vector<int> outchannel;
    MHAEvents::patchbay_t<audiometer_if_t> patchbay;
};

audiometer_if_t::audiometer_if_t(algo_comm_t iac,const char*,const char*)
    : MHAPlugin::plugin_t<signal_gen_t>(
        "This plugin mimicks an audiometer by playing a signal in a given sound level on a given channel.\n\n", iac),
      freq("Frequency in Hz.","440"),
      sigtype("Signal type","sine","[sine oct3_lnn2 oct3_lnn0 oct_lnn2 oct_lnn0]"),
      level("Level in dB (SPL) of the input file","0"),
      mode("Playback mode","input","[input mute left right]"),
      ramplen("Length of hanning ramp at level changes in seconds","0","[0,]"),
      //mapping("Channel mapping"),
      //numchannels("Number of channels in current file"),
      pmode(MHASignal::loop_wavefragment_t::input),
      outchannel(std::vector<int>(1,0))
{
    set_node_id("audiometerbackend");
    insert_member(freq);
    insert_member(sigtype);
    //insert_member(loop);
    insert_member(level);
    //insert_member(levelmode);
    //insert_member(channels);
    insert_member(mode);
    insert_member(ramplen);
    //insert_member(mapping);
    //insert_member(numchannels);
    patchbay.connect(&freq.writeaccess,this,&audiometer_if_t::update);
    patchbay.connect(&sigtype.writeaccess,this,&audiometer_if_t::update);
    //patchbay.connect(&loop.writeaccess,this,&audiometer_if_t::update);
    patchbay.connect(&level.writeaccess,this,&audiometer_if_t::set_level);
    //patchbay.connect(&levelmode.writeaccess,this,&audiometer_if_t::update);
    //patchbay.connect(&channels.writeaccess,this,&audiometer_if_t::update);
    patchbay.connect(&mode.writeaccess,this,&audiometer_if_t::change_mode);
    patchbay.connect(&ramplen.writeaccess,this,&audiometer_if_t::set_level);
}

void audiometer_if_t::change_mode()
{
    switch(mode.data.get_index()){
    case 0: // input
        pmode = MHASignal::loop_wavefragment_t::input;
        outchannel[0] = 0;
        break;
    case 1: // mute
        pmode = MHASignal::loop_wavefragment_t::mute;
        outchannel[0] = 0;
        break;
    case 2: // left
        pmode = MHASignal::loop_wavefragment_t::replace;
        outchannel[0] = 0;
        break;
    case 3: // right
        pmode = MHASignal::loop_wavefragment_t::replace;
        outchannel[0] = 1;
        break;
    }
}

void audiometer_if_t::set_level()
{
    if( level_adaptor::cfg )
        level_adaptor::push_config(new level_adapt_t(input_cfg(),ramplen.data,2e-5*pow(10.0,0.05*level.data),level_adaptor::cfg->get_level()));
    else
        level_adaptor::push_config(new level_adapt_t(input_cfg(),ramplen.data,2e-5*pow(10.0,0.05*level.data),0.0f));
}

void audiometer_if_t::update()
{
    //level_adaptor::push_config(new level_adapt_t(input_cfg(),ramplen.data,pow(10.0,0.05*level.data),0.0f));
    if( is_prepared() )
        generator::push_config(new signal_gen_t(freq.data,(int)(input_cfg().srate), sigtype.data.get_index()));
}

void audiometer_if_t::prepare(mhaconfig_t& tf)
{
    if( tf.channels != 2 )
        throw MHA_Error(__FILE__,__LINE__,"Only binaural processing supported.");
    if( tf.domain != MHA_WAVEFORM )
        throw MHA_Error(__FILE__,__LINE__,"Only waveform processing supported.");
    set_level();
    update();
    level_adaptor::poll_config();
}

mha_wave_t* audiometer_if_t::process(mha_wave_t* s)
{
    generator::poll_config();
    if( level_adaptor::cfg->can_update() )
        level_adaptor::poll_config();
    level_adaptor::cfg->update_frame();
    generator::cfg->playback(s,pmode,level_adaptor::cfg,outchannel);
    return s;
}

}

/*
 * This macro connects the plugin1_t class with the MHA plugin C interface
 * The first argument is the class name, the other arguments define the
 * input and output domain of the algorithm.
 */
MHAPLUGIN_CALLBACKS(audiometerbackend,audiometerbackend::audiometer_if_t,wave,wave)

/*
 * This macro creates code classification of the plugin and for
 * automatic documentation.
 *
 * The first argument to the macro is a space separated list of
 * categories, starting with the most relevant category. The second
 * argument is a LaTeX-compatible character array with some detailed
 * documentation of the plugin.
 */
MHAPLUGIN_DOCUMENTATION\
(audiometerbackend,
 "signalflow generator audiometer",
 "This plugin has been designed to perform calibrated listening experiments by playing a signal in a sound level in dB SPL "
 "on a user-defined channel both determined by the user. "
 "The sound level can be adapted online while the signal is played. By using these features, this plugin can be configured "
 "to conduct an audiogram measurement or other audiometric measurements. "
 "The sound level in dB SPL can be adjusted by setting the configuration variable \\textbf{level}. "
 "The signal to be played can be selected from a pre-defined list or the input signal to this plugin can be used as well. "
 "The choice between the incoming input signal or a signal from the pre-defined list, as well as on which channel the "
 " selected signal is played, is made by setting the playback mode. For this, the configuration variable \\textbf{mode} can "
 "be used. This variable is another pre-defined list of four options, as given below:\n"
 "\\begin{itemize}\n"
 "\\item \\textbf{input:} The incoming input signal is played\n"
 "\\item \\textbf{mute:} The selected signal is not played\n"
 "\\item \\textbf{left:} The selected signal is played on the left channel\n"
 "\\item \\textbf{right:} The selected signal is played on the right channel\n"
 "\\end{itemize}\n"
 "The list of possible signals is given in the following list:\n"
 "\\begin{itemize}\n"
 "\\item \\textbf{sine:} Sine wave\n"
 "\\item \\textbf{oct3\\_lnn2:} Third Octave Low-noise Noise, iterated twice\n"
 "\\item \\textbf{oct3\\_lnn0:} Third octave Low-noise Noise\n"
 "\\item \\textbf{oct\\_lnn2:} Octave Low-noise Noise, iterated twice\n"
 "\\item \\textbf{oct\\_lnn0:} Octave Low-noise Noise\n"
 "\\end{itemize}\n"
 "In order to be able to select the signal from this list, please set the configuration parameter \\textbf{sigtype}. "
 "For more details about how the low-noise noise (LNN) is generated, please refer to the article Kohlrausch et al 1997. "
 "The frequency of the signal to be played is determined by setting the configuration variable \\textbf{freq}. Finally, "
 "a Hanning ramp can be incorporated in order to obtain a smooth transition between level changes. The length of the "
 "Hanning ramp in seconds is defined by setting the configuration variable \\textbf{ramplen}."
 )

/*
 * Local Variables:
 * compile-command: "make"
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
