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


% Function for computing two-sided exponential decay function of current 
% spread along the electrode array
% 
% Usage:
% amplitude = compute_exponential_decay(place, active_electrode_idx, electrode_distance, lambda)
% 
% Input parameters:
% place                 place along the electrode arrray / m
% active_electrode_idx  active electrode index
% electrode_distance    electrode distance / m
% lambda                length constant of the exponential decay function / 1/m
% 
% Output parameter:
% amplitude             current amplitude / A
% 
% -------------------------------------------------------------------------

function amplitude = compute_exponential_decay(place, active_electrode_idx, electrode_distance, lambda)
    amplitude = ones(1, length(place)) .* exp(-abs(place - active_electrode_idx) * electrode_distance/lambda);
end
