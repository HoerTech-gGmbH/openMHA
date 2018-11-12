% This function tests that when two TCP clients connect to the MHA,
% and the first client closes its connection, then this does not cause
% MHA to also disconnect the second client connected.
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

function test_multiple_tcp_connections_close_independently
  % create an MHA for this test
  mha_conn1 = mha_start();
  unittest_teardown(@mha_set, mha_conn1, 'cmd', 'quit');

  % create a duplicate handle to the same MHA so that we have multiple
  % simultaneous connections
  mha_conn2 = mha_conn1;

  % different values for the connection timeout will cause the creation
  % of a second connection
  mha_conn2.timeout = mha_conn1.timeout + 1;

  % send something over the first connection 
  mha_set(mha_conn1, 'nchannels_in', 2);

  % These matlab to MHA connections idle out after 1.2 seconds without traffic.
  % During this time, while the first connection is still connected but idle,
  % establish a second connection
  mha_set(mha_conn2, 'nchannels_in', 3);

  % Repeatedly use the second connection with short enough pauses so that the
  % second connection will not idle out, but the first connection will.
  pause(0.8); % first connection is now at least 0.8 seconds idle
  mha_set(mha_conn2, 'nchannels_in', 4);
  pause(0.8); % first connection is now at least 1.6 seconds idle and closed
  mha_set(mha_conn2, 'nchannels_in', 5);

  % The last mha_set would have triggered an error if also the second connection
  % had been closed by the MHA in response to matlab closing the first
  % connection because of the unexpected closing of the second TCP connection
  % (reported as "broken pipe").
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
