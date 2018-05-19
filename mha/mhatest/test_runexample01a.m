## This file is part of the HörTech Open Master Hearing Aid (openMHA)
## Copyright © 2018 HörTech gGmbH
##
## openMHA is free software: you can redistribute it and/or modify
## it under the terms of the GNU Affero General Public License as published by
## the Free Software Foundation, version 3 of the License.
##
## openMHA is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Affero General Public License, version 3 for more details.
##
## You should have received a copy of the GNU Affero General Public License, 
## version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

## -*- texinfo -*- 
## @deftypefn {} test_runexample01a ()
## Execute MHA with example configuration 
## mha/examples/01-dynamic-compression/example_dc.cfg and check that the 
## expected output sound file is produced.
## @seealso{}
## @end deftypefn

## Author: tobias <tobias@openmha.com>
## Created: 2018-05-19

function test_runexample01a()
  dir = '../examples/01-dynamic-compression/';
  cfg = 'example_dc.cfg';
  inwav = '1speaker_diffNoise_2ch.wav'; 
  outwav = '1speaker_diffNoise_2ch_OUT.wav';
  % we only check the expected levels of input and output sound files.
  expected_levels = [-23.138  -21.963  -41.494  -41.065];
  [status, output] = system(['cd ' dir ' && mha "?"read:' cfg ' cmd=start cmd=quit']);
  unittest_teardown(@delete, [dir outwav]);
  assert_equal(0, status);
  indata = audioread([dir inwav]);
  outdata = audioread([dir outwav]);
  actual_levels = 10*log10(mean([indata outdata].^2));
  assert_difference_below(expected_levels, actual_levels, 1e-2);
end
