% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2022 Hörzentrum Oldenburg gGmbH
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

function stData = run_afc_debug(bPlot,early_stop,stMHA_param)
if nargin < 3
  stMHA_param = struct();
end
if nargin < 2
  early_stop = [];
end
if nargin < 1
  bPlot = true;
end

addpath(mha_install_dirs('mfiles'));
if isoctave()
    % We use functions from signal package
    pkg load signal;
end

% load feedback path
stData.FBfilter_true = [-.25; .25; .25; -.25];
stData.flt_len_true = length(stData.FBfilter_true);
stData.flt_len_estim = 20;

% read and start openMHA-cfg
mha = mha_start();
mha_query(mha,'','read:afc_debug.cfg');
stData.srate = mha_get(mha,'srate');
stData.fragsize = mha_get(mha,'fragsize');
stData.flt_len_estim = mha_get(mha, 'mha.afc.filter_length');
add_param = fieldnames(stMHA_param);
for idx = 1:length(add_param)
  mha_set(mha,stMHA_param.(add_param{idx}).name, ...
              stMHA_param.(add_param{idx}).value);
end

% generate target signal
stData.sig_len_sec = 100;
num_loops = ceil(stData.srate * stData.sig_len_sec / stData.fragsize);
randn('seed',42);
stData.target_sig = randn(1,stData.sig_len_sec * stData.srate + ...
                            (stData.fragsize - mod(stData.sig_len_sec * stData.srate, ...
                                                   stData.fragsize)));
% apply gain to prevent clipping or near-clipping values
stData.target_sig = stData.target_sig * 0.1;
stData.target_sig = reshape(stData.target_sig, stData.fragsize, num_loops);

% create additional variables to save states of input, output and AC variables
stData.LSsig = zeros(stData.fragsize, num_loops);
stData.FBsig = zeros(stData.fragsize, num_loops+1);
stData.FBfilter_estim = zeros(stData.flt_len_estim, num_loops);
stData.ERRsig = zeros(stData.fragsize, num_loops);
stData.current_power = zeros(stData.fragsize, num_loops);
stData.estim_err = zeros(stData.fragsize, num_loops);
LSsigBuffer = zeros(stData.flt_len_true,1);

if bPlot
  stPlot = preparePlot();
end

stData.nfft_sig = 128;
stData.vFreq_sig = linspace(0,stData.srate/2,stData.nfft_sig/2+1);
stData.nfft_flt = 512;
stData.vFreq_flt = linspace(0,stData.srate/2,stData.nfft_flt/2+1);
stData.TF_true = 20*log10(abs(fft(stData.FBfilter_true,stData.nfft_flt)));
stData.TF_true = stData.TF_true(1:stData.nfft_flt/2+1);

mha_set(mha,'cmd','prepare');
mha_set(mha,'cmd','start');
next_stop = 0;
for idx = 1:num_loops
  % generate input block for mha
  stData.input_block = stData.target_sig(:,idx)' + stData.FBsig(:,idx)';
  mha_set(mha,'io.input',stData.input_block);

  % get output of mha and compute new block of feedback signal
  stData.LSsig(:,idx) = mha_get(mha,'io.output')';
  % use LSsigBuffer for filtering instead of filter() to preserve filter
  % state, this is crucial!
  for kf = 1:stData.fragsize
    LSsigBuffer = [stData.LSsig(kf,idx); LSsigBuffer(1:end-1)];
    stData.FBsig(kf,idx+1) = LSsigBuffer' * stData.FBfilter_true;
  end
  
  % write AC-variables to respective matrices
  stData.FBfilter_estim(:,idx) = mha_get(mha,'mha.acmon.FBfilter_estim')';
  stData.ERRsig(:,idx) = mha_get(mha,'mha.acmon.ERRsig')';
  stData.current_power(:,idx) = mha_get(mha,'mha.acmon.current_power');
  stData.estim_err(:,idx) = mha_get(mha,'mha.acmon.estim_err');

  % define certain conditions at which the loop stops and the current data
  % is shown
  if (idx == 1 || idx == next_stop) && bPlot
    plotData(stData,stPlot,idx);
    disp(['The current index is ', num2str(idx)]);
    next_stop = input('Type in the index you want to stop at next: ');
    % if you hit enter without any input the loop will stop
    if isempty(next_stop)
      close all;
      break;
    end
  else
    if mod(idx,1000) == 0
      idx
    end
  end
  if idx == early_stop
    stData.TF_estim = 20*log10(abs(fft(stData.FBfilter_estim(:,idx),stData.nfft_flt)));
    close all;
    break;
  end
end
mha_set(mha,'cmd','quit');
save('afc_now_results.mat','stData');
end
