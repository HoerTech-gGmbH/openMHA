function clientdb2audprofdb
  ldb = oldlibclientdb();
  libaudprof();
  cClDB = ldb.get_clients();
  cADB = audprof.db_load();
  for kCl=1:size(cClDB,2)
    cClient = cClDB{2,kCl};
    if ~strcmp(cClient.id,'TT123456')
      if isfield(cClient,'audiograms')
	cAuds = cClient.audiograms;
	cClient = rmfield( cClient, 'audiograms' );
	cClient.audprofs = cell(2,0);
	for kAud=1:size(cAuds,2)
	  sAud = cAuds{2,kAud};
	  sAud.client_id = cClient.id;
	  sAudProf = aud2audprof( sAud );
	  cClient = audprof.client_audprof_add( cClient, sAudProf );
	end
	cADB = audprof.db_client_add( cADB, cClient );
      end
    end
  end
  audprof.db_save(cADB);
  
  
function lib = oldlibclientdb
  lib = struct;
  lib.get_clients = @get_clients;
  lib.set_clients = @set_clients;
  lib.emptyclient = @emptyclient;
  lib.prettyclientlist = @prettyclientlist;
  lib.sort_celllist = @sort_celllist;
  lib.get_audiograms = @get_audiograms;
  lib.nhaud = @nhaud;
  lib.flat_aud = @flat_aud;
  lib.clean_aud = @clean_aud;
  lib.get_testaud = @get_testaud;
  
function cClients = get_clients
  global client_datab_warn;
  if isempty(client_datab_warn)
    client_datab_warn = true;
    if exist('client_datab.mat')
      p = fileparts(which('client_datab.mat'));
      if ~strcmp(p,pwd)
	warning(sprintf(['\nUsing client database from directory ' ...
			 '"%s"!\nCurrent directory is "%s".'],p,pwd));
      end
    end
  end
  cfdb = libconfigdb();
  cClients = cfdb.readfile('client_datab.mat',...
			   'clients',cell([2,0]));
  sCl = emptyclient();
  sCl.lastname = 'test subject';
  sCl.firstname = '';
  sCl.birthday = '';
  sCl.id = 'TT123456';
  aud = nhaud();
  aud.id = 'test audiogram';
  aud.client_id = sCl.id;
  aud.frequencies = [125 250 500 750 1000 1500 2000 3000 4000 6000 8000];
  aud.l.htl = [10 15 30 40 45 50 65 65 75 65 65];
  aud.r.htl = [20 15 20 30 35 35 40 55 70 65 85];
  sCl.audiograms = cfdb.smap_set( sCl.audiograms, aud.id, aud );
  aud = flat_aud( 40 );
  aud.client_id = sCl.id;
  sCl.audiograms = cfdb.smap_set( sCl.audiograms, aud.id, aud );
  aud = flat_aud( 50 );
  aud.client_id = sCl.id;
  sCl.audiograms = cfdb.smap_set( sCl.audiograms, aud.id, aud );
  cClients = cfdb.smap_set( cClients, sCl.id, sCl );
  
function cC = sort_celllist( cC )
  [tmp,idx] = sort(cC(1,:));
  cC = cC(:,idx);
  
function set_clients( cClients )
  cfdb = libconfigdb();
  cfdb.writefile('client_datab.mat','clients',cClients);
  
function csList = prettyclientlist( cClients )
  csList = {};
  for k=1:size(cClients,2)
    scl = cClients{2,k};
    scl = merge_structs(scl,emptyclient);
    sName = cClients{1,k};
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
  
function sAud = flat_aud( HTL )
% default audiogram data. Audiogram data is always kept in SPL HL.
  sAud = struct;
  sAud.id = sprintf('flat %g dB',HTL);
  sAud.client_id = '';
  sAud.frequencies = [125 250 500 750 1000 1500 2000 3000 4000 6000 8000];
  sAud.l.htl = zeros(size(sAud.frequencies))+HTL;
  sAud.l.ucl = nan+[106.9 106.9 108.5 103.4  99.4  99.5  96.6  94.5 ...
		    97.9 102.4  94.9];
  sAud.r = sAud.l;
  
function sAud = nhaud()
  sAud = flat_aud( 0 );
  sAud.id = 'normal hearing';
  
function sClient = emptyclient()
  sClient = struct;
  sClient.audiograms = cell([2,0]);
  sClient.firstname = '';
  sClient.lastname = '';
  sClient.birthday = '1900-01-01';
  sClient.id = '';

function csAuds = get_audiograms( sClient, sAudID )
  cClients = get_clients();
  cfdb = libconfigdb();
  cClient = cfdb.smap_get( cClients, sClient );
  if isempty(cClient)
    error(['Client ID ''',sClient,''' not found!']);
  end
  csAuds = cClient.audiograms;
  if nargin == 2
    csAuds = cfdb.smap_get( csAuds, sAudID );
    if isempty(csAuds)
      error(['Audiogram ID ''',sAudID,''' not found!']);
    end
  end

function sAud = get_testaud
  sAud = get_audiograms('TT123456', 'test audiogram');
  
function sAud = clean_aud( sAud )
%
% remove inf/nan entries:
%
  idx = unique([find(~isfinite(sAud.l.htl)),...
		find(~isfinite(sAud.r.htl))]);
  sAud.frequencies(idx) = [];
  sAud.l.htl(idx) = [];
  sAud.r.htl(idx) = [];
  sAud.l.ucl(idx) = [];
  sAud.r.ucl(idx) = [];
  if isempty(sAud.frequencies)
    error('The audiogram does not contain any data!');
  end
    