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

  mha.timeout = 1;
  t0 = tic;
  try ;
    mha_set(mha,'sleep', 3); % should be interrupted by timeout
  catch
  end
  actual_timeout = toc(t0);

  % We expect actual_timeout to be a little bit longer than mha.timeout.
  % Both 0.5 constants together give the necessary leeway.
  assert_difference_below(mha.timeout+0.5,actual_timeout,0.5);
end
