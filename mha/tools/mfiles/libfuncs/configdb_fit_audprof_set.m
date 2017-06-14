function configdb_fit_audprof_set( mha, sAud )
  configdb_set_mhaconfig(mha,'client_id',sAud.client_id);
  configdb_set_mhaconfig(mha,'client_aud',sAud);
  selected_auds = ...
      configdb_get_mhaconfig(mha,'client_aud_ids',cell([2,0]));
  selected_auds = ...
      configdb_smap_set(selected_auds,sAud.client_id,sAud.id);
  configdb_set_mhaconfig(mha,'client_aud_ids',selected_auds);
