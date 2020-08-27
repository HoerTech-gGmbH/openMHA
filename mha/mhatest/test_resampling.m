% This function tests plugin resampling
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2018 HörTech gGmbH

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

function test_resampling
  dsc.instance = 'test_resampling';
    dsc.nchannels_in = 1;
    dsc.fragsize = 64;
    dsc.srate = 48000;

    dsc.mhalib = 'resampling';
    dsc.iolib = 'MHAIOParser';

    dsc.mha.plugin_name = 'identity';
    dsc.mha.srate = 16000;
    dsc.mha.fragsize = 20;

    mha = mha_start();
    
    unittest_teardown(@mha_set, mha, 'cmd', 'quit');

    mha_set(mha,'',dsc);
%    mha_set(mha, 'iolib', 'MHAIOParser'); %load iolib
    mha_set(mha, 'cmd', 'start');
    
    input_signal = [1:960]*1e-3;
    
    output_signal = mha_process_by_parser(mha, input_signal);
    assert_equal('', mha_get(mha,'asyncerror'));
    assert_difference_below(input_signal(15:806), output_signal(92+14:897), 4e-5);
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
