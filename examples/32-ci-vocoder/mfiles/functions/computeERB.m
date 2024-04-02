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


% Function for computing the equivalent rectangular bandwith (ERB) of an
% auditory filter (in Hz), according to Glasberg & Moore (1990):
%
% Glasberg, B. R., & Moore, B. C. J. (1990). Derivation of auditory filter
% shapes from notched-noise data. Hearing Research, 47, 103-138.
% https://doi.org/10.1016/0378-5955(90)90170-T
% 
% Usage:
% ERB = computeERB(fc)
% 
% Input parameter:
% fc   filter center frequency / Hz
% 
% Output parameter:
% ERB  equivalent rectangular bandwidth (ERB) of filter / Hz
% 
% -------------------------------------------------------------------------

function ERB = computeERB(fc)
    fc = fc/1000;  % Hz to kHz
    ERB = 24.7 * (4.37 * fc + 1);
end
