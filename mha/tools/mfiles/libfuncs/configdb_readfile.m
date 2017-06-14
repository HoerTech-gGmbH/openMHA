function value = configdb_readfile( filename, varname, defvalue )
% read data from a configuration file
%
% filename : Name of configuration (Matlab) file.
% varname  : Variable name to store configuration.
% defvalue : Data to be stored if no entry exists (of any Matlab type).
% value    : Data read from configuration.
%
% The variable name can refer to a member of a structure. In that
% case only the member is updated without overwriting the rest of
% the structure.
%
% Author: Giso Grimm
% Date: 2007
  ;
  if nargin < 3
    defvalue = [];
  end
  [tpath,tname,tmext] = fileparts(filename);
  global get_config_file_cache;
  basename = varname;
  idx = strfind(basename,'.');
  if ~isempty(idx)
    basename(idx(1):end) = [];
  end
  data = struct;
  if exist(filename,'file')
    if strcmp(tmext,'.m')
      data = load_mscript( filename );
    else
      if ~isempty(get_config_file_cache) && ...
	    isfield(get_config_file_cache,tname)
	data = get_config_file_cache.(tname);
      else
	data = load(filename);
	get_config_file_cache.(tname) = data;
      end
    end
  end
  
  try
    eval(['value=data.',varname,';']);
  catch
    value = defvalue;
  end
