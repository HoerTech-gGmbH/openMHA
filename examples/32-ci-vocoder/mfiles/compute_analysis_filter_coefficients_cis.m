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


% Script for computing filter coefficients for analysis, i.e., CI
% simulation (CIS strategy), plotting the results, and displaying them on
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

addpath(genpath('functions'));

if isunix
    addpath(genpath('/usr/lib/openmha/mfiles'));
elseif ispc
    addpath(genpath('C:/Program Files/openMHA/mfiles'));
end

if isoctave
    warning('off', 'Octave:possible-matlab-short-circuit-operator');
end

addpath(genpath('gammatone'));
if isempty(which('Gfb_Filter_new.m')) || isempty(which('Gfb_Analyzer_process.m'))
    error('The gammatone toolbox must be on the Matlab search path.');
end

% -------------------------------------------------------------------------

% For the CIS strategy, the filterbank for analysis (i.e., CI simulation)
% is implemented in this example as a weighted gammatone filterbank with a
% bandwidth factor of 3. The default, electrode-specific vector of center
% frequencies, which can be modified by the user (i.e., audiologist), is
% fc = [120 235 384 579 836 1175 1624 2222 3019 4084 5507 7410] Hz.

srate = 48000;         % sampling rate / Hz
fc = [120 235 384 579 836 1175 1624 2222 3019 4084 5507 7410];  % default center frequencies / Hz
n = 3;                 % filter order
bandwidth_factor = 3;  % bandwidth factor
precision = 6;         % precision (significant digits)

% Compute the weights for a gammatone filterbank and reduce the numeric
% precision to guarantee conformity with C++ single-precision
% floating-point format ("float", or "mha_real_t" in openMHA):
ERB = NaN(1, length(fc));  % equivalent rectangular bandwidth (ERB) of filter / Hz
weights = NaN(1, length(fc));  % filter weight as inverse ERB, normalized for ERB of lowest-frequency filter
for k = 1:length(ERB)
    ERB(k) = computeERB(fc(k));
    weights(k) = 1/(ERB(k)/ERB(1));
end
weights = eval(mat2str(weights, precision));

% Compute the coefficients for a gammatone filterbank, reduce the numeric
% precision to guarantee conformity with C++ single-precision
% floating-point format ("float", or "mha_real_t" in openMHA), and plot the
% magnitude response of the filterbank:
for k = 1:length(fc)
    analyzer.filters(1, k) = Gfb_Filter_new(srate, fc(k), n, bandwidth_factor);
    analyzer.filters(1, k).coefficient = eval(mat2str(analyzer.filters(1, k).coefficient, precision));
    analyzer.filters(1, k).normalization_factor = eval(mat2str(analyzer.filters(1, k).normalization_factor, precision));
end
impulse = [1, zeros(1, srate/2-1)];
analyzer.center_frequencies_hz = fc;
analyzer.fast = false;
impulse_response = Gfb_Analyzer_process(analyzer, impulse);
frequency_response = fft(real(impulse_response)');
frequency = linspace(0, srate, size(frequency_response, 1))';
figure;
C = get(gca, 'ColorOrder');
plot(frequency, 10*log10((abs(frequency_response)).^2.*weights), 'Color', C(1, :));  % the weights are applied to powers, not magnitudes
grid on;
xlim([0 srate/2]);
xticks(0:2000:srate/2);
xticklabels((0:2000:srate/2)/1000);
xlabel('Frequency / kHz');
ylim([-40 0]);
ylabel('Magnitude / dB');
title({'Analysis Filterbank (CIS)', ''});

% Display the results:
expression = '(?<=e[+-])0';
sWeights = strtrim(sprintf('%g ', weights));
sWeights = regexprep(sWeights, expression, '');
sFc = strtrim(sprintf('%.0f ', fc));
sFc = regexprep(sFc, expression, '');
fprintf('Analysis filterbank for CIS (Linux):\n\n');
fprintf('export GTFB_ANALYZER_ANALYSIS_ORDER=%u\n', n);
for k = 1:length(fc)
    coefficient = analyzer.filters(k).coefficient;
    sCoefficient = strtrim(sprintf('%g%+gi ', real(coefficient), imag(coefficient)));
    sCoefficient = regexprep(sCoefficient, expression, '');
    normalization_factor = analyzer.filters(k).normalization_factor;
    sNormalization_factor = strtrim(sprintf('%g ', normalization_factor));
    sNormalization_factor = regexprep(sNormalization_factor, expression, '');
    fprintf(['export GTFB_ANALYZER_ANALYSIS%u_COEFF="[(' sCoefficient ')]"\n'], k-1);
    fprintf(['export GTFB_ANALYZER_ANALYSIS%u_NORM_PHASE="[' sNormalization_factor ']"\n'], k-1);
end
fprintf('\n');
fprintf(['export ELECTRODOGRAM_WEIGHTS="[' sWeights ']"\n\n\n\n']);
fprintf('Analysis filterbank for CIS (Windows):\n\n');
fprintf('set GTFB_ANALYZER_ANALYSIS_ORDER=%u\n', n);
for k = 1:length(fc)
    coefficient = analyzer.filters(k).coefficient;
    sCoefficient = strtrim(sprintf('%g%+gi ', real(coefficient), imag(coefficient)));
    sCoefficient = regexprep(sCoefficient, expression, '');
    normalization_factor = analyzer.filters(k).normalization_factor;
    sNormalization_factor = strtrim(sprintf('%g ', normalization_factor));
    sNormalization_factor = regexprep(sNormalization_factor, expression, '');
    fprintf(['set GTFB_ANALYZER_ANALYSIS%u_COEFF=[(' sCoefficient ')]\n'], k-1);
    fprintf(['set GTFB_ANALYZER_ANALYSIS%u_NORM_PHASE=[' sNormalization_factor ']\n'], k-1);
end
fprintf('\n');
fprintf(['set ELECTRODOGRAM_WEIGHTS=[' sWeights ']\n\n\n\n']);
fprintf('Analysis filterbank center frequencies for CIS (default) / Hz:\n\n');
fprintf(['fc = [' sFc '];\n\n']);
