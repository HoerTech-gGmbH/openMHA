// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2008 2010 2012 2013 2014 2015 2016 2017 2018 2020 HörTech gGmbH
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
#include "mha_events.h"

// add normalization types in string list and in the definitions:
#define NORMALIZATION_TYPES "[none default sum]"
#define NORM_NONE 0
#define NORM_DEFAULT 1
#define NORM_SUM 2

// Estimation types define whether the current value of the input signal
// u[k] will be used for estimation of the filter coefficnets or not.
#define ESTIMATION_TYPES "[previous current]"
#define ESTIM_PREV 0
#define ESTIM_CUR 1

class rt_nlms_t 
{
public:
    rt_nlms_t(algo_comm_t iac,
              const std::string& name,
              const mhaconfig_t& cfg,
              unsigned int ntaps_,
              const std::string& name_u,
              const std::string& name_d,
              const std::string& name_e,
              const std::string& name_f,
              const int n_no_update);
    ~rt_nlms_t() {}

    mha_wave_t* process(mha_wave_t* sUD, mha_real_t rho, mha_real_t c, unsigned int norm_type, unsigned int estim_type, mha_real_t lambda_smooth);
    void insert();
private:
    algo_comm_t ac;
    unsigned int ntaps;
    unsigned int frames;
    unsigned int channels;
    MHA_AC::waveform_t F;
    MHASignal::waveform_t U; ///< \brief Input signal cache
    MHASignal::waveform_t Uflt; ///< \brief Input signal cache (second filter)
    MHASignal::waveform_t Pu; ///< \brief Power of input signal delayline
    MHASignal::waveform_t fu; ///< \brief Filtered input signal
    MHASignal::waveform_t fuflt; ///< \brief Filtered input signal
    MHASignal::waveform_t fu_previous;
    MHASignal::waveform_t y_previous;
    MHASignal::waveform_t P_Sum; //recursively est. power for NORM_SUM

    std::string name_u_;
    std::string name_d_;
    std::string name_e_;

    int n_no_update_;
    int no_iter;

    mha_wave_t s_E;
};

/*
 * adaptive filter (LMS algorithm)
 *
 * The input signal u[k] and the output signal y[k] are taken from AC
 * variable. The estimated filter f'[k] and the filtered input signal
 * f'[k]*u[k] are stored into an AC variable. The input signal is not
 * touched and can be either waveform or spectrum.
 */
class nlms_t : public MHAPlugin::plugin_t<rt_nlms_t>
{
public:
    nlms_t(algo_comm_t,const char*,const char*);
    void prepare(mhaconfig_t&);
    void release();
    mha_wave_t* process(mha_wave_t*);
private:
    void update();
    //bool prepared;
    MHAParser::float_t rho;
    MHAParser::float_t c;
    MHAParser::int_t ntaps;
    MHAParser::string_t name_u;
    MHAParser::string_t name_d;
    MHAParser::kw_t normtype;
    MHAParser::kw_t estimtype;
    MHAParser::float_t lambda_smoothing_power; //recursive smoothing coefficient for sum normalization rule
    MHAParser::string_t name_e;
    MHAParser::string_t name_f;
    MHAParser::int_t n_no_update;
    std::string algo;
    MHAEvents::patchbay_t<nlms_t> patchbay;
};

nlms_t::nlms_t(algo_comm_t ac,const char*,const char* ialg)
    : MHAPlugin::plugin_t<rt_nlms_t>(
          "This plugin adaptively estimates the coefficients of a filter by means of the NLMS algorithm.\n\n"
          "The estimated filter  is stored into an AC variable named by the algorithm configuration name or by the configuration variable name_f and the input signal is filtered by the current filter and returned as the output signal of the plugin.\n",ac),
      rho("convergence coefficient","0.01","]0,2]"),
      c("stabilization parameter","1e-5","]0,]"),
      ntaps("number of taps in filter","32","]0,]"),
      name_u("Name of input signal U",""),
      name_d("Name of desired signal D",""),
      normtype("Normalization type","default",NORMALIZATION_TYPES),
      estimtype("Estimation type defined whether the current value of the input signal u[k] will be incorporated in the estimation of the filter coefficients or not. Default value (previous) does not.", "previous", ESTIMATION_TYPES),
      lambda_smoothing_power("Recursive smoothing constant for sum normalization", "0.9", "[0,1["),
      name_e("Name of error signal E", ""),
      name_f("Name of the AC variable for saving the adapive filter", ""),
      n_no_update("Number of iterations without updating the filter coefficients", "0"),
      algo(ialg)
{
    insert_member(rho);
    insert_member(c);
    insert_member(ntaps);
    insert_member(name_u);
    insert_member(name_d);
    insert_member(normtype);
    insert_member(estimtype);
    insert_member(lambda_smoothing_power);
    insert_member(name_e);
    insert_member(name_f);
    insert_member(n_no_update);
    patchbay.connect(&ntaps.writeaccess,this,&nlms_t::update);
    patchbay.connect(&name_u.writeaccess,this,&nlms_t::update);
    patchbay.connect(&name_d.writeaccess,this,&nlms_t::update);
    patchbay.connect(&name_e.writeaccess,this,&nlms_t::update);
}

void nlms_t::update()
{
    if( is_prepared() )
        push_config(new rt_nlms_t(ac,algo,tftype,ntaps.data,name_u.data,name_d.data, name_e.data,name_f.data,n_no_update.data));
}

void nlms_t::prepare(mhaconfig_t& cf)
{
    if( cf.domain != MHA_WAVEFORM )
        throw MHA_Error(__FILE__,__LINE__,"Only waveform processing allowed.");
    if( !cf.channels )
        throw MHA_Error(__FILE__,__LINE__,"At least one input channel expected.");
    if( !cf.fragsize )
        throw MHA_Error(__FILE__,__LINE__,"Fragment size should be at least one.");
    tftype = cf;
    //cf.channels /= 2;
    //if( cf.channels*2 != tftype.channels )
    //  throw MHA_Error(__FILE__,__LINE__,"Even number of channels expected (input signal, desired signal).");
    //prepared = true;
    update();
    poll_config()->insert();
}

void nlms_t::release()
{
    //prepared = false;
}

void rt_nlms_t::insert()
{
    F.insert();
}

inline void make_friendly_number_by_limiting( mha_real_t& x )
{
    if( x > 1.0e20 )
        x = 1.0e20;
    if( x < -1.0e20 )
        x = -1.0e20;
}

mha_wave_t* rt_nlms_t::process(mha_wave_t* sUflt, mha_real_t rho, mha_real_t c, unsigned int norm_type, unsigned int estim_type, mha_real_t lambda_smooth)
{
    if (no_iter < n_no_update_) {
        no_iter++;
    }

    insert();
    mha_wave_t s_U(MHA_AC::get_var_waveform(ac,name_u_));
    mha_wave_t s_D(MHA_AC::get_var_waveform(ac,name_d_));

    if(!name_e_.empty())        
        s_E = MHA_AC::get_var_waveform(ac,name_e_);

    //check that AC adaptation variables have same channels as input
    if ( s_U.num_channels != channels )
    {
        throw MHA_Error(__FILE__,__LINE__,"Number of input channels %u doesn't match input channels %u in name_u:%s",
                        channels, s_U.num_channels, name_u_.c_str());
    }

    if ( s_D.num_channels != channels )
    {
        throw MHA_Error(__FILE__,__LINE__,"Number of input channels %u doesn't match input channels %u in name_d:%s",
                        channels, s_U.num_channels, name_d_.c_str());
    }

    unsigned int ch, kf, kh, idx, fidx;
    mha_real_t err, u_input, d_desired;
    for(ch=0;ch<channels;ch++)
        for(kf=0;kf<frames;kf++){
            // index into external buffers:
            idx = ch+channels*kf;
            u_input = value(s_U,kf,ch);
            d_desired = value(s_D,kf,ch);

            fu.buf[idx] = 0;
            fuflt.buf[idx] = 0;
            // e[k] = y[k] - f'[k-1] u[k]
            if(name_e_.empty())
                err = rho * (y_previous.buf[ch] - fu_previous.buf[ch]);
            else
                err = rho * value(s_E,kf,ch);

            // apply the filter and update filter coefficients:

            // This switch controls the way the filter coefficients are computed.
            switch (estim_type) {

            // In the case of ESTIM_PREV the current input is not involved in the computation of the
            // filter coefficients. Only the previous ntaps input samples are used. The current input
            // is only filtered with the new filter coefficients. This is the default behaviour.
            case ESTIM_PREV:

                // here goes the normalization:
                switch( norm_type ){

                case NORM_DEFAULT :
                    err /= (Pu[ch]+c);
                    break;

                case NORM_SUM :
                    //compute the recursive sum of power
                    value( P_Sum, 0, ch ) = lambda_smooth * P_Sum[ch]
                            + (1-lambda_smooth) * (err*err + value(U,kf,ch)*value(U,kf,ch) );

                    //normalize error according to sum
                    err /= ( value(P_Sum, 0, ch)*ntaps + c );

                    break;

                    // NORM_NONE is not needed to be handled explicitely.
                }

                y_previous.buf[ch] = d_desired;
                Pu[ch] = 0;

                for(kh=ntaps-1;kh>0;kh--){

                    fidx = ch+channels*kh;

                    // filter update based on AC input variable:
                    if (no_iter >= n_no_update_) {
                        F.buf[fidx] += err * U.buf[fidx];
                        make_friendly_number_by_limiting( F.buf[fidx] );
                    }

                    // time shift of input signals:
                    U.buf[fidx] = U.buf[fidx - channels];
                    Uflt.buf[fidx] = Uflt.buf[fidx - channels];

                    // filter energy:
                    Pu[ch] += U.buf[fidx]*U.buf[fidx];

                    // apply filter:
                    fu.buf[idx] += F.buf[fidx] * U.buf[fidx];
                    fuflt.buf[idx] += F.buf[fidx] * Uflt.buf[fidx];
                }

                F.buf[ch] += err * U.buf[ch];

                // update latest input sample:
                U.buf[ch] = u_input;
                Uflt.buf[ch] = value(sUflt,kf,ch);

                break;


            // In the case of ESTIM_CUR the current input is also involved in the computation of the
            // filter coefficients. Therefore, Only the previous ntaps input samples are used. The current input
            // is only filtered with the new filter coefficients. This is the alternative behaviour.
            case ESTIM_CUR:

                // update the filter energy:
                fidx = ch + channels * (ntaps - 1);

                // The contribution of the earliest input sample to the computation of the filter energy is
                // subtracted from the total filter energy as this sample will not be used in this iteration
                // for the computation of the new filter coefficients anymore.
                Pu[ch] -= U.buf[fidx]*U.buf[fidx];
                Pu[ch] += u_input * u_input;

                // here goes the normalization:
                switch( norm_type ){

                case NORM_DEFAULT :
                    err /= (Pu[ch] + c);
                    break;

                case NORM_SUM :
                    //compute the recursive sum of power
                    value( P_Sum, 0, ch ) = lambda_smooth * P_Sum[ch]
                            + (1-lambda_smooth) * (err*err + value(U,kf,ch)*value(U,kf,ch) );

                    //normalize error according to sum
                    err /= ( value(P_Sum, 0, ch)*ntaps + c );

                    break;

                    // NORM_NONE is not needed to be handled explicitely.
                }

                for(kh=ntaps-1;kh>0;kh--){

                    fidx = ch+channels*kh;

                    // time shift of input signals:
                    U.buf[fidx] = U.buf[fidx - channels];
                    Uflt.buf[fidx] = Uflt.buf[fidx - channels];

                    // filter update based on AC input variable:
                    if (no_iter >= n_no_update_) {
                        F.buf[fidx] += err * U.buf[fidx];
                        make_friendly_number_by_limiting( F.buf[fidx] );
                    }

                    // apply filter:
                    fu.buf[idx] += F.buf[fidx] * U.buf[fidx];
                    fuflt.buf[idx] += F.buf[fidx] * Uflt.buf[fidx];
                }

                // update latest input sample:
                U.buf[ch] = u_input;
                Uflt.buf[ch] = value(sUflt,kf,ch);

                if (no_iter >= n_no_update_)
                    F.buf[ch] += err * U.buf[ch];

                break;
            }


            // apply filter to zero delay:
            fu.buf[idx] += F.buf[ch] * U.buf[ch];
            fuflt.buf[idx] += F.buf[ch] * Uflt.buf[ch];
            fu_previous.buf[ch] = fu.buf[idx];
        }


    return &fuflt;
}

mha_wave_t* nlms_t::process(mha_wave_t* s)
{
    return poll_config()->process(s, rho.data, c.data, normtype.data.get_index(), estimtype.data.get_index(), lambda_smoothing_power.data);
}

rt_nlms_t::rt_nlms_t(algo_comm_t iac,
                     const std::string& name,
                     const mhaconfig_t& cfg,
                     unsigned int ntaps_,
                     const std::string& name_u,
                     const std::string& name_d,
                     const std::string& name_e,
                     const std::string& name_f,
                     const int n_no_update)
    : ac(iac),
      ntaps(ntaps_),
      frames(cfg.fragsize),
      channels(cfg.channels), //all input channels processed
      F(ac,name_f.empty() ? name : name_f,ntaps,channels,true), // Estimated filter is saved in the AC space and thereby made accessible to other plugins in the same chain
      U(ntaps,channels),
      Uflt(ntaps,channels),
      Pu(ntaps,channels),
      fu(frames,channels),
      fuflt(frames,channels),
      fu_previous(frames,channels),
      y_previous(frames,channels),
      P_Sum(1,channels), //assuming that we need 1 sum for each channel
      name_u_(name_u),
      name_d_(name_d),
      name_e_(name_e),
      n_no_update_(n_no_update),
      no_iter(0)
{
}

MHAPLUGIN_CALLBACKS(nlms_wave,nlms_t,wave,wave)
MHAPLUGIN_DOCUMENTATION\
(nlms_wave,
 "feedback-suppression adaptive",
 "This plugin implements the NLMS algorithm for re-estimating the"
 " coefficients of an adaptive filter in each iteration. "
 "The estimated filter coefficients are saved in an AC variable having the"
 " same name as the plugin in the current configuration."
 " The name of this AC variable can also be set differently by setting the"
 " configuration variable \\textbf{name\\_f}. "
 "The input signal is filtered by the filter estimated in the current"
 " iteration and returned as the current output of the plugin from within"
 " the processing callback. "
 "\n"
 "The estimation of the filter coefficients is performed using the"
 " update rule given as in the following:\n"
 "\\begin{eqnarray}\n"
 "  e[k] &=& y[k-1] - f[k-1] u[k-1]\\\\\n"
 "  f[k] &=& f[k-1] + rho/(|u|^2+c) u[k-1] e[k],\n"
 "\\end{eqnarray}\n"
 "where $e$ is the error signal, $y$ is the desired signal and"
 " $u$ is the input signal. "
 "All three signals are read from the AC space."
 " For this, the configuration variables \\textbf{name\\_e},"
 " \\textbf{name\\_d} and \\textbf{name\\_u} should be set. "
 "The error signal can also be computed within the plugin given the other"
 " two signals, when the corresponding configuration variable is left empty. "
 "The plugin can be configured to use also the current sample $u[k]$ of the"
 " input signal in the estimation by asigning the configuration variable"
 " \\textbf{estimtype} to the value \\textit{current}. "
 "However in the default case (\\textit{previous}), the previous values"
 " as long as the filter (\\textbf{ntaps}) but the current one are used."
 )

/*
 * Local Variables:
 * compile-command: "make"
 * c-basic-offset: 2
 * indent-tabs-mode: nil
 * End:
 */
