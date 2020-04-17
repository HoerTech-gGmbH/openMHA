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

% If mfiles directory is not default assume folder strucutre of self-compiled local installation
mfiles_dir_cur = fileparts(mfilename('fullpath'));
if ~strcmp(openMHA_dir.mfiles,[mfiles_dir_cur '/'])
    openMHA_dir.mfiles = fileparts(mfiles_dir_cur);
    openMHA_dir.examples = fullfile(fileparts(mfiles_dir_cur),'..','..','..','examples');
    openMHA_dir.reference_algorithms = fullfile(fileparts(mfiles_dir_cur),'..','..','..','reference_algorithms');
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



