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


% Script for computing the parameters of the loudness growth function in 
% acoustic-to-electric conversion (CIS strategy), plotting the results, and 
% displaying them on the screen in a format that is compatible with the 
% Linux shell scripts and Windows batch files included in this openMHA 
% example

clear;
close all;
clc;

addpath(genpath('functions'));

% -------------------------------------------------------------------------

B = 25;                        % base level of the input (acoustic) dynamic range / dB SPL
S = 100;                       % saturation level of the input (acoustic) dynamic range / dB SPL
compressionCoefficient = 500;  % compression coefficient of the loudness growth function
THR = 60;                      % threshold level of the output (electrical) dynamic range / cu
MCL = 600;                     % maximum comfortable level of the output (electrical) dynamic range / cu

% Compute loudness growth function, mapping of electrical dynamic range to 
% current units (cu), and input and output dynamic range, based on Nogueira 
% et al. (2005):
% 
% Nogueira, W., Büchner, A., Lenarz, T., & Edler, B. (2005). A 
% psychoacoustic "NofM"-type speech coding strategy for cochlear implants. 
% EURASIP Journal on Advances in Signal Processing, 2005, Article 101672. 
% https://doi.org/10.1155/ASP.2005.3044
% 
B_linear = dB2Pa(B);
S_linear = dB2Pa(S);
envelopeMagnitude = B_linear:0.0001:S_linear;
x_min = 0;
x_resolution = 0.2;
if mod(S_linear, x_resolution) == 0
    x_max = S_linear + x_resolution;
else
    x_max = ceil(S_linear/x_resolution) * x_resolution;
end
proportionOfDynamicRange = log(1 + compressionCoefficient * (envelopeMagnitude - B_linear)/(S_linear - B_linear))/log(1 + compressionCoefficient);
envelopeMagnitude = [x_min envelopeMagnitude x_max];
proportionOfDynamicRange = [0 proportionOfDynamicRange 1];
stimulationLevel = THR + ((MCL - THR) * proportionOfDynamicRange);
inputDynamicRange = S - B;
outputDynamicRange = 20 * log10(cu2A_cis(MCL)) - 20 * log10(cu2A_cis(THR));

% Reduce numeric precision to guarantee conformity with C++ single-
% precision floating-point format ("float", or "mha_real_t" in openMHA):
B = eval(mat2str(B, 6));
S = eval(mat2str(S, 6));
compressionCoefficient = eval(mat2str(compressionCoefficient, 6));
THR = eval(mat2str(THR, 6));
MCL = eval(mat2str(MCL, 6));

% Plot the loudness growth function:
figure;
plot(envelopeMagnitude, stimulationLevel);
hold on;
y_min = 0;
y_resolution = 100;
if mod(MCL, y_resolution) == 0
    y_max = MCL + y_resolution;
else
    y_max = ceil(MCL/y_resolution) * y_resolution;
end
plot([B_linear B_linear], [y_min y_max], 'r--');
plot([S_linear S_linear], [y_min y_max], 'r--');
plot([x_min x_max], [THR THR], 'r--');
plot([x_min x_max], [MCL MCL], 'r--');
text(B_linear, y_max, '{\it B}', 'Color', 'r', 'HorizontalAlignment', 'center', 'VerticalAlignment', 'bottom');
text(S_linear, y_max, '{\it S}', 'Color', 'r', 'HorizontalAlignment', 'center', 'VerticalAlignment', 'bottom');
text(x_max, THR, '{\it THR}', 'Color', 'r');
text(x_max, MCL, '{\it MCL}', 'Color', 'r');
hold off;
grid on;
xlim([x_min x_max]);
xticks(x_min:x_resolution:x_max);
xlabel('Envelope Magnitude / Pa');
ylim([y_min y_max]);
yticks(y_min:y_resolution:y_max);
ylabel('Stimulation Level / cu');
title({'Loudness Growth Function (CIS)', ''});

% Display the results:
expression = '(?<=e[+-])0';
sCompressionCoefficient = strtrim(sprintf('%g ', compressionCoefficient));
sCompressionCoefficient = regexprep(sCompressionCoefficient, expression, '');
sB_linear = strtrim(sprintf('%g ', B_linear));
sB_linear = regexprep(sB_linear, expression, '');
sS_linear = strtrim(sprintf('%g ', S_linear));
sS_linear = regexprep(sS_linear, expression, '');
sTHR = strtrim(sprintf('%g ', THR));
sTHR = regexprep(sTHR, expression, '');
sMCL = strtrim(sprintf('%g ', MCL));
sMCL = regexprep(sMCL, expression, '');
sInputDynamicRange = strtrim(sprintf('%.0f ', inputDynamicRange));
sInputDynamicRange = regexprep(sInputDynamicRange, expression, '');
sOutputDynamicRange = strtrim(sprintf('%.0f ', outputDynamicRange));
sOutputDynamicRange = regexprep(sOutputDynamicRange, expression, '');
fprintf('Loudness growth function for CIS (Linux):\n\n');
fprintf(['export ELECTRODOGRAM_COMPRESSION_COEFFICIENT=' sCompressionCoefficient '\n\n']);
fprintf(['export ELECTRODOGRAM_BASE_LEVEL=' sB_linear '\n\n']);
fprintf(['export ELECTRODOGRAM_SATURATION_LEVEL=' sS_linear '\n\n']);
fprintf(['export ELECTRODOGRAM_THRESHOLD_LEVEL="[' sTHR ']"\n\n']);
fprintf(['export ELECTRODOGRAM_COMFORT_LEVEL="[' sMCL ']"\n\n\n\n']);
fprintf('Loudness growth function for CIS (Windows):\n\n');
fprintf(['set ELECTRODOGRAM_COMPRESSION_COEFFICIENT=' sCompressionCoefficient '\n\n']);
fprintf(['set ELECTRODOGRAM_BASE_LEVEL=' sB_linear '\n\n']);
fprintf(['set ELECTRODOGRAM_SATURATION_LEVEL=' sS_linear '\n\n']);
fprintf(['set ELECTRODOGRAM_THRESHOLD_LEVEL=[' sTHR ']\n\n']);
fprintf(['set ELECTRODOGRAM_COMFORT_LEVEL=[' sMCL ']\n\n\n\n']);
fprintf(['Input (acoustic) dynamic range: ' sInputDynamicRange ' dB\n\n']);
fprintf(['Output (electric) dynamic range: ' sOutputDynamicRange ' dB\n\n']);
