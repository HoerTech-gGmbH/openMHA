% This function tests that multiple TCP connections are served by the MHA
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

function test_multiple_tcp_connections
  % create an MHA for this test
  mha_conn1 = mha_start();
  unittest_teardown(@mha_set, mha_conn1, 'cmd', 'quit');

  % create a duplicate handle to the same MHA so that we have multiple
  % simultaneous connections
  mha_conn2 = mha_conn1;

  % different values for the connection timeout will cause the creation
  % of a second connection
  mha_conn2.timeout = mha_conn1.timeout + 1;

  % now measure how long it takes to send simple commands over both
  % connections alternatingly.
  start_time = tic;
  mha_set(mha_conn1, 'nchannels_in', 2);
  mha_set(mha_conn2, 'nchannels_in', 3);
  mha_set(mha_conn1, 'nchannels_in', 4);
  mha_set(mha_conn2, 'nchannels_in', 5);
  duration = toc(start_time);

  % These matlab to MHA connections idle out after 1.2 seconds without traffic.
  % Without MHA accepting multiple connections, sending 4 commands over
  % alternating connections would have taken at least 3 idle-out-durations,
  % 3*1.2 seconds. With multiple connections, it will be much faster.
  assert_difference_below(0, duration, 2); % asserts duration < 2
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
