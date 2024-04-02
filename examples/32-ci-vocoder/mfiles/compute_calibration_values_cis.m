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


% Script for computing the calibrated input and output levels (in dB SPL) 
% corresponding to 0 dB FS (CIS strategy) and displaying them on the screen 
% in a format that is compatible with the Linux shell scripts and Windows 
% batch files included in this openMHA example

clear;
close all;
clc;

addpath(genpath('functions'));

% -------------------------------------------------------------------------

calibrationOffset_in = 0.0;  % calibration offset (input) / dB
calibrationOffset_out = 0.0;  % calibration offset (output) / dB

% Compute the calibrated input and output levels (in dB SPL) corresponding 
% to 0 dB FS:
calibrationPeakLevel_in = Pa2dB(1.0) + calibrationOffset_in;
calibrationPeakLevel_out = Pa2dB(1.0) + calibrationOffset_out;

% Reduce numeric precision to guarantee conformity with C++ single-
% precision floating-point format ("float", or "mha_real_t" in openMHA):
calibrationPeakLevel_in = eval(mat2str(calibrationPeakLevel_in, 6));
calibrationPeakLevel_out = eval(mat2str(calibrationPeakLevel_out, 6));

% Display the results:
expression = '(?<=e[+-])0';
sCalibrationPeakLevel_in = strtrim(sprintf('%g ', calibrationPeakLevel_in));
sCalibrationPeakLevel_in = regexprep(sCalibrationPeakLevel_in, expression, '');
sCalibrationPeakLevel_out = strtrim(sprintf('%g ', calibrationPeakLevel_out));
sCalibrationPeakLevel_out = regexprep(sCalibrationPeakLevel_out, expression, '');
fprintf('Calibration values for CIS (Linux):\n\n');
fprintf(['export CALIB_IN_PEAKLEVEL="[' sCalibrationPeakLevel_in ']"\n\n']);
fprintf(['export CALIB_OUT_PEAKLEVEL="[' sCalibrationPeakLevel_out ']"\n\n\n\n']);
fprintf('Calibration values for CIS (Windows):\n\n');
fprintf(['set CALIB_IN_PEAKLEVEL=[' sCalibrationPeakLevel_in ']\n\n']);
fprintf(['set CALIB_OUT_PEAKLEVEL=[' sCalibrationPeakLevel_out ']\n\n']);
