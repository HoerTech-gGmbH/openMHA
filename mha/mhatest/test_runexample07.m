% Execute MHA with example configuration 
% examples/07-single-channel-noise-reduction/single-channel-noise-reduction/ and check
% that the expected output sound file is produced.
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

function test_runexample07()
  dir = '../../examples/07-single-channel-noise-reduction/';
  cfg = 'single_channel_noise_reduction.cfg';
  inwav = '1speaker_diffNoise_2ch_16k.wav';
  outwav = 'denoised.wav';
  % we only check the expected levels of input and output sound files.
  expected_channelcounts = [2 2];
  expected_levels = [-23.0316  -21.8570  -31.1822  -27.0676];

  % execute mha with the given config file in the example directory,
  % start processing, quit
  old_dir = chdir(dir);
  unittest_teardown(@chdir, old_dir);
  mha = mha_start;
  mha_query(mha,'',['read:' cfg]);
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','quit');

  unittest_teardown(@delete, [outwav]);

  indata = audioread([inwav]);
  outdata = audioread([outwav]);
  assert_equal(expected_channelcounts, [size(indata,2), size(outdata,2)]);
  actual_levels = 10*log10(mean([indata outdata].^2));
  assert_difference_below(expected_levels, actual_levels, 1e-2);
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
