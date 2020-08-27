function plot_fresponse(fresponse)
% function plot_fresponse(fresponse)
% fresponse: struct with fields
% Frequencies, dBFSfor80dB, correctionsdB, REUG as created by measure_fresponse
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2018 HörTech gGmbH
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

  figure;
  gains = fresponse.dBFSfor80dB + fresponse.REUGdB + fresponse.correctionsdB;
  mingain = min(gains);
  gains = gains - mingain;
  semilogx(fresponse.Frequencies, gains);
  set(gca,'xtick',fresponse.Frequencies);
  labels = {};
  for f = fresponse.Frequencies
    labels = [labels, {num2str(f)}];
  end
  set(gca,'xticklabel',labels);
  grid on;

  ylabel('Required gain in dB');
  xlabel('Frequency in Hz');
  title('Calibration filter frequency response for the measured output hardware');
  
