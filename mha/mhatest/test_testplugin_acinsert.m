% This function tests one aspect of the test plugin: AC variable insertion.
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

function test_testplugin_acinsert
    mha = mha_start();
    unittest_teardown(@mha_set, mha, 'cmd', 'quit');
    mha_set(mha, 'instance', 'test_testplugin_acinsert');
    mha_set(mha, 'mhalib', 'testplugin');
    
    % testplugin needs another plugin loaded in order to prepare
    mha_set(mha, 'mha.plugin_name', 'identity');

    % publish one AC variable before prepare
    mha_set(mha, 'mha.ac.data_type', 'MHA_AC_MHAREAL');
    mha_set(mha, 'mha.ac.float_data', 0.5);
    mha_set(mha, 'mha.ac.insert_var', 'acname');
    
    % check insertion: delete local cache, then retrieve variable from AC space
    mha_set(mha, 'mha.ac.float_data', []);
    assert_not_equal(0.5, mha_get(mha, 'mha.ac.float_data'));
    mha_set(mha, 'mha.ac.get_var', 'acname');
    assert_equal(0.5, mha_get(mha, 'mha.ac.float_data'));
    
    % Updating existing variable after prepare is permitted
    mha_set(mha, 'mha.prepare', true);
    mha_set(mha, 'mha.ac.insert_var', 'acname');
    
    % Creating a new variable after prepare is not permitted
    caught_error = '';
    try
        mha_set(mha, 'mha.ac.insert_var', 'new');
    catch err
        caught_error = err.message;
    end
    % result of strfind is nonempty if the substring is found in message
    assert_all(~isempty(strfind(caught_error, ...
      'Attempt to create AC variable "new" during live signal processing.')));
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
