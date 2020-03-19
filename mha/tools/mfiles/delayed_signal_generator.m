function [front_mic, rear_mic, corrected_angle] = delayed_signal_generator(signal, srate, angle, d, accuracy)
% function [front_mic, rear_mic, corrected_angle] = delayed_signal_generator(signal, srate, angle, d, accuracy)
%
% front_mic       - at 0º, microphone facing the source
% rear_mic        - at 0º, microphone facing opposite to source
% corrected_angle - corrected angle to angle input, arising due to rounding error
% signal          - source signal
% srate           - sampling rate of source signal (Hz)
% angle           - 0º is frontal direction for the endfire array and 
%                   90º and 270º perpendicular to source (degrees)
% distance        - distance between frontal and rear microphones (m)
% accuracy        - accuracy necessary for the delay values (s)
%
% Function which finds the delay between front and rear microphones with a 
% sound source at different angles from it applying the delays to the signals provided
% and a corrected_angle based on precision used, sampling rate provided to aid 
% accurate results

% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2020 HörTech gGmbH

% openMHA is free software: you can redistribute it and/or modify
% it under the terms of the GNU Affero General Public License as published by
% the Free Software Foundation, version 3 of the License.
%
% openMHA is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU Affero General Public License, version 3 for more details.
%
% You should have received a copy of the GNU Affero General Public License, 
% version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

  if exist('OCTAVE_VERSION', 'builtin') ~=0
    pkg load signal
  end
  
  if angle<0 | angle>360
    error('Please input angle between 0 and 360 degrees.')
  end
  c = 340; %speed of sound in air m/s
  srate_upsampled = (1/accuracy)+1; %adaptive sampling rate depending on accuracy required
  f = round(srate_upsampled/srate); %factor with which to upsample signal
  upsampled_signal = resample(signal,f, 1);
  %delay value in samples computed
  delay_val = cosd(angle)*d/c;
  delay_s = delay_val*(f*srate+1);
  trail = zeros(1, round(abs(delay_s)));
  rounded_delay_val = round(abs(delay_s))/(f*srate+1);
  cosine_input = c*rounded_delay_val / d;
  %Floor to keep cosine input under 1 to get physically possible delays
  if cosine_input > abs(1)
    trail = zeros(1, floor(abs(delay_s)));
    rounded_delay_val = floor(abs(delay_s))/(f*srate+1);
    cosine_input = c*rounded_delay_val / d;
  end
  %Delay is added and the signal downsampled
  if delay_s>0
    delayed_signal = [trail, upsampled_signal];
    undelayed_signal = [upsampled_signal, trail];
    rear_mic = resample(delayed_signal,1,f);
    front_mic = resample(undelayed_signal,1,f);
    %Since trail requires a rounded delay_s, a corrected angle is computed from that
    corrected_angle = acosd(cosine_input);
  elseif delay_s<0    
    delayed_signal = [trail, upsampled_signal];
    undelayed_signal = [upsampled_signal, trail];
    front_mic = resample(delayed_signal,1,f);
    rear_mic = resample(undelayed_signal,1,f);
    corrected_angle = acosd(-cosine_input);
  else
    front_mic = signal;
    rear_mic = signal;
    corrected_angle = angle;
  end
  if angle>180 && angle~=270
    corrected_angle=360-corrected_angle;
  end
end
