function test_wave2spec_spec2wave_delay
% Testing the delay introduced by STFT analysis and resynthesis is as expected.
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

  dsc.instance = 'test_wave2spec_spec2wave_delay';
  dsc.nchannels_in = 2;
  dsc.mhalib = 'mhachain';
  dsc.mha.algos = {'wave2spec','spec2wave'};

  mha = mha_start();
  mha_set(mha, '', dsc);
  mha_set(mha, 'iolib', 'MHAIOParser');
  mha_set(mha, 'cmd', 'start');

  unittest_teardown(@mha_set, mha, 'cmd', 'quit');

  input_signal = 0.01*repeatable_rand(2,1000,1);
  output_signal = mha_process_by_parser(mha, input_signal);

  % calculate delay:
  fftlen = mha_get(mha, 'mha.wave2spec.fftlen');
  wndlen = mha_get(mha, 'mha.wave2spec.wndlen');
  hopsize = mha_get(mha, 'fragsize');

  % for symmetric zero-padding, the delay is
  delay = ((fftlen + wndlen)/2 - hopsize);

  % Make sure we are testing with meaningful default values
  assert_equal(512, fftlen);
  assert_equal(400, wndlen);
  assert_equal(200, hopsize);
  assert_equal(256, delay);

  % Without numeric errors, the samples 1:delay would be exactly zero;
  % They have small non-zero values here because the inverse FFT bleeds
  % rounding errors from the first non-zero samples also into the zero
  % padded areas.
  assert_difference_below(zeros(dsc.nchannels_in,delay),...
			  output_signal(:,1:delay), ...
			  1e-7);
  % the output should be the delayed input, with very low numeric
  % difference:
  assert_difference_below(input_signal(:,1:end-delay), ...
			  output_signal(:,delay+1:end), ...
			  1e-8);
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
