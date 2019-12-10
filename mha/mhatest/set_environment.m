function plugins = set_environment
% Prepare this matlab/octave instance to start and communicate with MHA
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2014 2015 2016 2017 2018 HörTech gGmbH
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
  
git_dir = fileparts(fileparts(fileparts(mfilename('fullpath'))));
config_mk = fopen([git_dir '/config.mk']);
config_mk_contents = fscanf(config_mk,'%c',inf);
config_mk_lines = strsplit(config_mk_contents,{'\r\n','\n','\r'});
fclose(config_mk);
build_dir_line_index = strmatch('BUILD_DIR',config_mk_lines);
build_dir_assignment = strsplit(config_mk_lines{build_dir_line_index(end)},'=');
build_dir = strtrim(build_dir_assignment{2});

global plugins;
plugins = find_all_plugins(git_dir, build_dir);

global MHA_INSTALL_DIR;
MHA_INSTALL_DIR = [git_dir '/mha/frameworks/' build_dir];
setenv('MHA_INSTALL_DIR',MHA_INSTALL_DIR);
addpath([git_dir '/mha/tools/mfiles']);
addpath([git_dir '/mha/mhatest']);
javaaddpath ([git_dir '/mha/tools/mfiles/mhactl_java.jar']);

if ispc
  bin_dir = [git_dir '\mha\bin'];
  mkdir(bin_dir);
  for p = plugins
      copyfile(p{1}, bin_dir);
  end
  setenv('PATH',[git_dir '\mha\libmha\' build_dir ';' git_dir '\external_libs\' build_dir '\bin\;' git_dir '\external_libs\' build_dir '\lib;' git_dir '\external_libs\windows\' build_dir ';' getenv('PATH')]);
  setenv('MHA_LIBRARY_PATH',[git_dir '\mha\bin']);
  try
    [dummy,hostname] = system('hostname');
    hostname = strtrim(hostname);
    copyfile([git_dir '/mha/doc/system/windows/' hostname '/mha.lic'], MHA_INSTALL_DIR)
  catch
  end
else
  if ~ismac()
    ldpath = 'LD_LIBRARY_PATH';
    setenv(ldpath,[getenv(ldpath) ':' git_dir '/mha/libmha/' build_dir]);
  end
  dirs = {};
  for p = plugins
      dirs = [dirs, {fileparts(p{1})}];
  end
  dirs = unique(dirs);
  setenv('MHA_LIBRARY_PATH',strjoin(dirs,';'));
end
if isoctave()
  pkg load signal;
end

function result = find_all_plugins(git_dir, build_dir)
if strfind(build_dir, 'MinGW')
    dll = '.dll';
elseif strfind(build_dir, 'linux')
    dll = '.so';
elseif strfind(build_dir, 'Darwin')
    dll = '.dylib';
else
    error(['dll file extension needs to be defined for ' build_dir]);
end
result_raw = find_files_ending_with([git_dir '/mha/plugins'], dll);
result = {};
for r = result_raw
    if strfind(r{1},build_dir)
        result = [result r];
    end
end

function result = find_files_ending_with(root_path, ending)
result = {};
root_dir = javaObject('java.io.File', root_path);
listing = root_dir.listFiles;
for i = 1:length(listing)
    e = listing(i); p = char(e.getPath);
    if e.isDirectory
        result = [result find_files_ending_with(p, ending)];
    elseif strmatch(fliplr(ending),fliplr(p))
        result = [result {char(p)}];
    end
end
