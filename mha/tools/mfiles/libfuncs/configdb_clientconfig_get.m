function [val,sClientID] = configdb_clientconfig_get( mha, varname, val )
  if nargin < 2
    val = [];
  end
  sClientID = configdb_get_mhaconfig(mha,'client_id','');
  cCDB = configdb_get_mhaconfig(mha,'client_datab',cell([2,0]));
  sCData = configdb_smap_get(cCDB,sClientID);
  if ~isempty(sCData)
    try
      eval(sprintf('val=sCData.%s;',varname));
    catch
    end
  end
