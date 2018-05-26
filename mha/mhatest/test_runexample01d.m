# Execute MHA with example configuration 
# mha/examples/01-dynamic-compression/dynamiccompression_live.cfg and check
# that the  expected audio connections in Jack are made, and that this
# MHA can be fitted with the fitting tool.

# This file is part of the HörTech Open Master Hearing Aid (openMHA)
# Copyright © 2018 HörTech gGmbH
#
# openMHA is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# openMHA is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License, version 3 for more details.
#
# You should have received a copy of the GNU Affero General Public License, 
# version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

function mha = test_runexample01d()

 % This test does live sound I/O. Only execute when specifically requested.
 global execute_live_tests;
 if execute_live_tests
  
  dir = '../examples/01-dynamic-compression/';
  cfg = 'dynamiccompression_live.cfg';

  % start jack asynchronously
  jack_pid = system('jackd -d alsa -r 44100 -p 64', false, 'async');
  assert_all(jack_pid > 0);
  % The PID we got is that of the shell that started jack, useless for killing jackd
  unittest_teardown(@system, 'killall -9 jackd');
  assert_all(wait_for_jack(2));
  
  mha = mha_start;
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  mha_query(mha,'',['read:' dir cfg]);
  
  expect_mha_jack_connections('');
  mha_set(mha,'cmd','start');
  expect_mha_jack_connections(sprintf(['MHA:in_1  system:capture_1\n' ...
                                       'MHA:in_2  system:capture_2\n' ...
                                       'MHA:out_1  system:playback_1\n' ...
                                       'MHA:out_2  system:playback_2\n']));

  % TODO: now check that we can fit the MHA with some fitting rules and audiograms
 end 
end

function success = wait_for_jack(timeout)
  success = false;
  [status, output] = system(sprintf('jack_wait -w -t %d', timeout));
  if (status == 0)
    if isequal(output, sprintf('server is available\n'))
      success = true;
    end
  end
end

function expect_mha_jack_connections(expected)
  [status, actual] = ...
    system('jack_lsp -c |sed -e :a -e ''$!N;s/\n //;ta'' -e "P;D" | grep ^MHA');
  assert_equal(expected, actual);
end
