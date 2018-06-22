function [calib, needed_gains, peak, fir] = calibration(side, sampling_rate, resampling_rate, calib, moon_peak, fir_length)
% function calibration(side, sampling_rate, resampling_rate)
% side : 0=left 1=right
% sampling_rate of sound card in Hz
% resampling_rate: internal sampling_rate of signal processing in Hz
% calib: moon levels that correspond to 80 dB SPL Kemar
% moon_peak: peaklevel used for assessment of calib, default 120
% fir_length: length of computed output fir filter

if (nargin < 1 || isempty(side))
  side = 0;
end
if (nargin < 2 || isempty(sampling_rate))
  sampling_rate = 48000;
end
if (nargin < 3 || isempty(resampling_rate))
  resampling_rate = 32000;
end
if (nargin < 4)
    calib = [];
end
if (nargin < 5 || isempty(moon_peak))
    moon_peak = 120;
end
if (nargin < 6 || isempty(fir_length))
    fir_length = 65;
end

reug.f = [[125 250 500 1000 1500 2000 3000 4000 6000 8000]];
reug.g = [0,1,2,3,5,12,16,14,4,2];

% measure levels

if isempty(calib)
    mha = mha_start;
    s.nchannels_in = 2;
    s.fragsize=64;
    s.srate = sampling_rate;
    s.mhalib = 'splcalib';
    s.mha.calib_out.peaklevel = [120 120];
    s.mha.plugin_name = 'resampling';
    s.mha.resampling.srate = resampling_rate;
    s.mha.resampling.plugin_name = 'sine';
    s.mha.resampling.sine.channels = side;
    s.iolib='MHAIOJackdb';
    s.io.con_out={'system:playback_1', 'system:playback_2'};
    mha_set(mha,'',s);
    mha_set(mha,'cmd','start');

    for fi = 1:length(reug.f)
      f = reug.f(fi);
      mha_set(mha, 'mha.resampling.sine.f', f);
      mha_set(mha, 'mha.resampling.sine.lev', 0);
      level = 69;
      change = 1;
      while change
        level = level + change;
        mha_set(mha, 'mha.resampling.sine.lev', level);
        change = input(sprintf('\n\ncurrent moon level is %d\nspl level should be 80\nwhat change is required? ', level));
      end
      calib(fi) = level;
    end

    mha_set(mha,'cmd','quit');
end

% compute FIR filter

needed_gains = reug.g + (calib - 80);
peak_correction = max(needed_gains);
needed_gains = needed_gains - peak_correction;
peak = moon_peak  - peak_correction;
fir = fir2(fir_length, [0 (reug.f / sampling_rate * 2) 1], 10.^([needed_gains(1), needed_gains, -inf] / 20));


