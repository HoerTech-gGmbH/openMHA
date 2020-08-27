%% This function tests the windnoise suppression in individual channels,
%% using two computer generated sine waves as signals - one above and one
%% below the threshold frequency, and tests if the unaffected channel 
%% is copied to the affected channel
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
function test_windnoise_suppression()
% This test is to confirm if affected channels get their signals swapped
% with unaffected channels; using generated sine waves as signals
    % A test signal.
    inwav = 'test_windnoise_in_2ch.wav';
    outwav = 'test_windnoise_out_2ch.wav';
    unittest_teardown(@delete, outwav);
    %instance for the test
    dsc.instance = 'test_windnoise_suppression';
    dsc.srate=48000;
    dsc.fragsize=64;
    dsc.nchannels_in=2;
    dsc.mhalib = 'mhachain';
    dsc.mha.algos = '[overlapadd]';
    dsc.mha.overlapadd.fftlen=256;
    dsc.mha.overlapadd.wnd.len=128;
    dsc.mha.overlapadd.plugin_name='windnoise';
    dsc.mha.overlapadd.windnoise.LowPassCutOffFrequency = 500;
    dsc.iolib='MHAIOFile';
    dsc.io.in=inwav;
    dsc.io.out=outwav;
    
    % time base for generating test signal  
    L=10000;
    ts = 1 / dsc.srate;
    t = (0:L-1)*ts;
    
    % Constant frequency signal below the threshold frequency
    f1=300;
    channel_1=sin(2*pi.*f1.*t);
    % Constant frequency signal above the threshold frequency
    f2=1000;
    channel_2=sin(2*pi.*f2.*t);
    %Two channel audio written for input file to windnoise plugin
    stereo=[channel_1(:), channel_2(:)];
    audiowrite(inwav, stereo, dsc.srate);
    unittest_teardown(@delete, inwav);
    
    % create a new MHA process
    mha = mha_start;
    mha_set(mha,'',dsc);
    % ensure MHA terminates after the test
    unittest_teardown(@mha_set, mha, 'cmd', 'quit');

    % configure the new MHA
    mha_set(mha,'cmd','start');
    mha_set(mha, 'cmd', 'release');

    %read the processed audio file
    [processed_2ch, fs]  = audioread(outwav);
    %Take signal after first chunck is processed
    processed=processed_2ch(dsc.mha.overlapadd.wnd.len+1:end, 1);
    %Remove last chunck to have equal length signal
    unprocessed=channel_2(1:length(channel_2)-dsc.mha.overlapadd.wnd.len)';
    
    %Expect processsed channel 1 to be euqal to unprocessed channel 1
    assert_difference_below(processed, unprocessed, 1e-2);

end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
