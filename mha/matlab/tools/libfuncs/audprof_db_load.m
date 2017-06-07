function cDB = audprof_db_load
% load the auditory profile database
  ;
  global audprof_warn_;
  if isempty(audprof_warn_)
    audprof_warn_ = true;
    if exist('audprofdb.mat')
      p = fileparts(which('audprofdb.mat'));
      if ~strcmp(p,pwd)
	warning(sprintf(['\nUsing auditory profile database from directory ' ...
			 '"%s"!\nCurrent directory is "%s".'],p,pwd));
      end
    end
  end
  cfdb = libconfigdb();
  cClients = cfdb.readfile('audprofdb.mat',...
			   'clients',cell([2,0]));
  sCl = audprof_client_example();
  cDB = cfdb.smap_set( cClients, sCl.id, sCl );
