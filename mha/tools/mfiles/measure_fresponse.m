function [fresponse] = measure_fresponse(jack_port, sampling_rate, mha)
% function measure_fresponse(jack_port, sampling_rate [,mha])
% jack_port:     Name of the sound card's output port where the sound output
%                hardware is connected
% sampling_rate: sampling rate of sound card in Hz
% mha:           Optional: struct with host and port of a remote
%                unconfigured mha instance.  *If not given* then a local mha
%                instance will be created on the computer that executes this
%                mfile.  This function will load the sine plugin and the Jack io
%                library and start the mha processing to produce test tones.
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2018 2019 HörTech gGmbH
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

  % frequencies in Hz that we will produce test tones at. Adjust as required.
  fresponse.Frequencies = [125 250 500 1000 1500 2000 3000 4000 6000 8000];

  % Real-Ear-Unaided-Gains in dB for the above frequencies, taken from Dillon 2012.
  fresponse.REUGdB = [0,1,2,3,5,12,16,14,4,2];

  % Correction vector, initially all conrrections are 0, no correction
  fresponse.correctionsdB = zeros(size(fresponse.Frequencies));

  % result vector, initially very low values for safety
  fresponse.dBFSfor80dB = ones(size(fresponse.Frequencies)) * -200;

  % sampling rate is required for filter creation
  fresponse.sampling_rate = sampling_rate;

  if nargin < 3
    mha = mha_start;
  end

  s.nchannels_in = 1;
  s.fragsize=3072; % least common multiple of 1024 and 24
  s.srate = sampling_rate;
  s.mhalib = 'transducers';
  s.mha.calib_out.peaklevel = [0];
  s.mha.calib_in.peaklevel = [-200];
  s.mha.plugin_name = 'sine';
  s.mha.sine.channels = [0];
  s.iolib='MHAIOJackdb';
  s.io.con_out={jack_port};
  mha_set(mha,'',s);
  mha_set(mha,'cmd','start');

  % measure levels
  for fi = 1:length(fresponse.Frequencies)
    f = fresponse.Frequencies(fi);
    fprintf('Starting measurements at %d Hz\n', f);
    mha_set(mha, 'mha.sine.lev', -100);
    mha_set(mha, 'mha.sine.f', f);
    level = -51;
    change = 1;
    while change
      level = level + change;
      if (level > -3)
        display('Warning: Cannot produce sinusoids > -3dB re FS RMS, limiting to -3dB')
        level = -3;
      end
      mha_set(mha, 'mha.sine.lev', level);
      change = input(sprintf('\n\nProducing sinusoid with %dHz at %f dB re FS\nWe try to achieve 80 dB coupler level.\nwhat change in dB is required? ', f, level));
    end
    fresponse.dBFSfor80dB(fi) = level;
    fprintf('Noting that we need %f dBFS at %d Hz to produce 80 dB coupler level.\n', level, f);
  end

  mha_set(mha,'cmd','quit');
end
