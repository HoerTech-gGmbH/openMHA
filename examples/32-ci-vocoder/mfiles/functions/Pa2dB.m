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


% Function for converting sound pressure p (in Pa) to sound pressure level 
% L (in dB SPL)
% 
% Usage:
% L = Pa2dB(p)
% 
% Input parameter:
% p  sound pressure / Pa
% 
% Output parameter:
% L  sound pressure level / dB SPL
% 
% -------------------------------------------------------------------------

function L = Pa2dB(p)
    p0 = 20 * 10^(-6);  % reference sound pressure / Pa

    L = 20 * log10(p/p0);
end
