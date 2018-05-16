function test_wave2spec_windowpos
% Testing the analysis part of overlap-add with different positions of 
% the analysis window.
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2018 HörTech gGmbH

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

  
% mha description
dsc.instance = 'test_wave2spec_windowpos';
dsc.fragsize = 64;
dsc.nchannels_in = 1;
dsc.mhalib = 'mhachain';
dsc.iolib = 'MHAIOParser';
dsc.mha.algos = {'wave2spec','save_spec','acmon','spec2wave'};
dsc.mha.wave2spec.wndlen = 128;
dsc.mha.wave2spec.fftlen = 364;

% Test three different analysis window positions in the fft buffer:
% centered, at the beginning, at the end
windowpositions = [0.5, 0, 1];

% For the three window positions compute the FFT here in octave and compare
% the spectrum with the one computed in MHA
for windowpos = windowpositions
  dsc.mha.wave2spec.wndpos = windowpos;

  % pseudo-random input signal
  input_signal = repeatable_rand(dsc.fragsize, dsc.nchannels_in, 0) - 0.5;
  input_signal = repmat(input_signal, 2, 1);

  % create analysis window and zeropadding as we expect the MHA to do
  [hanning, zero_padding_1, zero_padding_2] = ...
      mha_hanning(dsc.mha.wave2spec.wndlen, dsc.mha.wave2spec.fftlen, ...
	windowpos);

  % Compute the expected spectrum in octave or matlab
  matlab_spec = fft([zero_padding_1
                     input_signal .* hanning
                     zero_padding_2]).';

  % Positive frequencies only
  matlab_spec = matlab_spec(1:(floor(dsc.mha.wave2spec.fftlen / 2)+1));

  % configure the MHA to process the same signal with the same
  % analysis window setup
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  mha_set(mha,'',dsc);
  mha_set(mha,'cmd','start');

  % process the signal
  mha_process_by_parser(mha, input_signal')';

  % input signal length is twice the hop size and once the window length.
  % The last FFT that has been performed is the one where the complete
  % input signal was under the analysis window.
  mha_spec = mha_get(mha, 'mha.acmon.save_spec');

  % Compare and allow for numeric rounding errors.
  assert_almost(matlab_spec, mha_spec, 3.6e-5, ...
                sprintf('Failure while testing window position %f', ...
                        windowpos));
end
