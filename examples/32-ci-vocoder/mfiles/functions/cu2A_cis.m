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


% Function for converting current units (cu) to current I (in A) for the 
% CIS strategy
% 
% Usage:
% I = cu2A_cis(cu)
% 
% Input parameter:
% cu  current units / cu
% 
% Output parameter:
% I   current / A
% 
% -------------------------------------------------------------------------

function I = cu2A_cis(cu)
    I = cu * 10^(-6);  % µA to A
end
