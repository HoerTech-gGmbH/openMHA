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


% Script for computing automatic gain control (AGC) parameters (ACE 
% strategy), plotting the results, and displaying them on the screen in a 
% format that is compatible with the Linux shell scripts and Windows batch 
% files included in this openMHA example

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

% -------------------------------------------------------------------------

gtmax = 120;         % maximum input level / dB SPL
gtmin = 0;           % minimum input level / dB SPL
gtstep = 1;          % input level step size / dB
threshold = 65;      % compression threshold / dB SPL
tau_attack = 0.005;  % attack time / s
tau_decay = 0.075;   % decay time (i.e., release time) / s

% Compute the gaintable data (gains) for the dynamic compressor:
gtdata = zeros(1, round((gtmax-gtmin)/gtstep)+1);
n = 1;
for k = round((threshold-gtmin)/gtstep)+2:round((gtmax-gtmin)/gtstep)+1
    gtdata(k) = -gtstep * n;
    n = n + 1;
end

% Reduce numeric precision to guarantee conformity with C++ single-
% precision floating-point format ("float", or "mha_real_t" in openMHA):
gtdata = eval(mat2str(gtdata, 6));

% Plot the input-output characteristic (with logarithmic interpolation) of 
% the dynamic compressor:
figure;
dc_plot_io(gtmin, gtstep, gtdata, gtmin:gtmax, true);
grid on;
axis square;
xlim([gtmin gtmax]);
xlabel('Input Level / dB SPL');
ylim([gtmin gtmax]);
hold on;
plot([threshold threshold], ylim, 'r--');
text(threshold, gtmax, '{\it T}', 'Color', 'r', 'HorizontalAlignment', 'center', 'VerticalAlignment', 'bottom');
hold off;
ylabel('Output Level / dB SPL');
title({'Automatic gain control parameters (ACE)', ''});

% Display the results:
expression = '(?<=e[+-])0';
sGtdata = strtrim(sprintf('%g ', gtdata));
sGtdata = regexprep(sGtdata, expression, '');
length_sGtdata = length(sGtdata) + 4;  % account for brackets
maximumVariableLength_Windows = 8192;
if length_sGtdata > maximumVariableLength_Windows
    error('The maximum variable length in Windows batch files is %u characters (current length of DC_GTDATA: %u characters). To fix this, consider increasing the input level step size (currently %u dB).', maximumVariableLength_Windows, length_sGtdata, gtstep);
end
sGtmin = strtrim(sprintf('%g ', gtmin));
sGtmin = regexprep(sGtmin, expression, '');
sGtstep = strtrim(sprintf('%g ', gtstep));
sGtstep = regexprep(sGtstep, expression, '');
sTau_attack = strtrim(sprintf('%g ', tau_attack));
sTau_attack = regexprep(sTau_attack, expression, '');
sTau_decay = strtrim(sprintf('%g ', tau_decay));
sTau_decay = regexprep(sTau_decay, expression, '');
fprintf('Automatic gain control parameters for ACE (Linux):\n\n');
fprintf(['set DC_GTDATA="[[' sGtdata ']]"\n']);
fprintf(['export DC_GTMIN="[' strtrim(sprintf('%g ', gtmin)) ']"\n']);
fprintf(['export DC_GTSTEP="[' strtrim(sprintf('%g ', gtstep)) ']"\n']);
fprintf(['export DC_TAU_ATTACK="[' strtrim(sprintf('%g ', tau_attack)) ']"\n']);
fprintf(['export DC_TAU_DECAY="[' strtrim(sprintf('%g ', tau_decay)) ']"\n\n\n\n']);
fprintf('Automatic gain control parameters for ACE (Windows):\n\n');
fprintf(['set DC_GTDATA=[[' sGtdata ']]\n']);
fprintf(['set DC_GTMIN=[' strtrim(sprintf('%g ', gtmin)) ']\n']);
fprintf(['set DC_GTSTEP=[' strtrim(sprintf('%g ', gtstep)) ']\n']);
fprintf(['set DC_TAU_ATTACK=[' strtrim(sprintf('%g ', tau_attack)) ']\n']);
fprintf(['set DC_TAU_DECAY=[' strtrim(sprintf('%g ', tau_decay)) ']\n\n']);
