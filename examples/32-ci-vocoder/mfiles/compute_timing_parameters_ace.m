% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2024 Hörzentrum Oldenburg gGmbH
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


% Script for computing a set of interdependent timing parameters and 
% anti-aliasing filter coefficients for resampling (ACE strategy), plotting 
% the results, and displaying them on the screen in a format that is 
% compatible with the Linux shell scripts and Windows batch files included 
% in this openMHA example

clear;
close all;
clc;

if isunix
    addpath(genpath('/usr/lib/openmha/mfiles'));
elseif ispc
    addpath(genpath('C:/Program Files/openMHA/mfiles'));
end

if isoctave
    pkg load signal;
end

% -------------------------------------------------------------------------

desired_stimulation_rate = 800;  % desired (per-electrode) stimulation rate / pps
srate = 48000;                   % audio sampling rate / Hz
fragsize = 60;                   % fragment size / frames
n = 4;                           % filter order
precision = 6;                   % precision (significant digits)

% Compute the resulting parameters:
vocoder_srate = 16000;  % vocoder sampling rate / Hz
fftlen = 128;  % FFT length / bins
vocoder_fragsize = round(vocoder_srate/desired_stimulation_rate);  % vocoder fragment size / frames
resample_ratio = srate/vocoder_srate;  % resample ratio
if mod(resample_ratio, 1) ~= 0
    error('The audio sampling rate (currently %u Hz) must be an integer multiple of the vocoder sampling rate (%u Hz).', srate, vocoder_srate);
end
dbasync_fragsize = vocoder_fragsize * resample_ratio;  % inner fragment size / frames
if fragsize < dbasync_fragsize
    error('The fragment size (currently %u frames) must be at least %u frames.', fragsize, dbasync_fragsize);
end
dbasync_delay = dbasync_fragsize - gcd(dbasync_fragsize, fragsize);  % delay for dbasync / frames
wnd_len = 2 * dbasync_fragsize/resample_ratio;  % window length / frames
if wnd_len > fftlen
    error('The window length (currently %u frames) must be less than or equal to the FFT length (%u bins). To fix this, consider increasing the stimulation rate (currently %u pps).', wnd_len, fftlen, desired_stimulation_rate);
end
closest_stimulation_rate = vocoder_srate/vocoder_fragsize;  % closest achievable stimulation rate / pps

% Compute the coefficients for a Butterworth lowpass filter for 
% anti-aliasing before resampling:
fc = vocoder_srate/2;
if resample_ratio == 1
    b = 1;
    a = 1;
else
    Wn = fc/(srate/2);
    [b, a] = butter(n, Wn, 'low');
end

% Reduce numeric precision to guarantee conformity with C++ single-
% precision floating-point format ("float", or "mha_real_t" in openMHA):
b = eval(mat2str(b, 6));
a = eval(mat2str(a, 6));

% Plot the frequency response of the filter:
figure(1);
freqz(b, a, srate/2, srate);
subplot(2, 1, 1);
xticks(0:2000:srate/2);
xticklabels((0:2000:srate/2)/1000);
xlabel('Frequency / kHz');
ylim([-40 0]);
hold on;
plot([fc fc], ylim, 'r--');
fontSize = get(gca, 'FontSize');
text(fc, 0, '{\it f}_c', 'Color', 'r', 'FontSize', fontSize, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'bottom');
hold off;
ylabel('Magnitude / dB');
title({'Anti-Aliasing Filter (ACE)', ''});
subplot(2, 1, 2);
hold on;
plot([fc fc], ylim, 'r--');
yl = ylim;
text(fc, yl(2), '{\it f}_c', 'Color', 'r', 'FontSize', fontSize, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'bottom');
hold off;
xticks(0:2000:srate/2);
xticklabels((0:2000:srate/2)/1000);
xlabel('Frequency / kHz');
ylabel('Phase / degrees');

% Plot the pole-zero plot of the filter:
figure(2);
zplane(b, a);
gridColor = get(gca, 'GridColor');
h = findobj(gca, 'LineStyle', ':');
set(h, 'Color', gridColor, 'LineStyle', '-');
grid on;
axis square;
ylim(xlim);
xlabel('Real Part');
ylabel('Imaginary Part');
title({'Anti-Aliasing Filter (ACE)', ''});

% Verify that the filter is stable:
if ~isstable(b, a)
    error('The filter is unstable. Consider changing the filter order and/or cutoff frequency.');
end

% Reduce numeric precision to guarantee conformity with C++ single-
% precision floating-point format ("float", or "mha_real_t" in openMHA):
b = eval(mat2str(b, 6));
a = eval(mat2str(a, 6));

% Display the results:
expression = '(?<=e[+-])0';
sFragsize = strtrim(sprintf('%u ', fragsize));
sFragsize = regexprep(sFragsize, expression, '');
sSrate = strtrim(sprintf('%u ', srate));
sSrate = regexprep(sSrate, expression, '');
sDbasync_fragsize = strtrim(sprintf('%u ', dbasync_fragsize));
sDbasync_fragsize = regexprep(sDbasync_fragsize, expression, '');
sDbasync_delay = strtrim(sprintf('%u ', dbasync_delay));
sDbasync_delay = regexprep(sDbasync_delay, expression, '');
sResample_ratio = strtrim(sprintf('%u ', resample_ratio));
sResample_ratio = regexprep(sResample_ratio, expression, '');
sB = strtrim(sprintf('%g ', b));
sB = regexprep(sB, expression, '');
sA = strtrim(sprintf('%g ', a));
sA = regexprep(sA, expression, '');
sWnd_len = strtrim(sprintf('%u ', wnd_len));
sWnd_len = regexprep(sWnd_len, expression, '');
sDesired_stimulation_rate = strtrim(sprintf('%.0f ', desired_stimulation_rate));
sDesired_stimulation_rate = regexprep(sDesired_stimulation_rate, expression, '');
sClosest_stimulation_rate = strtrim(sprintf('%.0f ', closest_stimulation_rate));
sClosest_stimulation_rate = regexprep(sClosest_stimulation_rate, expression, '');
sVocoder_srate = strtrim(sprintf('%u ', vocoder_srate));
sVocoder_srate = regexprep(sVocoder_srate, expression, '');
fprintf('Timing parameters for ACE (Linux):\n\n');
fprintf(['export FRAGSIZE=' sFragsize '\n\n']);
fprintf(['export SRATE=' sSrate '\n\n']);
fprintf(['export DBASYNC_FRAGSIZE=' sDbasync_fragsize '\n\n']);
fprintf(['export DBASYNC_DELAY=' sDbasync_delay '\n\n']);
fprintf(['export RESAMPLE_RATIO=' sResample_ratio '\n']);
fprintf(['export RESAMPLE_ANTIALIAS_B="[' sB ']"\n']);
fprintf(['export RESAMPLE_ANTIALIAS_A="[' sA ']"\n\n']);
fprintf(['export WND_LEN=' sWnd_len '\n\n\n\n']);
fprintf('Timing parameters for ACE (Windows):\n\n');
fprintf(['set FRAGSIZE=' sFragsize '\n\n']);
fprintf(['set SRATE=' sSrate '\n\n']);
fprintf(['set DBASYNC_FRAGSIZE=' sDbasync_fragsize '\n\n']);
fprintf(['set DBASYNC_DELAY=' sDbasync_delay '\n\n']);
fprintf(['set RESAMPLE_RATIO=' sResample_ratio '\n']);
fprintf(['set RESAMPLE_ANTIALIAS_B=[' sB ']\n']);
fprintf(['set RESAMPLE_ANTIALIAS_A=[' sA ']\n\n']);
fprintf(['set WND_LEN=' sWnd_len '\n\n\n\n']);
fprintf(['Desired stimulation rate: ' sDesired_stimulation_rate ' pps\n']);
fprintf(['Closest achievable stimulation rate given the (fixed) vocoder sampling rate (' sVocoder_srate ' Hz): ' sClosest_stimulation_rate ' pps\n\n']);

figure(1);
