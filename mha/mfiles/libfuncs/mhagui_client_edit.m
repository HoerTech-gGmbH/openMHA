function sClient = mhagui_client_edit( sEditClient )
  sCfg = struct;
  libmhagui();
  libaudprof();
  if nargin < 1
    sCfg.client = audprof.client_new();
    sCfg.client.id = proposed_clientid( sCfg );
    b_edit = 0;
  else
    sCfg.client = merge_structs(sEditClient, ...
				audprof.client_new());
    b_edit = 1;
  end
  sClientEmpty = sCfg.client;
  fh = mhagui.figure('Enter new client data',...
		     'mhagui.client_edit',...
		     [360 260],...
		     'UserData',sCfg);
  vh = [];
  vh(end+1) = ...
      uicontrol_edit_lab( 'First name', [10 200 340], @mhagui_client_edit_update_cb );
  vh(end+1) = ...
      uicontrol_edit_lab( 'Last name', [10 140 340], @mhagui_client_edit_update_cb );
  vh(end+1) = ...
      uicontrol_edit_lab( 'Birthday (YYYY-MM-DD)', [10 80 180], @mhagui_client_edit_update_cb );
  vh(end+1) = ...
      uicontrol_edit_lab( 'Client ID', [210 80 140], @mhagui_client_edit_update_cb );
  if( b_edit )
    set(vh(end),'Enable','off');
  end
  mhagui_client_edit_update_gui( fh );
  sCfg = mhagui.waitfor( fh );
  if isempty(sCfg)
    sClient = [];
    return;
  end
  sClient = sCfg.client;
  if isequal(sClient,sClientEmpty)
    sClient = [];
  else
    if b_edit
      sClient.id = sEditClient.id;
    end
  end

function h = uicontrol_edit_lab( name, pos, fcn )
  pos(length(pos)+1:3) = 120;
  pos(length(pos)+1:4) = 25;
  h = uicontrol('Style','edit','tag',name,...
		'Position',pos,...
		'BackgroundColor',ones(1,3),...
		'HorizontalAlignment','left',...
		'Callback',fcn);
  uicontrol('Style','text','Position',pos+[0 pos(4) 0 -6],...
	    'HorizontalAlignment','left','String',[name,':']);
  
function mhagui_client_edit_update_cb( varargin )
  libaudprof();
  sCfg = get(gcbf,'UserData');
  tag = get(gcbo,'tag');
  val = get(gcbo,'string');
  switch tag
   case 'First name'
    sCfg.client.firstname = val;
   case 'Last name'
    sCfg.client.lastname = val;
   case 'Birthday (YYYY-MM-DD)'
    dn = 0;
    csFmt = {'yyyy-mm-dd','yyyy-mm','yyyy'};
    while (dn == 0) && (~isempty(csFmt))
      try
	dn = datenum(val,csFmt{1});
      catch
	csFmt(1) = [];
      end
    end
    if dn == 0
      fn = datenum('1900','yyyy');
    end
    sCfg.client.birthday = datestr(dn,'yyyy-mm-dd');
   case 'Client ID'
    while audprof.db_client_exists(val)
      val = [val,'_'];
    end
    sCfg.client.id = val;
   otherwise
    warning(['Unhandled tag: ',tag]);
  end
  switch tag
   case {'First name','Last name','Birthday (YYYY-MM-DD)'}
    sCfg.client.id = proposed_clientid(sCfg);
  end
  set(gcbf,'UserData',sCfg);
  mhagui_client_edit_update_gui(gcbf);
  
function id = proposed_clientid( sCfg )
  libaudprof();
  fn = '';
  if isfield(sCfg.client,'firstname')
    fn = sCfg.client.firstname;
  end
  fn(end+1:1) = '_';
  ln = '';
  if isfield(sCfg.client,'lastname')
    ln = sCfg.client.lastname;
  end
  ln(end+1:1) = '_';
  bd = '010101';
  if isfield(sCfg.client,'birthday')
    bd = sCfg.client.birthday;
    bd = datestr(datenum(bd,'yyyy-mm-dd'),'yymmdd');
  end
  id = upper(sprintf('%s%s%s',ln(1),fn(1),bd));
  while audprof.db_client_exists(id)
    id = [id,'_'];
  end
  
function mhagui_client_edit_update_gui( fh )
  sCfg = get(fh,'UserData');
  tags = struct;
  tags.firstname = 'First name';
  tags.lastname = 'Last name';
  tags.birthday = 'Birthday (YYYY-MM-DD)';
  tags.id = 'Client ID';
  for fn=fieldnames(tags)'
    if isfield(sCfg.client,fn{:})
      h = findobj(fh,'tag',tags.(fn{:}));
      set(h,'String',sCfg.client.(fn{:}));
    end
  end
