% Uses strfind to detect which entries in the cellstr match the pattern.
% Then returns a modified version of the cellstring, where the matching
% entries are removed.

% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2017 HörTech gGmbH
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

function cellstr = remove_matching_entries_from_cellstring(cellstr, pattern)
  
  % We find out which strings in the cell match
  matches = strfind(cellstr, pattern);
  
  % We want to keep the strings that had no match
  keeps = cellfun(@isempty, matches);
  
  % return the strings that did not match
  cellstr = cellstr(keeps);

% Local Variables:
% mode: octave
% indent-tabs-mode: nil
% coding: utf-8-unix
% End:
