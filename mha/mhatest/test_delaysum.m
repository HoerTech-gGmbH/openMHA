## This file is part of the HörTech Open Master Hearing Aid (openMHA)
## Copyright © 2017 HörTech gGmbH
##
## openMHA is free software: you can redistribute it and/or modify
## it under the terms of the GNU Affero General Public License as published by
## the Free Software Foundation, version 3 of the License.
##
## openMHA is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU Affero General Public License, version 3 for more details.
##
## You should have received a copy of the GNU Affero General Public License, 
## version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

## Author: Frasher Loshaj 
## Created: 2017-04-06

## This function tests plugin test_delaysum by comparing 
## the delay and sum of two random arrays with the plugin 
## output
function [retval] = test_delaysum 
    dsc.instance = 'test_delaysum';
    dsc.nchannels_in = 2;
    dsc.fragsize = 200;
    
    dsc.mhalib = 'delaysum';
    % randon mumber from 1 to 20
    dl = floor(1+19*rand);
    dsc.mha.delay = [0 dl];
    
    mha = mha_start();
    
    unittest_teardown(@mha_set, mha, 'cmd', 'quit');

    mha_set(mha,'',dsc);
    mha_set(mha, 'iolib', 'MHAIOParser'); %load iolib
    mha_set(mha, 'cmd', 'start');
    
    % random sequence with values from -0.5 to 0.5
    ch1_input = (rand(1,200)-0.5);
    ch2_input = (rand(1,200)-0.5);
    
    % two-channel delta at 10 - input signal
    in_sig = [ch1_input; ch2_input];
    
    % required output signal
    out_sig = ch1_input + [zeros(1,dl) ch2_input(1:(200-dl))];
    
    mha_set(mha, 'io.input', in_sig);
    output_signal = mha_get(mha, 'io.output');
    assert_equal(round(output_signal)/100000, round(out_sig)/100000);
endfunction
