function audprof_db_save( cDB )
% save auditory profile database
  cfdb = libconfigdb();
  cDB = cfdb.smap_rm( cDB, 'TT123456' );
  cfdb.writefile('audprofdb.mat','clients',cDB);
