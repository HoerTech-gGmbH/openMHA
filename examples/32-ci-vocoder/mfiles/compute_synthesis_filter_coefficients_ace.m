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


% Script for computing filter coefficients for synthesis, i.e., CI 
% auralization (ACE strategy), plotting the results, and displaying them on 
% the screen in a format that is compatible with the Linux shell scripts 
% and Windows batch files included in this openMHA example
% 
% This script depends on the gammatone toolbox by Hohmann and Herzke (2007), 
% which must be downloaded and then added to the Matlab/Octave search path:
% 
% Hohmann, V., & Herzke, T. (2007). Software for "Frequency analysis and 
% synthesis using a Gammatone filterbank" (Version 1.1). Zenodo. 
% https://doi.org/10.5281/zenodo.2643400

clear;
close all;
clc;

if isunix
    addpath(genpath('/usr/lib/openmha/mfiles'));
elseif ismac
    addpath(genpath('/usr/local/lib/openmha/mfiles'));
elseif ispc
    sProgramFilesPath = getenv('PROGRAMFILES');
    addpath(genpath([sProgramFilesPath '\openMHA\mfiles']));
end

if isoctave
    pkg load statistics;
    warning('off', 'Octave:possible-matlab-short-circuit-operator');
end

addpath(genpath('gammatone'));
if isempty(which('Gfb_Filter_new.m')) || isempty(which('Gfb_Analyzer_process.m'))
    error('The gammatone toolbox must be on the Matlab search path.');
end

% -------------------------------------------------------------------------

% The filterbank for synthesis (i.e., CI auralization) is implemented in 
% this example as a gammatone filterbank. The vector of center frequencies, 
% fc, for that filterbank is computed using the script 
% compute_synthesis_center_frequencies_ace.m.
% 
% So-called "dead regions" of the cochlea can be modeled by setting any of 
% the center frequencies in fc to NaN; e.g., for a dead region at the 
% location of the most basal (i.e., high-frequency) electrode, set 
% fc = [713 794 913 1056 1233 1428 1663 1941 2298 2678 3152 3625 4152 4749 5363 6042 6926 8242 9643 11190 12668 NaN]

srate = 48000;  % sampling rate / Hz
fc = [713 794 913 1056 1233 1428 1663 1941 2298 2678 3152 3625 4152 4749 5363 6042 6926 8242 9643 11190 12668 14221];  % center frequencies / Hz
n = 3;          % filter order
precision = 6;  % precision (significant digits)

% Compute the coefficients for a gammatone filterbank, reduce the numeric 
% precision to guarantee conformity with C++ single-precision 
% floating-point format ("float", or "mha_real_t" in openMHA), and plot the 
% magnitude response of the filterbank:
for k = 1:length(fc)
    analyzer.filters(1, k) = Gfb_Filter_new(srate, fc(k), n);
    if isnan(fc(k))
        analyzer.filters(1, k).coefficient = 0;
        analyzer.filters(1, k).normalization_factor = 0;
    else
        analyzer.filters(1, k).coefficient = eval(mat2str(analyzer.filters(1, k).coefficient, precision));
        analyzer.filters(1, k).normalization_factor = eval(mat2str(analyzer.filters(1, k).normalization_factor, precision));
    end
end
impulse = [1, zeros(1, srate/2-1)];
analyzer.center_frequencies_hz = fc;
analyzer.fast = false;
impulse_response = Gfb_Analyzer_process(analyzer, impulse);
frequency_response = fft(real(impulse_response)');
frequency = linspace(0, srate, size(frequency_response, 1))';
figure;
C = get(gca, 'ColorOrder');
plot(frequency, 20*log10(abs(frequency_response)), 'Color', C(1, :));
grid on;
xlim([0 srate/2]);
xticks(0:2000:srate/2);
xticklabels((0:2000:srate/2)/1000);
xlabel('Frequency / kHz');
ylim([-40 0]);
ylabel('Magnitude / dB');
title({'Synthesis Filterbank (ACE)', ''});

% Display the results:
expression = '(?<=e[+-])0';
fprintf('Synthesis filterbank for ACE (Linux):\n\n');
fprintf('export GTFB_ANALYZER_ORDER=%u\n', n);
for k = 1:length(fc)
    coefficient = analyzer.filters(k).coefficient;
    sCoefficient = strtrim(sprintf('%g%+gi ', real(coefficient), imag(coefficient)));
    sCoefficient = regexprep(sCoefficient, expression, '');
    normalization_factor = analyzer.filters(k).normalization_factor;
    sNormalization_factor = strtrim(sprintf('%g ', normalization_factor));
    sNormalization_factor = regexprep(sNormalization_factor, expression, '');
    fprintf(['export GTFB_ANALYZER%u_COEFF="[(' sCoefficient ')]"\n'], k-1);
    fprintf(['export GTFB_ANALYZER%u_NORM_PHASE="[' sNormalization_factor ']"\n'], k-1);
end
fprintf('\n\n\n');
fprintf('Synthesis filterbank for ACE (Windows):\n\n');
fprintf('set GTFB_ANALYZER_ORDER=%u\n', n);
for k = 1:length(fc)
    coefficient = analyzer.filters(k).coefficient;
    sCoefficient = strtrim(sprintf('%g%+gi ', real(coefficient), imag(coefficient)));
    sCoefficient = regexprep(sCoefficient, expression, '');
    normalization_factor = analyzer.filters(k).normalization_factor;
    sNormalization_factor = strtrim(sprintf('%g ', normalization_factor));
    sNormalization_factor = regexprep(sNormalization_factor, expression, '');
    fprintf(['set GTFB_ANALYZER%u_COEFF=[(' sCoefficient ')]\n'], k-1);
    fprintf(['set GTFB_ANALYZER%u_NORM_PHASE=[' sNormalization_factor ']\n'], k-1);
end
fprintf('\n');
