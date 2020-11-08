%% This function tests if the delay plugin delays the signal by the
%% configured amount.
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
function test_delay()
% This test is to confirm if channels are delayed by correct amount
    %instance for the test
    dsc.instance = 'test_delay';
    dsc.fragsize=128;
    dsc.nchannels_in=2;
    dsc.srate = 44100;
    dsc.mhalib = 'delay';
    
    % Set delay for each channel
    delay = [20, 40];
    dsc.mha.delay = delay;
    
    % time base for generating test signal  
    L= dsc.fragsize * 2;
    ts = 1 / dsc.srate;
    t = (0:L-1)*ts;
    
    %Two channel audio written for input file to delay plugin
    channel_1 = 0.5 * sin(2*pi*100*t);
    channel_2 = 0.5 * sin(2*pi*1000*t);
    stereo = [channel_1; channel_2];
    
    % create a new MHA process
    mha = mha_start;
    mha_set(mha,'',dsc);
    mha_set(mha, 'iolib', 'MHAIOParser');
    % ensure MHA terminates after the test
    unittest_teardown(@mha_set, mha, 'cmd', 'quit');

    % configure the new MHA
    mha_set(mha,'cmd','start');
    
    mha_set(mha, 'io.input', stereo(:, 1:dsc.fragsize));
    processed  = mha_get(mha, 'io.output');
    mha_set(mha, 'io.input', stereo(:, dsc.fragsize+1:end));
    processed  = [processed, mha_get(mha, 'io.output')];
    %Take signal after first chunck is processed
    out1 = [zeros(1, dsc.mha.delay(1)) channel_1(1:(dsc.fragsize*2-dsc.mha.delay(1)))];
    out2 = [zeros(1, dsc.mha.delay(2)) channel_2(1:(dsc.fragsize*2-dsc.mha.delay(2)))];
    expectation = [out1; out2];
    
    %Expect the signal to be delayed by the set amount of samples
    assert_all(processed, expectation);

end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
