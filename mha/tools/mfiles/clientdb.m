function sAudSelected = clientdb( sAud, varargin )
% CLIENTDB - GUI to the client database
%
% Usage:
% clientdb( cfg [, AddAudName, AddAudFun [, ...] ] )
%
% Parameters:
%   cfg        : default values for client database
%                (initial client ID etc)
%   AddAudName : name of function to add an audiogram
%   AddAudFun  : function handle to add an audiogram
%
% Author: Giso Grimm, 2007-2008
  ;
  if nargin < 1
    sAud = [];
  end
  libmhagui();
  libconfigdb();
  libaudprof();
  global client_datab_warn;
  client_datab_warn = [];
  % csAddAud contains functions to add an audiogram:
  csAddAud = cell([2,0]);
  if mod(length(varargin),2)
    error('Method/function handle pair expected');
  end
  for k=1:length(varargin)/2
    csAddAud{1,k} = varargin{2*k-1};
    csAddAud{2,k} = varargin{2*k};
    if ~ischar(csAddAud{1,k})
      error('method description string expected');
    end
    if ~strcmp(class(csAddAud{2,k}),'function_handle')
      error('function handle expected');
    end
  end
  csAddAud = configdb.smap_set(csAddAud,'Edit',mhagui.audprof_edit);
  csAddAud = configdb.smap_set(csAddAud,'Remove',@remove_audiogram);
  csAddAud = configdb.smap_set(csAddAud,'Print',@print_audiogram);
  % make sure that no other client database GUI is open:
  close(findobj('tag','clientdb'));
  if isempty( sAud )
    sAud = audprof.audprof_new();
  end
  sCfg = struct;
  sCfg.audprof = sAud;
  sCfg.client_id = sAud.client_id;
  sCfg.db = sort_celllist(audprof.db_load());
  sCfg.aud_selection = cell(2,0);
  fh = mhagui.figure('MHA audiogram database','clientdb',[800,600],...
		     'UserData',sCfg);
  uicontrol(fh,'Style','Listbox',...
	    'Position',[20 60 414 530],...
	    'FontSize',12,...
	    'Callback',@cb_update_client,...
	    'BackGroundColor',ones(1,3),'tag','list:client');
  uicontrol(fh,'Style','Pushbutton','String','New client',...
	    'Callback',@cb_add_user,...
	    'Position',[20 20 96 30]);
  uicontrol(fh,'Style','Pushbutton','String','Edit client',...
	    'Callback',@cb_edit_user,...
	    'Position',[126 20 96 30]);
  uicontrol(fh,'Style','Pushbutton','String','Remove client',...
	    'Callback',@cb_remove_user,...
	    'Position',[232 20 96 30]);
%  uicontrol(fh,'Style','Pushbutton','String','ACALOS...',...
%	    'Callback',@cb_scan_hfd_dir,...
%	    'Position',[338 20 96 30]);
  p = [450 320 0 0];
  uicontrol(fh,'Style','frame',...
	    'Position',[0 0 330 270]+p);
  uicontrol(fh,'Style','text','String','Available audiograms:',...
	    'HorizontalAlign','left',...
	    'Position',[10 240 270 20]+p);
  uicontrol(fh,'Style','listbox',...
	    'String',{},...
	    'BackgroundColor',ones(1,3),...
	    'Callback',@cb_update_audprof,...
	    'Position',[10 100 310 140]+p,...
	    'Tag','list:audprof');
  axes('Units','pixels','Position',[510 113 280 200],...
       'Tag','audiogram_axes');
  if ~isempty(csAddAud)
    s = struct;
    s.width = 310;
    s.dx = 10;
    s.n = size(csAddAud,2);
    s.ny = floor(sqrt(s.n));
    s.nx = ceil(s.n/s.ny);
    s.dist = (s.width+s.dx)/s.nx;
    s.wbut = s.dist-s.dx;
    s.pos = [10 10 0 0]+p;
    for k=1:s.n
      uicontrol('Style','Pushbutton',...
		'String',csAddAud{1,k},...
		'UserData',csAddAud{2,k},...
		'Callback',@add_aud_wrapper,...
		'Position',s.pos+[mod((k-1),s.nx)*s.dist 30*floor((k-1)/s.nx) s.wbut 25]);
    end
  end
  update_gui( fh );
  sCfg = mhagui.waitfor( fh );
  if isempty(sCfg)
    sAudSelected = [];
  else
    sAudSelected = sCfg.audprof;
  end
  
function update_gui( fh )
  libaudprof();
  sCfg = get( fh, 'UserData' );
  kSelected = strmatch(sCfg.audprof.client_id,sCfg.db(1,:),'exact');
  if isempty(kSelected)
    kSelected = 1;
    sCfg.audprof = audprof.audprof_new(sCfg.db{1,kSelected});
  end
  set(findobj('tag','list:client'),...
      'Value',kSelected,...
      'String',audprof.db_prettyclientlist(sCfg.db));
  set( fh, 'UserData', sCfg );
  update_client( fh );
  
function add_aud_wrapper( varargin )
  libaudprof();
  sCfg = get(gcbf,'UserData');
  addfun = get(gcbo,'UserData');
  try
    sAud = addfun( sCfg.client_id, sCfg.audprof );
    if ~isempty(sAud) && ~isequaln( sAud, sCfg.audprof );
      audprof.db_audprof_add( sAud );
    end
    reload_db( gcbf );
  catch
    disp_err_rethrow;
  end
  
function cb_add_user( varargin )
  libconfigdb();
  libaudprof();
  libmhagui();
  try
    sClient = mhagui.client_edit();
    if isempty(sClient)
      return;
    end
    cDB = audprof.db_load();
    cDB = configdb.smap_set( cDB, sClient.id, sClient );
    audprof.db_save(cDB);
    reload_db( gcbf );
  catch
    disp_err_rethrow;
  end

function cb_edit_user( varargin )
  libconfigdb();
  libaudprof();
  libmhagui();
  try
    cDB = audprof.db_load();
    sCfg = get(gcbf,'UserData');
    if ~audprof.db_client_exists( sCfg.client_id, cDB )
      warning('client not in database (data base changed?)');
      return
    end
    sC = mhagui.client_edit( ...
	audprof.db_client_get(cDB, sCfg.client_id));
    if isempty(sC)
      return
    end
    cDB = audprof.db_client_add(cDB,sC);
    audprof.db_save(cDB);
    reload_db( gcbf );
  catch
    disp_err_rethrow;
  end
  
function cb_remove_user( varargin )
  libconfigdb();
  libaudprof();
  try
    cDB = audprof.db_load();
    sCfg = get(gcbf,'UserData');
    if ~audprof.db_client_exists( sCfg.client_id, cDB )
      warning('client not in database (data base changed?)');
      return
    end
    sC = audprof.db_client_get(cDB, sCfg.client_id);
    msg = sprintf('Really remove client ''%s'' (%s, %s *%s)?',...
		  sCfg.client_id,sC.lastname, ...
		  sC.firstname,sC.birthday);
    answ = questdlg(msg,...
		  'Remove client','Yes','No','No');
    if ~strcmp(answ,'Yes')
      return;
    end
    cDB = audprof.db_client_rm(cDB,sC);
    audprof.db_save(cDB);
    reload_db( gcbf );
  catch
    disp_err_rethrow;
  end
  
function sAud = print_audiogram( client, sAud )
  libaudprof();
  audprof.audprof_print(sAud);
  
function sAud = remove_audiogram( client, sAud )
  msg = sprintf('Really remove audiogram ''%s'' for client %s?',...
		sAud.id,sAud.client_id);
  answ = questdlg(msg,...
		  'Remove audiogram','Yes','No','No');
  if ~strcmp(answ,'Yes')
    return;
  end
  libaudprof();
  audprof.db_audprof_rm( sAud );
  
function cC = sort_celllist( cC )
  [tmp,idx] = sort(cC(1,:));
  cC = cC(:,idx);
  
function cb_update_client( varargin )
  update_client( gcbf );
  
function update_client( fh )
  libconfigdb();
  libaudprof();
  sCfg = get(fh,'UserData');
  hCL = findobj(fh,'tag','list:client');
  idx = get(hCL,'Value');
  if idx > size(sCfg.db,2)
    idx = 1;
  end
  sCfg.client_id = sCfg.db{1,idx};
  sAudID = ...
      configdb.smap_get(sCfg.aud_selection,...
					  sCfg.client_id,'');
  cClient = ...
      configdb.smap_get(sCfg.db,...
					  sCfg.client_id,...
					  audprof.client_new(sCfg.client_id));
  kSelected = strmatch(sAudID,cClient.audprofs(1,:),'exact');
  if isempty( kSelected )
    kSelected = size(cClient.audprofs,2);
  end
  hAL = findobj(fh,'tag','list:audprof');
  set(hAL,...
      'String',cClient.audprofs(1,:),'Value',kSelected);
  set(fh,'UserData',sCfg);
  update_audprof( fh );
    
function cb_update_audprof( varargin )
  update_audprof( gcbf );
  
function update_audprof( fh )
  libconfigdb();
  libaudprof();
  libmhagui();
  sCfg = get(fh,'UserData');
  hAL = findobj(fh,'tag','list:audprof');
  idx = get(hAL,'Value');
  csID = get(hAL,'String');
  if isempty(csID)
    sID = '';
  else
    sID = csID{idx};
  end
  sCfg.aud_selection = ...
      configdb.smap_set(sCfg.aud_selection,...
				  sCfg.client_id,sID);
  if audprof.audprof_exists( sCfg.client_id, sID, sCfg.db )
    sCfg.audprof = audprof.audprof_get(sCfg.client_id, sID, sCfg.db);
  else
    sCfg.audprof = audprof.audprof_new(sCfg.client_id);
  end
  set(fh,'UserData',sCfg);
  mhagui.audprof_plot( sCfg.audprof );
  
function reload_db( fh )
  libaudprof();
  sCfg = get(fh,'UserData');
  sCfg.db = sort_celllist(audprof.db_load());
  set(fh,'UserData',sCfg);
  update_gui( fh );
  
function cb_scan_hfd_dir( varargin )
  p = uigetdir();
  if ~ischar(p)
    return;
  end
  p = [p,filesep];
  libaudprof();
  [s,sm] = audprof.db_import_hfd_dir(p);
  for k=1:numel(s)
    s{k} = ['  - ',s{k}];
  end
  for k=1:numel(sm)
    sm{k} = ['  - ',sm{k}];
  end
  msg = [{'ACALOS data found for:'};s(:);...
	 {'';'clients not in database:'};sm(:)];
  uiwait(msgbox(msg,'ACALOS import results'));
  reload_db(gcbf);