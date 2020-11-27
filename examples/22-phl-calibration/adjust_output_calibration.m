function [peaklevels, equalizer_gains] = adjust_output_calibration
  % IP address of mahalia running on PHL when using wireless
  mha.host = '192.168.7.2';
  % openMHA default TCP port
  mha.port= 33337;
  % MHA peaklevels used during calibration measurements
  measurement_peaklevels = 114
  % The sound level of the calibration noise this skript
  % generates nominally
  noise_level = measurement_peaklevels - 30;
  % The audio channels which correspond to the receivers (here 1-based indices)
  rec_ch = [1,2];
  % The audio channels which correspond to the front mics (here 1-based indices)
  mic_ch = [1,3]; % order: frontleft,rearleft,frontright,rearright
  % Duration audio recordings for averaging output spectra
  duration = 10; % seconds
  % Add the path of your jack_playrec installation's matlab directory
  addpath('/usr/lib/jack_playrec');
  % (Windows users: try C:\Program Files\jack-playrec\mfiles instead)
  
  % Expected output level of calibrator
  calibrator_level = 98.7; % dB SPL
  % The Jack input port of the sound card to which the ear simulator is connected.
  jack_port = 'system:capture_1';
  % The sampling rate and period size of the local Jack server
  local_srate = 48000;
  local_fragsize = 2048;
  % The sampling rate of the PHL MHA configuration
  PHL_srate = 24000;
  % The FFT length in the PHL MHA configuration
  PHL_fftlen = 162;
  % Clipping warning levels
  warning_level_sinusoid = -6; % dB re FS
  warning_level_broadband = -11; % dB re FS
  % temporary signal file names
  tmpinfilename = '/tmp/zeros.wav';
  tmpoutfilename = '/tmp/out.wav';
  % (Windows users: try 'C:\msys64\home\zima\openMHA\examples\22-phl-calibration\zeros.wav'
  % and 'C:\msys64\home\zima\openMHA\examples\22-phl-calibration\out.wav' instead.
  
  % frequency_range where we calibrate
  frequency_range = [1000,8000];
  % User-settable correction_factors. Default: No correction
  correction_factors = ones(PHL_fftlen/2+1,2);
  
  % Start measurements with a recording of the signal of the calibrator,
  % recorded through the IEC711 coupler with our PC sound card.
  calibrator_recording = ...
    record_static_signal('of calibrator', warning_level_sinusoid);
  peaklevel_in_pc = calibrator_level - db(calibrator_recording, warning_level_sinusoid)

  % Adapt all mha settings when changing the default mha configuration on PHL
  set_all_mha_algorithms_to_bypass_except_fresponse();
  reset_fresponse_filter()
  
  % Replace signal of left microphone with olnoise and record left speaker output
  mha_set(mha, 'mha.transducers.calib_in.speechnoise.mode', 'olnoise');
  mha_set(mha, 'mha.transducers.calib_in.speechnoise.channels', mic_ch(1)-1);

  fprintf('\nDisconnect calibrator from ear simulator, then attach left RIC.\n');

  RIC_left_recording = record_static_signal('from left RIC', warning_level_broadband);
  RIC_left_recording = apply_peaklevel(RIC_left_recording, peaklevel_in_pc);
  RIC_left_spectrum = PHL_spectrum(RIC_left_recording, local_srate);

  % Repeat with right microphone and right speaker output
  mha_set(mha, 'mha.transducers.calib_in.speechnoise.channels', mic_ch(2)-1);
  fprintf('\nDisconnect left RIC from ear simulator, attach right RIC.\n');

  RIC_right_recording = record_static_signal('from right RIC', warning_level_broadband);
  RIC_right_recording = apply_peaklevel(RIC_right_recording, peaklevel_in_pc);
  RIC_right_spectrum = PHL_spectrum(RIC_right_recording, local_srate);

  % Record a sample of the mha generated olnoise digitally
  reference_recording = record_olnoise_directly();
  reference_recording = apply_peaklevel(reference_recording, measurement_peaklevels);
  reference_spectrum = PHL_spectrum(reference_recording, PHL_srate);

  % REUG as linear factors in PHL STFT resolution
  reug = 1
  reug = compute_transfer_function('KEMAR-left-FF.mat','KEMAR-right-FF.mat');
  
  gains_left = reug .* reference_spectrum ./ RIC_left_spectrum;
  gains_right = reug .* reference_spectrum ./ RIC_right_spectrum;

  gains_left = restrict_range(gains_left, frequency_range);
  gains_right = restrict_range(gains_right, frequency_range);

  equalizer_gains = [gains_left,gains_right] .* correction_factors;
  
  % Because equalizer_gains also amplify and attenuate, peaklevels can stay as is.
  peaklevels = measurement_peaklevels;
  
  set_fresponse_filter(equalizer_gains')
  
  % Check that calibration is correct with 80 dB sinusoids
  reug_dB_corrections = interp1([0:PHL_fftlen/2]/PHL_fftlen*PHL_srate, ...
                                20*log10(reug), [1 2 4 8] * 1000)
  % Check right sinusoids
  mha_set(mha, 'mha.transducers.calib_in.speechnoise.mode', 'sin1k');
  mha_set(mha, 'mha.transducers.calib_in.speechnoise.level', 80-reug_dB_corrections(1));
  RIC_1k_right_recording = record_static_signal('1kHz from right RIC', warning_level_sinusoid);
  RIC_1k_right_recording = apply_peaklevel(RIC_1k_right_recording, peaklevel_in_pc);
  mha_set(mha, 'mha.transducers.calib_in.speechnoise.mode', 'sin2k');
  mha_set(mha, 'mha.transducers.calib_in.speechnoise.level', 80-reug_dB_corrections(2));
  RIC_2k_right_recording = record_static_signal('2kHz from right RIC', warning_level_sinusoid);
  RIC_2k_right_recording = apply_peaklevel(RIC_2k_right_recording, peaklevel_in_pc);
  mha_set(mha, 'mha.transducers.calib_in.speechnoise.mode', 'sin4k');
  mha_set(mha, 'mha.transducers.calib_in.speechnoise.level', 80-reug_dB_corrections(3));
  RIC_4k_right_recording = record_static_signal('4kHz from right RIC', warning_level_sinusoid);
  RIC_4k_right_recording = apply_peaklevel(RIC_4k_right_recording, peaklevel_in_pc);
  mha_set(mha, 'mha.transducers.calib_in.speechnoise.mode', 'sin8k');
  mha_set(mha, 'mha.transducers.calib_in.speechnoise.level', 80-reug_dB_corrections(4));
  RIC_8k_right_recording = record_static_signal('8kHz from right RIC', warning_level_sinusoid);
  RIC_8k_right_recording = apply_peaklevel(RIC_8k_right_recording, peaklevel_in_pc);

  % Check left sinusoids
  mha_set(mha, 'mha.transducers.calib_in.speechnoise.channels', mic_ch(1)-1);
  mha_set(mha, 'mha.transducers.calib_in.speechnoise.mode', 'sin1k');
  mha_set(mha, 'mha.transducers.calib_in.speechnoise.level', 80-reug_dB_corrections(1));
  fprintf('\nDisconnect right RIC from ear simulator, attach left RIC.\n');
  RIC_1k_left_recording = record_static_signal('1kHz from left RIC', warning_level_sinusoid);
  RIC_1k_left_recording = apply_peaklevel(RIC_1k_left_recording, peaklevel_in_pc);
  mha_set(mha, 'mha.transducers.calib_in.speechnoise.mode', 'sin2k');
  mha_set(mha, 'mha.transducers.calib_in.speechnoise.level', 80-reug_dB_corrections(2));
  RIC_2k_left_recording = record_static_signal('2kHz from left RIC', warning_level_sinusoid);
  RIC_2k_left_recording = apply_peaklevel(RIC_2k_left_recording, peaklevel_in_pc);
  mha_set(mha, 'mha.transducers.calib_in.speechnoise.mode', 'sin4k');
  mha_set(mha, 'mha.transducers.calib_in.speechnoise.level', 80-reug_dB_corrections(3));
  RIC_4k_left_recording = record_static_signal('4kHz from left RIC', warning_level_sinusoid);
  RIC_4k_left_recording = apply_peaklevel(RIC_4k_left_recording, peaklevel_in_pc);
  mha_set(mha, 'mha.transducers.calib_in.speechnoise.mode', 'sin8k');
  mha_set(mha, 'mha.transducers.calib_in.speechnoise.level', 80-reug_dB_corrections(4));
  RIC_8k_left_recording = record_static_signal('8kHz from left RIC', warning_level_sinusoid);
  RIC_8k_left_recording = apply_peaklevel(RIC_8k_left_recording, peaklevel_in_pc);

  fprintf('Recorded the following eardrum levels (all should be 80dB or close\n');
  fprintf('1kL: %.1f, 2kL: %.1f, 4kL: %.1f, 8kL: %.1f\n', ...
          db(RIC_1k_left_recording), db(RIC_2k_left_recording), ...
          db(RIC_4k_left_recording), db(RIC_8k_left_recording));
  fprintf('1kR: %.1f, 2kR: %.1f, 4kR: %.1f, 8kR: %.1f\n', ...
          db(RIC_1k_right_recording), db(RIC_2k_right_recording), ...
          db(RIC_4k_right_recording), db(RIC_8k_right_recording));

  function sounddata = apply_peaklevel(sounddata,peaklevel)
    % when sounddata contains a sound recording of 1 channel, and peaklevel contains
    % the input peaklevel of the recording equipment, then sounddata is scaled
    % so that db(sounddata) will immediately return the physical level without
    % further scaling.
    sounddata = sounddata * 10^(peaklevel/20.0);
  end
  
  function recording = record_static_signal(source, warning_level)
    % Inform user
    fprintf('Record signal %s for %d seconds:\n', source, duration);
    input('Press Enter','s');
    % Do not record sound from keypress: wait.
    pause(0.5);
    % Record signal
    recording = jack_playrec(zeros(local_srate*duration,1),'input',jack_port);
    % Check if signal has fluctuations
    flucts = fluctuations(recording)
    if flucts > 0.1 % dB
      fprintf('Fluctuations in signal %s too high: %.1fdB\n', source, flucts)
      if isequal('y',input('Retry? [y,n]','s'))
        recording = record_static_signal(source, warning_level);
      else
        error('Aborted')
      end
    end
    dB_FS = db(recording)
    if db(recording) > warning_level
      fprintf(['suspiciously high level %.1f dB re FS in recording signal %s,'
               ' may be clipped\n'], db(recording), source);
      if isequal('y',input('Retry? [y,n]','s'))
        recording = record_static_signal(source, warning_level);
      else
        error('Aborted')
      end
    end
  end

  function recording = record_olnoise_directly()
    % Record the original spectrum of the noise just recorded
    mha_local = mha_start;
    mha_set(mha_local, 'srate', PHL_srate);
    mha_set(mha_local, 'iolib', 'MHAIOFile');
    mha_set(mha_local, 'mhalib', 'transducers');
    mha_set(mha_local, 'mha.plugin_name', 'identity');
    mha_set(mha_local, 'mha.calib_in.peaklevel', measurement_peaklevels);
    mha_set(mha_local, 'mha.calib_out.peaklevel', measurement_peaklevels);
    mha_set(mha_local, 'mha.calib_in.speechnoise.level', noise_level);
    mha_set(mha_local, 'mha.calib_in.speechnoise.channels', 0);
    mha_set(mha_local, 'mha.calib_in.speechnoise.mode', 'olnoise');
  
    audiowrite(tmpinfilename, zeros(duration * PHL_srate,1), PHL_srate);
  
    mha_set(mha_local, 'io.in', tmpinfilename);
    mha_set(mha_local, 'io.out', tmpoutfilename);
    mha_set(mha_local, 'cmd', 'start');
    mha_set(mha_local, 'cmd', 'quit');
  
    recording = audioread(tmpoutfilename);
  end
  
  function rmsdb = db(signal)
    % function computes the RMS level in dB re FS
    rmsdb = 10*log10(mean(signal.^2));
  end

  function fluctuations_dB = fluctuations(signal)
    % function computes level difference between first and
    % second half of signal
    l = floor(length(signal)/2);
    db1 = db(signal(1:l));
    db2 = db(signal(l+1:2*l));
    fluctuations_dB = abs(db1-db2)
  end

  function spectrum = PHL_spectrum(timesignal, source_rate)
    % scaling the timesignal because
    % 1) Matlab introduces a scaling factor of sqrt(length(timesignal))
    % 2) When we divide by another sqrt(length(timesignal)), we can sum the
    %    squared spectral magnitudes instead of taking the mean.
    timesignal = timesignal ./ length(timesignal);
    spectrum = zeros(floor(PHL_fftlen/2)+1,1);
    % We redistribute STFT bins from a higher spectral resolution to a lower
    % resolution. We need to normalize each bin again after the combination.
    divideby = spectrum * 0;
    % We need to smooth long spectra.
    noisyspectrum = fft(timesignal);
    noisyspectrum = real(noisyspectrum .* conj(noisyspectrum));
    
    for index = 1:length(noisyspectrum)
      frequency = (index-1) / length(noisyspectrum) * source_rate;
      target_index = round(frequency / PHL_srate * PHL_fftlen + 1);
      if (target_index >= 1 && target_index <= length(spectrum))
        spectrum(target_index) = spectrum(target_index) + noisyspectrum(index);
        divideby(target_index) = divideby(target_index) + 1;
      end
    end
    divideby(divideby == 0) = 1;
    spectrum = sqrt(spectrum ./divideby);
  end

  function restricted_phl_factors = restrict_range(phl_factors, frequency_range)
    % keeps the phl_factors inside frequency range, applies ramp
    % at low frequencies, constant factors at high frequencies:
    bin_numbers = round(frequency_range / PHL_srate * PHL_fftlen + 1);
    restricted_phl_factors = phl_factors;
    rising_flank = 1:bin_numbers(1);
    falling_flank = bin_numbers(2):length(phl_factors);
    restricted_phl_factors(rising_flank) = rising_flank / max(rising_flank) * phl_factors(bin_numbers(1));
    restricted_phl_factors(falling_flank) = phl_factors(bin_numbers(2));
  end
  
  function transfer_function = compute_transfer_function(varargin)
    % computes the mean transfer function as linear factors in PHL bin resolution
    for index = 1:nargin
      % compute log spectrum of each data source
      mat = load(varargin{index});
      transfer_function(index, :) = 20*log10(abs(fft(mat.v_data)));
    end
    % compute mean of all sources in log domain
    transfer_function = mean(transfer_function);

    % interpolate to target bin frequencies in log domain, then convert to linear
    % magnitude
    transfer_function = interp1([0:length(transfer_function)-1]/length(transfer_function) * mat.srate, ...
                                transfer_function,
                                [0:floor(PHL_fftlen/2)] / PHL_fftlen * PHL_srate);
    transfer_function =  10.^(transfer_function/20);
    transfer_function = transfer_function(:);
  end

  function set_all_mha_algorithms_to_bypass_except_fresponse()
    % The default MHA chain running on the PHL includes several speech processing
    % algorithms. For the calibration, we want all behaviour switched off.
    % Except for the frequency response filter.
    % Set ADM to pass through the first microphone
    mha_set(mha,'mha.transducers.mhachain.split.bte.adm.bypass',1);
    % No comb filter compensation
    mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.decomb.select','identity');
    % No cohernce filtering
    mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.coh.select','identity');
    % No compression
    mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.select','identity');
    % No frequency shifting
    mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.fshift.select','identity');
    % Enable frequency response filter, but do not change any settings
    mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.fresponse.select','equalize');
    % Turn off the noise generators in trasducers and reset any level settings
    mha_set(mha,'mha.transducers.calib_in.speechnoise.level', noise_level);
    mha_set(mha,'mha.transducers.calib_in.speechnoise.channels', []);
    mha_set(mha,'mha.transducers.calib_out.speechnoise.level', noise_level);
    mha_set(mha,'mha.transducers.calib_out.speechnoise.channels', []);
    mha_set(mha,'mha.transducers.calib_out.peaklevel', measurement_peaklevels);
  end
  
  function reset_fresponse_filter()
    set_fresponse_filter(ones(4,PHL_fftlen / 2 + 1));
  end
  function set_fresponse_filter(equalizer_gains)
    if size(equalizer_gains,1) == 2
      equalizer_gains = [equalizer_gains;ones(size(equalizer_gains))];
    end
    mha_set(mha, ...
            'mha.transducers.mhachain.signal_processing.ola.c.fresponse.equalize.gains',...
            equalizer_gains);
  end
end
