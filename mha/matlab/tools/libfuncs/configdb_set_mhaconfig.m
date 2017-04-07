function configdb_set_mhaconfig( mha, name, val )
% Set a value in the Matlab configuration file linked to a given MHA
%
% mha      : Valid handle of a MHA
% varname  : Variable name to store configuration.
% defvalue : Data to be stored (of any Matlab type).
%
% The variable name can refer to a member of a structure. In that
% case only the member is updated without overwriting the rest of
% the structure.
%
% Author: Giso Grimm
% Date: 2007
  ;
  cfg = mha_get_basic_cfg_network( mha );
  configdb_writefile(cfg.cfgname,name,val);