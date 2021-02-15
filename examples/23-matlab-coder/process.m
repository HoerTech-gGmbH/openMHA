function signal_frag_out = process(signal_frag_in)
% function signal_frag_out = process(signal_frag_in)
%
% signal_frag_in  - input signal fragment
% signal_frag_out - output signal fragment
%
% Example process function for native compilation which can include any
% number of intermediate steps

% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2020 HörTech gGmbH

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

signal_frag_out = channel_switch(signal_frag_in);

end
