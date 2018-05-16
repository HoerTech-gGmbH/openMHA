function mhagui_jackconnection_manager( mode, mha )
  if nargin < 1
    mode = 'gui';
  end
  if nargin < 2
    mha = struct('host','localhost','port',33337);
  end
  libconfigdb();
  switch mode
   case 'gui'
    create_gui(mha);
   case 'upload'
    name = configdb.get_mhaconfig(mha,'iojack.current_preset','');
    if ~isempty(name)
      [tmp,csPresets] = mhagui_conpreset_list(mha);
      [sPreset,idx] = configdb.smap_get(csPresets,name);
      if isempty(idx)
	error(['No preset ''',name,''' in preset list.']);
      end
      configdb.set_mhaconfig(mha,'iojack.ports_in',sPreset.ports_in);
      configdb.set_mhaconfig(mha,'iojack.ports_out',sPreset.ports_out);
      configdb.set_mhaconfig(mha,'iojack.current_preset',name);
      mha_set(mha,'io.con_in',sPreset.ports_in);
      mha_set(mha,'io.con_out',sPreset.ports_out);
    end
  end
  
function create_gui(mhah)
  libconfigdb();
  libmhagui();
  fh = findobj('Tag','mhagui_jackconnection_window');
  if isempty(fh)
    fh = mhagui.figure('MHA audio connection manager',...
			   'mhagui_jackconnection_window',...
			   [700 510],...
		'Color',col_bg);
  elseif length(fh)>1
    close(fh(2:end));
  end
  delete(get(fh,'Children'));
  figure(fh);
  sPorts = mha_get(mhah,'io.ports');
  csMHAPortInput = mha_get(mhah,'io.con_in');
  csMHAPortOutput = mha_get(mhah,'io.con_out');
  csPortNamesIn = mha_get(mhah,'io.names_in');
  csPortNamesOut = mha_get(mhah,'io.names_out');
  configdb.set_mhaconfig(mhah,'iojack.ports_in',csMHAPortInput);
  configdb.set_mhaconfig(mhah,'iojack.ports_out',csMHAPortOutput);
  p = [370 260 0 0];
  set(fh,'UserData',mhah);
  uicontrol('Style','frame','Position',[0 0 320 240]+p,...
	    'BackgroundColor',col_bg);
  uicontrol('Style','text','String','Microphone input:',...
	    'Position',[10 210 300 20]+p,...
	    'HorizontalAlignment','left',...
	    'Fontweight','bold',...
	    'BackgroundColor',col_bg);
  uicontrol('Style','listbox','String',sPorts.physical_outputs,...
	    'tag','mhagui_physical_in',...
	    'Max',2,'Min',0,...
	    'callback',@update_connections,...
	    'BackGroundColor',ones(1,3),...
	    'Position',[120 10 190 200]+p);
  uicontrol('Style','listbox','String',csPortNamesIn,...
	    'tag','mhagui_mha_inputs',...
	    'callback',@update_display_cns,...
	    'Position',[10 80 100 130]+p);
  %
  % receiver:
  %
  p = [370 10 0 0];
  uicontrol('Style','frame','Position',[0 0 320 240]+p,...
	    'BackgroundColor',col_bg);
  uicontrol('Style','text','String','Receiver output:',...
	    'Position',[10 210 300 20]+p,...
	    'HorizontalAlignment','left',...
	    'Fontweight','bold',...
	    'BackgroundColor',col_bg);
  uicontrol('Style','listbox','String',sPorts.physical_inputs,...
	    'tag','mhagui_physical_out',...
	    'Max',2,'Min',0,...
	    'callback',@update_connections,...
	    'BackGroundColor',ones(1,3),...
	    'Position',[120 10 190 200]+p);
  uicontrol('Style','listbox','String',csPortNamesOut,...
	    'tag','mhagui_mha_outputs',...
	    'callback',@update_display_cns,...
	    'Position',[10 80 100 130]+p);
  update_display_cns;
  mhagui_conpreset_manager( fh, [10 10] );

function update_display_cns( varargin )
  update_display_cns_side( 'in' );
  update_display_cns_side( 'out' );
  
function update_display_cns_side( sSide )
  libconfigdb();
  mha = get_mhahandle;
  idx = get(findobj('tag',sprintf('mhagui_mha_%sputs',sSide)),...
	    'Value');
  h_physical = findobj('tag',sprintf('mhagui_physical_%s',sSide));
  csCurrent = configdb.get_mhaconfig(mha,sprintf('iojack.ports_%s',sSide));
  if length(csCurrent) >= idx
    sPort = csCurrent{idx};
  else
    sPort = ':';
  end
  csPhysical = get(h_physical,'String');
  con_idx = strmatch(sPort,csPhysical,'exact');
  %if isempty(con_idx)
  %  csPhysical{end+1} = sPort;
  %  con_idx = length(csPhysical);
  %  set(h_physical,'String',csPhysical);
  %end
  set(h_physical,'Value',con_idx);
  
function update_connections( varargin )
  update_connection_side( 'in' );
  update_connection_side( 'out' );
  
function update_connection_side( sSide )
  libconfigdb();
  mha = get_mhahandle;
  idx = get(findobj('tag',sprintf('mhagui_mha_%sputs',sSide)),...
	    'Value');
  h_physical = findobj('tag',sprintf('mhagui_physical_%s',sSide));
  val = get(h_physical,'Value');
  if length(val) > 1
    val = val(1);
  end
  set(h_physical,'Value',val);
  csPhysical = get(h_physical,'String');
  if ~isempty(val)
    sPort = csPhysical{val};
  else
    sPort = ':';
  end
  csCurrent = configdb.get_mhaconfig(mha,sprintf('iojack.ports_%s',sSide));
  csCurrent{idx} = sPort;
  configdb.set_mhaconfig(mha,sprintf('iojack.ports_%s',sSide),csCurrent);
  mha_set(mha,sprintf('io.con_%s',sSide),csCurrent);
  
function c = col_bg
  c = 0.7*ones(1,3);
  
function mhagui_conpreset_load( name )
  libconfigdb();
  mha = get_mhahandle;
  [tmp,csPresets] = mhagui_conpreset_list(mha);
  [sPreset,idx] = configdb.smap_get(csPresets,name);
  if isempty(idx)
    error(['No preset ''',name,''' in preset list.']);
  end
  configdb.set_mhaconfig( mha,'iojack.ports_in',sPreset.ports_in);
  configdb.set_mhaconfig( mha,'iojack.ports_out',sPreset.ports_out);
  configdb.set_mhaconfig( mha,'iojack.current_preset',name);
  update_display_cns;
  update_connections;
  
function mhagui_conpreset_manager( fh, pos )
  if nargin < 1
    fh = figure
  end
  if nargin == 1
    if ischar(fh)
      switch fh
       case 'update'
	mhagui_conpreset_update_listbox;
	return;
       otherwise
	error(['Invalid mode ''',fh,'''']);
      end
    end
  end
  if nargin < 2
    pos = [0 0];
  end
  pos(3:4) = 0;
  mha = get(fh,'UserData');
  uicontrol('Style','frame',...
	    'Position',pos+[0 0 350 490]);
  uicontrol('Style','text','String','Presets:',...
	    'Position',pos+[10 460 330 20],...
	    'FontWeight','bold',...
	    'HorizontalAlignment','left');
  uicontrol('Style','ListBox',...
	    'String',mhagui_conpreset_list(mha),...
	    'Value',1,...
	    'Position',pos+[10 50 330 410],...
	    'FontSize',14,...
	    'BackgroundColor',ones(1,3),...
	    'Tag','mhagui_conpreset_manager_listbox',...
	    'CallBack',@mhagui_conpreset_select);
  uicontrol('Style','PushButton',...
	    'String','Delete',...
	    'Position',pos+[240 10 100 30],...
	    'Callback',@mhagui_conpreset_delete_selected);
  uicontrol('Style','PushButton',...
	    'String','Save',...
	    'Position',pos+[125 10 100 30],...
	    'Callback',@mhagui_conpreset_save_selected);
  uicontrol('Style','PushButton',...
	    'String','Save as...',...
	    'Position',pos+[10 10 100 30],...
	    'Callback',@mhagui_conpreset_save_as);
  mhagui_conpreset_update_listbox;
  
function mhagui_conpreset_update_listbox
  libconfigdb();
  uih = findobj('Tag','mhagui_conpreset_manager_listbox');
  mha = get_mhahandle;
  list = mhagui_conpreset_list(mha);
  cconpreset = configdb.get_mhaconfig(mha,'iojack.current_preset','');
  idx = strmatch(cconpreset,list,'exact');
  if isempty(idx)
    idx = 1;
  end
  set(uih,'String',list,'Value',idx);
  
function [csList,csPresets] = mhagui_conpreset_list(mha)
  libconfigdb();
  csPresets = ...
      configdb.get_mhaconfig(mha,'iojack.presets',cell([2,0]));
  csInNames = mha_get(mha,'io.names_in');
  csOutNames = mha_get(mha,'io.names_out');
  sPreset = struct;
  sPreset.ports_in = {};
  sPreset.ports_out = {};
  for k=1:length(csInNames)
    sPreset.ports_in{k} = ':';
  end
  for k=1:length(csOutNames)
    sPreset.ports_out{k} = ':';
  end
  csPresets = configdb.smap_set(csPresets,'disconnect',sPreset);
  for k=1:length(csInNames)
    sPreset.ports_in{k} = sprintf('system:capture_%d',mod(k-1,2)+1);
    if ~isempty(strfind(csInNames{k},'left'))
      sPreset.ports_in{k} = 'system:capture_1';
    end
    if ~isempty(strfind(csInNames{k},'right'))
      sPreset.ports_in{k} = 'system:capture_2';
    end
  end
  for k=1:length(csOutNames)
    sPreset.ports_out{k} = sprintf('system:playback_%d',mod(k-1,2)+1);
    if ~isempty(strfind(csOutNames{k},'left'))
      sPreset.ports_out{k} = 'system:playback_1';
    end
    if ~isempty(strfind(csOutNames{k},'right'))
      sPreset.ports_out{k} = 'system:playback_2';
    end
  end
  csPresets = configdb.smap_set(csPresets,'stereo i/o',sPreset);
  csList = csPresets(1,:);
  
function mhagui_conpreset_select( varargin )
  set(gcbo,'Enable','off');
  drawnow;
  val = get(gcbo,'Value');
  list = get(gcbo,'String');
  if (val > 0) & (val<=length(list))
    % todo: query_for_changes
    mhagui_conpreset_load(list{val});
  end
  mhagui_conpreset_update_listbox;
  set(gcbo,'Enable','on');
  
function mhagui_conpreset_save_selected( varargin )
  set(gcbo,'Enable','off');
  drawnow;
  uih = findobj('Tag','mhagui_conpreset_manager_listbox');
  val = get(uih,'Value');
  list = get(uih,'String');
  if (val > 0) & (val<=length(list))
    % todo: query_for_changes
    name = list{val};
    mhagui_conpreset_save(list{val});
  else
    mhagui_conpreset_save_as;
  end
  mhagui_conpreset_update_listbox;
  set(gcbo,'Enable','on');
  
function mhagui_conpreset_delete_selected( varargin )
  libconfigdb();
  set(gcbo,'Enable','off');
  drawnow;
  uih = findobj('Tag','mhagui_conpreset_manager_listbox');
  val = get(uih,'Value');
  list = get(uih,'String');
  if isempty(list)
    set(gcbo,'Enable','on');
    return;
  end
  mha = get_mhahandle;
  [tmp,csPresets] = mhagui_conpreset_list(mha);
  sPreset = struct;
  sPreset.ports_in = configdb.get_mhaconfig(mha,'iojack.ports_in');
  sPreset.ports_out = configdb.get_mhaconfig(mha,'iojack.ports_out');
  csPresets = configdb.smap_rm(csPresets,list{val});
  configdb.set_mhaconfig(mha,'iojack.presets',csPresets);
  csList = mhagui_conpreset_list(mha);
  name = '';
  if ~isempty(csList)
    name = csList{1};
  end
  mhagui_conpreset_update_listbox;
  mhagui_conpreset_load( name );
  set(gcbo,'Enable','on');
  
function mhagui_conpreset_save_as( varargin )
  set(gcbo,'Enable','off');
  drawnow;
  resp = inputdlg({'preset name'});
  if isempty(resp)
    set(gcbo,'Enable','on');
    return
  end
  name = resp{1};
  if isempty(name)
    set(gcbo,'Enable','on');
    error('Invalid preset name');
  end
  mha = get_mhahandle;
  list = mhagui_conpreset_list(mha);
  if ~isempty(strmatch(name,list,'exact'))
    set(gcbo,'Enable','on');
    error(['A preset with name ''',name,''' already exists.']);
  end
  mhagui_conpreset_save( name );
  mhagui_conpreset_update_listbox;
  set(gcbo,'Enable','on');
  
function mhagui_conpreset_save( name )
  libconfigdb();
  mha = get_mhahandle;
  %
  [tmp,csPresets] = mhagui_conpreset_list(mha);
  sPreset = struct;
  sPreset.ports_in = configdb.get_mhaconfig(mha,'iojack.ports_in');
  sPreset.ports_out = configdb.get_mhaconfig(mha,'iojack.ports_out');
  csPresets = configdb.smap_set(csPresets,name,sPreset);
  configdb.set_mhaconfig(mha,'iojack.presets',csPresets);
  configdb.set_mhaconfig(mha,'iojack.current_preset',name);
  
function mha = get_mhahandle
  global mha_basic_cfg;
  mha = mha_basic_cfg.mha;
