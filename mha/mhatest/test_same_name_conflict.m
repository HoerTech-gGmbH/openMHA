%% This function tests plugin fshift by checking the frequency shift
%% applied by this plugin.
%%
%% This file is part of the HörTech Open Master Hearing Aid (openMHA)
%% Copyright © 2021 HörTech gGmbH

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

function test_same_name_conflict

  % create MHA instance
  mha = mha_start();

  % ensure MHA is exited after the test
  unittest_teardown(@mha_set,mha,'cmd','quit');

  % configure MHA to provoke an error
  mha_set(mha,'mhalib','mhachain');
  correct_error_handling = false;
  try
    mha_set(mha,'mha.algos','[noise save_wave:noise]');
  catch err
    if ~isempty(strfind(err.message,'(mha_parser) The entry "noise" already exists.'))
      correct_error_handling = true;
    end
  end

  assert_equal(true,correct_error_handling,'Same name conflict was not handled as expected!');
