%% This file is part of the HörTech Open Master Hearing Aid (openMHA)
%% Copyright © 2021 HörTech gGmbH
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

function test_transducers_softclipper

    % Create MHA instance for testing: transducers+identity, Parser IO
    mha=mha_start;
    unittest_teardown(@mha_set,mha,'cmd','quit');
    mha_set(mha,'instance','test_transducers_softclipper');
    mha_set(mha,'fragsize', 2);
    mha_set(mha,'nchannels_in', 1);
    mha_set(mha,'mhalib','transducers');
    mha_set(mha,'mha.calib_in.peaklevel',100);
    mha_set(mha,'mha.calib_out.peaklevel',100);
    mha_set(mha,'mha.calib_out.do_clipping',true);
    mha_set(mha,'mha.calib_out.softclip.tau_attack',0);
    mha_set(mha,'mha.calib_out.softclip.slope',0);
    mha_set(mha,'mha.calib_out.softclip.threshold',0.5);
    mha_set(mha,'mha.plugin_name', 'identity')
    mha_set(mha,'iolib','MHAIOParser');
    mha_set(mha,'cmd','start');

    % Feed large sample. output should be 0.5
    mha_set(mha, 'io.input', [1 1]);
    assert_equal([0.5 0.5], mha_get(mha, 'io.output'));

    % Changing softclip parameter during signal processing should have effect
    mha_set(mha,'mha.calib_out.softclip.threshold',0.25)
    mha_set(mha, 'io.input', [1 1]);
    assert_equal([0.25 0.25], mha_get(mha, 'io.output'));
