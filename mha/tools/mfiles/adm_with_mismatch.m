% This script computes delayed signal between two microphones and introduces a
% mismatch which is then used to simulate the directional behavior of ADM at
% different mismatches 
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2020 HörTech gGmbH
%
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

%White noise with parameters the same as microphone array
srate = 24000;
t = 0:1 / (srate):1;
signal = repeatable_rand(1, srate, 26129) - 0.5;
d=0.01;
angle=0:2:358;

accuracy=zeros(1, length(angle));
corrected_angles=zeros(1, length(angle));

%Accuracy necessary to compute with approximately 2 degree resolution
accuracy(1:3)=1e-8;
accuracy(4:29)=1e-7;
accuracy(30:67)=1e-6;
accuracy(68:88)=1e-7;
accuracy(89:90)=1e-8;
accuracy(91:180)=accuracy(1:90);

%Different mismatches introduced where the rear microphone (2nd channel) is attenuated
mismatch=[0 0.5 1 2 3 5]; %in dB
f=10.^(-mismatch/20);
mkdir adm_test_audio %Folder to store all output files

%Loop computes the delayed signal through all angles and mismatches
for i=1:length(angle)
  [front, rear, corrected_angle]=delayed_signal_generator(signal, srate, angle(i), d, accuracy(i));
  corrected_angles(i)=corrected_angle;
  for j=1:length(mismatch)
    output=[front; f(j)*rear]';
    audiowrite(sprintf('adm_test_audio/%ddB_%d_deg.wav', mismatch(j), corrected_angle), output, srate);
  end
end
