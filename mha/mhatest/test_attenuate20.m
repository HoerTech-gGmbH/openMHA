%% This function tests plugin attenuate by checking the attenuation
%% applied by this plugin.
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
function test_attenuate20
    % create a new MHA process
    mha = mha_start();

    % ensure MHA is exited after the test
    unittest_teardown(@mha_set, mha, 'cmd', 'quit');

    % configure the new MHA to attenuate 3 channels
    channels=3;fragsize=64;
    mha_set(mha, 'instance', 'test_attenuate20');
    mha_set(mha, 'nchannels_in', channels);
    mha_set(mha, 'fragsize', fragsize);
    mha_set(mha, 'mhalib', 'attenuate20');
    mha_set(mha, 'iolib', 'MHAIOParser');
    mha_set(mha, 'cmd', 'start');

    % create test signal.

    gain=-20; % 20 db attenuation, also used as seed next:
    s_in = repeatable_rand(channels,fragsize,gain) - 0.5;

    % Process
    mha_set(mha,'io.input',s_in);
    s_out = mha_get(mha,'io.output');

    % Compare
    actual = s_out;
    expected = s_in * 10^(gain/20);
    assert_difference_below(expected, actual, 1e-8);
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
