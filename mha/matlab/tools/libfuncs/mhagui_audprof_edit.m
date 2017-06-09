function sAud = mhagui_audprof_edit( sClientID, sAud )
  libaudprof();
  if nargin < 1
    sClientID = '';
  end
  if nargin < 2
    sAud = audprof.audprof_new();
  end
  sAud.client_id = sClientID;
  sAud = audprof.audprof_fillnan( sAud );
  fh = mhagui_figure(sprintf('Auditory profile editor for ''%s''.',sClientID),...
		     'mhagui_audprof_edit',...
		     [740,540],'UserData',sAud);
  draw_aud_editor(fh);
%  uicontrol('Style','PushButton','Position',[330,220-42,180,32],...
%	    'String','add ACALOS from file',...
%	    'Callback',@mhagui_audprof_edit_acalos);
%  uicontrol('Style','PushButton','Position',[330+180+10,220-42,180,32],...
%	    'String','remove ACALOS data',...
%	    'Tag','rmacalos',...
%	    'Enable',mhagui_audprof_edit_acalos_en(sAud),...
%	    'Callback',@mhagui_audprof_edit_rmacalos);
%  uicontrol('Style','PushButton','Position',[330,220-84,180,32],...
%	    'String','audiogram from ACALOS',...
%	    'Tag','acalos2aud',...
%	    'Enable',mhagui_audprof_edit_acalos_en(sAud),...
%	    'Callback',@mhagui_audprof_edit_acalos2aud);
  %
  uicontrol(fh,'Style','text','String','auditory profile ID',...
	    'Position',[330,99,280,18],...
	    'FontWeight','bold','fontsize',8,'HorizontalAlignment','left');
  uicontrol('Style','Edit','Position',[330,76,280,23],...
	    'String',sAud.id,...
	    'Callback',@mhagui_audprof_edit_audid,...
	    'Tag','aud.id',...
	    'BackgroundColor',[1,1,1],'HorizontalAlignment','left');
  uicontrol('Style','PushButton','Position',[620,76,80,23],...
	    'String','set date ID',...
	    'Callback',@mhagui_audprof_edit_audid2date);
  %
  axes('Units','Pixels','Position',[70,40,240,172],'Tag', ...
       'audiogram_axes');
  mhagui_audprof_plot(sAud);
  sAud = mhagui_waitfor( fh );
  if ~isempty(sAud)
    sAud = audprof.audprof_cleanup( sAud );
  end
  
function draw_aud_editor( fh )
  libaudprof();
  delete(findobj(fh,'Tag','aud.editor'));
  sAud = get(fh,'UserData');
  sCol.l = [0,0,0.7];
  sCol.r = [0.7,0,0];
  dy = 465;
  uicontrol(fh,'style','frame','Position',[20 220 700 300],'tag', ...
	    'aud.editor');
  sAud = audprof.audprof_fillnan( sAud );
  for side='rl'
    uicontrol(fh,'Style','text','String',upper(side),...
	      'Position',[40 dy-82 40 105],...
	      'tag','aud.editor',...
	      'FontWeight','bold','fontsize',32,'ForegroundColor',sCol.(side));
    for type={'htl_ac','htl_bc','ucl'}
      stype = type{:};
      uicontrol(fh,'Style','text','String',stype,...
		'Position',[80 dy 60 23],...
		'tag','aud.editor',...
		'FontWeight','bold','fontsize',12,'ForegroundColor',sCol.(side));
      for kf = 1:numel(sAud.(side).(stype).data)
	sAc = sAud.(side).(stype).data(kf);
	sUD = struct('f',sAc.f,'side',side,'type',stype);
	uicontrol(fh,'Style','text','String',numk2str(sAc.f),...
		  'Position',[140+(kf-1)*50 dy+23 50 18],...
		  'tag','aud.editor',...
		  'FontWeight','bold','fontsize',8);
	uicontrol(fh,'Style','Edit','String',num2str(sAc.hl),...
		  'Callback',@mhagui_audprof_validateentry,...
		  'Position',[140+(kf-1)*50 dy 50 23],...
		  'ForegroundColor',sCol.(side),...
		  'tag','aud.editor',...
		  'BackgroundColor',[1,1,1],...
		  'UserData',sUD);
      end
      dy = dy - 41;
    end
    dy = dy - 20;
  end
  set(fh,'UserData',sAud);

function s = numk2str( v )
  if v<1000
    s = sprintf('%g',v);
  else
    v = v/1000;
    s = sprintf('%dk',floor(v));
    if v-floor(v) > 0
      stmp = sprintf('%g',v-floor(v));
      s = [s,stmp(3:end)];
    end
  end
  
function mhagui_audprof_edit_audid2date( varargin )
  sAud = get(gcbf,'UserData');
  sAud.id = datestr(now,'YYYY-mm-dd HH:MM:SS');
  set(gcbf,'UserData',sAud);
  set(findobj(gcbf,'Tag','aud.id'),'String',sAud.id);
  
function mhagui_audprof_edit_audid( varargin )
  sAud = get(gcbf,'UserData');
  sAud.id = get(gcbo,'String');
  set(gcbf,'UserData',sAud);

function mhagui_audprof_validateentry( varargin )
  libaudprof();
  sAud = get(gcbf,'UserData');
  sPar = get(gcbo,'UserData');
  val = str2num(get(gcbo,'String'));
  if isempty(val)
    val = NaN;
  end
  sAud.(sPar.side).(sPar.type) = ...
      audprof.threshold_entry_add( sAud.(sPar.side).(sPar.type), ...
				   sPar.f, val );
  set(gcbf,'UserData',sAud);
  set(gcbo,'String',num2str(val));
  mhagui_audprof_plot(sAud);
  
function mhagui_audprof_edit_acalos( varargin )
  sAud = get(gcbf,'UserData');
  [fn,p,fi] = uigetfile('*.hfd;*.HFD','Open an ACALOS (hfd) file');
  if ischar(fn)
    libaudprof();
    sAud = audprof.acalos_hfdfile_load( sAud, [p,fn] );
    set(gcbf,'UserData',sAud);
    mhagui_audprof_plot(sAud);
    set(findobj(gcbf,'Tag','acalos2aud'),...
	'Enable',mhagui_audprof_edit_acalos_en(sAud));
    set(findobj(gcbf,'Tag','rmacalos'),...
	'Enable',mhagui_audprof_edit_acalos_en(sAud));
  end

function mhagui_audprof_edit_rmacalos( varargin )
  libaudprof();
  sAud = get(gcbf,'UserData');
  for side='lr'
    if isfield(sAud,side) &&  isfield(sAud.(side),'acalos')
      sAud.(side) = rmfield(sAud.(side),'acalos');
    end
  end
  set(gcbf,'UserData',sAud);
  mhagui_audprof_plot(sAud);
  set(findobj(gcbf,'Tag','acalos2aud'),...
      'Enable',mhagui_audprof_edit_acalos_en(sAud));
  set(findobj(gcbf,'Tag','rmacalos'),...
      'Enable',mhagui_audprof_edit_acalos_en(sAud));

function sEnable = mhagui_audprof_edit_acalos_en( sAud )
  sEnable = 'off';
  for side='lr'
    if isfield(sAud,side) &&  isfield(sAud.(side),'acalos')
      sEnable = 'on';
    end
  end
    
function mhagui_audprof_edit_acalos2aud( varargin )
  libaudprof();
  sAud = get(gcbf,'UserData');
  changed = false;
  for side='lr'
    if isfield(sAud,side) &&  isfield(sAud.(side),'acalos')
      changed = true;
      sAc = sAud.(side).acalos;
      L0 = 0.1*round(10*([sAc.lcut] - 25./[sAc.mlow]));
      L50 = 0.1*round(10*([sAc.lcut] + 25./[sAc.mhigh]));
      sAud.(side).htl_ac = ...
	  audprof.threshold_entry_add([],...
				      [sAc.f],L0);
      sAud.(side).ucl = ...
	  audprof.threshold_entry_add([],...
				      [sAc.f],L50);
    end
  end
  if changed
    set(gcbf,'UserData',sAud);
    draw_aud_editor( gcbf );
    mhagui_audprof_plot(sAud);
  end