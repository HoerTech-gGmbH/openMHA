function audiometercalib( mha, addr )
  if nargin < 1
    mha = [];
  end
  mha = mha_ensure_mhahandle( mha );
  if nargin < 2
    scfg = mha_get_basic_cfg_network( mha );
    if ~isfield(scfg.base,'audiometerbackend')
      error('No audiometer backend found in current MHA');
    end
    addr = scfg.base.audiometerbackend;
  end
  global sAudiometercalib;
  libconfigdb();
  sAudiometercalib.calib.retSPL_fun = '';
  sAudiometercalib = struct;
  sAudiometercalib.mha = mha;
  sAudiometercalib.addr = addr;
  sAudiometercalib.reference_level = 80;
  sAudiometercalib.side = [];
  sAudiometercalib.sigtype = mha_get(mha,[addr,'.sigtype']);
  sAudiometercalib.ramplen = mha_get(mha,[addr,'.ramplen']);
  sAudiometercalib.f = [];
  sAudiometercalib.c = [];
  % read data from config file:
  load_current;
  % setup screen:
  fh = setup_screen;
  % do the calibration:
  uiwait(fh);
  
  
% debug:
%sAudiometercalib.calib

function csFuns = list_retSPL_funs
  [pathstr,name,ext] = fileparts(which(mfilename));
  d1 = dir([pathstr,filesep,'retspl_*.m']);
  d2 = dir([pathstr,filesep,'retspl_*.',mexext]);
  d = [d1(:);d2(:)];
  csFuns = cell(length(d),1);
  for k=1:length(d)
    [pathstr,name,ext] = fileparts(d(k).name);
    csFuns{k} = name(8:end);
  end


function save_current
  global sAudiometercalib;
  libconfigdb();
  eval(['[vf_corr,c2hl] = retspl_',sAudiometercalib.calib.retSPL_fun,';']);
  vf_corr = [0.5*vf_corr(1);vf_corr(:);2*vf_corr(end)];
  c2hl = [c2hl(1);c2hl(:);c2hl(end)];
  coupler2hl = interp1(log(vf_corr),c2hl,log(sAudiometercalib.calib.f),...
		       'linear','extrap');
  sAudiometercalib.calib.spl2hl = ...
      sAudiometercalib.calib.corr + ...
      repmat(coupler2hl(:)',[2,1]);
  configdb.set_mhaconfig(sAudiometercalib.mha,cfgpath,sAudiometercalib.calib);
  
function load_current
  global sAudiometercalib;
  libconfigdb();
  sAudiometercalib.calib = ...
      configdb.get_mhaconfig(...
	  sAudiometercalib.mha,cfgpath,...
	  struct('f',[],'corr',[],'retSPL_fun',''));

function sPath = cfgpath
  global sAudiometercalib;
  addr = sAudiometercalib.addr;
  addr(1:max(find(addr=='.'))) = [];
  sPath = ...
      ['aud_calib.',addr,...
       '.',sAudiometercalib.sigtype];
  
function fh = setup_screen
  global sAudiometercalib;
  libmhagui();
  close(findobj('tag','audcal:audiometercalib'));
  fh = mhagui.figure(['Audiometer calibration (',sAudiometercalib.sigtype,')'],...
		     'audcal:audiometercalib',[800,600]);
  sAudiometercalib.fh = fh;
  vBG = get(fh,'Color');
  % frequency selection:
  add_label([200,570,130,20],'Frequency / Hz:');
  uicontrol('style','edit','tag','audcal:add_freq','callback',@add_freq,...
	    'Position',[200,540,150,30]);
  uicontrol('style','listbox','tag','audcal:select_freq','string', ...
	    sAudiometercalib.calib.f,'callback',@select_freq, ...
	    'FontSize',14,...
	    'Position',[200,380,150,160]);
  uicontrol('style','pushbutton','String','remove freq.',...
	    'callback',@remove_freq,...
	    'Position',[360,540,110,30]);
  uicontrol('style','pushbutton','String','clear all',...
	    'callback',@remove_allfreq,...
	    'Position',[360,500,110,30]);
  uicontrol('style','pushbutton','String','add standard',...
	    'callback',@addstd_freq,...
	    'Position',[360,460,110,30]);

  % channel selection:
  add_label([20,570,100,20],'Channels:');
  uicontrol('style','togglebutton','String','left',...
	    'UserData',0,'Callback',@select_side,...
	    'tag','audcal:select_side',...
	    'BackgroundColor',[0.4,0.4,0.8],...
	    'Position',[20,530,160,40]);
  uicontrol('style','togglebutton','String','right',...
	    'UserData',1,'Callback',@select_side,...
	    'tag','audcal:select_side',...
	    'BackgroundColor',[0.8,0.4,0.4],...
	    'Position',[20,480,160,40]);
  uicontrol('style','togglebutton','String','MUTE',...
	    'UserData',[],'Callback',@select_side,...
	    'Value',1,...
	    'tag','audcal:select_side',...
	    'Position',[20,380,160,90]);
  
  % level control:
  add_label([20,340,160,20],'Reference level / dB:');
  uicontrol('style','edit','tag','audcal:ref_level','string',sAudiometercalib.reference_level,...
	    'Position',[20,310,160,30],'callback',@set_ref_level);
  
  add_label([20,280,160,20],'Correction / dB:');
  uicontrol('style','edit','tag','audcal:corr_value','string',0,...
	    'callback',@update_level_ed,...
	    'Position',[20,250,160,30]);
  
  uicontrol('style','pushbutton','String','Cancel',...
	    'Position',[540,20,110,37],'Callback',@cancel_quit);
  uicontrol('style','pushbutton','String','Save & Quit',...
	    'Position',[670,20,110,37],'Callback',@save_and_quit);
  
  ax = axes('Units','pixel',...
	    'tag','audcal:corr_axes',...
	    'Position',[530,410,190,150],...
	    'XScale','log','XLim',[100,20000],...
	    'XTick',1000*2.^[-3:4],...
	    'XTickLabel',2.^[-3:4],...
	    'NextPlot','add');
  xlabel('Frequency / kHz');
  ylabel('Correction / dB');
  plot(ax,[100,20000],[0,0],'k-');
  grid on;
  
  csFuns = list_retSPL_funs;
  if isempty(csFuns)
    error('No retSPL table found!');
  end
  idx = strmatch(sAudiometercalib.calib.retSPL_fun,csFuns,'exact');
  if isempty(idx)
    idx = 1;
    sAudiometercalib.calib.retSPL_fun = csFuns{1};
  end
  add_label([200,340,130,20],'retSPL:');
  sHelp = help(['retspl_',sAudiometercalib.calib.retSPL_fun]);
  uicontrol('style','listbox','String',csFuns,...
	    'Position',[200,150,180,190],...
	    'Value',idx,...
	    'FontSize',14,...
	    'Callback',@select_receiver_type);
  uicontrol('style','frame','Position',[380,150,400,190]);
  uicontrol('style','listbox','tag','audcal:receiver_help','String',str2cell(sHelp),...
	    'Position',[382,151,396,186],'HorizontalAlignment','left',...
	    'Min',0,'Max',2,'Value',[],'Enable','inactive',...
	    'FontSize',8);
  %'BackgroundColor',get(fh,'Color'),...
  
  %drawnow;
  % level changers:
  vdl = [-16,-4,-1,-0.25];
  vdl = sort([vdl,-vdl]);
  vh = [];
  for dl=vdl
    vh(end+1) = ...
	uicontrol('Style','Pushbutton',...
		  'String',sprintf('%g dB',dl),...
		  'UserData',dl,...
		  'callback',@update_level_but);
  end
  dx = 5;
  wpos = get(gcf,'Position');
  p = [20 70];
  p(3:4) = 0;
  wx = (wpos(3)-40+dx)/length(vh);
  for k=1:length(vh)
    col = 2*((k-1)/(length(vh)-1)-0.5);
    if col > 0
      vcol = [0.7+0.2*col 0.7-0.3*col 0.7-0.3*col];
    else
      col = -col;
      vcol = [0.7-0.3*col 0.7+0.2*col 0.7-0.3*col];
    end
    set(vh(k),'Position',[round(wx*(k-1)) 0 round(wx-dx) 55]+p,...
	      'BackgroundColor',vcol);
  end
  set(findobj(gcf,'style','edit'),'BackgroundColor',ones(1,3),...
		    'HorizontalAlignment','left');
  
  if ~isempty(sAudiometercalib.calib.f)
    select_freqv(sAudiometercalib.calib.f(1));
  end
  plot_corr;
  
function add_freq( varargin )
  try
    v = str2num(get(gcbo,'String'));
    if isfinite(v) && (numel(v)==1)
      global sAudiometercalib;
      select_freqv( v );
    end
  catch
    disp_err_rethrow;
  end
  
function select_freq( varargin )
  try
    vf = str2num(get(gcbo,'String'));
    idx = get(gcbo,'Value');
    if ~isempty(idx)
      if idx>0
	select_freqv( vf(idx));
      end
    end
  catch
    disp_err_rethrow;
  end
  
  
function select_freqv( f )
  try
    global sAudiometercalib;
    if ~isempty(f)
      idx = find(sAudiometercalib.calib.f==f);
      if isempty(idx)
	sAudiometercalib.calib.f(end+1) = f;
	sAudiometercalib.calib.corr(1:2,numel(sAudiometercalib.calib.f)) = 0;
	[sAudiometercalib.calib.f,sortidx] = ...
	    sort(sAudiometercalib.calib.f);
	sAudiometercalib.calib.corr = sAudiometercalib.calib.corr(:,sortidx);
	idx = find(sAudiometercalib.calib.f==f);
      end
      hList = findobj(sAudiometercalib.fh,'tag','audcal:select_freq');
      set(hList,'String',sAudiometercalib.calib.f,'Value',idx);
      hEd = findobj(sAudiometercalib.fh,'tag','audcal:add_freq');
      set(hEd,'String',f);
      sAudiometercalib.f = f;
      sAudiometercalib.c = ...
	  sAudiometercalib.calib.corr(sAudiometercalib.side+1,idx);
      set(findobj(sAudiometercalib.fh,'tag','audcal:corr_value'),'String',sAudiometercalib.c);
      plot_corr;
      set_mha_freq;
      %) && (~isempty(sAudiometercalib.side))
    end
  catch
    disp_err_rethrow;
  end
  
function select_side( varargin )
  try
    global sAudiometercalib;
    vH = findobj(sAudiometercalib.fh,'tag','audcal:select_side');
    h_other = setdiff(vH,gcbo);
    sAudiometercalib.side = get(gcbo,'UserData');
    set(gcbo,'Value',1);
    set(h_other,'Value',0);
    select_freqv(sAudiometercalib.f);
    %set(gcbo,'Value'
  catch
    disp_err_rethrow;
  end

function update_level_but( varargin )
  try
    global sAudiometercalib;
    if ~isempty(sAudiometercalib.side)
      dL = get(gcbo,'UserData');
      idx = find(sAudiometercalib.calib.f==sAudiometercalib.f);
      sAudiometercalib.calib.corr(sAudiometercalib.side+1,idx) = ...
	  sAudiometercalib.calib.corr(sAudiometercalib.side+1,idx) + dL;
      sAudiometercalib.c = ...
	  sAudiometercalib.calib.corr(sAudiometercalib.side+1,idx);
      set(findobj(sAudiometercalib.fh,'tag','audcal:corr_value'),'String',sAudiometercalib.c);
      set_mha_level;
      plot_corr
    end
  catch
    disp_err_rethrow;
  end
  
function update_level_ed( varargin )
  try
    global sAudiometercalib;
    if ~isempty(sAudiometercalib.side)
      L = str2num(get(gcbo,'String'));
      if isfinite(L) && (numel(L)==1)
	idx = find(sAudiometercalib.calib.f==sAudiometercalib.f);
	sAudiometercalib.calib.corr(sAudiometercalib.side+1,idx) = L;
	sAudiometercalib.c = ...
	    sAudiometercalib.calib.corr(sAudiometercalib.side+1,idx);
	set(findobj(sAudiometercalib.fh,'tag','audcal:corr_value'),'String',sAudiometercalib.c);
	set_mha_level;
	plot_corr
      end
    end
  catch
    disp_err_rethrow;
  end
  
function plot_corr
  global sAudiometercalib;
  ax = findobj(sAudiometercalib.fh,'tag','audcal:corr_axes');
  delete(get(ax,'Children'));
  xl = [125,16000];
  if ~isempty(sAudiometercalib.f)
    xl(1) = min(xl(1),min(sAudiometercalib.f));
    xl(2) = max(xl(2),max(sAudiometercalib.f));
  end
  plot(ax,xl,[0,0],'k-');
  set(ax,'Xlim',xl);
  if ~isempty(sAudiometercalib.f)
    if ~isempty(sAudiometercalib.side)
      col = sAudiometercalib.side*[1,0,0]+...
	    (1-sAudiometercalib.side)*[0,0,1];
      plot(ax,sAudiometercalib.f,sAudiometercalib.c,...
	   'o','MarkerSize',15,'linewidth',2,...
	   'Color',0.5*col,...
	   'MarkerFaceColor',0.3*col+0.7);
    end
    vp = plot(ax,...
	      sAudiometercalib.calib.f',...
	      sAudiometercalib.calib.corr',...
	      '*-');
    set(vp(1),'Color',[0,0,1]);
    set(vp(2),'Color',[1,0,0]);
  end
  drawnow;
  
function set_mha_level
  global sAudiometercalib;
  if ~isempty(sAudiometercalib.c)
    mha_set(sAudiometercalib.mha,...
	    [sAudiometercalib.addr,'.level'],...
	    sAudiometercalib.c+sAudiometercalib.reference_level);
  end
  
function set_mha_freq
  global sAudiometercalib;
  mha_set(sAudiometercalib.mha,...
	  [sAudiometercalib.addr,'.level'],...
	  -100);
  pause(sAudiometercalib.ramplen+0.05);
  cfg = struct;
  cfg.freq = sAudiometercalib.f;
  if isempty(sAudiometercalib.side)
    cfg.mode = 'mute';
  else
    csSide = {'left','right'};
    cfg.mode = csSide{sAudiometercalib.side+1};
  end
  mha_set(sAudiometercalib.mha,...
	  sAudiometercalib.addr,...
	  cfg);
  set_mha_level;

function save_and_quit(varargin);
  try
    % save data to config file:
    global sAudiometercalib;
    mha_set(sAudiometercalib,...
	    [sAudiometercalib.addr,'.mode'],...
	    'input');
    save_current;
    close(gcbf);
  catch
    disp_err_rethrow;
  end

function cancel_quit(varargin);
  try
    % save data to config file:
    global sAudiometercalib;
    mha_set(sAudiometercalib,...
	    [sAudiometercalib.addr,'.level'],...
	    -20);
    close(gcbf);
  catch
    disp_err_rethrow;
  end
  
function set_ref_level( varargin )
  try
    global sAudiometercalib;
    v = str2num(get(gcbo,'String'));
    if isfinite(v) && (numel(v)==1)
      sAudiometercalib.reference_level = v;
      set_mha_level;
    end
  catch
    disp_err_rethrow;
  end
  
function add_label(pos,s)
  uicontrol('style','text','Position',pos,'String',s,...
	    'HorizontalAlignment','left',...
	    'FontWeight','bold',...
	    'BackgroundColor',get(gcf,'Color'));
  
function select_receiver_type(varargin)
  try
    global sAudiometercalib;
    csList = get(gcbo,'String');
    sFun = csList{get(gcbo,'Value')};
    
    sAudiometercalib.calib.retSPL_fun = sFun;
    sHelp = help(['retspl_',sAudiometercalib.calib.retSPL_fun]);
    set(findobj(sAudiometercalib.fh,'tag','audcal:receiver_help'),...
	'String',str2cell(sHelp));
  catch
    disp_err_rethrow;
  end
  
function cStr = str2cell( s )
  idx = find(s==sprintf('\n'));
  if isempty(idx)
    cStr = {s};
  else
    cStr = {};
    lidx = 1;
    for k=1:length(idx)
      subs = s(lidx:(idx(k)-1));
      cStr{end+1} = subs;
      lidx = idx(k)+1;
    end
    cStr{end+1} = s(idx(end)+1:end);
  end

function remove_freq( varargin )
  try
    global sAudiometercalib;
    hList = findobj(sAudiometercalib.fh,'tag','audcal:select_freq');
    hEd = findobj(sAudiometercalib.fh,'tag','audcal:add_freq');
    idx = get(hList,'Value');
    sAudiometercalib.calib.f(idx) = [];
    sAudiometercalib.calib.corr(:,idx) = [];
    idx = 1;
    if isempty(sAudiometercalib.calib.f)
      sAudiometercalib.f = [];
      sAudiometercalib.c = [];
    else
      sAudiometercalib.f = sAudiometercalib.calib.f(idx);
      sAudiometercalib.c = sAudiometercalib.calib.corr(sAudiometercalib.side+1,idx);
    end
    set(hList,'String',sAudiometercalib.calib.f,'Value',1);
    set(hEd,'String',sAudiometercalib.f);
    set(findobj(sAudiometercalib.fh,'tag','audcal:corr_value'),'String',sAudiometercalib.c);
    plot_corr;
    if ~isempty(sAudiometercalib.f)
      set_mha_freq;
    end
  catch
    disp_err_rethrow;
  end
  

function remove_allfreq( varargin )
  try
    global sAudiometercalib;
    sAudiometercalib.calib.f = [];
    sAudiometercalib.calib.corr = [];
    sAudiometercalib.f = [];
    sAudiometercalib.c = [];
    plot_corr;
    hList = findobj(sAudiometercalib.fh,'tag','audcal:select_freq');
    set(hList,'String',sAudiometercalib.calib.f,'Value',1);
    hEd = findobj(sAudiometercalib.fh,'tag','audcal:add_freq');
    set(hEd,'String',sAudiometercalib.f);
    set(findobj(sAudiometercalib.fh,'tag','audcal:corr_value'),'String',sAudiometercalib.c);
  catch
    disp_err_rethrow;
  end
  
function addstd_freq( varargin )
  try
    vf = stdaud_freq;
    global sAudiometercalib;
    vf = setdiff(vf(:),sAudiometercalib.calib.f(:))';
    nc = zeros(2,length(vf));
    sAudiometercalib.calib.f = [sAudiometercalib.calib.f,vf];
    sAudiometercalib.calib.corr = [sAudiometercalib.calib.corr,nc];
    [sAudiometercalib.calib.f,sortidx] = ...
	sort(sAudiometercalib.calib.f);
    sAudiometercalib.calib.corr = sAudiometercalib.calib.corr(:, ...
						  sortidx);
    if isempty(sAudiometercalib.f)
      idx = [];
    else
      idx = find(sAudiometercalib.calib.f==sAudiometercalib.f);
    end
    if isempty(idx)
      idx = 1;
      sAudiometercalib.f = sAudiometercalib.calib.f(1);
    end
    sAudiometercalib.c = ...
	sAudiometercalib.calib.corr(sAudiometercalib.side+1,idx);
    hList = findobj(sAudiometercalib.fh,'tag','audcal:select_freq');
    set(hList,'String',sAudiometercalib.calib.f,'Value',idx);
    hEd = findobj(sAudiometercalib.fh,'tag','audcal:add_freq');
    set(hEd,'String',sAudiometercalib.f);
    set(findobj(sAudiometercalib.fh,'tag','audcal:corr_value'),'String',sAudiometercalib.c);
    plot_corr;
    set_mha_freq;
    %
    %if ~isempty(f)
    %  idx = find(sAudiometercalib.calib.f==f);
    %  if isempty(idx)
    %    sAudiometercalib.calib.f(end+1) = f;
    %    sAudiometercalib.calib.corr(1:2,numel(sAudiometercalib.calib.f)) = 0;
    %    [sAudiometercalib.calib.f,sortidx] = ...
    %	  sort(sAudiometercalib.calib.f);
    %    sAudiometercalib.calib.corr = sAudiometercalib.calib.corr(:,sortidx);
    %    idx = find(sAudiometercalib.calib.f==f);
    %  end
    %  hList = findobj(sAudiometercalib.fh,'tag','audcal:select_freq');
    %  set(hList,'String',sAudiometercalib.calib.f,'Value',idx);
    %  hEd = findobj(sAudiometercalib.fh,'tag','audcal:add_freq');
    %  set(hEd,'String',f);
    %  sAudiometercalib.f = f;
    %  sAudiometercalib.c = ...
    %	sAudiometercalib.calib.corr(sAudiometercalib.side+1,idx);
    %  set(findobj(sAudiometercalib.fh,'tag','audcal:corr_value'),'String',sAudiometercalib.c);
    %  plot_corr;
    %  set_mha_freq;
    %  %) && (~isempty(sAudiometercalib.side))
    %end
  catch
    disp_err_rethrow;
  end
  
function vf = stdaud_freq
  vf = unique([1000*2.^[-3:3],1500*2.^[-1:2]])';
  