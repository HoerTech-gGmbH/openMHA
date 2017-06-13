function b = audprof_db_client_exists( s, cDB )
  cfdb = libconfigdb();
  if nargin < 2
    cDB = audprof_db_load();
  end
  [sC,idx] = cfdb.smap_get(cDB,s);
  b = ~isempty(idx);