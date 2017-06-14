function sCl = audprof_client_example()
% return example client
  cfdb = libconfigdb();
  sCl = audprof_client_new();
  sCl.lastname = 'test subject';
  sCl.firstname = '';
  sCl.birthday = '';
  sCl.id = 'TT123456';
  for csProfs={'flat40','flat50','slope'}
    prof=csProfs{:};
    sProf = audprof_audprof_example( prof );
    sCl.audprofs = ...
	cfdb.smap_set( sCl.audprofs, prof, sProf );
  end
