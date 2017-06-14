function sAudProf = audprof_audprof_remove_from_db( sClientID, sAudProf )
  libconfigdb();
  cDB = audprof_db_load();
  sClient = audprof_db_client_get( cDB, sClientID );
  if ~isempty(sClient)
    sClient.audprofs = ...
	configdb.smap_rm( sClient.audprofs, ...
					sAudProf.id );
    cDB = audprof_db_client_add( cDB, sClient );
    audprof_db_save( cDB );
  end