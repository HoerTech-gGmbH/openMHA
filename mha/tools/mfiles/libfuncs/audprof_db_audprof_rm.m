function audprof_db_audprof_rm( audprof )
  cDB = audprof_db_load();
  if ~audprof_db_client_exists( audprof.client_id, cDB )
    return;
  end
  cClient = audprof_db_client_get( cDB, audprof.client_id );
  cClient = audprof_client_audprof_rm( cClient, audprof );
  cDB = audprof_db_client_add( cDB, cClient );
  audprof_db_save( cDB );