% Execute MHA with example configuration 
% mha/examples/06-binaural-beamformer/beamformer.cfg and check
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

function test_runexample06()
  dir = '../examples/06-binaural-beamformer/';
  cfg = 'beamformer.cfg';
  inwav = '2speaker_4ch_16k.wav'; 
  outwav = 'beamformer_out.wav';
  % we only check the expected levels of input and output sound files.
  expected_channelcounts = [4 2];
  expected_levels = [-28.8514  -29.5057  -28.5562  -29.8354  -30.9217  -28.4742];

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
