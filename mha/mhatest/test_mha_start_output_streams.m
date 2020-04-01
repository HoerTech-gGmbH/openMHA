% This function tests accessing output and error streams from MHA instances
% started with mha_start.
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2020 HörTech gGmbH

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

function test_mha_start_output_streams

  % Test 1 : accessing the stdout stream of the new MHA process
  [mha,process] = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');

  if isempty(process)
    % This platform does not support returning the Java process object, skip
    return;
  end

  stdout = javaObject('de.hoertech.mha.control.StreamGobbler', ...
                      process.getInputStream, ... % InputStream is mha's stdout.
                      true);                      % Store received text.
  % we need to let mha get a chance to print the string to its output
  pause(0.1);
  assert_all(~isempty(strfind(stdout.get(),'Copyright (c) 2005-2020 HoerTech')))

  % Test 2 : discarding stdout of the new MHA process but accessing stderr
  [mha,process] = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');

  stdout = javaObject('de.hoertech.mha.control.StreamGobbler', ...
                      process.getInputStream, ... % InputStream is mha's stdout.
                      false);                     % Discard received text.
  % we need to let mha get a chance to print the string to its output
  pause(0.1);
  assert_all(isempty(stdout.get())); % stdout is empty because it does not
  %                                  % store, not because it has not received.

  stderr = javaObject('de.hoertech.mha.control.StreamGobbler', ...
                      process.getErrorStream(), true); % Store received text.
  % We have not caused an error yet
  assert_all(isempty(stderr.get()));

  % Configure MHA to encounter an error detected during signal processing.
  % This (normally asynchronous) error will not be reported by mha_set, but
  % is stored in asyncerror and printed on stderr.
  mha_set(mha,'fragsize','1');
  mha_set(mha,'iolib','MHAIOParser');
  mha_set(mha,'mhalib','route');
  % route should use the first audio channel of AC variable 'AC' as output.
  mha_set(mha,'mha.out',{'AC:0'});
  mha_set(mha,'cmd','start');

  % This will fail in the first processing block when plugin route realises that
  % AC variable 'AC' does not exist. Right now, everything is still fine:
  assert_all(isempty(stderr.get()));
  assert_all(isempty(mha_get(mha, 'asyncerror')));

  % Process first audio signal block
  mha_set(mha,'io.input',1);
  % Now the same error should be present in asyncerror and stderr
  expected_asyncerror = ...
    sprintf(['(mhapluginloader) Error in module "route:route":\n'...
             '(mha_algo_comm) AC error (AC): '...
             'Invalid or non-existing variable name']);
  actual_asyncerror = mha_get(mha, 'asyncerror');
  expected_stderr = sprintf('MHA Error: %s\n', expected_asyncerror);
  actual_stderr = stderr.get();

  assert_equal(expected_asyncerror, actual_asyncerror);
  assert_equal(expected_stderr, actual_stderr);

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
