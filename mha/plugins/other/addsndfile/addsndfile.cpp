// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2007 2009 2010 2011 2012 2013 2014 2015 2016 2018 HörTech gGmbH
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
#include "mha_filter.hh"
#include "mhasndfile.h"

#ifdef _WIN32
#include <windows.h>
#include <stdio.h>
#else
#include <dirent.h>
#include <fnmatch.h>
#endif

#define DEBUG(x) std::cerr << __FILE__ << ":" << __LINE__ << " " #x "=" << x << std::endl

namespace addsndfile {

    /** Specifies the resampling mode in resampled_soundfile_t */
    enum addsndfile_resampling_mode_t {
        /* Do not resample, just use the samples from the sound file
         * at the current sample rate, even if the sample rate of
         * the sound file differs. */
        DONT_RESAMPLE_PERMISSIVE,
        /** Do not resample, if the sample rate of the MHA differs
         *  from the sample rate of the sound file, raise an error. */
        DONT_RESAMPLE_STRICT,
        /** Resample */
        DO_RESAMPLE
    };

    /** Class helps to specify which instance of MHASignal_waveform_t parent
     * instance is meant in resampled_soundfile_t. */
    class waveform_proxy_t : public MHASignal::waveform_t {
    public:
        waveform_proxy_t(unsigned frames, unsigned channels) :
            MHASignal::waveform_t(frames, channels)
        {}
    };

    /** Reads sound from file and resamples it if necessary and wanted. Sound data
     * can then be used by addsndfile. */
    class resampled_soundfile_t : private MHASndFile::sf_wave_t,
                                  public waveform_proxy_t {
    public:
        /** Reads sound from file and resamples if necessary and wanted.
         * If the sound file does not specify a sampling rate, then
         * the sound data is always used without resampling.
         *
         * @param name
         *        Sound file name
         * @param mha_sampling_rate
         *        The sampling rate of the MHA signal processing
         *        at the point of the addsndfile plugin
         * @param resampling_mode
         *        DONT_RESAMPLE_STRICT:
         *               Do not resample, just use the samples from the sound file
         *               at the current sample rate, even if the sample rate of
         *               the sound file differs.
         *        DONT_RESAMPLE_PERMISSIVE:
         *               Do not resample, if the sample rate of the MHA differs
         *               from the sample rate of the sound file, raise an error.
         *        DO_RESAMPLE:
         *               Resample.
         * @throw MHA_Error
         *        If the sampling rate of the file does not match the sampling
         *        rate of the MHA and DONT_RESAMPLE_STRICT was requested.
         *        If resampling failed (e.g. due to non-rational quotient of
         *        MHA sampling rate and sound file sampling rate). */
        resampled_soundfile_t(const std::string & name,
                              float mha_sampling_rate,
                              addsndfile_resampling_mode_t resampling_mode);
    };

    static unsigned resampled_num_frames(unsigned num_source_frames,
                                         float source_rate,
                                         float target_rate,
                                         addsndfile_resampling_mode_t
                                         resampling_mode)
    {
        switch (resampling_mode) {
        case DONT_RESAMPLE_STRICT:
            if (source_rate == target_rate)
                return num_source_frames;
            break;
        case DONT_RESAMPLE_PERMISSIVE:
            return num_source_frames;
        case DO_RESAMPLE:
            if (source_rate == target_rate)
                return num_source_frames;
            return static_cast<unsigned>(ceilf(num_source_frames * target_rate
                                               / source_rate));
        }
        return 1U;
    }

    resampled_soundfile_t::resampled_soundfile_t(const std::string & name,
                                                 float mha_sampling_rate,
                                                 addsndfile_resampling_mode_t
                                                 /**/            resampling_mode)
        :  MHASndFile::sf_wave_t(name, 93.9794),
        waveform_proxy_t(resampled_num_frames(MHASndFile::sf_wave_t
                                              ::num_frames,
                                              MHASndFile::sf_wave_t
                                              ::samplerate,
                                              mha_sampling_rate,
                                              resampling_mode),
                         MHASndFile::sf_wave_t::num_channels)
    {
        switch (resampling_mode) {
        case  DONT_RESAMPLE_STRICT:
            if (MHASndFile::sf_wave_t::samplerate == mha_sampling_rate)
                waveform_proxy_t::copy(static_cast<MHASndFile::sf_wave_t*>(this));
            else
                throw MHA_Error(__FILE__,__LINE__,
                                "Sampling rate %d of sound file does not match"
                                " sampling rate %f of MHA and you have requested"
                                " no resampling and strict checking",
                                MHASndFile::sf_wave_t::samplerate,
                                mha_sampling_rate);
            break;
        case DONT_RESAMPLE_PERMISSIVE:
            waveform_proxy_t::copy(static_cast<MHASndFile::sf_wave_t*>(this));
            break;
        case DO_RESAMPLE:
            if (MHASndFile::sf_wave_t::samplerate == mha_sampling_rate)
                waveform_proxy_t::copy(static_cast<MHASndFile::sf_wave_t*>(this));
            else {
                MHAFilter::blockprocessing_polyphase_resampling_t
                    resampling(MHASndFile::sf_wave_t::samplerate,
                               MHASndFile::sf_wave_t::num_frames,
                               mha_sampling_rate, 1U,
                               0.85f, 7e-4f,
                               waveform_proxy_t::num_channels, false);
                resampling.write(static_cast<MHASndFile::sf_wave_t &>(*this));
                MHASignal::waveform_t w(1, waveform_proxy_t::num_channels);
                for (unsigned k = 0; k < waveform_proxy_t::num_frames
                         && resampling.can_read(); ++k) {
                    resampling.read(w);
                    waveform_proxy_t::copy_from_at(k,1,w,0);
                }
            }
            break;
        }
    }

    class sndfile_t : public MHASignal::loop_wavefragment_t
    {
    public:
        sndfile_t(const std::string& name,
                  bool loop,
                  unsigned int level_mode,
                  std::vector<int> channels_,
                  unsigned int nchannels,
                  std::vector<int>& mapping,
                  int& numchannels,unsigned int startpos,
                  float mha_sampling_rate,
                  addsndfile_resampling_mode_t resampling_mode);
    };

    sndfile_t::sndfile_t(const std::string& name,
                         bool loop,
                         unsigned int level_mode,
                         std::vector<int> channels_,
                         unsigned int nchannels,
                         std::vector<int>& mapping,
                         int& numchannels,
                         unsigned int startpos,
                         float mha_sampling_rate,
                         addsndfile_resampling_mode_t resampling_mode)
        : MHASignal::loop_wavefragment_t((name.size()>0
                                          ?static_cast<waveform_proxy_t>(resampled_soundfile_t(name, mha_sampling_rate, resampling_mode))
                                          :MHASignal::waveform_t(1,1)),
                                         loop,
                                         (loop_wavefragment_t::level_mode_t)level_mode,channels_,startpos)
    {
        mapping = get_mapping(nchannels);
        numchannels = loop_wavefragment_t::num_channels;
    }

    class level_adapt_t : public MHASignal::waveform_t
    {
    public:
        level_adapt_t(mhaconfig_t cf,
                      mha_real_t adapt_len,
                      mha_real_t l_new_,
                      mha_real_t l_old_);
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
    typedef MHAPlugin::plugin_t<sndfile_t> wave_reader;

    class addsndfile_if_t : public wave_reader, private level_adaptor
    {
    public:
        addsndfile_if_t(algo_comm_t,const char*,const char*);
        mha_wave_t* process(mha_wave_t*);
        void prepare(mhaconfig_t&);
        void release();
    private:
        void update();
        void change_mode();
        void set_level();
        void scan_dir();
        MHAParser::string_t filename;
        MHAParser::string_t path;
        MHAParser::bool_t loop;
        MHAParser::float_t level;
        MHAParser::kw_t levelmode;
        MHAParser::kw_t resamplingmode;
        MHAParser::vint_t channels;
        MHAParser::kw_t mode;
        MHAParser::float_t ramplen;
        MHAParser::int_t startpos;
        MHAParser::vint_mon_t mapping;
        MHAParser::int_mon_t numchannels;
        MHAParser::int_mon_t mhachannels;
        MHAParser::int_mon_t active;
        MHAParser::string_t search_pattern;
        MHAParser::vstring_mon_t search_result;
        unsigned int uint_mode;
        MHAEvents::patchbay_t<addsndfile_if_t> patchbay;
    };

    addsndfile_if_t::addsndfile_if_t(algo_comm_t iac,const char*,const char*)
        : MHAPlugin::plugin_t<sndfile_t>(
                                         "Add data from a sound file to some channels of input signal.\n\n"
                                         "The sound file is read into memory and scaled to a given peak\n"
                                         "level, i.e. the RMS level of an abstract signal with 0 dB full\n"
                                         "scale.\n"
                                         "Changing any parameter will start playing the file from the\n"
                                         "beginning. Playback of the file starts only after the complete\n"
                                         "file has been read.  If the sound file has fewer channels then\n"
                                         "the 'channels' vector has elements, then the file channels will\n"
                                         "be repeated.\n"
                                         ,iac),
        filename("File name of the sound file.",""),
        path("Optional path to sound file",""),
        loop("Infinitely loop the playback of the sound file","yes"),
        level("Level in dB (SPL) of the input file","0"),
        levelmode("Level mode","relative","[relative peak rms rms_limit40]"),
        resamplingmode("Resampling mode","do_resample","[dont_resample_permissive dont_resample_strict do_resample]"),
        channels("Output signal channels in which to store the individual sound file channels.  Output channel indices"
                 " start from 0.  Note: The addsndfile plugin does not change the number of MHA audio channels.  If you"
                 " specify a channel index >= the number of MHA audio channels, then that channel from the sound file"
                 " will not be used.","[0]","[0,]"),
        mode("Playback mode","add","[add replace input mute]"),
        ramplen("Length of hanning ramp at level changes in seconds","0","[0,]"),
        startpos("Starting position in samples, loop will begin from zero","0","[0,]"),
        mapping("Channel mapping"),
        numchannels("Number of channels in current file"),
        mhachannels("Number of MHA channels at plugin position"),
        active("indicates whether sound is currently played back"),
        search_pattern("Search pattern for file list","*.wav"),
        search_result("Available files"),
        uint_mode(0)
    {
        set_node_id("addsndfile");
        insert_item("path",&path);
        insert_item("filename",&filename);
        insert_item("loop",&loop);
        insert_item("level",&level);
        insert_item("levelmode",&levelmode);
        insert_member(resamplingmode);
        insert_item("channels",&channels);
        insert_item("mode",&mode);
        insert_item("ramplen",&ramplen);
        insert_member(startpos);
        insert_item("mapping",&mapping);
        insert_item("filechannels",&numchannels);
        insert_item("mhachannels",&mhachannels);
        insert_member(active);
        insert_item("search_pattern",&search_pattern);
        insert_item("files",&search_result);
        patchbay.connect(&filename.writeaccess,this,&addsndfile_if_t::update);
        patchbay.connect(&loop.writeaccess,this,&addsndfile_if_t::update);
        patchbay.connect(&level.writeaccess,this,&addsndfile_if_t::set_level);
        patchbay.connect(&levelmode.writeaccess,this,&addsndfile_if_t::update);
        patchbay.connect(&resamplingmode.writeaccess,this,&addsndfile_if_t::update);
        patchbay.connect(&channels.writeaccess,this,&addsndfile_if_t::update);
        patchbay.connect(&mode.writeaccess,this,&addsndfile_if_t::change_mode);
        patchbay.connect(&ramplen.writeaccess,this,&addsndfile_if_t::set_level);
        patchbay.connect(&startpos.writeaccess,this,&addsndfile_if_t::update);
        patchbay.connect(&search_result.prereadaccess,this,&addsndfile_if_t::scan_dir);
        active.data = 0;
    }

    void addsndfile_if_t::scan_dir()
    {
        search_result.data.clear();
#ifdef _WIN32
        WIN32_FIND_DATA FindFileData;
        HANDLE hFind;

        hFind = FindFirstFile((path.data + search_pattern.data).c_str(), &FindFileData);
        if (hFind == INVALID_HANDLE_VALUE)
            return;
        do {
            search_result.data.push_back(FindFileData.cFileName);
        } while (FindNextFile(hFind, &FindFileData) != 0);
        FindClose(hFind);

#else
        struct dirent **namelist;
        int n;
        std::string dir(path.data);

        if( !dir.size() )
            dir = ".";
        n = scandir(dir.c_str(), &namelist, NULL, alphasort );
        if( n >= 0 ){
            for(int k=0;k<n;k++){
                if( fnmatch(search_pattern.data.c_str(),namelist[k]->d_name,0)==0 )
                    search_result.data.push_back(namelist[k]->d_name);
                free(namelist[k]);
            }
            free(namelist);
        }
#endif
    }

    void addsndfile_if_t::change_mode()
    {
        uint_mode = mode.data.get_index();
    }

    void addsndfile_if_t::set_level()
    {
        if( level_adaptor::cfg )
            level_adaptor::push_config(new level_adapt_t(tftype,
                                                         ramplen.data,
                                                         2e-5*pow(10.0,0.05*level.data),
                                                         level_adaptor::cfg->get_level()));
        else
            level_adaptor::push_config(new level_adapt_t(tftype,
                                                         ramplen.data,
                                                         2e-5*pow(10.0,0.05*level.data),
                                                         0.0f));
    }

    void addsndfile_if_t::update()
    {
        if (!is_prepared()) return;
        if( filename.data.size() )
            wave_reader::push_config(new sndfile_t(path.data+filename.data,
                                                   loop.data,
                                                   levelmode.data.get_index(),
                                                   channels.data,
                                                   tftype.channels,
                                                   mapping.data,
                                                   numchannels.data,startpos.data,
                                                   tftype.srate,
                                                   addsndfile_resampling_mode_t(resamplingmode.data.get_index())));
        else
            wave_reader::push_config(new sndfile_t(filename.data,
                                                   false,
                                                   levelmode.data.get_index(),
                                                   channels.data,
                                                   tftype.channels,
                                                   mapping.data,
                                                   numchannels.data,startpos.data,
                                                   tftype.srate,
                                                   addsndfile_resampling_mode_t(resamplingmode.data.get_index())));
    }

    void addsndfile_if_t::prepare(mhaconfig_t& tf)
    {
        tftype = tf;
        mhachannels.data = tf.channels;
        set_level();
        update();
        level_adaptor::poll_config();
    }
    void addsndfile_if_t::release() {
        active.data = false;
    }

    mha_wave_t* addsndfile_if_t::process(mha_wave_t* s)
    {
        wave_reader::poll_config();
        active.data = wave_reader::cfg->is_playback_active();
        if( level_adaptor::cfg->can_update() )
            level_adaptor::poll_config();
        level_adaptor::cfg->update_frame();
        wave_reader::cfg->playback(s,(MHASignal::loop_wavefragment_t::playback_mode_t)uint_mode,level_adaptor::cfg);
        return s;
    }

}

MHAPLUGIN_CALLBACKS(addsndfile,addsndfile::addsndfile_if_t,wave,wave)
MHAPLUGIN_DOCUMENTATION(addsndfile,"signalflow",
                        "Add data from a sound file to some channels of input signal.\n\n"
                        "The sound file is read into memory and scaled to a given peak\n"
                        "level, i.e. the RMS level of an abstract signal with 0 dB full\n"
                        "scale.\n"
                        "Changing any parameter will start playing the file from the\n"
                        "beginning. Playback of the file starts only after the complete\n"
                        "file has been read.  If the sound file has fewer channels then\n"
                        "the 'channels' vector has elements, then the file channels will\n"
                        "be repeated.\n"
                        "The addsndfile plugin does not change the number of MHA audio"
                        " channels.  If you specify a channel index $\\ge$ the number of"
                        " MHA audio channels, then the channel from the sound file will"
                        " not be used."
                        )

/*
 * Local Variables:
 * compile-command: "make"
 * c-basic-offset: 4
 * End:
 */
