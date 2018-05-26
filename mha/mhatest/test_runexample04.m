% Execute MHA with example configuration 
% mha/examples/04-prerelease-combination/prerelease_combination.cfg and check
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

function test_runexample04()
  dir = '../examples/04-prerelease-combination/';
  cfg = 'prerelease_combination.cfg';
  inwav = '1speaker_diffNoise_4ch.wav'; 
  outwav = '1speaker_diffNoise_4ch_OUT.wav';
  % we only check the expected levels of input and output sound files.
  expected_channelcounts = [4 2];
  expected_levels = [-23.138  -21.963  -23.445  -22.189 -21.517 -19.710];
  [status, output] = system(['cd ' dir ' && mha "?"read:' cfg ' cmd=start cmd=quit']);
  unittest_teardown(@delete, [dir outwav]);
  assert_equal(0, status);
  indata = audioread([dir inwav]);
  outdata = audioread([dir outwav]);
  assert_equal(expected_channelcounts, [size(indata,2), size(outdata,2)]);
  actual_levels = 10*log10(mean([indata outdata].^2));
  assert_difference_below(expected_levels, actual_levels, 1e-2);
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
