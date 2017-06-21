function csList = audprof_db_prettyclientlist( cDB )
  csList = {};
  for k=1:size(cDB,2)
    scl = cDB{2,k};
    scl = merge_structs(scl,audprof_client_new());
    sName = cDB{1,k};
    if ~isempty(scl.lastname)
      sName = sprintf('%s  %s', sName, scl.lastname);
    end
    if ~isempty(scl.firstname)
      sName = sprintf('%s, %s', sName, scl.firstname);
    end
    if ~isempty(scl.birthday)
      sName = sprintf('%s (%s)', sName, scl.birthday);
    end
    csList{end+1} = sName;
  end
