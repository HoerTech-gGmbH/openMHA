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


% Function for converting clinical current units (CU) to current I (in A) 
% for the ACE strategy
% 
% Usage:
% I = CU2A_ace(CU)
% 
% Input parameter:
% CU  clinical current units / CU
% 
% Output parameter:
% I   current / A
% 
% -------------------------------------------------------------------------

function I = CU2A_ace(CU)
    I_min = 17.5;  % minimum current / µA
    CU_max = 255;  % maximum clinical current unit (CU)
    
    I = I_min * 100^(CU/CU_max);
    I = I * 10^(-6);  % µA to A
end
