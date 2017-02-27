// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2009 2013 2016 HörTech gGmbH
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

#ifndef MHA_GAINTABLE_H
#define MHA_GAINTABLE_H

#include "mha_defs.h"
#include "mha_error.hh"
#include "auditory_profile.h"

//#define DEBUG(x) std::cerr << __FILE__ << ":" << __LINE__ << " " << #x " = " << x << std::endl

namespace DynComp {

    /**
       \brief One-dimensional linear interpolation.

       \param vX Vector with input samples.
       \param vY Vector with values at input samples.
       \param X Input value to be interpolated.
       \retval Interpolated value Y(X) at position X.

     */
    mha_real_t interp1(const std::vector<mha_real_t>& vX, const std::vector<mha_real_t>& vY, mha_real_t X);

    /**
       \brief Linear interpolation in a two-dimensional field.

       \param vX Vector with input samples, first dimension.
       \param vY Vector with input samples, second dimension.
       \param mZ Field with values at input samples.
       \param X First dimension of input value to be interpolated.
       \param Y Second dimension of input value to be interpolated.
       \retval Interpolated value Z(X,Y) at position X,Y.

     */
    mha_real_t interp2(const std::vector<mha_real_t>& vX, const std::vector<mha_real_t>& vY, const std::vector<std::vector<mha_real_t> >& mZ, mha_real_t X, mha_real_t Y);

    /**
       \brief Gain table class.

       This gain table is intended to efficient table lookup, i.e,
       interpolation of levels, and optional interpolation of
       frequencies. Sample input levels and sample frequencies are
       given in the constructor. The gain entries can be updated with
       the update() member function via a gain prescription rule from
       an auditory profile.
     */
    class gaintable_t
    {
    public:
        /**
           \brief Constructor

           \param LInput Input level samples, in equivalent LTASS_combined dB SPL.
           \param FCenter Frequency samples in Hz (e.g., center frequencies of filterbank).
           \param channels Number of audio channels (typically 2).
         */
        gaintable_t(const std::vector<mha_real_t>& LInput,const std::vector<mha_real_t>& FCenter,unsigned int channels);
        ~gaintable_t();
        /**
           \brief Update gains from an external table.

           \param newGain New gain table entries.
           
           Dimension change is not allowed. The number of entries are checked.
         */
        void update(std::vector<std::vector<std::vector<mha_real_t> > > newGain);
        /**
           \brief Read Gain from gain table.
           \param Lin Input level
           \param Fin Input frequency (no match required)
           \param channel Audio channel

         */
        mha_real_t get_gain(mha_real_t Lin,mha_real_t Fin,unsigned int channel);
        /**
           \brief Read Gain from gain table.
           \param Lin Input level
           \param band Input frequency band
           \param channel Audio channel
         */
        mha_real_t get_gain(mha_real_t Lin, unsigned int band, unsigned int channel);
        /**
           \brief Read Gains from gain table.
           \param Lin Input levels.
           \param Gain Output gain.

           The number of channels in Lin and Gain must match the
           number of bands times number of channels in the gaintable.
         */
        void get_gain(const mha_wave_t& Lin,mha_wave_t& Gain);
        /**
           \brief Return number of frequency bands.
         */
        unsigned int nbands() const { return num_F; };
        /**
           \brief Return number of audio channels.
         */
        unsigned int nchannels() const { return num_channels; };
        /**
           \brief Return current input-output function.
         */
        std::vector<std::vector<mha_real_t> > get_iofun() const;
        std::vector<mha_real_t> get_vL() const { return vL;};
        std::vector<mha_real_t> get_vF() const { return vF;};
    private:
        unsigned int num_L;
        unsigned int num_F;
        unsigned int num_channels;
        std::vector<mha_real_t> vL;
        std::vector<mha_real_t> vF;
        std::vector<mha_real_t> vFlog;
        std::vector<std::vector<std::vector<mha_real_t> > > data;
    };

}

#endif

// Local Variables:
// mode: c++
// compile-command: "make -C .."
// coding: utf-8-unix
// End:
