% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2022 Hörzentrum Oldenburg gGmbH
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

clear;
close all;
addpath(mha_install_dirs('mfiles'));

% start an mha and read the configuration
mha = mha_start();
mha_query(mha,[],'read:afc_live_example.cfg');
% get several variable values
fragsize = mha_get(mha,'fragsize');
filter_length = mha_get(mha,'mha.afc.filter_length');
channels = mha_get(mha,'nchannels_in');
fs = mha_get(mha,'srate');
measured_roundtrip_latency = mha_get(mha, 'mha.afc.measured_roundtrip_latency');

% start the configuration, let it run for 6s and close the mha
mha_set(mha,'cmd','prepare');
mha_set(mha,'cmd','start');
pause(6);
mha_set(mha,'cmd','release');
mha_set(mha,'cmd','quit');

% read data for evaluation
fid = fopen('FBfilter_estim.dat');
filter_coeff = fread(fid,Inf,'double');
fclose(fid);
flt_sorted = reshape(filter_coeff, filter_length*channels, ...
    length(filter_coeff)/(filter_length*channels));
% get most recent filter estimation
FBfilter_estim = flt_sorted(:,end);

% plot results
figure('units','normalized','Position',[0.1 0.1 0.8 0.8]);
% plot FBfilter_estim
subplot(2,3,1);
hold all;
plot(FBfilter_estim);
title('Impulse Response (Filter Coefficients)');
xlabel('Filter Taps [#]');
ylabel('Filter Coefficient Value [ ]');
% plot the transfer function of FBfilter_estim
subplot(2,3,4);
hold all;
nfft=1024;
vFreq = linspace(0,fs/2,nfft/2+1);
tf_estim = 20*log10(abs(fft(FBfilter_estim,nfft)));
plot(vFreq,tf_estim(1:nfft/2+1));
xlim([0 fs/2]);
title('Transfer Function (Freq)');
xlabel('Frequency [Hz]');
ylabel('Magnitude [dB]');
% plot the beginning of the input and output signal
[in_sig, fs] = audioread('input_sig.wav');
[out_sig, fs] = audioread('output_sig.wav');
plot_len = 10000;
subplot(2,3,2);
hold all;
plot(in_sig(1:plot_len), '-bo');
plot(out_sig(1:plot_len), '-rx');
ylim([-1 1]);
title('Input and Output Signal');
xlabel('Time [samples]');
ylabel('Amplitude [ ]');
legend('in','out');
% plot the whole input and output signal
subplot(2,3,5);
hold all;
out_sig_del = [zeros(measured_roundtrip_latency, 1); out_sig];
plot(in_sig(1:plot_len), '-bo');
plot(out_sig_del(1:plot_len), '-rx');
ylim([-1 1]);
title('Input and delayed Output Signal');
xlabel('Time [samples]');
ylabel('Amplitude [ ]');
legend('in', 'out');
% plot the end of the output signal
subplot(2,3,3);
plot(out_sig(end-plot_len:end));
title('End of Output Signal');
% plot the whole output signal
subplot(2,3,6);
plot(out_sig);
title('Whole Output Signal');
