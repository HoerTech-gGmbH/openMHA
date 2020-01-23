function [peak,eq] = fresponse_to_eq(fresponse,srate,fftlen,print_result)
% function [peak,eq] = fresponse_to_eq(fresponse,srate,fftlen,print_result)
% computes peak level and equalizer settings from fresponse
% INPUT:
% fresponse: struct with fields "Frequencies", "dBFSfor80dB",
%            "correctionsdB", "REUG" as created by measure_fresponse
% srate:     sampling rate / Hz
% fftlen:    FFT length in samples
% print_result: Optional boolean parameter. Suggested MHA settings will be
%               printed when true
% OUTPUT:
% peak: suggested output peak level from measurement and correction data
% eq:   suggested equalizer setting to apply the correction filter.

% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2019 HörTech gGmbH
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

  gains = fresponse.dBFSfor80dB + fresponse.REUGdB + fresponse.correctionsdB;
  peak_correction = max(gains);
  gains = gains - peak_correction;
  peak = 0  - peak_correction + 80; % the level we used for calibration
  frequencies=linspace(0,srate/2,fftlen/2+1);
  eq=interp1(fresponse.Frequencies,gains,frequencies,'linear','extrap');
  eq = 10.^(eq/20);
  if ~exist('print_result','var')
    print_result=false;
  end
  if print_result
     fprintf('The following values can be configured into the transducers plugin for the audio output channel where this output hardware is connected:\n');
     fprintf('calib_out.peaklevel = [... %f ...]\n', peak);
     fprintf('equalize.gains=[...;[');
     fprintf('%.2f ',eq);
     fprintf(']; ...]\n');
  end
