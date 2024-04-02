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


% Script for computing the spread of excitation (i.e., current spread) 
% along the cochlea (CIS strategy), plotting the results, and displaying 
% them on the screen in a format that is compatible with the Linux shell 
% scripts and Windows batch files included in this openMHA example

clear;
close all;
clc;

addpath(genpath('functions'));

% -------------------------------------------------------------------------

active_electrode_idx = 1;   % active electrode index (for plotting; 1: most apical, 12: most basal)
electrode_length = 0.0264;  % electrode length / m
M = 12;                     % total number of electrodes
s = 2.8;                    % slope of exponential spread of excitation / dB/mm (no spread when s = inf)

% Compute electrode distance (assuming equidistant spacing of electrodes), 
% length constant of the exponential decay function, and spread of 
% excitation:
electrode_distance = electrode_length/(M - 1);
if s <= 0
    error('The value of s must be positive.');
end
lambda = slope2lambda(s);
place_function = 1:0.1:M;
amplitude_function =  compute_exponential_decay(place_function, active_electrode_idx, electrode_distance, lambda);
place_electrodes = 1:M;
amplitude_electrodes_no_spread = zeros(1, M);
amplitude_electrodes_no_spread(active_electrode_idx) = 1;
amplitude_electrodes = compute_exponential_decay(place_electrodes, active_electrode_idx, electrode_distance, lambda);

% Reduce numeric precision to guarantee conformity with C++ single-
% precision floating-point format ("float", or "mha_real_t" in openMHA):
electrode_distance = eval(mat2str(electrode_distance, 6));
lambda = eval(mat2str(lambda, 6));

% Plot the spread of excitation:
figure;
C = get(gca, 'ColorOrder');
if lambda == 0
    stem(place_electrodes, amplitude_electrodes_no_spread, 'Color', C(1, :));
else
    plot(place_function, amplitude_function, 'r--');
    hold on;
    stem(place_electrodes, amplitude_electrodes, 'Color', C(1, :));
    hold off;
end
text(1, 1, 'Apical', 'Color', 'r', 'HorizontalAlignment', 'left', 'VerticalAlignment', 'bottom');
text(M, 1, 'Basal', 'Color', 'r', 'HorizontalAlignment', 'right', 'VerticalAlignment', 'bottom');
grid on;
xlim([1 M]);
set(gca, 'XTick', 1:M);
% set(gca, 'XTickLabel', M:-1:1);
xlabel('Electrode Index');
ylabel('Normalized Amplitude');
title({'Spread of Excitation (CIS)', ''});

% Display the results:
expression = '(?<=e[+-])0';
sElectrode_distance = strtrim(sprintf('%g ', electrode_distance));
sElectrode_distance = regexprep(sElectrode_distance, expression, '');
sLambda = strtrim(sprintf('%g ', lambda));
sLambda = regexprep(sLambda, expression, '');
fprintf('Spread of excitation for CIS (Linux):\n\n');
fprintf(['export STIMULATION_SIGNAL_ELECTRODE_DISTANCE=' sElectrode_distance '\n\n']);
fprintf(['export STIMULATION_SIGNAL_LAMBDA=' sLambda '\n\n\n\n']);
fprintf('Spread of excitation for CIS (Windows):\n\n');
fprintf(['set STIMULATION_SIGNAL_ELECTRODE_DISTANCE=' sElectrode_distance '\n\n']);
fprintf(['set STIMULATION_SIGNAL_LAMBDA=' sLambda '\n\n']);
