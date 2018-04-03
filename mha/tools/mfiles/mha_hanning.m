function [hanning, zero_padding_1, zero_padding_2] = ...
                                              mha_hanning(wndlen,fftlen,wndpos)
% [hanning, zero_padding_1, zero_padding_2] = ...
%     mha_hanning(wndlen, fftlen [,wndpos])
%
% produce the same hanning window as used in the mha for the given window
% length, fft length, and window position. Zero padding vectors are returned
% as optional return parameters.
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2005 2006 2015 2018 HörTech gGmbH

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
							   
  if nargin < 3
    wndpos = 0.5;
  end
 
  zero_padding_1 = zeros(floor((fftlen - wndlen) * wndpos),1);
  zero_padding_2 = zeros(fftlen - wndlen - size(zero_padding_1, 1), 1);

  % This is the same hanning window as used in the mha
  hanning = (1-cos(2*pi*[0:(wndlen-1)] / wndlen)') / 2 / ...
            sqrt(0.375*wndlen/fftlen) / fftlen;

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
