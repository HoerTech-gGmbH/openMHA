function test_wave2spec_spec2wave

% Check for different combinations of fft length, window length, hop
% size, window position, that MHA overlat-add STFT as carried out by a
% combination of wave2spec and spec2wave produces the correct signal
% delay and signal reconstruction.

dsc.mhalib = 'mhachain';
dsc.mha.algos = {'wave2spec','spec2wave'};

% use as input signal sinusoidals in uncorrelated noise, with the
% sinusoidals phase shifted between channels
input_signal = reshape(sin([1:4000]/117),2,2000)/2 ...
    + repeatable_rand(2,2000,6) / 20;
input_signal(1,:) *= -1;

fftlen = [512 512 512 500 162 128 364 364 364];
wndlen = [400 400 411 300 128  64 128 128 128];
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

  % apply postwindowing only if analysis window sits in center analysis
  % buffer
  dsc.mha.spec2wave.ramplen = double(wndpos(scenario) == 0.5);
  
  mha = mha_start;
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  
  mha_set(mha, '', dsc);
  mha_set(mha, 'iolib', 'MHAIOParser');
  
  mha_set(mha, 'cmd', 'start');
                      
  
  ola_delay = compute_ola_delay(wndlen(scenario), ...
                                wndshift(scenario), ...
                                fftlen(scenario), ...
                                wndpos(scenario));

  output_signal = mha_process_by_parser(mha,input_signal);
  
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
