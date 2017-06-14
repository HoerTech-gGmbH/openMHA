function configdb_fit_preset_load( mha, name )
  sPlug = configdb_get_mhaconfig(mha,'current_compressor');
  csPresets = configdb_clientconfig_get(mha,[sPlug.addr,'.presets'],cell([2 0]));
  idx = strmatch(name,csPresets(1,:),'exact');
  if isempty(idx)
    warndlg(['No preset ''',name,''' in preset list.']);
    return;
  end
  sPlug = csPresets{2,idx(1)};
  configdb_set_mhaconfig(mha,'current_compressor',sPlug);
  configdb_fit_upload_current( mha );
  configdb_clientconfig_set(mha,[sPlug.addr,'.current_preset'],name);
