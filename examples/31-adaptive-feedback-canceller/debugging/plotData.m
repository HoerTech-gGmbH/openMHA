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

function plotData(stData,stPlot,idx)

    plot(stPlot.hAx1,1:stData.flt_len_true,stData.FBfilter_true, ...
              1:stData.flt_len_estim,stData.FBfilter_estim(:,idx));
    title(stPlot.hAx1,'FB Filter (Time)');
    xlabel(stPlot.hAx1,'Filter Coefficients [ ]');
    ylabel(stPlot.hAx1,'Coefficient Value [ ]');
    hLeg1 = legend(stPlot.hAx1,'true','estim');
    box(stPlot.hAx1,'on');
    grid(stPlot.hAx1,'on');
    
    plot(stPlot.hAx2, ...
              1:stData.fragsize,stData.LSsig(:,idx), ...
              1:stData.fragsize,filter(stData.FBfilter_estim(:,idx),1, ...
                                       stData.LSsig(:,idx)), ...
              1:stData.fragsize,stData.ERRsig(:,idx));
    title(stPlot.hAx2,'LS, est. FB and ERR signal (Time)');
    xlabel(stPlot.hAx2,'Time [samples]');
    ylabel(stPlot.hAx2,'Amplitude [ ]');
    hLeg2 = legend(stPlot.hAx2,'LS','FBest','ERR');
    box(stPlot.hAx2,'on');
    grid(stPlot.hAx2,'on');
    
    plot(stPlot.hAx3,1:stData.fragsize,stData.input_block);
    title(stPlot.hAx3,'Input signal (Time)');
    xlabel(stPlot.hAx3,'Time [samples]');
    ylabel(stPlot.hAx3,'Amplitude [ ]');
    box(stPlot.hAx3,'on');
    grid(stPlot.hAx3,'on');
    
    TF_estim = 20*log10(abs(fft(stData.FBfilter_estim(:,idx),stData.nfft_flt)));
    TF_estim = TF_estim(1:stData.nfft_flt/2+1);
    plot(stPlot.hAx4,stData.vFreq_flt,stData.TF_true, ...
              stData.vFreq_flt,TF_estim);
    title(stPlot.hAx4,'FB Filter (Freq)');
    xlabel(stPlot.hAx4,'Frequency [Hz]');
    ylabel(stPlot.hAx4,'Magnitude [dB]');
    hLeg4 = legend(stPlot.hAx4,'true','estim');
    box(stPlot.hAx4,'on');
    grid(stPlot.hAx4,'on');
    
    LSsig_spec = 20*log10(abs(fft(stData.LSsig(:,idx),stData.nfft_sig)));
    FBsig_estim_spec = 20*log10(abs(fft( ...
                              filter(stData.FBfilter_estim(:,idx),1, ...
                                     stData.LSsig(:,idx)), ...
                              stData.nfft_sig)));
    plot(stPlot.hAx5,stData.vFreq_sig,LSsig_spec(1:stData.nfft_sig/2+1), ...
              stData.vFreq_sig,FBsig_estim_spec(1:stData.nfft_sig/2+1));
    title(stPlot.hAx5,'LS and est. FB signal (Freq)');
    xlabel(stPlot.hAx5,'Frequency [Hz]');
    ylabel(stPlot.hAx5,'Magnitude [dB]');
    hLeg5 = legend(stPlot.hAx5,'LSsig','FBest');
    box(stPlot.hAx5,'on');
    grid(stPlot.hAx5,'on');
    
    plot(stPlot.hAx6,1:stData.fragsize,stData.estim_err(:,idx), ...
              1:stData.fragsize,stData.current_power(:,idx));
    title(stPlot.hAx6,'Estimation Error and LSsig power (Time)');
    xlabel(stPlot.hAx6,'Time [samples]');
    ylabel(stPlot.hAx6,' ');
    hLeg6 = legend(stPlot.hAx6,'estim err','power');
    box(stPlot.hAx6,'on');
    grid(stPlot.hAx6,'on');
end
