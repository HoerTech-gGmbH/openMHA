function mha_set_mfiles_dir(mfiles_dir)

% set_openMHA_mfiles_dir - adds mfiles directory to search path, adds 
% openMHA java class 
%
% Usage:
% set_openMHA_mfiles_dir - Assumes deafult installation directoryies for
%                          openMHA
% or
% set_openMHA_mfiles_dir(mfiles_dir) - Uses custom path.
%
% Input:
% mfiles_dir - custom path to openMHA mfiles directory
%
%
% Author: Hendrik Kayser, 2020


% Assume openMHA default installation directory for the different operating
% systems
if nargin < 1
    if ismac % Macos
        openMHA_mfiles_dir = '/usr/local/lib/openmha/mfiles/';
    elseif ispc % Windows
        openMHA_mfiles_dir = 'C:\Program Files\openMHA\mfiles\';
    else % Linux
        openMHA_mfiles_dir = '/usr/lib/openmha/mfiles/';
    end
else
    openMHA_mfiles_dir = mfiles_dir;
end

if ~exist(openMHA_mfiles_dir,'dir')
    error(['Directory "' openMHA_mfiles_dir '" not found.'])
else
    addpath(openMHA_mfiles_dir)
    javaaddpath(fullfile(openMHA_mfiles_dir,'mhactl_java.jar'))
end

