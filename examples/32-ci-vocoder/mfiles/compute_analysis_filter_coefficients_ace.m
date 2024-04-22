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
% simulation (ACE strategy), plotting the results, and displaying them on
% the screen in a format that is compatible with the Linux shell scripts
% and Windows batch files included in this openMHA example

clear;
close all;
clc;

if isunix
    addpath(genpath('/usr/lib/openmha/mfiles'));
elseif ismac
    error('Depending on the Mac processor type, the correct path to the openMHA mfiles directory must be specified manually.');
    % addpath(genpath('/usr/local/lib/openmha/mfiles'));  % Macs with Intel processor
    % addpath(genpath('/opt/homebrew/lib/openmha/mfiles'));  % Macs with ARM processor
elseif ispc
    sProgramFilesPath = getenv('PROGRAMFILES');
    addpath(genpath([sProgramFilesPath '\openMHA\mfiles']));
end

if isoctave
    pkg load statistics;
end

% -------------------------------------------------------------------------

% For the ACE strategy, the filterbank for analysis (i.e., CI simulation)
% is not a gammatone filterbank. Instead, it is a fixed, frequency domain
% filterbank that cannot be modified by the user, which is shown here for
% reference only. The electrode-specific vector of center frequencies is
% fc = [242 370 496 622 747 873 998 1123 1248 1432 1683 1933 2184 2493 2869 3303 3804 4364 4990 5675 6485 7421] Hz
% (using the geometric mean of the filter cutoff frequencies). The
% parameters below are taken from Nogueira et al. (2005):
%
% Nogueira, W., Büchner, A., Lenarz, T., & Edler, B. (2005). A
% psychoacoustic "NofM"-type speech coding strategy for cochlear implants.
% EURASIP Journal on Advances in Signal Processing, 2005, Article 101672.
% https://doi.org/10.1155/ASP.2005.3044

srate_ace = 16000;  % sampling rate used by the ACE strategy / Hz
nfft = 128;         % FFT length / bins
M = 22;             % number of electrodes
nbins = [2 1 1 1 1 1 1 1 1 1 2 2 2 2 3 3 4 4 5 5 6 7 8 1];  % number of FFT bins per filter (the first and last filter being unused)
weights = [0.98 0.98 0.98 0.98 0.98 0.98 0.98 0.98 0.98 0.68 0.68 0.68 0.68 0.65 0.65 0.65 0.65 0.65 0.65 0.65 0.65 0.65];  % weights
srate = 48000;      % sampling rate / Hz

% Compute the default center frequencies / Hz:
bin_idx_start = zeros(1, length(nbins));
bin_idx_end = zeros(1, length(nbins));
bin_idx_end(1) = bin_idx_start(1) + nbins(1) - 1;
for k = 2:length(nbins)
    bin_idx_start(k) = bin_idx_end(k-1) + 1;
    bin_idx_end(k) = bin_idx_start(k) + nbins(k) - 1;
end
fc_arithmetic = NaN(1, length(nbins)-2);
fc_geometric = NaN(1, length(nbins)-2);
for k = 2:length(nbins)-1
    fc_arithmetic(k-1) = floor(mean([bin_idx_start(k)-0.5 bin_idx_end(k)+0.5]) * srate_ace/nfft);  % reported in Nogueira et al. (2005)
    fc_geometric(k-1) = round(geomean([bin_idx_start(k)-0.5 bin_idx_end(k)+0.5]) * srate_ace/nfft);  % reported in this openMHA example
end

% Plot the frequency response of the filterbank:
figure;
bin_width = srate_ace/nfft;
hold on;
C = get(gca, 'ColorOrder');
for k = 1:M
    x = [(bin_idx_start(k+1)-0.5)*bin_width ...
        (bin_idx_start(k+1)-0.5)*bin_width ...
        (bin_idx_end(k+1)+0.5)*bin_width ...
        (bin_idx_end(k+1)+0.5)*bin_width];
    y = [eps ...
        weights(k) ...
        weights(k) ...
        eps];
    plot(x, 10*log10(y), 'Color', C(1, :));  % the weights are applied to powers, not magnitudes
end
hold off;
box on;
grid on;
xlim([0 srate/2]);
xticks(0:2000:srate/2);
xticklabels((0:2000:srate/2)/1000);
xlabel('Frequency / kHz');
ylim([-40 0]);
ylabel('Magnitude / dB');
title({'Analysis Filterbank (ACE)', ''});

% Display the results:
expression = '(?<=e[+-])0';
sFc_arithmetic = strtrim(sprintf('%.0f ', fc_arithmetic));
sFc_arithmetic = regexprep(sFc_arithmetic, expression, '');
sFc_geometric = strtrim(sprintf('%.0f ', fc_geometric));
sFc_geometric = regexprep(sFc_geometric, expression, '');
fprintf('Analysis filterbank center frequencies for ACE based on arithmetic mean (Nogueira et al., 2005) / Hz:\n\n');
fprintf(['fc = [' sFc_arithmetic '];\n\n\n\n']);
fprintf('Analysis filterbank center frequencies for ACE based on geometric mean (this openMHA example) / Hz:\n\n');
fprintf(['fc = [' sFc_geometric '];\n\n\n\n']);
