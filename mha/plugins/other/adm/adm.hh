// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2004 2006 2014 2016 2017 HörTech gGmbH
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

#ifndef ADM_HH
#define ADM_HH

#include <cassert>
#include <algorithm>
#include <cmath>
#include <float.h>

namespace ADM {

  const double PI = 3.14159265358979312;
  const double C = 340;
  const double DELAY_FREQ = 2000;
  const double START_BETA = 0.5;


  /**
   * An efficient linear-phase fir filter implementation
   */
  template <class F>
  class Linearphase_FIR {
  public:
    /**
     * Create linear-phase FIR filter
     * @param order
     *     filter order of this FIR filter.
     *     restriction: must be even.
     * @param alphas
     *     pointer to Array of alpha coefficients. Since this class is for
     *     linear phase FIR filters only, only (order / 2 + 1) coefficients
     *     will be read. (Coefficients for linear-phase FIR filters are
     *     symmetric.)
     */
    Linearphase_FIR(unsigned order,
                    const F * alphas);

    ~Linearphase_FIR();

    /**
     * Filter one sample with this linear-phase FIR filter.
     * @param in_sample
     *   the current input sample
     * @return
     *   the computed output sample
     */
    inline
    F process(const F & in_sample)
    {
      F addend;
      signed out_index_1 = m_now;
      signed out_index_2 = (m_now + m_order) % (m_order + 1);
      for (unsigned filter_index = 0;
           filter_index < m_order / 2;
           ++filter_index,
             (out_index_1 = (out_index_1 + 1) % (m_order + 1)),
             (out_index_2 = (out_index_2 + (m_order + 1) - 1) % (m_order + 1))) {
        addend = m_alphas[filter_index] * in_sample;
        m_output[out_index_1] += addend;
        m_output[out_index_2] += addend;
      }
      assert(out_index_1 == out_index_2);
      addend = m_alphas[m_order / 2] * in_sample;
      m_output[out_index_1] += addend;

      F out_sample(m_output[m_now]);
      m_output[m_now] = F(0);
      m_now = (m_now + 1) % (m_order + 1);
      return out_sample;
    }

  private:
    /**
     * The filter order of this linear-phase FIR filter
     */
    unsigned m_order;

    /**
     * FIR filter coefficients. Only m_order / 2 + 1 coefficients need to be 
     * stored since coefficients of linear-phase FIR filters are symmetric
     */
    F * m_alphas;

    /**
     * Ringbuffer for building future output
     */
    F * m_output;

    /**
     * current start of ringbuffer
     */
    unsigned m_now;
  };

  /**
   * A delay-line class which can also do subsample-delays for a limited 
   * frequency range below fs/4.
   */
  template <class F>
  class Delay {
  public:
    /**
     * Create a signal delay object
     *
     * @param samples
     *   number of samples to delay (may be non-integer)
     * @param f_design
     *   subsampledelay is exact for this frequency
     * @param fs
     *   sampling frequency
     */
    Delay(F samples, F f_design, F fs);

    ~Delay();

    /**
     * Apply delay to signal
     *
     * @param in_sample
     *   The current input signal sample
     * @return
     *   The computed output sample
     */
    inline
    F process(const F & in_sample)
    {
      const unsigned m_now_out = (m_now_in + 1) % (m_fullsamples + 1);
      m_state[m_now_in] = 
        m_state[(m_now_in+(m_fullsamples+1) -1) % (m_fullsamples+1)] * m_coeff +
        (m_norm) * in_sample;
      m_now_in = (m_now_in + 1) % (m_fullsamples + 1);
      return m_state[m_now_out];
    }

  private:
    /**
     * Integer part of delay
     */
    unsigned m_fullsamples;

    /**
     * coefficient for 1st order IIR lowpass filter which does the subsample
     * delay
     */
    F m_coeff;

    /**
     * normalization for the IIR subsample delay filter
     */
    F m_norm;

    /**
     * Ringbuffer: Delayline
     */
    F * m_state;

    /**
     * current position for inserting new samples into m_state ringbuffer
     */
    unsigned m_now_in;
  };


  /**
   * Adaptive differential microphone, working for speech frequency range
   */
  template <class F>
  class ADM {
  public:
    /**
     * Create adaptive differential microphone
     *
     * @param fs
     *   Sampling rate / Hz
     * @param dist
     *   Distance between physical microphones / m
     * @param lp_order
     *   Filter order of FIR lowpass filter used for adaptation
     * @param lp_alphas
     *   Pointer to array of alpha coefficients for the lowpass filter used
     *   for adaptation. Since this class uses linear phase FIR filters only,
     *   only the first half (order/2 + 1) of the coefficients will be read
     *   (coefficients for linear-phase FIR filters are symmetric).
     * @param decomb_order
     *   Filter order of FIR compensation filter (compensates for comb filter
     *   characteristic)
     * @param decomb_alphas
     *   Pointer to array of alpha coefficients for the compensation filter
     *   used to compensate for the comb filter characteristic. Since this
     *   class uses linear phase FIR filters only, only the first half
     *   (order/2 + 1)of the coefficients will be read (coefficients
     *   for linear-phase FIR filters are symmetric).
     * @param mu_beta
     *   Adaptation step size for each set of ADMs (e.g. left and right)
     * @param tau_beta
     *   Time constant of the lowpass filter used for averaging the power of
     *   the output signal
     */
    ADM(F fs, F dist,
        unsigned lp_order, const F* lp_alphas,
        unsigned decomb_order, const F* decomb_alphas,
        F mu_beta, F tau_beta = F(50e-3));

    /**
     * ADM processes one frame
     *
     * @param front
     *   The current front input signal sample
     * @param back
     *   The current rear input signal sample
     * @param external_beta
     *   If >= 0, this is used as the "beta" parameter for direction to 
     *   filter out. Else, the beta parameter is adapted to filtered out a
     *   direction so that best reduction of signal intensity from the back
     *   hemisphere is achieved.
     * @return
     *   The computed output sample
     */
    inline
    F process(const F & front, const F & back,
              const F & external_beta = F(-1))
    {
      // apply delay
      F delayed_front = m_delay_front.process(front);
      F delayed_back = m_delay_back.process(back);

      // sum (cause for comb filter)
      F front_facing = front - delayed_back;
      F back_facing = back - delayed_front;

      // compute result (affected by comb filter)
      F comb_result = front_facing - m_beta * back_facing;

      if (external_beta >= 0)
        m_beta = external_beta;
      else {
        // lowpass filter signals used in adaption
        F lp_back_facing = m_lp_bf.process(back_facing);
        F lp_comb_result = m_lp_result.process(comb_result);
        
        // adapt beta
        m_powerfilter_state = 
          m_powerfilter_state * m_powerfilter_coeff +
          m_powerfilter_norm * lp_back_facing * lp_back_facing;
        F increment =
            m_mu_beta * lp_back_facing * lp_comb_result / m_powerfilter_state;
        if ( ! (std::isnan(increment)) )
            m_beta = m_beta + increment;
        if (m_beta < 0) m_beta = -m_beta;
        if (m_beta > 1) m_beta = 1;
      }

      // Apply comb filter compensation
      return m_decomb.process(comb_result);
    }
  public:
    F beta() const {return m_beta;}
  private:
    Delay<F> m_delay_front, m_delay_back;
    Linearphase_FIR<F> m_lp_bf, m_lp_result, m_decomb;
    F m_beta, m_mu_beta;
    F m_powerfilter_coeff, m_powerfilter_norm, m_powerfilter_state;
  };

  // Implementation

  /** compute IIR coefficient for subsample delay
   * @param samples
   *   Constraint: 0.0 <= samples < 1.0;
   *   Amount of sub-sample delay
   * @param f_design
   *   design frequency (subsample delay is accurate for this frequency)
   * @param fs
   *   sampling rate
   * @return
   *  IIR coefficient for subsample delay
   */
  static
  double subsampledelay_coeff(double samples, double f_design, double fs = 1.0)
  {
    double omega = 2 * PI * f_design / fs;
    return 1.0 / (sin(omega) / tan(samples * omega) + cos(omega));
  }

  template <class F>
  Delay<F>::Delay(F samples, F f_design, F fs)
    : m_fullsamples(static_cast<unsigned>(samples)),
      m_coeff(subsampledelay_coeff(samples - m_fullsamples, f_design, fs)),
      m_norm(1-m_coeff),
      m_state(new F[m_fullsamples + 1]),
      m_now_in(0)
  {
    std::fill_n(m_state, m_fullsamples + 1, F(0));
  }

  template <class F>
  Delay<F>::~Delay()
  { delete [] m_state; }

  template <class F>
  Linearphase_FIR<F>::Linearphase_FIR(unsigned order, const F * alphas)
    : m_order(order),
      m_alphas(new F[m_order / 2 + 1]),
      m_output(new F[m_order + 1]),
      m_now(0U)
  {
    assert(order % 2 == 0);
    std::copy(alphas, alphas + (order / 2 + 1), m_alphas);
    std::fill(m_output, m_output + (order + 1), F(0));
  }

  template <class F>
  Linearphase_FIR<F>::~Linearphase_FIR()
  {
    delete [] m_alphas;
    delete [] m_output;
  }

  template <class F>
  ADM<F>::ADM(F fs, F dist,
              unsigned lp_order, const F* lp_alphas,
              unsigned decomb_order, const F* decomb_alphas,
              F mu_beta, F tau_beta)
    : m_delay_front(dist / C * fs, DELAY_FREQ, fs),
      m_delay_back(dist / C * fs, DELAY_FREQ, fs),
      m_lp_bf(lp_order, lp_alphas),
      m_lp_result(lp_order, lp_alphas),
      m_decomb(decomb_order, decomb_alphas),
      m_beta(START_BETA),
      m_mu_beta(mu_beta),
      m_powerfilter_coeff(F(exp(-1.0/fs/tau_beta))),
      m_powerfilter_norm(F(1) - m_powerfilter_coeff),
      m_powerfilter_state(mu_beta) // avoid division by zero
  {}
}
#endif

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// coding: utf-8-unix
// indent-tabs-mode: nil
// End:
