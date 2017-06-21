function [val,sClientID] = configdb_clientconfig_set( mha, varname, val )
  sClientID = configdb_get_mhaconfig(mha,'client_id');
  cCDB = configdb_get_mhaconfig(mha,'client_datab',cell([2,0]));
  sCData = configdb_smap_get(cCDB,sClientID);
  if isempty(sCData)
    sCData = struct;
  end
  eval(sprintf('sCData.%s=val;',varname));
  cCDB = configdb_smap_set(cCDB,sClientID,sCData);
  configdb_set_mhaconfig(mha,'client_datab',cCDB);
  