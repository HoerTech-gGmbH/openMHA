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

% IMPORTANT: before running this file, start a Jack server

clear;
close all;
addpath(mha_install_dirs('mfiles'));

if isoctave
  pkg load signal;
end

% start an mha
mha = mha_start();
% read configuration file
mha_query(mha,[],'read:afc_sim.cfg');
% get several variables
fragsize = mha_get(mha,'fragsize');
filter_length = mha_get(mha,'mha.afc.filter_length');
channels = mha_get(mha,'nchannels_in');
fs = mha_get(mha,'srate');
FBfilter_true = mha_get(mha,'mha.FBfilter_true.B');
% start the configuration, let it run for 10s and close the mha
mha_set(mha,'cmd','prepare');
mha_set(mha,'cmd','start');
pause(10);
mha_set(mha,'cmd','release');
mha_set(mha,'cmd','quit');

% read data for evaluation
fid = fopen('FBfilter_estim.dat');
filter_coeff = fread(fid,Inf,'double');
fclose(fid);
flt_sorted = reshape(filter_coeff, filter_length * channels, ...
                     length(filter_coeff)/(filter_length*channels));
% get most recent FBfilter_estim
FBfilter_estim = flt_sorted(:,end);

% plot results
figure('units','normalized','Position',[0.1 0.1 0.8 0.8]);
% feedback filter coefficients (true and estimated)
subplot(2,2,1);
hold all;
plot(FBfilter_true);
plot(FBfilter_estim);
legend('true','estim');
title('Impulse Response (Time)');
xlabel('Filter Taps [#]');
ylabel('Filter Coefficient Value [ ]');
% feedback filter transfer functions (true and estimated)
subplot(2,2,2);
hold all;
nfft=1024;
vFreq = linspace(0,fs/2,nfft/2+1);
tf_true = 20*log10(abs(fft(FBfilter_true,nfft)));
tf_estim = 20*log10(abs(fft(FBfilter_estim,nfft)));
plot(vFreq,tf_true(1:nfft/2+1));
plot(vFreq,tf_estim(1:nfft/2+1));
xlim([0 fs/2]);
legend('true','estim');
title('Transfer Function (Freq)');
xlabel('Frequency [Hz]');
ylabel('Magnitude [dB]');
% beginning of input and output signal
[in_sig, fs] = audioread('input_sig.wav');
[out_sig, fs] = audioread('output_sig.wav');
plot_len = 10000;
subplot(2,2,3);
hold all;
plot(in_sig(1:plot_len), '-bo');
plot(out_sig(1:plot_len), '-rx');
ylim([-1 1]);
title('Input and Output Signal');
xlabel('Time [samples]');
ylabel('Amplitude [ ]');
legend('in','out');
