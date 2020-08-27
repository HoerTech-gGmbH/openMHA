function mhagui_fitting( mha )
% MHAGUI_FITTING - fitting interface for MHA dynamic compression
% scheme 'dc_simple', with optional components for finetuning and
% audiometry.
%
%
% Usage:
% mhagui_fitting( [ mha ] )
%
% Parameters:
% - mha  : Handle of MHA connection, see mhactl for details.
%           (default: host = 'localhost', port = 33337)
%
% The user database and configuration settings are stored in the
% file 'fitting_datab.mat' in the current directory.
%
% Author: Giso Grimm
% Date: 2007, 5/2009
  ;
  % start with a selection of the client ID. In the panel created
  % by 'select_user' an existing client ID can be selected or a new
  % client ID can be created. If the client ID is existing in the
  % simple fitting tool database, it's audiogram is loaded from the
  % database.
  if nargin < 1
    mha = [];
  end
  mha = mha_ensure_mhahandle( mha );
  global mha_basic_cfg;
  mha_get_basic_cfg_network( mha );
  libconfigdb();
  %cfg_db = struct;
  %cfg_db.selected_client = configdb.get_mhaconfig(mha,'client_id','');
  %cfg_db.selected_auds = configdb.get_mhaconfig(mha,'client_aud_ids',cell([2,0]));
  close(findobj('tag','mhagui_fitting'));
%  clientdb_args = {'Audiometer',@mha_audiometer_wrap};
%  if exist('mha_afcaudiometer')
%    clientdb_args{end+1} = 'AFC-aud.';
%    clientdb_args{end+1} = @mha_afcaudiometer;
%  end
  clientdb_args = cell(0);
  if exist('audiogram_averager')
    clientdb_args{end+1} = 'average';
    clientdb_args{end+1} = @audiogram_averager;
  end
  sAud = configdb.fit_audprof_get( mha );
  sAud = clientdb(sAud, clientdb_args{:});
  if ~isempty(sAud)
    main( sAud.client_id, sAud );
  end

function main( sClientID, sAud )
  global mha_basic_cfg;
  libmultifit();
  libconfigdb();
  sAud.client_id = sClientID;
  %sAud = audprof2aud(sAud);
  % store selected client_id and audiogram in configuration file:
  configdb.fit_audprof_set(mha_basic_cfg.mha,sAud);
  % create main window:
  csPlugs = multifit.query(mha_basic_cfg.mha);
  if length(csPlugs)>1
    gp = [770,max(300,50*length(csPlugs)+80)];
    libmhagui();
    fh = mhagui.figure(sprintf('MHA fitting tool for %s@%s, %s',...
      sClientID,mha_basic_cfg.mha.host,sAud.id),...
      'mhagui_fitting_plug_selector',gp);
    nch = [];
    for k=1:length(csPlugs)
      nch(k) = csPlugs{1}.fitmodel.channels;
    end
    if ~any(diff(nch))
      configdb.fit_set_current( mha_basic_cfg.mha, csPlugs{1} );
      configdb.fit_set_all( mha_basic_cfg.mha, csPlugs );
      create_gainrule_selector( fh, [10,60], @create_first_fit_all );
    end
    uicontrol('style','frame','position',[10,10,280,51]);
    uicontrol( 'style','pushbutton',...
	       'String','Create First Fit by plugin label',...
	       'Position',[20,20,260,30],...
	       'Callback',@create_first_fit_by_name);
    
    for k=1:length(csPlugs)
      sPlug = csPlugs{k};
      sUD = struct;
      sUD.client_id = sClientID;
      sUD.audprof = sAud;
      sUD.plug = sPlug;
      uicontrol('style','pushbutton','String',sprintf('%s (%s)',sPlug.addr,sPlug.plugin),...
		'Position',[300 gp(2)-50*k,460,40],'UserData',sUD,'callback',@cb_start_plugin_fitting);
    end
    uicontrol('style','pushbutton','String','Close',...
	      'Position',[300 10,460,40],'callback','close(gcf);');
  elseif length(csPlugs)==1
    configdb.fit_set_current(mha_basic_cfg.mha,csPlugs{1});
    mhagui_fitting_main;
  else
    msgbox('No fittable plugins available');
  end
  
function cb_start_plugin_fitting(varargin)
  global mha_basic_cfg;
  sUD = get(gcbo,'UserData');
  libconfigdb();
  configdb.fit_set_current(mha_basic_cfg.mha,sUD.plug);
  mhagui_fitting_main;
  
function mhagui_fitting_main;
  try
    global mha_basic_cfg;
    libfinetuning();
    libconfigdb();
    [sPlug,sClientID,sAud] = ...
	configdb.fit_get_current( mha_basic_cfg.mha );
    close(findobj('tag','mhagui_fitting'));
    guip = [1000,700];
    libmhagui();
    fh = mhagui.figure(sprintf('MHA fitting tool for %s@%s, %s',...
				   sClientID,mha_basic_cfg.mha.host,sAud.id),...
			   'mhagui_fitting',guip);
    figure(fh)
    switch sPlug.fitmodel.channels
     case 1
      x_lateral = 'uni';
     case 2
      x_lateral = 'bi';
     otherwise
      sPlug.fitmodel.channels
      error(sprintf('Only uni- and bi-lateral fits supported (%d channels)',sPlug.fitmodel.channels));
    end
    uicontrol('style','text','string',sprintf('%s\n(plugin: ''%s'', %slateral)',sPlug.addr,sPlug.plugin,x_lateral),...
	      'Position',[10,guip(2)-60,guip(1)-20,50],'HorizontalAlignment','left',...
	      'FontSize',14);
    uicontrol('style','text','String','Right',...
    	      'position',[10,guip(2)-100,guip(1)/2-160,30],...
    	      'Fontsize',14,'backgroundcolor',[0.7,0.5,0.5]);
    uicontrol('style','text','String','Left',...
    	      'position',[guip(1)/2+150,guip(2)-100,guip(1)/2-160,30],...
    	      'Fontsize',14,'backgroundcolor',[0.5,0.5,0.7]);
    uicontrol('style','pushbutton','String','Close',...
	      'position',[guip(1)/2-140,10,280,40],...
	      'Fontsize',14,...
	      'callback','close(gcf);');
    axb = 40;
    [vXTick,csXTickLabel] = freq_xtick;
    ax1 = axes('tag','target_gain_r','Units','pixel',...
	       'Position',[10+axb,230,guip(1)/2-160-axb,350],...
	       'XScale','log',...
	       'XLim',[63,16000],...
	       'XTick',vXTick,...
	       'XTickLabel',csXTickLabel,...
	       'YLim',[-10 120],...
	       'NextPlot','replacechildren');
    xlabel('frequency / Hz');
    ylabel('3rd octave level / dB SPL');
    ax2 = axes('tag','target_gain_l','Units','pixel',...
	       'Position',[guip(1)/2+150+axb,230,guip(1)/2-160-axb,350],...
	       'XScale','log',...
	       'XLim',[63,16000],...
	       'XTick',vXTick,...
	       'XTickLabel',csXTickLabel,...
	       'YLim',[-10 120],...
	       'NextPlot','replacechildren');
    xlabel('frequency / Hz');
    ylabel('3rd octave level / dB SPL');
    if isfield(sPlug,'finetuning')
      sFT = sPlug.finetuning;
    else
      sFT = finetuning.init();
    end
    finetuning_pos = [guip(1)/2-160,165];
    finetuning.drawgui(sFT,1,[10,10,finetuning_pos],fh,@upload_current_fit);
    finetuning.drawgui(sFT,2,[guip(1)/2+150,10,finetuning_pos],fh,@upload_current_fit);
    drawnow;
    create_gainrule_selector( fh, [guip(1)/2-140 guip(2)-290] );
    mhagui_preset_manager( fh, [guip(1)/2-140,80] );
  catch
    %close(fh);
    disp_err_rethrow;
  end
  
function update_finetuning
  global mha_basic_cfg;
  libfinetuning();
  libconfigdb();
  sPlug = configdb.fit_get_current( mha_basic_cfg.mha );
  if isfield(sPlug,'finetuning')
    sFT = sPlug.finetuning;
  else
    sFT = finetuning.init();
  end
  finetuning.updategui(sFT);

function toggle_select_side(varargin)
  global mha_basic_cfg;
  libconfigdb();
  sPlug = configdb.fit_get_current( mha_basic_cfg.mha );
  sData = struct;
  for ch='lr'
    sData.h.(ch) = findobj(gcf,'tag',['mhagui_fitting_checkbox_select_side_',ch]);
  end
  tag = get(gcbo,'tag');
  if strncmp(tag,'mhagui_fitting_checkbox_select_side_',36)
    channel = tag(end);
    otherchannel = setdiff('lr',channel);
    if sPlug.fitmodel.channels == 2
      set(sData.h.r,'value',1,'Enable','off');
      set(sData.h.l,'value',1,'Enable','off');
    else
      set(sData.h.(otherchannel),'value',1-get(sData.h.(channel),'value'));
    end
  else
    if sPlug.fitmodel.channels == 2
      set(sData.h.r,'value',1,'Enable','off');
      set(sData.h.l,'value',1,'Enable','off');
    else
      set(sData.h.r,'value',0);
      set(sData.h.l,'value',1);
    end
  end
  for ch='lr'
    sData.v.(ch) = get(sData.h.(ch),'value');
  end
  sSide = '';
  if sData.v.l
    sSide(end+1) = 'l';
  end
  if sData.v.r
    sSide(end+1) = 'r';
  end
  configdb.set_mhaconfig(mha_basic_cfg.mha,'next_fit_side',sSide);
  
function sAud = mha_audiometer_wrap( clientid, sAud )
  global mha_basic_cfg;
  issndfile = [];
  if isfield(mha_basic_cfg.base,'audiometerbackend')
    base = mha_basic_cfg.base.audiometerbackend;
  else
    error('No audiometer backend found');
  end
  sAudOrig = sAud;
  sACfg = struct;
  sACfg.mha = mha_basic_cfg.mha;
  sACfg.base = base;
  sAud = mha_audiometer(sACfg,sAud);
  if isempty(sAud)
    return
  end
  if isequal(sAud,sAudOrig)
    sAud = [];
  else
    sAud.id = sprintf('%s in-situ',datestr(now,'yyyy-mm-dd HH-MM'));
    resp = inputdlg('Enter new audiogram ID:','Audiogram ID',1, ...
		    {sAud.id});
    if ~isempty(resp)
      sAud.id = resp{:};
    end
  end
  return
  
function create_first_fit( varargin )
  global mha_basic_cfg;
  libmultifit();
  libconfigdb();
  set(gcbo,'Enable','off');
  drawnow;
  try
    [sPlug,sClientID,sAud,sRule,sSide] = configdb.fit_get_current( mha_basic_cfg.mha );
    sPlug = multifit.firstfit(sPlug,sRule,sAud,sSide);
    configdb.fit_set_current(mha_basic_cfg.mha,sPlug);
    upload_current_fit;
    sPreset = ['FirstFit (',upper(sSide),') ',sRule,', ',sAud.id];
    configdb.fit_preset_save(mha_basic_cfg.mha,sPreset);
    mhagui_preset_update_listbox;
    set(gcbo,'Enable','on');
  catch
    set(gcbo,'Enable','on');
    disp_err_rethrow;
  end
  
function create_first_fit_all( varargin )
  global mha_basic_cfg;
  libmultifit();
  libconfigdb();
  csPlugs = configdb.fit_get_all( mha_basic_cfg.mha );
  set(gcbo,'Enable','off');
  drawnow;
  try
    for k=1:length(csPlugs)
      configdb.fit_set_current(mha_basic_cfg.mha,csPlugs{k});
      [sPlug,sClientID,sAud,sRule,sSide] = configdb.fit_get_current( mha_basic_cfg.mha );
      sPlug = multifit.firstfit(sPlug,sRule,sAud,sSide);
      configdb.fit_set_current(mha_basic_cfg.mha,sPlug);
      configdb.fit_upload_current(mha_basic_cfg.mha);
      sPreset = ['FirstFit (',upper(sSide),') ',sRule,', ',sAud.id];
      configdb.fit_preset_save(mha_basic_cfg.mha,sPreset);
      %mhagui_preset_update_listbox;
    end
    set(gcbo,'Enable','on');
  catch
    set(gcbo,'Enable','on');
    disp_err_rethrow;
  end
  
function create_first_fit_by_name( varargin )
  global mha_basic_cfg;
  libmultifit();
  libconfigdb();
  csPlugs = configdb.fit_get_all( mha_basic_cfg.mha );
  set(gcbo,'Enable','off');
  drawnow;
  try
    for k=1:length(csPlugs)
      configdb.fit_set_current(mha_basic_cfg.mha,csPlugs{k});
      [sPlug,sClientID,sAud,sRule,sSide] = configdb.fit_get_current( mha_basic_cfg.mha );
      sRule = sPlug.addr;
      sRule(1:max(find(sRule=='.'))) = [];
      if ~exist(['gainrule_',sRule])
	msg = ['Gainrule ''',sRule,''' does not exist.'];
	errordlg(msg);
	error(msg);
      end
      sPlug = multifit.firstfit(sPlug,sRule,sAud,sSide);
      configdb.fit_set_current(mha_basic_cfg.mha,sPlug);
      configdb.fit_upload_current(mha_basic_cfg.mha);
      sPreset = ['FirstFit (',upper(sSide),') ',sRule,', ',sAud.id];
      configdb.fit_preset_save(mha_basic_cfg.mha,sPreset);
    end
    set(gcbo,'Enable','on');
  catch
    err = lasterror;
    errordlg(err.message);
    set(gcbo,'Enable','on');
    disp_err_rethrow;
  end
  
function upload_current_fit( sFT )
  global mha_basic_cfg;
  libconfigdb();
  if nargin >= 1
    configdb.fit_upload_current( mha_basic_cfg.mha,sFT);
  else
    configdb.fit_upload_current( mha_basic_cfg.mha );
  end
  update_current_fit_gui;

function create_gainrule_selector( fh, pos, first_fit_cb )
  global mha_basic_cfg;
  libmultifit();
  libconfigdb();
  if nargin < 3
    first_fit_cb = @create_first_fit;
  end
  pos(3:4) = 0;
  uiw = 280;
  uih = 220;
  csRules = multifit.list_gainrules();
  uicontrol(fh,'Style','frame',...
	    'Position',pos+[0 0 uiw uih]);
  uicontrol(fh,'Style','text',...
	    'String','Gain prescription rule:',...
	    'Position',pos+[10 uih-25 uiw-20 20],...
	    'HorizontalAlign','left');
  huih = ...
      uicontrol(fh,'Style','listbox','String',csRules,...
		'Position',pos+[10 70 uiw-20 uih-95],...
		'BackgroundColor',ones(1,3),...
		'FontSize',14,...
		'Callback',@gainrule_selection);
  uicontrol(fh,'Style','pushbutton',...
	    'String','Create First Fit',...
	    'Position',pos+[10 10 uiw-20 30],...
	    'Callback',first_fit_cb);
  if length(csRules) < 1
    error('No gain rules found');
  end
  sRule = configdb.clientconfig_get(mha_basic_cfg.mha, 'gain_rule', '' );
  idx = strmatch(sRule,csRules,'exact');
  if isempty(idx)
    idx = 1;
    sRule = csRules{idx};
  end
  configdb.clientconfig_set(mha_basic_cfg.mha, 'gain_rule', sRule );
  set(huih,'Value',idx);
  uicontrol('style','radiobutton','String','Right',...
	    'position',pos+[10,45,120,20],...
	    'backgroundcolor',[0.7,0.5,0.5],...
	    'value',1,'tag','mhagui_fitting_checkbox_select_side_r',...
	    'callback',@toggle_select_side);
  uicontrol('style','radiobutton','String','Left',...
	    'position',pos+[150,45,120,20],...
	    'backgroundcolor',[0.5,0.5,0.7],...
	    'value',1,'tag','mhagui_fitting_checkbox_select_side_l',...
	    'callback',@toggle_select_side);
  toggle_select_side;

function gainrule_selection(varargin)
  global mha_basic_cfg;
  libconfigdb();
  v = get(gcbo,'Value');
  csRules = get(gcbo,'String');
  if v>length(csRules)
    error('No gainrule available');
  end
  configdb.clientconfig_set(mha_basic_cfg.mha, 'gain_rule', csRules{v} );

function mhagui_preset_manager( fh, pos )
  global mha_basic_cfg;
  libconfigdb();
  pos(3:4) = 0;
  uiw = 280;
  uih = 320;
  uicontrol(fh,'Style','frame',...
	    'Position',pos+[0 0 uiw uih]);
  uicontrol(fh,'Style','text','String','Presets:',...
	    'Position',pos+[10 uih-25 uiw-20 20],...
	    'HorizontalAlignment','left');
  [csList,sCur,Idx] = configdb.fit_preset_list(mha_basic_cfg.mha);
  uicontrol(fh,'Style','ListBox',...
	    'String',csList,...
	    'Value',Idx,...
	    'Position',pos+[10 50 uiw-20 uih-75],...
	    'FontSize',14,...
	    'BackgroundColor',ones(1,3),...
	    'Tag','mhagui_preset_manager_listbox',...
	    'CallBack',@mhagui_preset_select);
  dx = floor((uiw-40)/3);
  uicontrol(fh,'Style','PushButton',...
	    'String','Save as...',...
	    'Position',pos+[10 10 dx 30],...
	    'Callback',@mhagui_preset_save_as);
  uicontrol(fh,'Style','PushButton',...
	    'String','Save',...
	    'Position',pos+[20+dx 10 dx 30],...
	    'Callback',@mhagui_preset_save_selected);
  uicontrol(fh,'Style','PushButton',...
	    'String','Delete',...
	    'Position',pos+[30+2*dx 10 dx 30],...
	    'Callback',@mhagui_preset_delete_selected);
  mhagui_preset_update_listbox;
  if ~isempty(sCur)
    configdb.fit_preset_load(mha_basic_cfg.mha,sCur);
    update_current_fit_gui;
    update_finetuning;
  end
  
function mhagui_preset_update_listbox
  global mha_basic_cfg;
  libconfigdb();
  uih = findobj('Tag','mhagui_preset_manager_listbox');
  list = configdb.fit_preset_list( mha_basic_cfg.mha );
  sPlug = configdb.fit_get_current( mha_basic_cfg.mha );
  cpreset = configdb.clientconfig_get( mha_basic_cfg.mha,[sPlug.addr,'.current_preset'],'');
  idx = strmatch(cpreset,list,'exact');
  if isempty(idx)
    idx = 1;
    if ~isempty(list)
      configdb.fit_preset_load( mha_basic_cfg.mha,list{1});
      update_current_fit_gui;
      update_finetuning;
    end
  end
  set(uih,'String',list,'Value',idx);
  
function mhagui_preset_select( varargin )
  global mha_basic_cfg;
  libconfigdb();
  try
    set(gcbo,'Enable','off');
    drawnow;
    val = get(gcbo,'Value');
    list = get(gcbo,'String');
    if (val > 0) & (val<=length(list))
      %%% todo: query_for_changes
      %if ~modified_ok
      %  mhagui_preset_update_listbox;
      %  set(gcbo,'Enable','on');
      %  return
      %end
      configdb.fit_preset_load( mha_basic_cfg.mha,list{val});
      update_current_fit_gui;
    end
    mhagui_preset_update_listbox;
    update_finetuning;
    set(gcbo,'Enable','on');
  catch
    set(gcbo,'Enable','on');
    disp_err_rethrow;
  end
  
function mhagui_preset_delete_selected( varargin )
  global mha_basic_cfg;
  libconfigdb();
  try
    sPlug = configdb.fit_get_current( mha_basic_cfg.mha );
    csPresets = configdb.clientconfig_get(mha_basic_cfg.mha,[sPlug.addr,'.presets'],cell([2 0]));
    set(gcbo,'Enable','off');
    drawnow;
    uih = findobj('Tag','mhagui_preset_manager_listbox');
    val = get(uih,'Value');
    list = get(uih,'String');
    if (val > 0) & (val<=length(list))
      % todo: query_for_changes
      name = list{val};
      idx = strmatch(name,csPresets(1,:),'exact');
      answ = questdlg(['Do you really want to delete the preset' ...
		       ' ''',name,'''?'],'Delete preset','Yes','No','No');
      if strcmp(answ,'Yes')
	csPresets = csPresets(:,setdiff(1:size(csPresets,2),idx));
	configdb.clientconfig_set(mha_basic_cfg.mha,[sPlug.addr,'.presets'],csPresets);
	mhagui_preset_update_listbox;
      end
    end
    set(gcbo,'Enable','on');
  catch
    set(gcbo,'Enable','on');
    disp_err_rethrow;
  end
  
function mhagui_preset_save_selected( varargin )
  global mha_basic_cfg;
  libconfigdb();
  try
    set(gcbo,'Enable','off');
    drawnow;
    uih = findobj('Tag','mhagui_preset_manager_listbox');
    val = get(uih,'Value');
    list = get(uih,'String');
    if (val > 0) & (val<=length(list))
      % todo: query_for_changes
      name = list{val};
      %if strncmp(name,'First fit,',10)
      if ~isempty(findstr(name,'FirstFit'))
	errordlg('Cannot overwrite first fit! Please select another name.');
	set(gcbo,'Enable','on');
	return
      end
      configdb.fit_preset_save(mha_basic_cfg.mha,list{val});
    else
      mhagui_preset_save_as;
    end
    mhagui_preset_update_listbox;
    set(gcbo,'Enable','on');
  catch
    set(gcbo,'Enable','on');
    disp_err_rethrow;
  end
  
  
function mhagui_preset_save_as( varargin )
  global mha_basic_cfg;
  libconfigdb();
  try
    set(gcbo,'Enable','off');
    drawnow;
    resp = inputdlg({'Preset name'});
    if isempty(resp)
      return
    end
    name = resp{1};
    if isempty(name)
      error('Invalid preset name');
    end
    list = configdb.fit_preset_list(mha_basic_cfg.mha);
    if ~isempty(strmatch(name,list,'exact'))
      error(['A preset with name ''',name,''' already exists.']);
    end
    configdb.fit_preset_save(mha_basic_cfg.mha, name );
    mhagui_preset_update_listbox;
    set(gcbo,'Enable','on');
  catch
    set(gcbo,'Enable','on');
    disp_err_rethrow;
  end

function [vXTick,csXTickLabel] = freq_xtick
  vXTick = 1000*2.^[-3:3];
  csXTickLabel = cell(size(vXTick));
  for k=1:length(vXTick)
    if vXTick(k)<1000
      csXTickLabel{k} =sprintf('%g',vXTick(k));
    else
      csXTickLabel{k} =sprintf('%gk',vXTick(k)/1000);
    end  
  end
  
function update_current_fit_gui
  global mha_basic_cfg;
  libmultifit();
  libconfigdb();
  libaudprof();
  sPlug = configdb.fit_get_current( mha_basic_cfg.mha );
  cf_children = get(gcf,'children');
  cf_tags = get(cf_children,'tag');
  if isfield(sPlug,'final_gaintable')
    sPlugFinal = sPlug;
    %% todo: target returns final gaintable automatically if available
    sPlugFinal.gaintable = sPlug.final_gaintable;
    sTarget = multifit.targetgain(sPlugFinal,[40,65,90]);
    % update plots:
    [vXTick,csXTickLabel] = freq_xtick;
    for ch=setdiff('lr',sPlugFinal.fitmodel.side)
      ax = cf_children(strcmp(cf_tags,['target_gain_',ch]));
      delete(get(ax,'Children'));
      set(ax,'XScale','log',...
      	     'XLim',minmax(sTarget.f),...
	     'XTick',vXTick,...
	     'XTickLabel',csXTickLabel,...
      	     'YLim',[-10 120],...
	     'Tag',['target_gain_',ch]);
      axes(ax);
      xlabel('frequency / Hz');
      ylabel('3rd octave level / dB SPL');
      set(cf_children(strcmp(cf_tags,['mhagui_fitting_checkbox_select_side_',ch])),'Value',0);
    end
    for ch=sPlugFinal.fitmodel.side
      set(cf_children(strcmp(cf_tags,['mhagui_fitting_checkbox_select_side_',ch])),'Value',1);
      idxmid = round(length(sTarget.f)/2);
      ax = cf_children(strcmp(cf_tags,['target_gain_',ch]));
      axes(ax);
      ca = get(ax,'children');
      ca_tags = get(ca,'tag');
      bExist = true;
      for kl=1:length(sTarget.levels)
        h = ca(strcmp(ca_tags,['outgain_' ch '_tagetGain_' num2str(sTarget.levels(kl))]));
        ht = ca(strcmp(ca_tags,['textoutgain_' ch '_tagetGain_' num2str(sTarget.levels(kl))]));
        if ~isempty(h)
          h_outgain(kl,1) = h;
          h_outgain(kl,2) = ht; 
        else
          bExist = false;
        end
      end
      if bExist
        for kl=1:length(sTarget.levels)
          set(h_outgain(kl,1),'ydata',sTarget.(ch).outlevel(kl,:));
          set(h_outgain(kl,2),'position',[sTarget.f(idxmid) sTarget.(ch).outlevel(kl,idxmid) 0]);
        end
        drawnow expose;
      else
        sHTL = audprof.threshold_get( sPlugFinal.audprof, ch, ...
					  'htl_ac' );
        vaudF = [sHTL.data.f];
        vaudH = [sHTL.data.hl] + isothr(vaudF);
        vFAudP = [sTarget.f(1),sTarget.f(1),vaudF,sTarget.f(end),sTarget.f(end)];
        hold off;
        if strcmp(ch,'l')
          audstyle = 'b-x';
        else
          audstyle = 'r-o';
        end
        plot(1,1,'k-');
        hold all;
        patch(vFAudP,...
        [-10,vaudH(1),vaudH,vaudH(end),-10],...
        [0.7,0.75,0.7]);
        plot(vaudF,vaudH,audstyle,'MarkerSize',10);
        plot(sTarget.f,sTarget.inlevel,'linestyle','--');
        for kl=1:length(sTarget.levels)
          plot(sTarget.f,sTarget.(ch).outlevel(kl,:),'-','linewidth',3,...
          'tag',['outgain_' ch '_tagetGain_' num2str(sTarget.levels(kl))]);
        end
        patch([sTarget.f(1),sTarget.f,sTarget.f(end)],...
        [-10,isothr(sTarget.f),-10],0.7*ones(1,3));
        for kl=1:length(sTarget.levels)
    text(sTarget.f(idxmid),sTarget.(ch).outlevel(kl,idxmid),...
         sprintf('%g dB',sTarget.levels(kl)),...
         'VerticalAlignment','bottom','Fontweight','bold',...
         'tag',['textoutgain_' ch '_tagetGain_' num2str(sTarget.levels(kl))]);
        end
        %hold off;
        set(ax,'XScale','log',...
               'XLim',minmax(sTarget.f),...
         'XTick',vXTick,...
         'XTickLabel',csXTickLabel,...
               'YLim',[-10 120],...
         'Tag',['target_gain_',ch]);
        xlabel('frequency / Hz');
        ylabel('3rd octave level / dB SPL');
      end
    end
  end

function mm = minmax(x)
  mm = [min(x),max(x)];
