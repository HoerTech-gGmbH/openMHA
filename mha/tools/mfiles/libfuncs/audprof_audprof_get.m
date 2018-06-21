function sAudProf = audprof_audprof_get( sClientID, sAudID, cDB )
% get auditory profile for a given client ID
  if nargin < 3
    cDB = audprof_db_load();
  end
  cfdb = libconfigdb();
  cClient = cfdb.smap_get( cDB, sClientID );
  if isempty(cClient)
    error(['Client ID ''',sClientID,''' not found!']);
  end
  sAudProf = cfdb.smap_get( cClient.audprofs,...
					      sAudID );
  if isempty(sAudProf)
    error(['Auditory profile ''',sAudID,''' not found!']);
  end
  sAudProf.client_id = sClientID;