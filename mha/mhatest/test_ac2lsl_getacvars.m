% In plugin ac2lsl this function tests AC variable retrieval.
% Sending LSL is not tested here.
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2022 Hörzentrum Oldenburg gGmbH

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

function test_ac2lsl_getacvars
    % Start mha, load plugin, configure.
    mha = mha_start();
    unittest_teardown(@mha_set, mha, 'cmd', 'quit');
    mha_set(mha, 'instance', 'test_ac2lsl_getacvars');
    mha_set(mha, 'mhalib', 'testplugin');
    mha_set(mha, 'mha.plugin_name', 'ac2lsl');
    mha_set(mha, 'mha.ac2lsl.rt_strict', false);
    mha_set(mha, 'mha.ac2lsl.activate', false);

    % Prepare to publishing an AC variable that does not exist in AC space.
    % This should fail. Check error message.
    mha_set(mha, 'mha.ac2lsl.vars', {'no_such_variable'});
    message = '';
    try
        mha_set(mha, 'mha.prepare', true); % should fail, check error message:
    catch err
        message = err.message;
    end
    % result of strfind is nonempty if the substring is found in message
    assert_all(~isempty(strfind(message, ...
      'No algorithm communication variable "no_such_variable".')));
    
    % put two AC variables into AC space
    mha_set(mha, 'mha.ac.data_type', 'MHA_AC_MHAREAL');
    mha_set(mha, 'mha.ac.float_data', 0.5);
    mha_set(mha, 'mha.ac.insert_var', 'var1');
    mha_set(mha, 'mha.ac.insert_var', 'var2');

    % clear vars so that ac2lsl publishes all AC variables
    mha_set(mha, 'mha.ac2lsl.vars', {});
    mha_set(mha, 'mha.prepare', true);
    unittest_teardown(@mha_set, mha, 'mha.prepare', false);
    assert_equal({'var1','var2'}, mha_get(mha, 'mha.ac2lsl.vars'));
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
