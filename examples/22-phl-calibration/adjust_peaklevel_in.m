function peaklevel = adjust_peaklevel_in
  % IP address of mahalia running on PHL when using wireless
  mha.host = '10.0.0.1'
  % openMHA default TCP port
  mha.port= 33337
  % The sound level of the calibration noise that plays
  % while this script is executed, in dB SPL at the position
  % of the artificial head, measured without the head.
  noise_level = 80;
  % The first 4 channels correspond to the microphones (here 1-based indices)
  mic_ch = 1:4;
  % Repetitions when averaging microphone input levels
  repetitions = 20;
  % pause between level retrieval
  p = 0.6; % seconds
  
  % Retrieve current input sound levels for all channels 20 times from MHA
  for i = 1:repetitions
    levels(i,:) = mha_get(mha,'mha.transducers.calib_in.rmslevel');
    pause(p)
  end

  % Retrieve old input peaklevel from MHA
  peaklevel = mha_get(mha,'mha.transducers.calib_in.peaklevel');

  % Adjust the input peak levels of the microphones (first 4 channels)
  peaklevel(mic_ch) = peaklevel(mic_ch) - mean(levels(:,mic_ch)) + noise_level;
  mha_set(mha,'mha.transducers.calib_in.peaklevel', peaklevel)

  % Check if the adjustment had the desired effect: All input sound levels
  % should be 80 now.
  
  for i = 1:repetitions
    levels(i,:) = mha_get(mha,'mha.transducers.calib_in.rmslevel');
    pause(p)
  end

  mean_microphone_levels_after_adjustment = mean(levels)

  if any(abs(mean_microphone_levels_after_adjustment(mic_ch) - noise_level) > 0.1)
    error('Adjusting the peaklevel did not have the expected effect. Ensure that the calibration noise is playing and no interfering noises occur.')
  end
