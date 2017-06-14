function cDB = audprof_db_client_rm( cDB, cClient )
  libconfigdb();
  cDB = configdb.smap_rm( cDB, cClient.id );