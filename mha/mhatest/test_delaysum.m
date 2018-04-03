% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2017 HörTech gGmbH
%
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

% Author: Frasher Loshaj 
% Created: 2017-04-06

% This function tests plugin test_delaysum by comparing 
% the delay and sum of two cosine arrays with the plugin 
% output
function [retval] = test_delaysum 
    dsc.instance = 'test_delaysum';
    dsc.nchannels_in = 2;
    dsc.fragsize = 200;
    
    dsc.mhalib = 'delaysum';
    % fixed delay of 15 samples
    dl = 15;
    dsc.mha.delay = [0 dl];
    
    mha = mha_start();
    
    unittest_teardown(@mha_set, mha, 'cmd', 'quit');

    mha_set(mha,'',dsc);
    mha_set(mha, 'iolib', 'MHAIOParser'); %load iolib
    mha_set(mha, 'cmd', 'start');
    
    % range
    x = [1:dsc.fragsize];
    
    % cosine input sequences
    ch1_input = 0.5*cos(pi*0.1*x);
    ch2_input = 0.5*cos(pi*0.2*x);
    
    % two-channel delta at 10 - input signal
    in_sig = [ch1_input; ch2_input];
    
    % required output signal
    out_sig = ch1_input + [zeros(1,dl) ch2_input(1:(200-dl))];
    
    mha_set(mha, 'io.input', in_sig);
    output_signal = mha_get(mha, 'io.output');
    assert_almost(output_signal,out_sig,1e-6);
end
