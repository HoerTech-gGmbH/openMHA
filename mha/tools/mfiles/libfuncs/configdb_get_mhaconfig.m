function val = configdb_get_mhaconfig( mha, name, defval )
% Get a value from the Matlab configuration file linked to a given MHA
%
% mha      : Valid handle of a MHA
% name     : Variable name to store configuration.
% defval   : Data to be stored if no entry exists (of any Matlab type).
% val      : Data read from configuration.
%
% The variable name can refer to a member of a structure. In that
% case only the member is updated without overwriting the rest of
% the structure.
%
% Author: Giso Grimm
% Date: 2007
  ;
  if nargin < 3
    defval = [];
  end
  cfg = mha_get_basic_cfg_network( mha );
  defval = configdb_readfile('mha_ini.mat',name,defval);
  defval = configdb_readfile(cfg.ininame,name,defval);
  defval = configdb_readfile(cfg.ininame(1:end-2),name,defval);
  val = configdb_readfile(cfg.cfgname,name,defval);