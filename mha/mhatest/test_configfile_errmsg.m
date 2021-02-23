% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2021 HörTech gGmbH

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

function test_configfile_errmsg

  % start MHA instance without configuration
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');

  % create two configuration files:
  f = fopen('test_configfile_errmsg_EMPTY.cfg', 'w'); % file with 1 empty line
  fprintf(f, '\n');
  fclose(f);
  unittest_teardown(@delete, 'test_configfile_errmsg_EMPTY.cfg');

  f = fopen('test_configfile_errmsg_NONempty.cfg', 'w'); % nonempty file
  fprintf(f, '%s\n%s\n%s\n', ...
          '?read:test_configfile_errmsg_EMPTY.cfg', ...% reading empty is ok
          'nchannels_in = 2',                       ...% setting channels is ok
          'cmd = start');                              % error: no plugin loaded
  fclose(f);
  unittest_teardown(@delete, 'test_configfile_errmsg_NONempty.cfg');

  expected_msg = 'while parsing "test_configfile_errmsg_NONempty.cfg" line 3';
  actual_msg = '';
  try % reading configuration files will generate an error response
    mha_query(mha,'','read:test_configfile_errmsg_NONempty.cfg')
  catch error
    actual_msg = error.message;
  end
  % expected MHA error message is contained exactly once in matlab error message
  assert_equal(1, numel(findstr(actual_msg, expected_msg)), ...
               sprintf('%s\n--->"%s"<---\n%s\n--->"%s"<---\n',...
                       'Expected error message to contain', ...
                       expected_msg, ...
                       'but it reads:', ...
                       actual_msg));
% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
