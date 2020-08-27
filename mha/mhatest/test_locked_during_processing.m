%% This function tests that our STFT plugins overlapadd, wave2spec and 
%% spec2wave lock their variables before processing and unlock them again
%% after processing.
%%
%% This file is part of the HörTech Open Master Hearing Aid (openMHA)
%% Copyright © 2020 HörTech gGmbH

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
function test_locked_during_processing()
  % allocate an mha instance for the test
  mha = mha_start;
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');

  % load the plugins under test into the mha instance
  mha_set(mha,'instance','test_locked_during_processing');
  mha_set(mha,'iolib','MHAIOParser');
  mha_set(mha,'mhalib','mhachain');
  mha_set(mha,'mha.algos',{'wave2spec','spec2wave','overlapadd'});
  mha_set(mha,'mha.overlapadd.plugin_name','identity');

  % Check that these plugins lock their variables during signal processing
  check_plugins_lock_variables(mha, {'mha.wave2spec','mha.spec2wave','mha.overlapadd'});
end

function check_plugins_lock_variables(mha, plugins)
  % Get the writable variables of each plugin and check that they are writable.
  for index = 1:length(plugins)
    mutable_variables{index} = get_mutable_variables(mha, plugins{index});
    test_writability(mha, mutable_variables{index}, true);
  end
  % Prepare MHA for signal processing.
  mha_set(mha,'cmd','prepare');
  % Check that all writable variables are now locked
  for index = 1:length(plugins)
    test_writability(mha, mutable_variables{index}, false);
  end
  % Terminate MHA signal processing
  mha_set(mha,'cmd','release');
  % Check that all writable variables are now unlocked again
  for index = 1:length(plugins)
    test_writability(mha, mutable_variables{index}, true);
  end
end

function mutable_variables = get_mutable_variables(mha, base)
  all_variables = struct2mhacfg(mha_get(mha,base),'',base);
  for index = 1:length(all_variables)
    mutability(index) = get_mutability(mha, all_variables{index});
  end
  mutable_variables = all_variables(mutability);
end

function mutable = get_mutability(mha, assignment)
  mutable = true;
  try
    mhactl_wrapper(mha,assignment);
  catch
    mutable = false;
  end
end

function test_writability(mha, variables, expectation)
  for index = 1:length(variables);
    % disp(sprintf('checking %d for %s', expectation,variables{index}))
    assert_equal(expectation, get_mutability(mha, variables{index}), ...
                 ['unexpected mutability for assignment ', variables{index}]);
  end
end
