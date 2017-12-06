% When executing in octave on windows, remove directories containing
% the string octave (case insensitive) from the environment variable PATH
% unless the global variable KEEP_OCTAVE_DIRECTORIES_IN_PATH is truthy.
% 
% functionality can be switched off by setting global variable
% KEEP_OCTAVE_DIRECTORIES_IN_PATH to true

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

function remove_octave_directories_from_windows_path()

  % functionality can be switched off by setting this global variable to true
  global KEEP_OCTAVE_DIRECTORIES_IN_PATH;
  if KEEP_OCTAVE_DIRECTORIES_IN_PATH
    return
  end

  if ispc() && isoctave()
				% We are in octave on windows
    PATH = getenv('PATH');
    delimiter = ';'; % Windows uses semicolon to separate directories in PATH
    
    % split the path into individual directories
    PATH_directories = split_string(PATH, delimiter);
    
    % remove all directories that point into the octave installation
    PATH_directories = ...
      remove_matching_entries_from_cellstring(PATH_directories, OCTAVE_HOME);
    
    % rebuild the PATH with our adjustments
    PATH = strjoin(PATH_directories, delimiter);
    setenv('PATH', PATH);
  end

% Local Variables:
% mode: octave
% indent-tabs-mode: nil
% coding: utf-8-unix
% End:
