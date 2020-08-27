%% This function tests plugin MHAIOFile for regressions
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
function test_mhaiofile_empty_filenames_error_message
% Checks that MHA raises understandable error messages if io.in or io.out
% are not given.
  
  % Basic MHA setup
  dsc.instance = 'test_mhaiofile_empty_filenames_error_message';
  dsc.mhalib =  'identity';
  dsc.iolib = 'MHAIOFile';
  mha=mha_start;
  mha_set(mha,'',dsc);
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');


  % we have not set io.in or io.out here, therefore preparing should fail
  has_understandable_error_message_for_empty_input_filename = [];
  has_understandable_error_message_for_empty_output_filename = [];

  % Check error message for empty input filename
  try
    mha_set(mha,'cmd','prepare');
  catch err
    % result of strfind is nonempty if the substring is found in message
    has_understandable_error_message_for_empty_input_filename = ...
      strfind(err.message, '(MHAIOFile) Input filename not provided');
  end
  
  % Check error message for empty output filename
  mha_set(mha,'io.in','no-such-file,-just-a-nonempty-name')
  try
    mha_set(mha,'cmd','prepare');
  catch err
    % result of strfind is nonempty if the substring is found in message
    has_understandable_error_message_for_empty_output_filename = ...
      strfind(err.message, '(MHAIOFile) Output filename not provided');
  end

  % Check that both error message are understandable
  assert_all([
    ~isempty(has_understandable_error_message_for_empty_input_filename)
    ~isempty(has_understandable_error_message_for_empty_output_filename)
              ]);
end
