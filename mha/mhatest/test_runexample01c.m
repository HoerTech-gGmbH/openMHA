% Execute MHA with example configuration 
% examples/01-dynamic-compression/example_dc_live.cfg and check
% that the  expected audio connections in Jack are made, and that this
% MHA can be fitted with the fitting tool.
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2018 2019 2021 HörTech gGmbH

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

function mha = test_runexample01c()

 % This test does live sound I/O. Only execute when specifically requested.
 global execute_live_tests;
 if execute_live_tests

  dir = '../../examples/01-dynamic-compression/';
  cfg = 'example_dc_live.cfg';

  % on windows this may work around a failing connection to jack
  if ispc()
    pause(1);
  end
  % start jack asynchronously
  jack_pid = system('jackd -d dummy -r 44100 -p 256', false, 'async');
  assert_all(jack_pid > 0);
  unittest_teardown(@waitpid,jack_pid);
  if ispc()
    unittest_teardown(@system,['taskkill /F /IM jackd.exe']);
  else
    unittest_teardown(@system, 'killall -9 jackd');
  end

  assert_all(wait_for_jack(2));
  
  mha = mha_start;
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  mha_query(mha,'',['read:' dir cfg]);
  
  assert_mha_jack_connections(1,'');

  mha_set(mha,'cmd','start');

  if ispc()
    assert_mha_jack_connections(0,sprintf(['system:capture_1  MHA:in_1\n' ...
                                           'system:capture_2  MHA:in_2\n' ...
                                           'system:playback_1  MHA:out_1\n' ...
                                           'system:playback_2  MHA:out_2\n' ...
                                           'MHA:in_1  system:capture_1\n' ...
                                           'MHA:in_2  system:capture_2\n' ...
                                           'MHA:out_1  system:playback_1\n' ...
                                           'MHA:out_2  system:playback_2\n']));
  else
    assert_mha_jack_connections(0,sprintf(['MHA:in_1  system:capture_1\n' ...
                          'MHA:in_2  system:capture_2\n' ...
                          'MHA:out_1  system:playback_1\n' ...
                          'MHA:out_2  system:playback_2\n']));
  end
  % TODO: now check that we can fit the MHA with some fitting rules and audiograms
 end
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
