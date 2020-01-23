function [peak,fir] = fresponse_to_cfg(fresponse)
% function fresponse_to_cfg(fresponse)
% fresponse: struct with fields
% Frequencies, dBFSfor80dB, correctionsdB, REUG as created by measure_fresponse
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2018 2019 HörTech gGmbH
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

  fir_length = 65; % Adjust as desired.
  % Greater Length equals better calibration result but also more delay.
  
  gains = fresponse.dBFSfor80dB + fresponse.REUGdB + fresponse.correctionsdB;
  peak_correction = max(gains);
  gains = gains - peak_correction;
  peak = 0  - peak_correction + 80; % the level we used for calibration
  fir = fir2(fir_length, [0 (fresponse.Frequencies / fresponse.sampling_rate * 2) 1], 10.^([gains(1), gains, -inf] / 20));

  fprintf('The following values can be configured into the transducers plugin for the audio output channel where this output hardware is connected:\n')
  fprintf('calib_out.peaklevel = [... %f ...]\n', peak)
  fprintf('calib_out.fir = [... ;[')
  fprintf('%.7f ', fir)
  fprintf(']; ...]\n')
  
