function audprof_db_audprof_add( audprof )
  cDB = audprof_db_load();
  if ~audprof_db_client_exists( audprof.client_id, cDB )
    error(['Client ID ''',audprof.client_id,''' not found in database.']);
  end
  cClient = audprof_db_client_get( cDB, audprof.client_id );
  cClient = audprof_client_audprof_add( cClient, audprof );
  cDB = audprof_db_client_add( cDB, cClient );
  audprof_db_save( cDB );