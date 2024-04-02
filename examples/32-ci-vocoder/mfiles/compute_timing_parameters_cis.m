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


% Script for computing a set of interdependent timing parameters (CIS 
% strategy) and displaying them on the screen in a format that is 
% compatible with the Linux shell scripts and Windows batch files included 
% in this openMHA example

clear;
close all;
clc;

% -------------------------------------------------------------------------

desired_stimulation_rate = 2000;  % desired (per-electrode) stimulation rate / pps
fragsize = 1024;                  % outer fragment size / frames

% Compute the resulting parameters:
vocoder_srate = 48000;  % vocoder sampling rate / Hz
srate = 48000;  % audio sampling rate / Hz
vocoder_fragsize = round(vocoder_srate/desired_stimulation_rate);  % vocoder fragment size / frames
dbasync_fragsize = vocoder_fragsize;  % inner fragment size / frames
dbasync_delay = dbasync_fragsize - gcd(dbasync_fragsize, fragsize);  % delay for dbasync / frames
closest_stimulation_rate = vocoder_srate/vocoder_fragsize;  % closest achievable stimulation rate / pps

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
sDesired_stimulation_rate = strtrim(sprintf('%.0f ', desired_stimulation_rate));
sDesired_stimulation_rate = regexprep(sDesired_stimulation_rate, expression, '');
sClosest_stimulation_rate = strtrim(sprintf('%.0f ', closest_stimulation_rate));
sClosest_stimulation_rate = regexprep(sClosest_stimulation_rate, expression, '');
sVocoder_srate = strtrim(sprintf('%u ', vocoder_srate));
sVocoder_srate = regexprep(sVocoder_srate, expression, '');
fprintf('Timing parameters for CIS (Linux):\n\n');
fprintf(['export FRAGSIZE=' sFragsize '\n\n']);
fprintf(['export SRATE=' sSrate '\n\n']);
fprintf(['export DBASYNC_FRAGSIZE=' sDbasync_fragsize '\n\n']);
fprintf(['export DBASYNC_DELAY=' sDbasync_delay '\n\n\n\n']);
fprintf('Timing parameters for CIS (Windows):\n\n');
fprintf(['set FRAGSIZE=' sFragsize '\n\n']);
fprintf(['set SRATE=' sSrate '\n\n']);
fprintf(['set DBASYNC_FRAGSIZE=' sDbasync_fragsize '\n\n']);
fprintf(['set DBASYNC_DELAY=' sDbasync_delay '\n\n\n\n']);
fprintf(['Desired stimulation rate: ' sDesired_stimulation_rate ' pps\n']);
fprintf(['Closest achievable stimulation rate given the (fixed) vocoder sampling rate (' sVocoder_srate ' Hz): ' sClosest_stimulation_rate ' pps\n\n']);
