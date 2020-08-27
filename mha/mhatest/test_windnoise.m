%% This function tests the windnoise detection in the plugin windnoise 
%% by checking the lowpass_quotient_q and how it changes depending on
%% frequency
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
function test_windnoise()
% This test is to confirm that the lowpass quotient decreases after a
% set threshold frequency.
    % A test signal.
    inwav = 'test_windnoise_in.wav';
    outwav = 'test_windnoise_out.wav';
    unittest_teardown(@delete, outwav);
    %instance for the test
    dsc.instance = 'test_windnoise';
    dsc.srate=48000;
    dsc.fragsize=64;
    dsc.mhalib = 'mhachain';
    dsc.mha.algos = '[overlapadd acsave]';
    dsc.mha.acsave.fileformat='mat4';
    dsc.mha.acsave.name = 'windnoise.mat';
    unittest_teardown(@delete, dsc.mha.acsave.name);
    dsc.mha.acsave.reclen=10; %t is 10 seconds for this test
    dsc.mha.acsave.vars='windnoise_lowpass_quotient';
    dsc.mha.overlapadd.fftlen=256;
    dsc.mha.overlapadd.wnd.len=128;
    dsc.mha.overlapadd.plugin_name='windnoise';
    dsc.mha.overlapadd.windnoise.LowPassCutOffFrequency = 500;
    dsc.iolib='MHAIOFile';
    dsc.io.in=inwav;
    dsc.io.out=outwav;
    
    % time base for generating test signal  
    ts = 1 / dsc.srate;
    t=0:ts:10;
    
    % linear frequency sweep signal where the frequency exceeds
    % the windnoise detector cut-off frequence in the second half.
    f1=0;
    f2=1000;
    f=f1:(f2-f1)/(length(t)-1):f2;
    y=sin(2*pi.*f.*t);
 
    audiowrite(inwav, y, dsc.srate);
    unittest_teardown(@delete, inwav);
    
    % create a new MHA process
    mha = mha_start;
    mha_set(mha,'',dsc);
    % ensure MHA terminates after the test
    unittest_teardown(@mha_set, mha, 'cmd', 'quit');

    % configure the new MHA
    mha_set(mha,'cmd','start');
    mha_set(mha, 'cmd', 'release');

 
    % load the saved ac varialbe of lowpass_q
    load(dsc.mha.acsave.name);

    % Calculate total sample index corresponding to each entry in 
    % the mat file created by acsave
    sample_no =0:dsc.fragsize:(length(windnoise_lowpass_q)-1)*dsc.fragsize;
    
    % Find index where frequency of the sweep signal crosses the cut-off frequency
    sample_no_f = t*48000;
    f_index = sample_no_f...
    (f==dsc.mha.overlapadd.windnoise.LowPassCutOffFrequency);

    % Takes only the lowpass_q indices where frequency is greater than
    % threshold frequency - 
    lowpass_q_threshold = windnoise_lowpass_q(sample_no>f_index);
    
    % After threshold frequency, if difference between current lowpass_q and
    % next is lesser than 0, it means the lowpass_q is decreasing 
    % making test successful
    assert_all(diff(lowpass_q_threshold)<0)
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
