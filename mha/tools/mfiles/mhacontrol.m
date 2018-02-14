function mhacontrol( hostname, port, b_wait_fh, advanced_callback )
% MHACONTROL - Audiological interface to the MHA
%
% Usage:
% mhacontrol( hostname [, port [, uiwait ] ] )
%
% hostname : Name or IP address of MHA host
% port     : Port number of MHA host (default: 33337)
% uiwait   : Flag if Matlab should wait for user interface
%            (default: 1)
%
% Author: Giso Grimm
% Date: 7/2007
  global mha_basic_cfg;
  libconfigdb();
  libmhagui();
  mha_basic_cfg = [];
  try
    if nargin < 1
      hostname = 'localhost';
    end
    if nargin < 2
      port = 33337;
    end
    if nargin < 3
      b_wait_fh = 1;
    end
    if nargin < 4
      advanced_callback = [];
    end
    if isstruct(hostname)
      mha = hostname;
    else
      mha = struct('host',hostname,'port',port);
    end
    %
    sz = get(0,'ScreenSize');
    sz = round(sz(3:4)/2-150);
    img = configdb.readfile('mha_ini.mat','bitmap.splash',ones(300,300,3));
    hsplash = figure('Name','Starting MHA control','NumberTitle','off',...
		     'Position',[sz,300,318],'MenuBar', ...
		     'none','WindowStyle','modal');
    axes('Unit','normalized','Position',[0,18/318,1,300/318]);
    image(img);
    axes('Unit','normalized','Position',[0,0,1,18/318],...
	 'XLim',[0,1],'NextPlot','add','XTick',[],'YTick',[]);
    plot([0,1],[0,0],'LineWidth',18,'Color',[0,0,0]/255);
    hold on;
    wbh = plot([0,0],[0,0],'LineWidth',18,'Color',[134,159,49]/255);
    drawnow;
    %hsplash = waitbar(0,'Starting MHA control');
    mha_get_basic_cfg_network( mha );
    %waitbar(0.25,hsplash);
    set(wbh,'XData',[0,0.25]);drawnow;
    %if isfield(mha_basic_cfg.base,'transducers')
    %  mhagui_calibration_dialog( 'upload', mha );
    %end
    set(wbh,'XData',[0,0.5]);drawnow;
    %waitbar(0.75,hsplash);
    if isfield(mha_basic_cfg.base,'MHAIOJack')
      try
	mhagui_jackconnection_manager( 'upload', mha );
      catch
	warning(lasterr);
      end
    end
    set(wbh,'XData',[0,0.75]);drawnow;
    %waitbar(0.5,hsplash);
    set(wbh,'XData',[0,1]);drawnow;
    %waitbar(1,hsplash);
    pause(0.1);
    close(hsplash);
    fh = setup_ctlgui(advanced_callback);drawnow;
    setup_windowmenus;
    if b_wait_fh
      uiwait(fh);
    end
  catch
    err = lasterror;
    uiwait(errordlg(err.message,'Error'));
    close(findobj('Tag','mhacontrol_mainwindow'));
    if ishandle(hsplash)
      close(hsplash);
    end
    msg = err.message;
    for k=1:length(err.stack)
      msg = sprintf('%s\n%s:%d',msg,err.stack(k).file,err.stack(k).line);
    end
    warning(msg);
    rethrow(err);
  end
  
function setup_windowmenus
  global mha_basic_cfg;
  libconfigdb();
  h = uimenu('Label','&File');
  uimenu(h,'Label','&Export client database','Callback',@mhagui_export_auds);
  uimenu(h,'Label','&Quit mhacontrol','Callback',@mhagui_quit);
  h = uimenu('Label','&Windows');
  if isfield( mha_basic_cfg.base,'transducers' )
    uimenu(h,'Label','Calibration dialogue',...
	   'Callback',@start_calib);
    uimenu(h,'Label','Level meter',...
	   'Callback',@start_levelmeter);
  end
  if isfield(mha_basic_cfg.base,'MHAIOJack')
    uimenu(h,'Label','Connection manager',...
	   'Callback',@start_audio_connection);
  end
  uimenu(h,'Label','Generic control interface',...
	 'Callback',@start_generic_gui);
  uimenu(h,'Label','Fitting',...
	 'Callback',@start_fitting);
  if isfield(mha_basic_cfg.base,'addsndfile')
    uimenu(h,'Label','In-Situ sound files',...
	   'Callback',@start_insitu);
  else
    sndf_idx = strmatch('addsndfile',mha_basic_cfg.all_id_plugs(:,2)','exact');
    if ~isempty(sndf_idx)
      for k=sndf_idx(:)'
	addr = mha_basic_cfg.all_id_plugs{k,1};
	uimenu(h,...
	       'Label',['In-Situ sound files (',addr,')'],...
	       'UserData',addr,...
	       'Callback',@start_insitu_addr);
      end
    end
  end
  if isfield(mha_basic_cfg.base,'testbox')
    uimenu(h,'Label','Virtual test box',...
	   'Callback',@start_testbox);
  end
  uimenu(h,'Label','Graphical plugin profiler',...
	 'Callback',@start_profiling);
  if ~isempty(strmatch('dc_afterburn',...
		       mha_basic_cfg.all_id_plugs(:,2)))
    uimenu(h,'Label','MPO configuration',...
	   'Callback',@start_mpoconfig);
  end
  cUserMenu = ...
      configdb.get_mhaconfig(mha_basic_cfg.mha,'usermenu',{});
  if ~isempty(cUserMenu)
    for kMenu=1:size(cUserMenu,1)
      h = findobj('type','uimenu','label',cUserMenu{kMenu,1});
      if isempty(h)
	h = uimenu('Label',cUserMenu{kMenu,1});
      end
      h = h(1);
      cSubMenu = cUserMenu{kMenu,2};
      for kPD=1:size(cSubMenu,1)
	uimenu(h,'Label',cSubMenu{kPD,1},'Callback',cSubMenu{kPD,2});
      end
    end
  end
  
function fh = setup_ctlgui(advanced_callback)
  global mha_basic_cfg;
  libconfigdb();
  libmhagui();
  close(findobj('Tag','mhacontrol_mainwindow'));
  wsize = [660 630];
  if isfield(mha_basic_cfg.base, 'mastergain' )
    wsize = wsize + [70,0];
  end
  fh = mhagui.figure('mhacontrol - Control panel for the HoerTech openMHA',...
			 'mhacontrol_mainwindow',wsize,...
			 'Color',col_bg);
  if isfield(mha_basic_cfg.base, 'mastergain' )
    uicontrol('style','frame',...
	      'Position',[wsize(1)-69,1,69,wsize(2)],...
	      'BackgroundColor',[0.8 0.4 0.4]);
    mhagui_scalar(mha_basic_cfg.mha,...
		  [mha_basic_cfg.base.mastergain,'.gain'],...
		  [wsize(1),160]);
    delete(findobj('Tag',['label:',mha_basic_cfg.base.mastergain,'.gain']));
    set(findobj('Tag',[mha_basic_cfg.base.mastergain,'.gain']),...
	'Position',[wsize(1)+20-70,60,30,wsize(2)-70]);
    set(findobj('Tag',[mha_basic_cfg.base.mastergain,'.gain=str']),...
	'Position',[wsize(1)+10-70,20,50,30]);
  end
  %set(fh,'Position',pos-[0 round(pos(4)/2) 0 0]);
  %mhagui_restore_windowpos(fh);
  ax = axes('units','pixels','position',[1 581 660 50]);
  bm_header = ...
      configdb.get_mhaconfig(mha_basic_cfg.mha,'bitmap.header',...
		     uint8(255*ones(40,660,3)));
  bm_quit = ...
      configdb.get_mhaconfig(mha_basic_cfg.mha,'bitmap.quit',...
		     uint8(179*ones(26,23,3)));
  image(double(bm_header)/256);
  set(ax,'XTick',[],'YTick',[]);
  if isfield(mha_basic_cfg.base,'altconfig')
    p = [20 280 0 0];
    uicontrol('Style','frame',...
	      'Position',p+[0 0 300 280]);
    mhagui_keyword(mha_basic_cfg.mha,[mha_basic_cfg.base.altconfig,'.select'],[40 495 300]);
    uih = findobj(fh,'Tag',[mha_basic_cfg.base.altconfig,'.select']);
    set(uih,'Style','listbox','Position',[1 1 298 239]+p,...
	    'FontSize',14,'TooltipString','');
    uih = findobj(fh,'Style','text','String','select');
    set(uih,'Position',[1 240 298 39]+p,...
	    'Style','ToggleButton',...
	    'Enable','inactive',...
	    'Value',1,...
	    'BackgroundColor',[0.7 0.8 0.7],...
	    'FontSize',14,'String','Algorithm:');
  elseif isfield(mha_basic_cfg.base,'altplugs')
    p = [20 280 0 0];
    uicontrol('Style','frame',...
	      'Position',p+[0 0 300 280]);
    mhagui_keyword_listbox(mha_basic_cfg.mha,[mha_basic_cfg.base.altplugs,'.select'],[40 495 300]);
    uih = findobj(fh,'Tag',[mha_basic_cfg.base.altplugs,'.select']);
    set(uih,'Position',[1 1 298 239]+p,...
	    'FontSize',14,'TooltipString','');
    uih = findobj(fh,'Style','text','String','select');
    set(uih,'Position',[1 240 298 39]+p,...
	    'BackgroundColor',[0.7 0.8 0.7],...
	    'FontSize',14,'String','Algorithm:');
%'Style','ToggleButton',...
%	    'Enable','inactive',...
%	    'Value',1,...
  end
  [p,ps] = mhacontrol_panel([340 330],230,'Actions:',[0.7 0.8 0.7]);
  uih = [];
  uih(end+1) = ...
      uicontrol(fh,'Style','Pushbutton','Callback',@start_fitting,...
		'String','Fitting',...
		'Position',[11 ps-length(uih)*60 278 50]+p);
  if isfield(mha_basic_cfg.base,'addsndfile')
    uih(end+1) = ...
	uicontrol(fh,'Style','Pushbutton','Callback',@start_insitu,...
		  'String','in-situ signals',...
		  'Position',[11 ps-length(uih)*60 278 50]+p);
  end
  if isfield(mha_basic_cfg.base,'transducers')
    uih(end+1) = ...
	uicontrol(fh,'Style','pushbutton','Callback',@start_levelmeter,...
		  'String','Levels','Position',[11 ps-length(uih)*60 278 50]+p);
  end
  set(uih,'fontsize',14,'BackgroundColor',col_bg);
  %
  % Advanced administration panel
  %
  [p,ps] = mhacontrol_panel([340 80],230,'Administration:',[0.8 0.7 0.7]);
  uih = [];
  if isfield(mha_basic_cfg.base,'MHAIOJack')
    uih(end+1) = ...
	uicontrol(fh,'Style','Pushbutton',...
		  'Callback',@start_audio_connection,...
		  'String','Audio connections',...
		  'Position',[11 ps-length(uih)*60 278 50]+p);
  end
% if isfield(mha_basic_cfg.base,'transducers')
%   uih(end+1) = ...
%	uicontrol(fh,'Style','Pushbutton',...
%		  'Callback',@start_calib,...
%		  'String','Calibration',...
%		  'Position',[11 ps-length(uih)*60 278 50]+p);
%  end
  if isempty(advanced_callback)
    advanced_callback = configdb.get_mhaconfig(mha_basic_cfg.mha, ...
				       'gui_advanced_callback',@mhagui_generic );
  end
  advanced_name = configdb.get_mhaconfig(mha_basic_cfg.mha, ...
				 'gui_advanced_name', 'Advanced control');
  uih(end+1) = ...
      uicontrol(fh,'Style','Pushbutton','Callback',@start_mhagui,...
		'UserData',advanced_callback,...
		'String',advanced_name,'Position',[11 ps-length(uih)*60 278 50]+p);
  set(uih,'fontsize',14,'BackgroundColor',col_bg);
  %
  % Quit
  %
  m_quit = 0.7*ones(26,280,3);
  m_quit(:,11:33,:) = double(bm_quit)/255;
  uicontrol(fh,'Style','Pushbutton',...
	    'Callback',@mhagui_quit,...
	    'FontSize',14,...
	    'CData',m_quit,...
	    'BackgroundColor',col_bg,...
	    'String','Quit','Position',[340 20 300 40]);
  %
  % configuration selector:
  %
  extraCfg = configdb.get_mhaconfig(mha_basic_cfg.mha,'extraCfg',cell([2,0]));
  if ~isempty(extraCfg)
    p = mhacontrol_panel([20 20],240,'Configuration-Addons:',[0.7 0.7 0.8]);
    uicontrol('Style','listbox','Position',[1 1 298 199]+p,...
	      'FontSize',14,'TooltipString','','String',extraCfg(1,:),...
	      'UserData',extraCfg,'Value',1,'callback',@eval_extracfg);
    mha_set(mha_basic_cfg.mha,'',extraCfg{2,1});
  else
    extraQuery = configdb.get_mhaconfig(mha_basic_cfg.mha,'extraQuery',cell([2,0]));
    if ~isempty(extraQuery)
      extraQueryTitle = ...
	  configdb.get_mhaconfig(mha_basic_cfg.mha, ...
			 'extraQueryTitle','Configuration-Addons:');
      p = mhacontrol_panel([20 20],240,extraQueryTitle,[0.7 0.7 0.8]);
      uicontrol('Style','listbox','Position',[1 1 298 199]+p,...
		'FontSize',14,'TooltipString','','String',extraQuery(1,:),...
		'UserData',extraQuery,'Value',1,'callback',@eval_extraquery);
      mha_query(mha_basic_cfg.mha,'',extraQuery{2,1});
    end
  end
  return
  
function eval_extracfg(varargin)
  extraCfg = get(gcbo,'UserData');
  val = get(gcbo,'Value');
  cfg = extraCfg{2,val};
  global mha_basic_cfg;
  mha_set(mha_basic_cfg.mha,'',cfg);
  
function eval_extraquery(varargin)
  extraQuery = get(gcbo,'UserData');
  val = get(gcbo,'Value');
  cfg = extraQuery{2,val};
  global mha_basic_cfg;
  mha_query(mha_basic_cfg.mha,'',cfg);
  
function [p,ps] = mhacontrol_panel(p0,h,str,c)
  if nargin < 4
    c = [0.7 0.7 0.8];
  end
  p = [p0(1) p0(2) 0 0];
  uicontrol('Style','frame',...
	    'Position',p+[0 0 300 h]);
  uicontrol('Style','ToggleButton',...
	    'String',str,...
	    'Enable','inactive',...
	    'Value',1,...
	    'BackgroundColor',c,...
	    'FontSize',14,'FontWeight','bold',...
	    'Position',[1 h-40 298 39]+p);
  ps = h-99;
  
function mhagui_quit( varargin )
  close(findobj('Tag','mhagui_calib_window'));
  close(findobj('Tag','mhacontrol_mainwindow'));
  close(findobj('Tag','mhagui_fitting'));
  close(findobj('Tag','mhagui_bandlevelmeter'));
  close(findobj('Tag','mhagui_insitu_window'));
  close(findobj('Tag','clientdb'));
  
function mha = get_mha
  global mha_basic_cfg;
  mha = mha_basic_cfg.mha;
  
function start_calib( varargin )
  mhagui_calibration_dialog( 'gui', get_mha );
  
function start_mpoconfig( varargin )
  mpoconfig(get_mha);
  
function start_audio_connection( varargin )
  mhagui_jackconnection_manager( 'gui', get_mha );
  
function start_mhagui(varargin)
  cb = get(gcbo,'UserData');
  cb( get_mha );
  
function start_generic_gui(varargin)
  mhagui_generic(get_mha);
  
function start_fitting(varargin)
  mhagui_fitting( get_mha )
  
function start_levelmeter(varargin)
  mhagui_bandlevelmeter( get_mha );
  
function start_insitu(varargin)
  mhagui_addsndfile( get_mha );
  
function start_insitu_addr(varargin)
  mhagui_addsndfile( get_mha,get(gcbo,'UserData') );
  
function start_testbox(varargin)
  testbox( get_mha );
  
function start_profiling(varargin)
  mha_profiling_plot(mha_profiling_get( get_mha));
  
function c = col_bg
  c = 0.7*ones(1,3);
  
function mhagui_export_auds(varargin)
  [fname,fpath] = uiputfile('*.csv','Export client database');
  if ischar(fname)
    clientdb_export_auds([fpath,fname]);
  end
