function configdb_writefile( filename, varname, value )
% store data in a configuration file
%
% filename : Name of configuration (Matlab) file.
% varname  : Variable name to store configuration.
% value    : Data to be stored (of any Matlab type).
%
% The variable name can refer to a member of a structure. In that
% case only the member is updated without overwriting the rest of
% the structure.
%
% Author: Giso Grimm
% Date: 2007
  ;
  basename = varname;
  idx = strfind(basename,'.');
  if ~isempty(idx)
    basename(idx(1):end) = [];
  end
  [tpath,tname,tmext] = fileparts(filename);
  global get_config_file_cache;
  if ~exist(filename,'file')
    data = struct;
  else
    if ~isempty(get_config_file_cache) && ...
	  isfield(get_config_file_cache,tname)
      data = get_config_file_cache.(tname);
    else
      data = load(filename);
    end
  end
  eval(sprintf('data.%s=value;',varname));
  get_config_file_cache.(tname) = data;
  if isoctave
    if ~exist(filename,'file')
      save(filename,'-binary','-struct','data',basename);
    else
      save(filename,'-append','-binary','-struct','data',basename);
    end
  else
    if ~exist(filename,'file')
      save(filename,'-struct','data',basename);
    else
      save(filename,'-append','-struct','data',basename);
    end
  end