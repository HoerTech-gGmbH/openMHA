%% This file is part of the HörTech Open Master Hearing Aid (openMHA)
%% Copyright © 2017 HörTech gGmbH
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
%%
%% Author: Tobias Herzke
%% Created: 2017-05-29
%%
%% This function tests plugin fshift_hilbert by checking the frequency shift
%% applied by this plugin.

function test_fshift_hilbert

    return;

    % basic mha config for 2 channel stft
    desc.instance = 'test_fshift_hilbert';
    desc.nchannels_in = 2;
    desc.fragsize = 64;
    desc.srate = 44100;
    desc.mhalib = 'overlapadd';
    desc.iolib = 'MHAIOParser';
    desc.mha.fftlen = 256;
    desc.mha.wnd.type = 'hanning';
    desc.mha.wnd.len = 128;

    % load fshift_hilbert into the stft processing
    desc.mha.plugin_name = 'fshift_hilbert';
    desc.mha.fshift_hilbert.df = 40;
    desc.mha.fshift_hilbert.fmin = 4000;
    desc.mha.fshift_hilbert.fmax = 9000;

    % create a new MHA process
    mha = mha_start();

    % ensure MHA is exited after the test
    unittest_teardown(@mha_set, mha, 'cmd', 'quit');

    % configure the new MHA
    mha_set(mha,'',desc);
    mha_set(mha, 'cmd', 'start');
    
    % time base for generating test signals (two blocks to fill stft window)
    t = [1:desc.fragsize * 2] / desc.srate;
    
    % first channel has sinusoids below and above the shifted frequency
    % range, and is unaffected
    s_in(1,:) = 0.5*(sin(2*pi*2000*t) + sin(2*pi*12000*t));

    % second channel has sinusoid in shifted range
    s_in(2,:) = sin(2*pi*6000*t);
    
    % process the signal in two blocks
    mha_set(mha,'io.input',s_in(:,1:desc.fragsize));
    mha_set(mha,'io.input',s_in(:,desc_fragsize+1:end));
    s_out = mha_get(mha,'io.output');

    
    ch1_input + [zeros(1,dl) ch2_input(1:(200-dl))];
    
    mha_set(mha, 'io.input', in_sig);
    output_signal = mha_get(mha, 'io.output');
    assert_almost(output_signal,out_sig,1e-6);
end
