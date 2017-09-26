% function parts = split_string(string, delimiter)
%
% split the string <string> into <parts> separated by <delimiter>
% Result is a cell array of strings.

% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2004 2006 2013 2014 2015 2017 HörTech gGmbH
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

function parts = split_string(string, delimiter)
  parts = {};
  % data given as string
  while length(string)
    [part, string] = strtok(string, delimiter);
    parts = [parts, {part}];
  end

% Local Variables:
% mode: octave
% indent-tabs-mode: nil
% coding: utf-8-unix
% End:
