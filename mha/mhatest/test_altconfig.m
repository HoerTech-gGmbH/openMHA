%% This function tests plugin altconfig for regressions
%%
%% This file is part of the HörTech Open Master Hearing Aid (openMHA)
%% Copyright © 2023 Hörzentrum Oldenburg gGmbH
%%
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

function test_altconfig
  % Basic MHA setup:
  dsc.instance = 'test_altconfig';
  dsc.mhalib =  'altconfig';
  % Load simple plugin into altconfig for testing.
  dsc.mha.plugin_name = 'gain';
  % Prepare two simple example configurations with easy names.
  dsc.mha.algos = {'attenuation', 'amplification'};
  dsc.mha.attenuation='gain=-6';
  dsc.mha.amplification='gain=+6';

  % Test in MHA:
  mha=mha_start;
  mha_set(mha,'',dsc);
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');

  % Assert that selectall and select are the last entries in the plugin config.
  % Needed for proper order of entries in output of ?save and ?saveshort.
  config_entries = mha_query(mha, 'mha', 'entries');
  expected_last_two_entries = ' selectall select]';
  size_for_last_two_entries = length(expected_last_two_entries);
  if length(config_entries) < size_for_last_two_entries
    error('Plugin config does not have enough entries');
  end
  assert_equal(expected_last_two_entries, ...
               config_entries(end-size_for_last_two_entries+1:end));
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
