// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2007 2009 2010 2014 2015 2016 2017 HörTech gGmbH
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

// Directional Mic Plugin (ADM = Adaptive Differential Microphone)

#include "adm.hh"

// MHA interface for external algorithms:
#include "mha_signal.hh"
#include "mha_error.hh"
#include "mha_parser.hh"
#include "mha_plugin.hh"
#include "mha_events.h"
#include "mha_filter.hh"

#include <stdlib.h>
#include <math.h>
#include <string>
#include <cassert>
#include <cstdio>

MHASignal::waveform_t* adm_fir_lp(unsigned int fs,unsigned f_pass, unsigned int f_stop,unsigned int order)
{
    MHASignal::spectrum_t f(fs/2+1,1);
    unsigned int k;
    // pass band:
    for(k=0;k<f_pass;k++)
        f[k] = mha_complex(1,0);
    // Hann ramp:
    for(k=f_pass;k<f_stop;k++)
        f[k] = mha_complex(pow(cos(0.5*M_PI*(k-f_pass)/(f_stop - f_pass)),2.0),0);
    // stop band:
    for(k=f_stop;k<f.num_frames;k++)
        f[k] = mha_complex(0,0);
    return MHAFilter::spec2fir(&f,fs,MHAWindow::fun_t(order,MHAWindow::hamming,-1,1,true,true),false);
}

MHASignal::waveform_t* adm_fir_decomb(unsigned int fs,float dist_m,unsigned int order)
{
    unsigned int f_min = std::min(200u,fs/2+1);
    float amplitude = 1.9;
    MHASignal::spectrum_t f(fs/2+1,1);
    unsigned int f_max = std::min(fs/2+1,(unsigned int)(0.25*ADM::C/dist_m));
    for(unsigned int k=0;k<f.num_frames;k++)
        f[k] = mha_complex(1.0/std::max((double)(1.0-(double)k/f_min),
                                        std::max((double)(sin(0.5*k*M_PI/f_max)*amplitude),(double)(k>f_max))));
    return MHAFilter::spec2fir(&f,fs,MHAWindow::fun_t(order,MHAWindow::hamming,-1,1,true,true),false);
}

class adm_rtconfig_t {
public:
    typedef ADM::ADM<mha_real_t> adm_t;
private:
    /** Indices of channels containing the signals from the front microphones */
    std::vector<int> front_channels;

    /** Indices of channels containing the signals from the rear microphones */
    std::vector<int> rear_channels;

    /** Lowpass filter coefficients */
    MHASignal::waveform_t* lp_coeffs;

    /** Decomb filter coefficients */
    std::vector<MHASignal::waveform_t*> decomb_coeffs;

    /** ADMs */
    std::vector<adm_t *> adms;

private:
    /** Index checking for all internal arrays
     * \throw MHA_Error if index is out of range */
    inline void check_index(unsigned index) const
    { if (index >= adms.size()) throw MHA_ErrorMsg("BUG:Index out of range"); }

public:
    /**
        \brief Construct new ADMs. Used when configuration changes.
        \param nchannels_in   Number of input channels
        \param nchannels_out  Number of output channels
        \param front_channels Parser's front_channels setting
        \param rear_channels  Parser's front_channels setting
        \param fs             Sampling rate / Hz
        \param distances      Distances between microphones / m
        \param lp_order       Filter order of FIR lowpass filter for adaptation
        \param decomb_order   Filter order of FIR compensation filter
                              (compensates for comb filter characteristic)
        \param mu_beta        Adaptation step sizes
     */
    adm_rtconfig_t(unsigned nchannels_in,
                   unsigned nchannels_out,
                   const std::vector<int> & front_channels,
                   const std::vector<int> & rear_channels,
                   const mha_real_t fs,
                   const std::vector<mha_real_t> & distances,
                   const int lp_order,
                   const int decomb_order,
                   const std::vector<mha_real_t> & mu_beta);

    virtual ~adm_rtconfig_t();

    inline size_t num_adms() const { return adms.size(); }

    /** Returns adm object number index */
    inline adm_t & adm(unsigned index)
    { check_index(index); return *adms[index]; }

    /** Returns index of front channel for adm number index */
    inline int front_channel(unsigned index) const
    { check_index(index); return front_channels[index]; }

    /** Returns index of rear channel for adm number index */
    inline int rear_channel(unsigned index) const
    { check_index(index); return rear_channels[index]; }
};

adm_rtconfig_t::adm_rtconfig_t(unsigned nchannels_in,
                               unsigned nchannels_out,
                               const std::vector<int> & front_channels_,
                               const std::vector<int> & rear_channels_,
                               const mha_real_t fs,
                               const std::vector<mha_real_t> & distances,
                               const int lp_order,
                               const int decomb_order,
                               const std::vector<mha_real_t> & mu_beta)
    : front_channels(front_channels_),
      rear_channels(rear_channels_),
      decomb_coeffs(front_channels_.size(), NULL ),
      adms(front_channels_.size(), static_cast<adm_t *>(0))
{
    if (front_channels.size() != nchannels_out) {
        throw MHA_Error(__FILE__,__LINE__,
                        "Number of front microphone channels (%d) and number"
                        " of output channels (%d) do not match",
                        int(front_channels.size()), int(nchannels_out));
    }
    if (rear_channels.size() != nchannels_out) {
        throw MHA_Error(__FILE__,__LINE__,
                        "Number of rear microphone channels (%d) and number"
                        " of output channels (%d) do not match",
                        int(front_channels.size()), int(nchannels_out));
    }
    if (distances.size() != nchannels_out) {
        throw MHA_Error(__FILE__,__LINE__,
                        "Number of configured distances (%d) does not match"
                        " the number of output channels (%d)",
                        int(distances.size()), int(nchannels_out));
    }
    if (mu_beta.size() != nchannels_out) {
        throw MHA_Error(__FILE__,__LINE__,
                        "Number of configured values for mu_beta (%d) does not"
                        " match the number of output channels (%d)",
                        int(mu_beta.size()), int(nchannels_out));
    }
    lp_coeffs = adm_fir_lp((unsigned int)fs,5000,6000,lp_order);
    for (std::vector<adm_t>::size_type i = 0; i < num_adms(); ++i) {
        if (front_channels[i] < 0)
            throw MHA_ErrorMsg("Front_channels vector contains negative"
                             " channel index");
        if (rear_channels[i] < 0)
            throw MHA_ErrorMsg("Rear_channels vector contains negative"
                             " channel index");
        if (front_channels[i] >= int(nchannels_in))
            throw MHA_Error(__FILE__, __LINE__,
                            "Front_channels vector contains channel index"
                            " %d, but there are only %d input channels"
                            " (channel indices start from zero)",
                            front_channels[i], int(nchannels_in));
        if (rear_channels[i] >= int(nchannels_in))
            throw MHA_Error(__FILE__, __LINE__,
                            "Rear_channels vector contains channel index"
                            " %d, but there are only %d input channels"
                            " (channel indices start from zero)",
                            rear_channels[i], int(nchannels_in));
        decomb_coeffs[i] = adm_fir_decomb((unsigned int)fs,distances[i],decomb_order);
        adms[i] = new adm_t(fs, distances[i], lp_order, lp_coeffs->buf,
                            decomb_order, decomb_coeffs[i]->buf, mu_beta[i]);
    }
}

adm_rtconfig_t::~adm_rtconfig_t()
{
    for (std::vector<adm_t>::size_type i = 0; i < num_adms(); ++i) {
        delete adms[i];
        adms[i] = 0;
        delete decomb_coeffs[i];
        decomb_coeffs[i] = NULL;
    }
    delete lp_coeffs;
}

class adm_if_t : public MHAPlugin::plugin_t<adm_rtconfig_t> {
public:
    adm_if_t(const algo_comm_t& ac,
             const std::string& thread_name,
             const std::string& algo_name);
    mha_wave_t * process(mha_wave_t * in);

    virtual void prepare(mhaconfig_t &);
    virtual void release();
private:
    MHASignal::waveform_t * out;
    MHAParser::vint_t front_channels;
    MHAParser::vint_t rear_channels;
    MHAParser::vfloat_t distances;
    MHAParser::int_t lp_order;
    MHAParser::int_t decomb_order;
    MHAParser::int_t bypass;
    MHAParser::float_t beta;
    MHAParser::vfloat_t mu_beta;
    MHAParser::vfloat_mon_t coeff_lp;
    MHAParser::vfloat_mon_t coeff_decomb;

    unsigned input_channels;
    mha_real_t srate;

    MHAEvents::patchbay_t<adm_if_t> patchbay;

    void update();
    bool is_prepared() { return out != 0; }
};

adm_if_t::adm_if_t(const algo_comm_t& ac,
                   const std::string& thread_name,
                   const std::string& algo_name)
    : MHAPlugin::plugin_t<adm_rtconfig_t>("Adaptive differential microphone",
                                          ac),
      out(0),
      front_channels("Channel indices for front microphones", "[0 1]", "[0,["),
      rear_channels("Channel indices for rear microphones", "[2 3]", "[0,["),
      distances("Distance between front and rear microphones",
                "[0.0108  0.0108]",
                "[0.0008,0.08]"),
      lp_order("Filter order of FIR lowpass filter", "46", "[46,128]"),
      decomb_order("Filter order of FIR comb compensation filter",
                   "54", "[46,128]"),
      bypass("If 1, output front microphones directly; "\
             "if 2, output rear microphones directly",
             "0", "[0,2]"),
      beta("Explicit fixed beta (-1 for adaptive filtering)","-1"),
      mu_beta("Adaptation step size for each set of ADMs (e.g. left and right)",
              "[1e-4 1e-4]", "[0,1]"),
      coeff_lp("Lowpass coefficients"),
      coeff_decomb("Decomb coefficients"),
      input_channels(0)
{
    insert_item("front_channels", &front_channels);
    insert_item("rear_channels", &rear_channels);
    insert_item("distances", &distances);
    insert_item("lp_order", &lp_order);
    insert_item("decomb_order", &decomb_order);
    insert_item("bypass", &bypass);
    insert_member(beta);
    insert_item("mu_beta", &mu_beta);
    insert_item("coeff_lp",&coeff_lp);
    insert_item("coeff_decomb",&coeff_decomb);
    patchbay.connect(&writeaccess, this, &adm_if_t::update);
}

void adm_if_t::prepare(mhaconfig_t & cfg)
{
    if (is_prepared()) {
        throw MHA_ErrorMsg("BUG: prepare called, but adm plugin is already in"
                           " prepared state");
    }
    input_channels = cfg.channels;
    srate = cfg.srate;
    if (cfg.domain != MHA_WAVEFORM) {
        throw MHA_ErrorMsg("Plugin supports only waveform processing");
    }
    cfg.channels = front_channels.data.size();
    out = new MHASignal::waveform_t(cfg.fragsize, cfg.channels);
    try {
        update();
        MHASignal::waveform_t* fir = adm_fir_lp((unsigned int)srate,5000,6000,lp_order.data);
        coeff_lp.data = std_vector_float(*fir);
        delete fir;
        if( distances.data.size() ){
            MHASignal::waveform_t* fir = adm_fir_decomb((unsigned int)srate,distances.data[0],decomb_order.data);
            coeff_decomb.data = std_vector_float(*fir);
            delete fir;
        }else{
            coeff_decomb.data.clear();
        }
    }
    catch(...) {
        release();
        throw;
    }
}
void adm_if_t::release()
{
    delete out;
    out = 0;
}

void adm_if_t::update()
{
    if ( ! is_prepared() )
        return;
    if (out == 0)
        throw MHA_ErrorMsg("BUG: adm_if_t::update():out==0");

    // odd filter orders are not supported
    lp_order.data &= ~1;
    decomb_order.data &= ~1;

    push_config(new adm_rtconfig_t(input_channels,
                                   out->num_channels,
                                   front_channels.data,
                                   rear_channels.data,
                                   srate,
                                   distances.data,
                                   lp_order.data,
                                   decomb_order.data,
                                   mu_beta.data));
}

mha_wave_t* adm_if_t::process(mha_wave_t * in)
{
    if (in->num_channels != input_channels) {
        throw MHA_Error(__FILE__,__LINE__,
                        "Processing sound: Expected %u input channels, got %u",
                        input_channels, in->num_channels);
    }
    poll_config();

    out->num_frames = in->num_frames;
    for (unsigned out_channel = 0;
         out_channel < out->num_channels;
         ++out_channel) {
        int front_channel = cfg->front_channel(out_channel);
        int rear_channel = cfg->rear_channel(out_channel);
        for (unsigned frame = 0;
             frame <  out->num_frames;
             ++frame) {
          switch (bypass.data) {
          case 0:
              out->value(frame, out_channel) =
                  cfg->adm(out_channel).process(value(in,frame, front_channel),
                                                value(in,frame, rear_channel),beta.data);
              break;
          case 1:
              out->value(frame, out_channel) = value(in,frame, front_channel);
              break;
          case 2:
          default:
              out->value(frame, out_channel) = value(in,frame, rear_channel);
              break;
          }
      }
    }
    return out;
}

// define MHA callbacks
MHAPLUGIN_CALLBACKS(adm,adm_if_t,wave,wave)

MHAPLUGIN_DOCUMENTATION(adm,"beamforming multichannel",
"This plugin implements one or more adaptive first-order differential\n"
"microphones, each based on the output of two omnidirectional microphones,\n"
"e.g. two hearing-aid microphones (cf. Elko \\& Nguyen Pong, 1995). This is\n"
"achieved by first subtracting the outputs of the two omnidirectional\n"
"microphones with fixed delays to create a forward-facing and a\n"
"backward-facing cardioid microphone, respectively; then, in a second step,\n"
"the signal from the backward-facing cardioid is amplified by a variable gain\n"
"factor and subtracted from the signal from the forward-facing cardioid.\n"
"Finally, a lowpass filter and a filter compensating for comb-filter effect\n"
"is applied to the output signal.\n"
"\n"
"The gain factor, {\\tt beta}, is determined adaptively such that the power\n"
"of the output signal is minimized, under the constraint that the null of the\n"
"ADM is located in the rear half-plane. The adaptation step size,\n"
"{\\tt mu\\_beta}, can be chosen in order to find the optimal combination of\n"
"adaptation speed and accuracy.\n"
"\n"
"\\MHAfigure{Output signals illustrating convergence of the ADM algorithm for\n"
"three different values of {\\tt mu\\_beta} (input signal: white Gaussian\n"
"noise exactly from behind)}{adm_convergence}\n"
)

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
