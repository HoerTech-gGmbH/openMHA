%% This function tests plugin MHAIODummy for regressions.
%%
%% This file is part of the HörTech Open Master Hearing Aid (openMHA)
%% Copyright © 2022 Hörzentrum Oldenburg gGmbH

%% openMHA is free software: you can redistribute it and/or modify
%% it under the terms of the GNU Affero General Public License as published by
%% the Free Software Foundation, version 3 of the License.
%%
%% openMHA is distributed in the hope that it will be useful,
%% but WITHOUT ANY WARRANTY; without even the implied warranty of
%% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%% GNU Affero General Public License, version 3 for more details.
%%
%% You should have received a copy of the GNU Affero General Public License, 
%% version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.
%%
function test_mhaiodummy
  % Basic MHA setup
  dsc.instance = 'test_mhaiodummy';
  dsc.mhalib =  'identity';
  dsc.iolib = 'MHAIODummy';
  mha=mha_start;
  mha_set(mha,'',dsc);
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');

  % Start MHA signal processing thread
  mha_set(mha,'cmd','start');
  % previous command changed state to 'starting', and it will automatically
  % advance to 'running' when the dummy plugin has produced the first block of
  % audio data in the processing thread. Synchronization by waiting is only
  % ok for tests like this.
  pause(0.25);
  assert_equal('running', mha_get(mha,'state'));

  % Stop signal processing thread and release resources
  mha_set(mha,'cmd','release');
  pause(0.25);
  assert_equal('unprepared', mha_get(mha,'state'));

  % Start MHA signal processing thread again
  mha_set(mha,'cmd','start');
  pause(0.25);
  assert_equal('running', mha_get(mha,'state'));
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
