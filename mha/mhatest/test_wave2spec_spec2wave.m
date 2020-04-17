function test_wave2spec_spec2wave

% Check for different combinations of fft length, window length, hop
% size, window position, that MHA overlap-add STFT as carried out by a
% combination of wave2spec and spec2wave produces the correct signal
% delay and signal reconstruction.
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2005 2006 2007 2018 HörTech gGmbH
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

dsc.mhalib = 'mhachain';
dsc.mha.algos = {'wave2spec','spec2wave'};

% use as input signal sinusoidals in uncorrelated noise, with the
% sinusoidals phase shifted between channels
input_signal = reshape(sin([1:4000]/117),2,2000)/2 ...
    + repeatable_rand(2,2000,6) / 20;
input_signal(1,:) = -1*input_signal(1,:);
input_filename = 'test_wave2spec_spec2wave_input_signal.wav';
output_filename = 'test_wave2spec_spec2wave_output_signal.wav';

audiowrite(input_filename,input_signal', 44100, 'BitsPerSample', 32);
unittest_teardown(@delete, input_filename);
audiowrite(output_filename,input_signal', 44100, 'BitsPerSample', 32);
unittest_teardown(@delete, output_filename);

fftlen = [512 512 512 500 162 128 364 364 364];
wndlen = [400 400 402 280 128  64 128 128 128];
wndshift=[200 100 201 140  64  32  64  64  64];
wndpos = [0.5 0.5 0.5 0.5 0.5 0.5 0.5   0   1];
wndexp = [  1   1   1   1   1   1   1   1   1];

for scenario = 1:length(fftlen)  
  errmsg = sprintf('Scenario %d failed: ', scenario);
  dsc.instance = ['test_ola/check_delay_and_reconstruction_' num2str(scenario)];
  dsc.nchannels_in = 2;
  dsc.mha.wave2spec.fftlen = fftlen(scenario);
  dsc.mha.wave2spec.wndlen = wndlen(scenario);
  dsc.mha.wave2spec.wndpos = wndpos(scenario);
  dsc.fragsize = wndshift(scenario);
  dsc.mha.wave2spec.wndexp = wndexp(scenario);
  dsc.mha.wave2spec.wndtype = 'hanning';
  dsc.mha.spec2wave.wndexp = 1-wndexp(scenario);
  dsc.mha.spec2wave.wndtype = 'hanning';
  dsc.iolib = 'MHAIOFile';
  dsc.io.in = input_filename;
  dsc.io.out = output_filename;
  
  % apply postwindowing only if analysis window sits in center analysis
  % buffer
  dsc.mha.spec2wave.ramplen = double(wndpos(scenario) == 0.5);
  
  mha = mha_start;
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  
  mha_set(mha, '', dsc);
  
  ola_delay = compute_ola_delay(wndlen(scenario), ...
                                wndshift(scenario), ...
                                fftlen(scenario), ...
                                wndpos(scenario));

  mha_set(mha, 'cmd', 'start');
  mha_set(mha, 'cmd', 'release');
  output_signal = audioread(output_filename)';
  
  real_delay(1) = maxxcorr(output_signal(1,:), input_signal(1,:));
  real_delay(2) = maxxcorr(output_signal(2,:), input_signal(2,:));
  assert_equal(ola_delay, round(real_delay(1)), errmsg);
  assert_equal(ola_delay, round(real_delay(2)), errmsg);

  % reconstruction will only work well if window length is a multiple of
  % hop size
  if mod(wndlen(scenario), wndshift(scenario)) == 0
    assert_difference_below(input_signal(:, 1:(end - ola_delay)), ...
                            output_signal(:, (1 + ola_delay):end), ...
                            1/70000, ...
                            errmsg);
  else
    difference = abs(input_signal(:,1:(end - ola_delay)) - ...
                     output_signal(:,(1 + ola_delay):end));
    assert_all(any(any(difference > 1/70000)), errmsg);
  end %endif
end %endfor
end %endfunction





function index = maxxcorr(sig1,sig2)
% index of maximum of cross correlation (interpolated)
  
if ~isequal(size(sig1),size(sig2))
    error('Signal sizes differ');
end %endif

xc = xcorr(sig1,sig2);
[c,index] = max(xc);
a = (xc(index+1)+xc(index-1)) / 2 - c;
b = (xc(index+1)-xc(index-1)) / 2;
index = -b/2/a + (index - length(sig1));
end % endfunction

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
