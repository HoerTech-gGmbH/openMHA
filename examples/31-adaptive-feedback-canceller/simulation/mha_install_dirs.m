function openMHA_dir = mha_install_dirs(dir_name)
% mha_install_dirs - returns default installation directories of openMHA
% tools, examples and reference algorithms
%
% Usage:
% openMHA_dirs = mha_install_dirs();
% or
% openMHA_dir = mha_install_dirs(dir_name);
%
% Input:
% dir_name  - name of requested installation directory
%             'mfiles': Matlab/Octave tools
%             'examples': Example openMHA configuration files
%             'reference_algorithms': Reference algorithms provided with
%             the openMHA software package
%
% Output:
% openMHA_dirs: struct containing fields with all available directories
% or
% openMHA_dir: string with the requested directory
%
% Author: Hendrik Kayser, 2020
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2020 Hörtech gGmbH
% Copyright © 2022 Hörzentrum Oldenburg gGmbH
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

% Assume openMHA default installation directory for the different operating
% systems
if ismac % Macos
    openMHA_dir.mfiles = '/usr/local/lib/openmha/mfiles/';
    openMHA_dir.examples = '/usr/local/share/openmha/examples/';
    openMHA_dir.reference_algorithms = '/usr/local/share/openmha/reference_algorithms/';
elseif ispc % Windows
    openMHA_dir.mfiles = 'C:\Program Files\openMHA\mfiles\';
    openMHA_dir.examples = 'C:\Program Files\openMHA\examples\';
    openMHA_dir.reference_algorithms = 'C:\Program Files\openMHA\reference_algorithms\';
else % Linux
    openMHA_dir.mfiles = '/usr/lib/openmha/mfiles/';
    openMHA_dir.examples = '/usr/share/openmha/examples/';
    openMHA_dir.reference_algorithms = '/usr/share/openmha/reference_algorithms/';
end

% If this mfile is not inside the default examples directory, then assume folder strucutre of self-compiled local installation
examples_dir_cur = fileparts(fileparts(fileparts(mfilename('fullpath'))));
if ~strcmp(openMHA_dir.examples, [examples_dir_cur filesep])
    openMHA_git_dir = fileparts(fileparts(fileparts(fileparts(mfilename('fullpath')))));
    openMHA_dir.mfiles = fullfile(openMHA_git_dir, 'mha', 'tools', 'mfiles');
    openMHA_dir.examples = fullfile(openMHA_git_dir,'examples');
    openMHA_dir.reference_algorithms = fullfile(openMHA_git_dir,'reference_algorithms');
end

if nargin < 1
    for csField = fieldnames(openMHA_dir).'
        if ~exist(openMHA_dir.(csField{:}),'dir')
            warning(['Default installation directory for "' csField{:} '" (' openMHA_dir.(csField{:}) ') does not exist. Please provide manually.'])
            openMHA_dir.(csField{:}) = '';
        end
    end
    
    return
elseif isfield(openMHA_dir,dir_name)
    if ~exist(openMHA_dir.(dir_name),'dir')
        error(['Default installation directory for "' dir_name '" (' openMHA_dir.(dir_name) ') does not exist. Please provide manually.'])
    end
    openMHA_dir = openMHA_dir.(dir_name);
else
    all_fields = fieldnames(openMHA_dir);
    dir_list = '';
    for k=1:numel(all_fields)
        dir_list = [dir_list,' ''',all_fields{k},''''];
    end
    error(['Directory "' dir_name '" does not exist (available:' dir_list  ').'])
end
