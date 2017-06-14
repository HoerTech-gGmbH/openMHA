function b = audprof_audprof_exists( sClientID, sAudID, cDB )
% get auditory profile for a given client ID
  libconfigdb();
  if nargin < 3
    cDB = audprof_db_load();
  end
  cClient = audprof_db_client_get( cDB, sClientID );
  if isempty(cClient)
    error(['Client ID ''',sClientID,''' not found!']);
  end
  sAudProf = configdb.smap_get( cClient.audprofs,...
					      sAudID );
  b = ~isempty(sAudProf);
