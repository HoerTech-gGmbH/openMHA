% This file is part of the HörTech Open Master Hearing Aid (openMHA);
% Copyright © 2020 HörTech gGmbH;

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

% Regression test of TCP communication timeout
function test_mhactl_java_timeout

  mha = mha_start;
  unittest_teardown(@mha_set,mha,'cmd','quit');

  % We tell mha to sleep 3 seconds before responding.
  sleep_time = 3;
  % But we tell Matlab to give up waiting for a response earlier, after 1 s.
  mha.timeout = 1;
  t0 = tic;
  try ;
    mha_set(mha,'sleep', sleep_time); % should be interrupted by timeout
  catch
  end
  actual_timeout = toc(t0); % How many seconds Matlab has spent in mha_set.

  % actual_timeout will normally be a bit longer than mha.timeout,
  % but can also be a little bit shorter (Observation on Windows).
  % It should always be significantly shorter than the configured sleep time.
  assert_all([(actual_timeout < sleep_time  - 1.0) % shorter than sleep time
              (actual_timeout < mha.timeout + 1.0) % Can be longer than timeout
              (actual_timeout > mha.timeout - 0.1) % Can be little bit shorter
             ]);
end
