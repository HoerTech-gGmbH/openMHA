%% This function tests plugin fshift by checking the frequency shift
%% applied by this plugin.
%%
%% This file is part of the HörTech Open Master Hearing Aid (openMHA)
%% Copyright © 2018 HörTech gGmbH

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
function test_fshift

    % basic mha config for 2 channel stft
    desc.instance = 'test_fshift';
    desc.nchannels_in = 2;
    desc.fragsize = 64;
    desc.srate = 16000;
    desc.mhalib = 'overlapadd';
    desc.iolib = 'MHAIOParser';
    desc.mha.fftlen = 256;
    desc.mha.wnd.type = 'hanning';
    desc.mha.wnd.len = 128;

    % load fshift_hilbert into the stft processing
    desc.mha.plugin_name = 'fshift';
    desc.mha.fshift.df = desc.srate/desc.mha.fftlen;
    desc.mha.fshift.fmin=5000;
    desc.mha.fshift.fmax=7000;
    df=desc.mha.fshift.df;
    % create a new MHA process
    mha = mha_start();

    % ensure MHA is exited after the test
    unittest_teardown(@mha_set, mha, 'cmd', 'quit');

    % configure the new MHA
    mha_set(mha,'',desc);
    mha_set(mha, 'cmd', 'start');

    % create test signal. The delay of overlapadd as used here is 128 samples,
    % so we need to put in 3 blocks to get the first block out correctly.
    % We put one more buffer through, 4 altogether, to account for adaptation
    % effects.

    % time base for generating test signal
    t = [1:desc.fragsize * 4] / desc.srate;
    
    % first channel has sinusoids below and above the shifted frequency
    % range, and is unaffected
    s_in(1,:) = 0.5*(sin(2*pi*2000*t) + sin(2*pi*8000*t));

    % second channel has sinusoid in shifted range
    f0=6000;
    s_in(2,:) = 0.5*sin(2*pi*f0*t);
    
    % process the signal in three blocks
    sample_indices = 1:desc.fragsize;
    for block = 0:3
      mha_set(mha,'io.input',s_in(:,sample_indices + block*desc.fragsize));
    end
    % last output corresponds to second input block
    s_out = mha_get(mha,'io.output');
    
    % Compare first channel input and output signal.
    actual = s_out(1,:);
    expected = s_in(1,sample_indices + desc.fragsize);
    % We expect a minor but clearly detectable effect of the frequency shifting
    % going on between the two test frequencies on the test signal because of the
    % short analysis window. The effect will be in the order of -50dB SDR.
    assert_difference_below(expected, actual, 10^(-40/20));

    % check second channel output signal: Input frequency was 6000 Hz,
    % output frequency should be 6125Hz.
    actual = s_out(2,:);
    
    % helper function to compute squared error for a given phase
    err = @(phi) mean((actual - 0.5*sin(2*pi*(f0+df)*t(1:desc.fragsize) + phi)) .^ 2);

    % initial search parameters
    phi = 0.1;

    % match the phase
    [phi,e] = fminsearch(err, phi);

    % the error in the shifted range should be in the order of -40 dB SDR
    assert_difference_below(0,sqrt(e),10^(-40/20));

end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
