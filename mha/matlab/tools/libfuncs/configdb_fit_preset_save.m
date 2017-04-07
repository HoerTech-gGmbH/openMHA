function configdb_fit_preset_save( mha, name )
  sPlug = configdb_get_mhaconfig(mha,'current_compressor');
  csPresets = ...
      configdb_clientconfig_get(mha,[sPlug.addr,'.presets'],cell([2 0]));
  idx = strmatch(name,csPresets(1,:),'exact');
  if isempty(idx)
    csPresets(:,end+1) = {name;sPlug};
  else
    csPresets(:,idx(1)) = {name;sPlug};
  end
  configdb_clientconfig_set(mha,[sPlug.addr,'.presets'],csPresets);
  configdb_clientconfig_set(mha,[sPlug.addr,'.current_preset'],name);
