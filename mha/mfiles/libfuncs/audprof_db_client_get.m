function sClient = audprof_db_client_get( cDB, sClientID )
  libconfigdb();
  sClient = configdb.smap_get(cDB,sClientID);
