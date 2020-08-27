// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2005 2006 2007 2009 2010 2013 2014 2015 2018 2019 HörTech gGmbH
// Copyright © 2020 HörTech gGmbH
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
#include "mha_signal.hh"
#include "mha_events.h"
#include "mha_defs.h"
#include "mha_filter.hh"

namespace fftfilter {

    /** \internal
     * Return the length of the longest vector in irs.
     * @param irs "Matrix" of floats parser variable
     * @return length of the longest vector in irs, or 1 if all vectors are empty
     */
    unsigned int irs_length(const MHAParser::mfloat_t& irs)
    {
        unsigned int irslen = 1;
        for(unsigned int ch=0;ch<irs.data.size();ch++)
            if( irs.data[ch].size() > irslen )
                irslen = irs.data[ch].size();
        return irslen;
    }

    /** \internal
     * Validity checks.  Throws Error if parameters are invalid:
     * Number of channels must be > 0, fftlen must be >= fragsize.
     * The number of rows in irs has to match the number of channels,
     * or has to be exactly 1. None of the row vectors may be empty.
     * The longest supported impulse response is (fftlen - fragsize + 1).
     * Impulse responses longer than (fftlen - fragsize + 1) would cause
     * temporal aliasing.
     * @param irs The matrix containing the impulse responses (one response per
     *            channel, or the same response for every channels) as set by
     *            the parser.
     * @param channels The number of prepared audio channels for this MHA plugin.
     * @param fragsize The block size (samples per channel) for waveform audio data
     * @param fftlen   FFT length used for filtering.
     * @return the length of the longest impulse response vector in irs.
     */
    unsigned int irs_validator(const MHAParser::mfloat_t& irs,
                               const unsigned int& channels,
                               const unsigned int& fragsize,
                               const unsigned int& fftlen)
    {
        if( !channels )
            throw MHA_Error(__FILE__,__LINE__,
                            "No channels defined.");
        if( fragsize > fftlen )
            throw MHA_Error(__FILE__,__LINE__,
                            "The fragment size (%u) should not be greater than the FFT length (%u).",
                            fragsize, fftlen);
        if( (irs.data.size() != 1) && (irs.data.size() != channels) )
            throw MHA_Error(__FILE__,__LINE__,
                            "Please provide either one impulse response"
                            " (used for all channels) or %u impulse responses (one for each channel)",channels);
        unsigned int ch;
        for(ch=0;ch<irs.data.size();ch++){
            if( !irs.data[ch].size() )
                throw MHA_Error(__FILE__,__LINE__,
                                "Empty impulse response in channel %u.",ch);
            if( irs.data[ch].size()-1 > fftlen-fragsize )
                throw MHA_Error(__FILE__,__LINE__,
                                "The impulse response in channel %u is too"
                                " long.\nPlease increase the FFT length to avoid circular aliasing.",ch);
        }
        return irs_length( irs );
    }


    /** \internal
     * Run-time configuration class for the fftfilter MHA plugin.
     */
    class fftfilter_t {
    public:
        fftfilter_t(const MHAParser::mfloat_t& irs,
                    const unsigned int& fragsize,
                    const unsigned int& channels,
                    const unsigned int& fftlen);
        mha_wave_t* process(mha_wave_t*);
    private:
        /** Length of the longest impulse response applied. */
        unsigned int irslen;
        /** The block size (samples per channel) for waveform audio data. */
        unsigned int fragsize;
        /** FFT length used for filtering */
        unsigned int fftlen;
        /** Number of prepared audio channels processed by this MHA plugin. */
        unsigned int channels;
        /** The filter object */
        MHAFilter::fftfilter_t fftfilt;
    };

    /** \internal
     * \brief Initialization of new run-time configuration from channel-specific impulse
     * responses.
     * @param irs The matrix containing the impulse responses (one response per
     *            channel, or the same response for every channels) as set by
     *            the parser.
     * @param fragsize_ The block size (samples per channel) for waveform audio data
     * @param channels_ The number of prepared audio channels for this MHA plugin.
     * @param fftlen_   FFT length used for filtering
     */
    fftfilter_t::fftfilter_t(const MHAParser::mfloat_t& irs,
                             const unsigned int& fragsize_,
                             const unsigned int& channels_,
                             const unsigned int& fftlen_)
        : irslen(irs_validator(irs,channels_,fragsize_,fftlen_)),
          fragsize(fragsize_),
          fftlen(fftlen_),
          channels(channels_),
          fftfilt(fragsize,channels,fftlen)
    {
        MHASignal::waveform_t wwave(irslen,channels);
        unsigned int ch;
        unsigned int k;
        if( irs.data.size() == 1 ){
            for(ch=0;ch<channels;ch++)
                for(k=0;k<irs.data[0].size();k++)
                    wwave.value(k,ch) = irs.data[0][k];
        }else{
            for(ch=0;ch<channels;ch++)
                for(k=0;k<irs.data[ch].size();k++)
                    wwave.value(k,ch) = irs.data[ch][k];
        }
        fftfilt.update_coeffs(&wwave);
    }

    /** \internal Let fftfiler object handle the filtering */
    mha_wave_t* fftfilter_t::process(mha_wave_t* s)
    {
        CHECK_VAR(s);
        fftfilt.filter(s,&s);
        return s;
    }

    /** \internal Implements the MHA plugin interface for FFTFilter */
    class interface_t : public MHAPlugin::plugin_t<fftfilter_t> {
    public:
        interface_t(const algo_comm_t&,const std::string&,const std::string&);
        mha_wave_t* process(mha_wave_t*);
        void prepare(mhaconfig_t&);
    private:
        void update();
        MHAParser::mfloat_t irs;
        MHAParser::int_t fftlen;
        MHAParser::int_mon_t fftlen_final;
        MHAEvents::patchbay_t<interface_t> patchbay;
    };

    void interface_t::update()
    {
        if( tftype.channels ){
            unsigned int lfftlen = fftlen.data;
            if( lfftlen == 0 ) {
                lfftlen = tftype.fragsize + irs_length(irs) - 1;
            }
            push_config(new fftfilter_t(irs,
                                        tftype.fragsize,
                                        tftype.channels,
                                        lfftlen));
            fftlen_final.data = lfftlen;
        }
    }

    interface_t::interface_t(const algo_comm_t& iac,const std::string&,const std::string&)
        : MHAPlugin::plugin_t<fftfilter_t>("FFT based FIR filter",iac),
        irs("Impulse responses, one row for each channel\n(or single row to use in all channels)","[[1]]"),
        fftlen("FFT length used for FIR filter. If zero, the FFT length\n"
               "is fragsize + impulse response length - 1 (assuming that\n"
               "the discrete Dirac delta function has the IRS length 1).","0","[0,]"),
        fftlen_final("FFT length used by FFT filter (computed during prepare)")
    {
        insert_item("irs",&irs);
        insert_item("fftlen",&fftlen);
        insert_item("fftlen_final",&fftlen_final);
        patchbay.connect(&writeaccess,this,&interface_t::update);
    }

    void interface_t::prepare(mhaconfig_t& tf)
    {
        if( tf.domain != MHA_WAVEFORM )
            throw MHA_ErrorMsg("fftfilter: Only waveform processing is supported.");
        tftype = tf;
        update();
    }

    mha_wave_t* interface_t::process(mha_wave_t* s)
    {
        poll_config();
        return cfg->process(s);
    }

}

MHAPLUGIN_CALLBACKS(fftfilter,fftfilter::interface_t,wave,wave)
MHAPLUGIN_DOCUMENTATION(fftfilter,"filter","The 'fftfilter' plugin implements a generic FFT-based FIR filter."
                        " The overlap-save method is used to apply the impulse response to each block of the signal."
                        " The default FFT length used is computed from the fragsize and the inpulse response length"
                        " and set to the minimum required FFT length to perform the overlap-save operation"
                        " (see documentation of configuration variable \\texttt{fftlen}). If this is not a power of two"
                        ", the computation may be inefficient, and it should be considered to increase it to the next"
                        " power of two larger than the required minimum."
                        "\n\n"
                        "The 'fftfilter' plugin does not introduce additional"
                        " delay.  Regardless of fragsize, length of impulse"
                        " response, or fft length, the computed output of"
                        " plugin 'fftfilter' is the same as if the output"
                        " had been computed by performing the convolution"
                        " on the same signal blocks in the time domain,"
                        " except for numerical errors.")

// Local Variables:
// compile-command: "make"
// c-basic-offset: 4
// indent-tabs-mode: nil
// coding: utf-8-unix
// End:
