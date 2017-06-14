function sAudProfs = audprof_audprof_getall( sClientID )
% get auditory profile for a given client ID
  cDB = audprof_db_load();
  cfdb = libconfigdb();
  cClient = cfdb.smap_get( cDB, sClientID );
  if isempty(cClient)
    error(['Client ID ''',sClientID,''' not found!']);
  end
  if isfield(cClient,'audprofs')
    sAudProfs = cClient.audprofs;
  else
    sAudProfs = cell([2,0]);
  end