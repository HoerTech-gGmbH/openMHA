function bExists = audprof_client_exists( sClient )
  cDB = audprof_db_load();
  bExists = ~isempty(strmatch( sClient, cDB(1,:), 'exact'));
  