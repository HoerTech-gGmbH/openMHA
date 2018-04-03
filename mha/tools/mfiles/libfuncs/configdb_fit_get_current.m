function [sPlug,sClientID,sAud,sRule,sSide] = configdb_fit_get_current( mha )
  libaudprof();
  sPlug = configdb_get_mhaconfig( mha, 'current_compressor' );
  sAud = configdb_get_mhaconfig( mha, 'client_aud' );
  [sRule,sClientID] = configdb_clientconfig_get( mha, 'gain_rule','nogain' );
  sSide = configdb_get_mhaconfig( mha, 'next_fit_side','' );
  sAud.client_id = sClientID;
  sAud = audprof.audprof_cleanup( sAud );
