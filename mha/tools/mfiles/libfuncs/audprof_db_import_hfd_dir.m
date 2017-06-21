function [csID,csIDmissing] = audprof_db_import_hfd_dir( sPath )
  d = [dir([sPath,'*.hfd']);
       dir([sPath,'*.HFD']);];
  libconfigdb();
  cDB = audprof_db_load();
  csID = {};
  csIDmissing = {};
  for k=1:numel(d)
    sClientID = upper(d(k).name(1:end-4));
    if audprof_db_client_exists(sClientID,cDB)
      csID{end+1} = sClientID;
      cClient = audprof_db_client_get( cDB, sClientID );
      sAud = audprof_audprof_new();
      if isfield(cClient,'audprofs') && ~isempty(cClient.audprofs)
	sAud = cClient.audprofs{2,end};
      end
      sAud = audprof_acalos_hfdfile_load( sAud, [sPath,d(k).name] );
      sAud.id = ['ACALOS from ',[sPath,d(k).name],' ',sAud.id];
      cClient = audprof_client_audprof_add( cClient, sAud );
      cDB = audprof_db_client_add( cDB, cClient );
    else
      csIDmissing{end+1} = sClientID;
    end
  end
  audprof_db_save( cDB );
  %csID = d;