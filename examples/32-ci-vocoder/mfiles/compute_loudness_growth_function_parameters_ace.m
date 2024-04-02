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
% acoustic-to-electric conversion (ACE strategy), plotting the results, and 
% displaying them on the screen in a format that is compatible with the 
% Linux shell scripts and Windows batch files included in this openMHA 
% example

clear;
close all;
clc;

addpath(genpath('functions'));

% -------------------------------------------------------------------------

B = 25;                           % base level of the input (acoustic) dynamic range / dB SPL
S = 65;                           % saturation level of the input (acoustic) dynamic range / dB SPL
compressionCoefficient = 416.2;   % compression coefficient of the loudness growth function
T = 96;                           % threshold level of the output (electrical) dynamic range / CU
C = 160;                          % comfort level of the output (electrical) dynamic range / CU

% Compute loudness growth function, mapping of electrical dynamic range to 
% clinical current units (CU), and input and output dynamic range, based on 
% Nogueira et al. (2005):
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
x_resolution = 0.005;
if mod(S_linear, x_resolution) == 0
    x_max = S_linear + x_resolution;
else
    x_max = ceil(S_linear/x_resolution) * x_resolution;
end
proportionOfDynamicRange = log(1 + compressionCoefficient * (envelopeMagnitude - B_linear)/(S_linear - B_linear))/log(1 + compressionCoefficient);
envelopeMagnitude = [x_min envelopeMagnitude x_max];
proportionOfDynamicRange = [0 proportionOfDynamicRange 1];
stimulationLevel = T + ((C - T) * proportionOfDynamicRange);
inputDynamicRange = S - B;
outputDynamicRange = 20 * log10(CU2A_ace(C)) - 20 * log10(CU2A_ace(T));

% Reduce numeric precision to guarantee conformity with C++ single-
% precision floating-point format ("float", or "mha_real_t" in openMHA):
B = eval(mat2str(B, 6));
S = eval(mat2str(S, 6));
compressionCoefficient = eval(mat2str(compressionCoefficient, 6));
T = eval(mat2str(T, 6));
C = eval(mat2str(C, 6));

% Plot the loudness growth function:
figure;
plot(envelopeMagnitude, stimulationLevel);
hold on;
y_min = 0;
y_resolution = 20;
if mod(C, y_resolution) == 0
    y_max = C + y_resolution;
else
    y_max = ceil(C/y_resolution) * y_resolution;
end
plot([B_linear B_linear], [y_min y_max], 'r--');
plot([S_linear S_linear], [y_min y_max], 'r--');
plot([x_min x_max], [T T], 'r--');
plot([x_min x_max], [C C], 'r--');
text(B_linear, y_max, '{\it B}', 'Color', 'r', 'HorizontalAlignment', 'center', 'VerticalAlignment', 'bottom');
text(S_linear, y_max, '{\it S}', 'Color', 'r', 'HorizontalAlignment', 'center', 'VerticalAlignment', 'bottom');
text(x_max, T, '{\it T}', 'Color', 'r');
text(x_max, C, '{\it C}', 'Color', 'r');
hold off;
grid on;
xlim([x_min x_max]);
xticks(x_min:x_resolution:x_max);
xlabel('Envelope Magnitude / Pa');
ylim([y_min y_max]);
yticks(y_min:y_resolution:y_max);
ylabel('Stimulation Level / CU');
title({'Loudness Growth Function (ACE)', ''});

% Display the results:
expression = '(?<=e[+-])0';
sCompression_coefficient = strtrim(sprintf('%g ', compressionCoefficient));
sCompression_coefficient = regexprep(sCompression_coefficient, expression, '');
sB_linear = strtrim(sprintf('%g ', B_linear));
sB_linear = regexprep(sB_linear, expression, '');
sS_linear = strtrim(sprintf('%g ', S_linear));
sS_linear = regexprep(sS_linear, expression, '');
sT = strtrim(sprintf('%g ', T));
sT = regexprep(sT, expression, '');
sC = strtrim(sprintf('%g ', C));
sC = regexprep(sC, expression, '');
sInputDynamicRange = strtrim(sprintf('%.0f ', inputDynamicRange));
sInputDynamicRange = regexprep(sInputDynamicRange, expression, '');
sOutputDynamicRange = strtrim(sprintf('%.0f ', outputDynamicRange));
sOutputDynamicRange = regexprep(sOutputDynamicRange, expression, '');
fprintf('Loudness growth function for ACE (Linux):\n\n');
fprintf(['export ELECTRODOGRAM_COMPRESSION_COEFFICIENT=' sCompression_coefficient '\n\n']);
fprintf(['export ELECTRODOGRAM_BASE_LEVEL=' sB_linear '\n\n']);
fprintf(['export ELECTRODOGRAM_SATURATION_LEVEL=' sS_linear '\n\n']);
fprintf(['export ELECTRODOGRAM_THRESHOLD_LEVEL="[' sT ']"\n\n']);
fprintf(['export ELECTRODOGRAM_COMFORT_LEVEL="[' sC ']"\n\n\n\n']);
fprintf('Loudness growth function for ACE (Windows):\n\n');
fprintf(['set ELECTRODOGRAM_COMPRESSION_COEFFICIENT=' sCompression_coefficient '\n\n']);
fprintf(['set ELECTRODOGRAM_BASE_LEVEL=' sB_linear '\n\n']);
fprintf(['set ELECTRODOGRAM_SATURATION_LEVEL=' sS_linear '\n\n']);
fprintf(['set ELECTRODOGRAM_THRESHOLD_LEVEL=[' sT ']\n\n']);
fprintf(['set ELECTRODOGRAM_COMFORT_LEVEL=[' sC ']\n\n\n\n']);
fprintf(['Input (acoustic) dynamic range: ' sInputDynamicRange ' dB\n\n']);
fprintf(['Output (electric) dynamic range: ' sOutputDynamicRange ' dB\n\n']);
