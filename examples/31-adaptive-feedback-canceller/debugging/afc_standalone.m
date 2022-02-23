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

function afc_standalone()
  
addpath(mha_install_dirs('mfiles'));
if isoctave()
    pkg load signal;
end
randn('seed',42);
vTarget = 0.1 * randn(16000,1);
vTarget = filter(fir1(64,.1,'high'),1,vTarget);
vFBfilter_true = [-.25;.25;.25;-.25;zeros(296,1)];
iNumSampDelay = 1;
fGainDB = 0.0;
mFBfilter_estim = zeros(length(vFBfilter_true),length(vTarget)+1);
fStepSize = 0.01;
fAlpha = 1e-20;
iFs = 16000;
iNfft = 256;
vMICsig      = zeros(length(vTarget),1);
vLSsig       = zeros(length(vTarget)+length(vFBfilter_true),1);
vLSsigBuffer = zeros(length(vFBfilter_true),1);
vERRsig       = zeros(length(vTarget),1);
vERRsigBuffer = zeros(iNumSampDelay+1,1);
vCurrentPower = zeros(length(vTarget),1);
vGainPath    = [zeros(iNumSampDelay,1); 10^(fGainDB/10)];
vNfce        = zeros(length(vTarget),1);
vMsg         = zeros(length(vTarget),1);

vFbPathFreq = fft(vFBfilter_true,iNfft);
next_stop = 0;
for idx = 1:length(vTarget)
    % compute MICsig
    vMICsig(idx) = vTarget(idx) + vLSsigBuffer' * vFBfilter_true;
    % compute ERRsig, the filtering of LSsig belongs to the backward path
    vERRsig(idx) = vMICsig(idx) - vLSsigBuffer' * mFBfilter_estim(:,idx);
    vERRsigBuffer = [vERRsig(idx); vERRsigBuffer(1:end-1)];
    % BACKWARD PATH: compute current_power and update the filter estimation 
    % via the NLMS method
    vCurrentPower(idx) = vLSsigBuffer' * vLSsigBuffer;
    mFBfilter_estim(:,idx+1) = mFBfilter_estim(:,idx) + fStepSize ./ ...
        (fAlpha + vCurrentPower(idx)) .* ...
         vLSsigBuffer .* vERRsig(idx);
    % FORWARD PATH: apply gain to ERRsig
    vLSsig(idx) = vERRsigBuffer.' * vGainPath; 
    vLSsigBuffer = [vLSsig(idx) ; vLSsigBuffer(1:end-1,1)];
    % to show performance of algorithm compute normalized filter
    % coefficient error (NFCE)
    vNfce(idx,1) = (vFBfilter_true - mFBfilter_estim(:,idx))' * ...
        (vFBfilter_true - mFBfilter_estim(:,idx)) / (vFBfilter_true' * vFBfilter_true);
    
    vAdapFFreq = fft(mFBfilter_estim(:,idx),iNfft);
    % to show performance of algorithm compute maximum stable gain (MSG)
    vMsg(idx,1) = 10 * log10(min(1 ./ ...
        abs(vFbPathFreq(1:iNfft/2+1) - vAdapFFreq(1:iNfft/2+1)) .^ 2));
    if mod(idx,1000) == 0
      idx
    end
end
save('afc_orig_results.mat','vTarget','vMICsig','mFBfilter_estim', ...
    'vLSsig','vERRsig','vCurrentPower','vNfce','vMsg');
end
