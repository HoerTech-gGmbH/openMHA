%% This file is part of the HörTech Open Master Hearing Aid (openMHA)
%% Copyright © 2020 HörTech gGmbH
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

function test_acsave_double_precision
  
  % Test that acsave saves ac variables of type double with sufficient number
  % of digits in all target formats.

  basename = 'data_acsave_double_precision_data';
  formats =    {'txt', 'm', 'mat4'};
  extensions = {'.txt','.m','.mat'};
  num_tests = length(formats);

  % The binary representation of these numbers differ when stored as doubles,
  % and they need at least 17 significant decimal digits to differenciate
  % between them in decimal representation.
  numbers = [12.345678901234546, 12.345678901234553];
  
  for test = [3 2 1] % order does not matter, but mat4 has worked before T1248
    format = formats{test};

    % Register data output file for cleanup after test
    filename = [basename extensions{test}];
    fclose(fopen(filename, 'w'));
    unittest_teardown(@delete, filename);

    % Set up mha to store AC variable of type double as data file with acsave.
    mha = mha_start;
    unittest_teardown(@mha_set, mha, 'cmd', 'quit');
    mha_set(mha,'fragsize',1);
    mha_set(mha,'iolib','MHAIOParser');
    mha_set(mha,'mhalib','mhachain');
    mha_set(mha,'mha.algos',{'double2acvar','acsave'});
    mha_set(mha,'mha.acsave.name',filename);
    mha_set(mha,'mha.acsave.fileformat',formats{test});
    mha_set(mha,'cmd','prepare');
    mha_set(mha,'mha.double2acvar',sprintf('%.17g',numbers(1)));
    mha_set(mha,'io.input',0);
    mha_set(mha,'mha.double2acvar',sprintf('%.17g',numbers(2)));
    mha_set(mha,'io.input',0);
    mha_set(mha,'cmd','release');
    assert_equal(numbers, read_numbers(filename, basename, format))
  end
end

function numbers = read_numbers(filename, basename, format)
  numbers = zeros(1,2);
  if isequal(format,'m')
    eval(basename);
    numbers=double2acvar';
  elseif isequal(format,'txt')
    numbers = textread(filename,'','headerlines',1);
    numbers = numbers(1:2)';
  elseif isequal(format,'mat4')
    numbers = load(filename);
    numbers = numbers.double2acvar';
  end
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
