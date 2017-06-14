function cDB = audprof_db_client_add( cDB, cClient )
  libconfigdb();
  cDB = configdb.smap_set( cDB, cClient.id, cClient );