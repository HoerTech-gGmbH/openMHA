function mhagui_calibration_dialog( mode, mha )
  if nargin < 1
    mode = 'gui';
  end
  if nargin < 2
    mha = struct('host','localhost','port',33337);
  end
  libconfigdb();
  mha_get_basic_cfg_network( mha );
  switch mode
   case 'upload'
    calib_upload;
   case 'gui'
    calib_gui;
   otherwise
    error(sprintf('Invalid calibration mode ''%s''.',mode));
  end

function calib_gui
  global mha_basic_cfg;
  libconfigdb();
  libmhagui();
  fh = findobj('Tag','mhagui_calib_window');
  if isempty(fh)
    fh = mhagui.figure('MHA Calibration','mhagui_calib_window',...
			   [652 513],...
			   'Color',col_bg);
  elseif length(fh)>1
    close(fh(2:end));
  end
  delete(get(fh,'Children'));
  figure(fh);
  calib_current_calib;
  cal_current = configdb.get_mhaconfig(mha_basic_cfg.mha,'calib_current');
  if isempty(cal_current)
    cal_current = '';
  end
  [tmp,csCals] = get_calib_db;
  calib_idx = strmatch(cal_current,csCals,'exact');
  if isempty(calib_idx)
    calib_idx = 1;
  end
  wpos = get(gcf,'Position');
  wpos = wpos(3)-40;
  uicontrol('Style','listbox','String',csCals,...
	    'Position',[20 70 wpos 250],'Tag','mhagui_calib_listbox',...
	    'Callback',@calib_select_from_list,...
	    'FontSize',14,...
	    'BackgroundColor',ones(1,3),...
	    'Value',calib_idx);
  vh = [];
  vh(end+1) = ...
      uicontrol('Style','PushButton',...
		'String','New',...
		'Callback',@calib_clone_and_edit);
  vh(end+1) = ...
      uicontrol('Style','PushButton',...
		'String','Remove',...
		'Callback',@calib_remove_calib);
  vh(end+1) = ...
      uicontrol('Style','PushButton',...
		'String','Close',...
		'Callback','close(gcf)');
  dx = 5;
  wx = (wpos+(length(vh)-1)*dx)/length(vh);
  for k=1:length(vh)
    set(vh(k),'Position',[20+round(wx*(k-1)) 20 round(wx-dx) 40]);
  end
  set(vh,'BackgroundColor',col_bg,...
	 'FontSize',14);
  draw_calib_data( calib_default, [20 350] );
  calib_upload;
  
function draw_calib_data( sCalib, p )
  global mha_basic_cfg;
  fs = mha_get(mha_basic_cfg.mha, ...
	       [mha_basic_cfg.base.transducers, ...
		'.calib_out.config.srate']);
  p(3:4) = 0;
  create_pl_uicontrols( 'in', p );
  create_pl_uicontrols( 'out', p+[160 0 0 0] );
  ax = axes('Units','pixels','Position',[340 20 260 120]+p,...
	    'Layer','top');
  cCol = ['kbr'];
  hold off;
  for ch=1:mha_basic_cfg.nch.out
    plot(100,0,[cCol(mha_basic_cfg.side.out(ch)+1),'-'],...
		'tag',sprintf('mhagui_calib_recresp_pl%d',ch),...
		'linewidth',2);
    hold on;
  end
  set(ax,'XLim',[150 fs/2],'YLim',[-30 30],...
	 'XTick',250*2.^[0:round(log2(fs))],'XScale','log',...
	 'Layer','top');
  grid('on');
  update_current_calib_gui(sCalib);
  
function create_pl_uicontrols( tag, p )
  global mha_basic_cfg;
  vSide = mha_basic_cfg.side.(tag);
  csLabels = mha_basic_cfg.names.(tag);
  nch = length(csLabels);
  mCol = [0 0 0;0 0 1;1 0 0];
  for k=1:nch
    kSide = vSide(k)+1;
    uicontrol('Style','text',...
	      'String','xxx',...
	      'Position',[100 140-k*20 60 20]+p,...
	      'Tag',sprintf('mhagui_calib_peaklevel_%s_text_%d',tag,k),...
	      'HorizontalAlignment','left',...
	      'BackgroundColor',col_bg,...
	      'ForegroundColor',mCol(kSide,:));
    uicontrol('Style','text',...
	      'String',csLabels{k},...
	      'Position',[0 140-k*20 95 20]+p,...
	      'HorizontalAlignment','right',...
	      'BackgroundColor',col_bg,...
	      'FontWeight','bold');
  end
  
function calib_remove_calib( varargin )
  global mha_basic_cfg;
  libconfigdb();
  try
    name = configdb.get_mhaconfig(mha_basic_cfg.mha,'calib_current');
    resp = questdlg(sprintf(['Do you really want to remove the' ...
		    ' calibration\n''%s''?'],name),'Confirmation', ...
		    'No','Yes','No');
    if ~strcmp(resp,'Yes')
      return
    end
    cCalDB = get_calib_db;
    [sCalib,idx] = configdb.smap_get(cCalDB,name);
    if isempty(idx)
      return
    end
    if isfield(sCalib,'writable') && (~sCalib.writable)
      error(sprintf('Unable to remove ''%s'' (write protected)', ...
		    name));
    end
    cCalDB = configdb.smap_rm(cCalDB,name);
    set_calib_db(cCalDB);
    configdb.set_mhaconfig(mha_basic_cfg.mha,...
		   'calib_current',cCalDB{1,1});
  catch
    errordlg(lasterr);
    return
  end
  calib_upload;
  calib_gui;

function h = infomessage( fmt, varargin )
  if nargin < 1
    fmt = '';
  end
  msg = sprintf(fmt,varargin{:});
  h = uicontrol('Style','text','position',[20 70 572 380],...
		'HorizontalAlignment','left',...
		'Fontsize',14,...
		'BackgroundColor',col_bg,...
		'String',msg);


function calib_clone_and_edit( varargin )
  global mha_basic_cfg;
  fh = gcbf;
  sWCout = {};
  for k=1:mha_basic_cfg.nch.out
    sWCout{end+1} = ...
	struct('callback',@mhagui_calib_out_next,...
	       'draw',@mhagui_calib_out_draw,...
	       'callbackdata',k,...
	       'drawdata',k,...
	       'tag','cal_out',...
	       'title',sprintf('Calibrate output channel %d (%s)',...
			       k,mha_basic_cfg.names.out{k}));
  end
  sWCinSerial = {};
  for k=1:mha_basic_cfg.nch.in
    sWCinSerial{end+1} = ...
	struct('callback',@mhagui_calib_in_next,...
	       'draw',@mhagui_calib_in_draw,...
	       'callbackdata',struct('channel',k,'insert',0),...
	       'drawdata',k,...
	       'tag','cal_in_serial',...
	       'title',sprintf('Calibrate input channel %d (%s)',...
			       k,mha_basic_cfg.names.in{k}));
    sWCinSerial{end+1} = ...
	struct('callback',@mhagui_calib_in_next,...
	       'draw',@mhagui_calib_in_draw,...
	       'callbackdata',struct('channel',k,'insert',1),...
	       'drawdata',k,...
	       'tag','cal_in_serial_insert',...
	       'title',sprintf('Calibrate input channel %d (%s)',...
			       k,mha_basic_cfg.names.in{k}));
  end
  sWCinPar = {};
  sWCinPar{end+1} = struct('callback',@mhagui_calib_in_next,...
		    'draw',@mhagui_calib_in_draw,...
		    'callbackdata',struct('channel',1:mha_basic_cfg.nch.in,'insert',0),...
		    'drawdata',1:mha_basic_cfg.nch.in,...
		    'tag','cal_in_parallel',...
		    'title',sprintf('Calibrate all %d input channels',...
				    mha_basic_cfg.nch.in));
  sWCinPar{end+1} = struct('callback',@mhagui_calib_in_next,...
		    'draw',@mhagui_calib_in_draw,...
		    'callbackdata',struct('channel',1:mha_basic_cfg.nch.in,'insert',1),...
		    'drawdata',1:mha_basic_cfg.nch.in,...
		    'tag','cal_in_parallel_insert',...
		    'title',sprintf('Calibrate all %d input channels',...
				    mha_basic_cfg.nch.in));
  sWC = {struct('callback',@mhagui_calib_start_next,...
		'draw',@mhagui_calib_start_draw,...
		'title','Calibration wizard'),...
	 sWCout{:},...
	 sWCinSerial{:},...
	 sWCinPar{:},....
	 struct('callback',@mhagui_calib_finish_next,...
		'draw',@mhagui_calib_finish_draw,...
		'title','Upload and save calibration'),...
	};
  mhagui_wizard_framework(sWC,fh);
  if ishandle(fh)
    calib_gui;
  end

function mhagui_calib_finish_draw
  global mha_basic_cfg;
  libconfigdb();
  h = infomessage(['Review the calibration values, then press finish to ',...
		   'save and upload your calibration.']);
  pos = get(h,'Position');
  pos = pos + [0 300 0 -300];
  set(h,'Position',pos);
  sCalib = configdb.get_mhaconfig(mha_basic_cfg.mha,'calib_editcal');
  draw_calib_data( sCalib, [20 210] );
  wpos = get(gcf,'Position');
  wpos = wpos(3)-40;
  sValIn = mhagui_num2str( sCalib.cfg.calib_in.peaklevel );
  sValOut = mhagui_num2str( sCalib.cfg.calib_out.peaklevel );
  uicontrol('Position',[20,165,wpos,20],...
	    'Style','text',...
	    'HorizontalAlignment','left',...
	    'String','Input calibration:');
  uicontrol('Position',[20,140,wpos,25],'BackgroundColor',ones(1,3),...
	    'Style','edit','tag','mhagui_calib_review_calib_in',...
	    'HorizontalAlignment','left',...
	    'String',sValIn);
  uicontrol('Position',[20,110,wpos,20],...
	    'Style','text',...
	    'HorizontalAlignment','left',...
	    'String','Output calibration:');
  uicontrol('Position',[20,85,wpos,25],'BackgroundColor',ones(1,3),...
	    'Style','edit','tag','mhagui_calib_review_calib_out',...
	    'HorizontalAlignment','left',...
	    'String',sValOut);
  drawnow;
  
function sVal = mhagui_num2str( val )
  sVal = sprintf('%g ',val);
  sVal(end) = '';
  sVal = ['[',sVal,']'];

function mhagui_calib_finish_next
  global mha_basic_cfg;
  libconfigdb();
  sCalib = configdb.get_mhaconfig(mha_basic_cfg.mha,'calib_editcal');
  hEdIn = findobj('tag','mhagui_calib_review_calib_in');
  hEdOut = findobj('tag','mhagui_calib_review_calib_out');
  peak_in = str2num(get(hEdIn,'String'));
  peak_out = str2num(get(hEdOut,'String'));
  if ~isequal(size(peak_in),size(sCalib.cfg.calib_in.peaklevel))
    error('Invalid data in input calibration edit field (mismatching size)');
  end
  if ~isequal(size(peak_out),size(sCalib.cfg.calib_out.peaklevel))
    error('Invalid data in output calibration edit field (mismatching size)');
  end
  sCalib.cfg.calib_in.peaklevel = peak_in;
  sCalib.cfg.calib_out.peaklevel = peak_out;
  configdb.set_mhaconfig(mha_basic_cfg.mha,'calib_editcal', sCalib);
  calib_add_calib( sCalib );
  calib_upload;

function mhagui_calib_start_draw
  global mha_basic_cfg;
  sCalib = calib_current_calib;
  set(infomessage(['You are aboute to re-calibrate the MHA ''%s'' on' ...
		   ' %s, based on the calibration ''%s''.\n',...
		   'Please enter a new calibration ID and select ',...
		   'the input calibration mode.'],...
		  mha_basic_cfg.instance,...
		  mha_basic_cfg.mha.host,sCalib.id),...
      'Fontsize',12);
  wpos = get(gcf,'Position');
  uicontrol('Style','text','position',[20 245 wpos(3)-40 18],...
	    'String','Calibration ID:',...
	    'HorizontalAlign','left',...
	    'Fontweight','bold');
  uicontrol('Style','edit','position',[20 210 wpos(3)-40 35],...
	    'Tag','calib_wizard_edit_newname',...
	    'BackgroundColor',[1 1 1],...
	    'Fontsize',14,...
	    'HorizontalAlignment','left',...
	    'String',[datestr(now,'yyyy-mm-dd'),' ',sCalib.id]);
  uicontrol('Style','text','position',[20 185 wpos(3)-40 18],...
	    'String','input calibration mode:',...
	    'HorizontalAlign','left',...
	    'Fontweight','bold');
  uicontrol('Style','listbox','position',[20 125 180 60],...
	    'Tag','calib_input_cal_mode',...
	    'BackgroundColor',[1 1 1],...
	    'Callback',@set_input_cal_mode_msg,...
	    'Fontsize',14,...
	    'String',{'serial','parallel'});
  uicontrol('Style','text','position',[210 125 wpos(3)-230 60],...
	    'Tag','calib_input_cal_mode_msg',...
	    'HorizontalAlignment','left',...
	    'Fontsize',12,...
	    'String','');
  uicontrol('Style','checkbox','position',[20 90 round(wpos(3)/2)-40 30],...
	    'Tag','calib_skip_output',...
	    'Value',1,...
	    'HorizontalAlignment','left',...
	    'Callback',@set_input_cal_mode_msg,...
	    'Fontsize',14,...
	    'String','  Output calibration');
  uicontrol('Style','checkbox','position',[20 60 round(wpos(3)/2)-40 30],...
	    'Tag','calib_skip_input',...
	    'Value',1,...
	    'HorizontalAlignment','left',...
	    'Callback',@set_input_cal_mode_msg,...
	    'Fontsize',14,...
	    'String','  Input calibration');
  uicontrol('Style','checkbox','position',[round(wpos(3)/2) 90 round(wpos(3)/2)-40 30],...
	    'Tag','calib_copy_input_to_output',...
	    'Value',0,...
	    'Enable','off',...
	    'HorizontalAlignment','left',...
	    'Callback',@set_input_cal_mode_msg,...
	    'Fontsize',14,...
	    'String','  Insert calibration');
  set_input_cal_mode_msg;
  mha_set(mha_basic_cfg.mha,[mha_basic_cfg.base.transducers,'.calib_out.speechnoise.mode'],'off');
  mha_set(mha_basic_cfg.mha,[mha_basic_cfg.base.transducers,'.calib_in.speechnoise.mode'],'off');

function set_input_cal_mode_msg( varargin )
  global mha_basic_cfg;
  b_skin = 1-get(findobj('Tag','calib_skip_input'),'Value');
  b_skout = 1-get(findobj('Tag','calib_skip_output'),'Value');
  hsel = findobj('Tag','calib_input_cal_mode');
  hinsert = findobj('Tag','calib_copy_input_to_output');
  hmsg = findobj('Tag','calib_input_cal_mode_msg');
  b_insert = get(hinsert,'Value') && b_skout && (mha_basic_cfg.nch.in==mha_basic_cfg.nch.out);
  if b_insert
    sInsertMsg = ['Input calibration values will be copied to output' ...
		  ' calibration values.'];
  else
    sInsertMsg = '';
  end
  val = get(hsel,'Value');
  if val==1
    sInMsg = 'Input channels will be calibrated one after each other. ';
  else
    sInMsg = ['All inputs will be calibrated at once, requiring that' ...
	      ' the input level is the same at all channels. '];
  end
  if b_skin
    sInMsg = '';
    set(hsel,'Enable','off');
  else
    set(hsel,'Enable','on');
  end
  if b_skout  && (mha_basic_cfg.nch.in==mha_basic_cfg.nch.out)
    set(hinsert,'Enable','on');
  else
    set(hinsert,'Enable','off');
  end
  sOutMsg = 'Output channels will be calibrated. ';
  if b_skout
    sOutMsg = '';
  end
  msg = [sOutMsg,sInMsg,sInsertMsg];
  if isempty(msg)
    msg = 'Nothing will be done.';
  end
  set(hmsg,'String',msg);

function mhagui_calib_start_next
  global mha_basic_cfg;
  libconfigdb();
  sCalib = calib_current_calib;
  uih_mode = findobj('tag','calib_input_cal_mode');
  uih_name = findobj('tag','calib_wizard_edit_newname');
  kMode = get(uih_mode,'Value');
  if kMode == 1
    mhagui_wizard_skip_page('cal_in_parallel');
    mhagui_wizard_skip_page('cal_in_parallel_insert');
  else
    mhagui_wizard_skip_page('cal_in_serial');
    mhagui_wizard_skip_page('cal_in_serial_insert');
  end
  sCalibID = get(uih_name,'String');
  sCalib.id = sCalibID;
  b_skin = 1-get(findobj('Tag','calib_skip_input'),'Value');
  b_skout = 1-get(findobj('Tag','calib_skip_output'),'Value');
  hinsert = findobj('Tag','calib_copy_input_to_output');
  b_insert = get(hinsert,'Value') && b_skout && (mha_basic_cfg.nch.in==mha_basic_cfg.nch.out);
  if b_skin
    mhagui_wizard_skip_page('cal_in_parallel');
    mhagui_wizard_skip_page('cal_in_serial');
    mhagui_wizard_skip_page('cal_in_parallel_insert');
    mhagui_wizard_skip_page('cal_in_serial_insert');
  end
  if b_insert
    mhagui_wizard_skip_page('cal_in_parallel');
    mhagui_wizard_skip_page('cal_in_serial');
  else
    mhagui_wizard_skip_page('cal_in_parallel_insert');
    mhagui_wizard_skip_page('cal_in_serial_insert');
  end
  if b_skout
    mhagui_wizard_skip_page('cal_out');
  end
  %
  % verify calibration name:
  %
  if isempty(sCalibID)
    error('Please enter a valid name!');
  end
  cCalibDB = get_calib_db;
  [sCalibExist,idx] = configdb.smap_get(cCalibDB,sCalib.id);
  if ~isempty(idx)
    if isfield(sCalibExist,'writable') && (~sCalibExist.writable)
      error(sprintf(['Please enter an other calibration ID, ' ...
		     '''%s'' already exists in list'],sCalib.id));
    end
    resp = questdlg(sprintf(['Replace calibration\n''%s''?'],name),...
		    'Confirmation','No','Yes','No');
    if ~strcmp(resp,'Yes')
      error('not overwriting existing calibration');
    end
  end
  % end verification
  sCalib.writable = 1;
  configdb.set_mhaconfig(mha_basic_cfg.mha,'calib_editcal',sCalib);
  calib_upload(sCalib);
  
  
function [cc,ctype] = get_coupler_corr
  global mha_basic_cfg;
  libconfigdb();
  cc = configdb.get_mhaconfig(mha_basic_cfg.mha,'coupler_correction',0);
  ctype = configdb.get_mhaconfig(mha_basic_cfg.mha,'coupler_type','');

function spnoise_out_changelevel(dl)
  global mha_basic_cfg;
  l = mha_get(mha_basic_cfg.mha,[mha_basic_cfg.base.transducers, ...
		    '.calib_out.speechnoise.level']);
  l = l+dl;
  try
    mha_set(mha_basic_cfg.mha,[mha_basic_cfg.base.transducers, ...
		    '.calib_out.speechnoise.level'],l);
  catch
  end
  
function spnoise_out_changelevel_p16(varargin)
  spnoise_out_changelevel(16);
function spnoise_out_changelevel_m16(varargin)
  spnoise_out_changelevel(-16);
function spnoise_out_changelevel_p4(varargin)
  spnoise_out_changelevel(4);
function spnoise_out_changelevel_m4(varargin)
  spnoise_out_changelevel(-4);
function spnoise_out_changelevel_p1(varargin)
  spnoise_out_changelevel(1);
function spnoise_out_changelevel_m1(varargin)
  spnoise_out_changelevel(-1);
function spnoise_out_changelevel_p_25(varargin)
  spnoise_out_changelevel(0.25);
function spnoise_out_changelevel_m_25(varargin)
  spnoise_out_changelevel(-0.25);

function calib_out_select_response_for_channel( varargin )
  global mha_basic_cfg;
  try
    hb = findobj('tag','mhagui_calib_out_response_list');
    str = get(hb,'String');
    channel = get(hb,'UserData');
    response_id = str{get(hb,'Value')};
    [irs,fs] = mha_get_response(mha_basic_cfg.mha,response_id);
    irs = irs';
    old_irs = mha_get(mha_basic_cfg.mha, ...
		      [mha_basic_cfg.base.transducers, ...
		       '.calib_out.fir']);
    old_irs(channel,1:size(irs,2)) = irs;
    if size(irs,2) < size(old_irs,2)
      old_irs(channel,size(irs,2)+1:end) = 0;
      if size(irs,2) == 0
	old_irs(channel,1) = 1;
      end
    end
    if size(old_irs,1) < mha_basic_cfg.nch.out
      old_irs(size(old_irs,1)+1:mha_basic_cfg.nch.out,:) = 0;
    end
    if size(old_irs,1) > mha_basic_cfg.nch.out
      old_irs(mha_basic_cfg.nch.out+1:end,:) = [];
    end
    if ~any(old_irs(:))
      old_irs = [];
    end
    mha_set(mha_basic_cfg.mha, ...
	    [mha_basic_cfg.base.transducers, ...
	     '.calib_out.fir'],old_irs);
  catch
    disp_err_rethrow;
  end
  

function mhagui_calib_out_draw( channel )
  global mha_basic_cfg;
  libconfigdb();
  h = findobj('tag','mhagui_wizard_title');
  col = (mha_basic_cfg.side.out(channel)==1)*[0.6 0.6 1] + ...
	(mha_basic_cfg.side.out(channel)==2)*[1 0.6 0.6] + ...
	(mha_basic_cfg.side.out(channel)==0)*[0.8 0.8 0.8];
  set(h,'BackgroundColor',col);
  ref_lev = 80;
  [cc,ctype] = get_coupler_corr;
  infomessage(['Please select the receiver equalization, then adjust the' ...
	       ' output level until your external level meter shows %g' ...
	       ' dB SPL RMS free field or %g dB SPL RMS coupler level %s.'],...
	      ref_lev,ref_lev-cc,ctype);
  %
  % Receiver response selection:
  %
  wpos = get(gcf,'Position');
  out_resp = ...
      configdb.get_mhaconfig(mha_basic_cfg.mha,'calib_editcal.output.responses',{});
  if length(out_resp) >= channel
    out_resp = out_resp{channel};
    if isempty(out_resp)
      out_resp = '';
    end
  else
    out_resp = '';
  end
  csResps = mha_get_response_db(mha_basic_cfg.mha,'list');
  idx = strmatch(out_resp,csResps);
  if isempty(idx)
    idx = 1;
  end
  uicontrol('Style','Listbox',...
	    'String',csResps,...
	    'UserData',channel,....
	    'BackgroundColor',[1 1 1],...
	    'Tag','mhagui_calib_out_response_list',...
	    'Callback',@calib_out_select_response_for_channel,...
	    'FontSize',14,...
	    'UserData',channel,...
	    'value',idx(1),...
	    'Position',[20 165 floor(wpos(3)/2)-40 200]);
  %
  % level control:
  %
  p = [20 90];
  p(3:4) = 0;
  vh = [];
  vh(end+1) = ...
      uicontrol('Style','Pushbutton','String','-16 dB',...
	    'callback',@spnoise_out_changelevel_m16);
  vh(end+1) = ...
  uicontrol('Style','Pushbutton','String','-4 dB',...
	    'callback',@spnoise_out_changelevel_m4);
  vh(end+1) = ...
  uicontrol('Style','Pushbutton','String','-1 dB',...
	    'callback',@spnoise_out_changelevel_m1);
  vh(end+1) = ...
  uicontrol('Style','Pushbutton','String','-0.25 dB',...
	    'callback',@spnoise_out_changelevel_m_25);
  vh(end+1) = ...
  uicontrol('Style','Pushbutton','String','+0.25 dB',...
	    'callback',@spnoise_out_changelevel_p_25);
  vh(end+1) = ...
  uicontrol('Style','Pushbutton','String','+1 dB',...
	    'callback',@spnoise_out_changelevel_p1);
  vh(end+1) = ...
  uicontrol('Style','Pushbutton','String','+4 dB',...
	    'callback',@spnoise_out_changelevel_p4);
  vh(end+1) = ...
  uicontrol('Style','Pushbutton','String','+16 dB',...
	    'callback',@spnoise_out_changelevel_p16);
  dx = 5;
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
  drawnow;
  calib_out_select_response_for_channel;
  cfg = struct;
  cfg.level = 80;
  cfg.channels = channel-1;
  cfg.mode = 'on';
  mha_set(mha_basic_cfg.mha,[mha_basic_cfg.base.transducers, ...
		    '.calib_out.speechnoise'],cfg);
  spn_var = [mha_basic_cfg.base.transducers,...
	     '.calib_out.speechnoise.mode'];
  hspn  = mhagui_keyword_listbox(mha_basic_cfg.mha,...
			 spn_var,...
			 [floor(wpos(3)/2)+20,165,floor(wpos(3)/2)-40]);
  set(hspn,...
	   'Position',[floor(wpos(3)/2)+20,165,floor(wpos(3)/2)-40, ...
		    200],...
	   'BackgroundColor',[1 1 1],...
	   'FontSize',14);
  delete(findobj(gcf,'tag',['label:',spn_var]));
  drawnow;
  
function mhagui_calib_out_next( channel )
  global mha_basic_cfg;
  libconfigdb();
  vPeakLevel = mha_get(mha_basic_cfg.mha, ...
		       [mha_basic_cfg.base.transducers,'.calib_out.peaklevel']);
  if length(vPeakLevel) == 1
    vPeakLevel = repmat(vPeakLevel,[1 ...
		    mha_basic_cfg.nch.out]);
  end
  lev = mha_get(mha_basic_cfg.mha, ...
		[mha_basic_cfg.base.transducers,'.calib_out.speechnoise.level']);
  plev = vPeakLevel(channel)+80-lev;
  hb = findobj('tag','mhagui_calib_out_response_list');
  str = get(hb,'String');
  response_id = str{get(hb,'Value')};
  out_resp = ...
      configdb.get_mhaconfig(mha_basic_cfg.mha,'calib_editcal.output.responses',{});
  out_resp{channel} = response_id;
  configdb.set_mhaconfig(mha_basic_cfg.mha,'calib_editcal.output.responses',out_resp);
  peaklevel = configdb.get_mhaconfig(mha_basic_cfg.mha,'calib_editcal.cfg.calib_out.peaklevel');
  peaklevel(channel) = plev;
  configdb.set_mhaconfig(mha_basic_cfg.mha,'calib_editcal.cfg.calib_out.peaklevel',peaklevel);
  irs = mha_get(mha_basic_cfg.mha, ...
		[mha_basic_cfg.base.transducers, ...
		 '.calib_out.fir']);
  fs = mha_get(mha_basic_cfg.mha, ...
	       [mha_basic_cfg.base.transducers, ...
		'.calib_out.config.srate']);
  configdb.set_mhaconfig(mha_basic_cfg.mha,'calib_editcal.cfg.calib_out.fir',irs);
  configdb.set_mhaconfig(mha_basic_cfg.mha,'calib_editcal.fs',fs);
  cfg = struct;
  cfg.mode = 'off';
  cfg.channels = [];
  mha_set(mha_basic_cfg.mha,[mha_basic_cfg.base.transducers,'.calib_out.speechnoise'],cfg);
  mha_set(mha_basic_cfg.mha, ...
	  [mha_basic_cfg.base.transducers,'.calib_out.peaklevel'],peaklevel);
  
function mhagui_calib_in_draw( channel )
  global mha_basic_cfg;
  libconfigdb();
  peaklevel = configdb.get_mhaconfig(mha_basic_cfg.mha, ...
			     'calib_editcal.cfg.calib_out.peaklevel',[]);
  if ~isempty(peaklevel)
    mha_set(mha_basic_cfg.mha, ...
	    [mha_basic_cfg.base.transducers,'.calib_out.peaklevel'], ...
	    peaklevel);
  end
  if length(channel)==1
    h = findobj('tag','mhagui_wizard_title');
    col = (mha_basic_cfg.side.in(channel)==1)*[0.6 0.6 1] + ...
	  (mha_basic_cfg.side.in(channel)==2)*[1 0.6 0.6] + ...
	  (mha_basic_cfg.side.in(channel)==0)*[0.8 0.8 0.8];
    set(h,'BackgroundColor',col);
  end
  infomessage(['Please play back a speech shaped stationary noise' ...
	       ' signal through a free field loud speaker. Please' ...
	       ' ensure that the RMS level at the microphone position' ...
	       ' is 80 dB SPL unweighted.']);
  wpos = get(gcf,'Position');
  wpos = wpos(3)-40;
  wx = round((wpos-20)/2);
  uicontrol('Style','checkbox','position',[20 220 wx 30],...
	    'Tag','calib_use_output_channel',...
	    'Callback',@use_output_channel_select,...
	    'Fontsize',14,...
	    'Value',configdb.get_mhaconfig(mha_basic_cfg.mha,'last_selection.use_mha_for_calib_out',0),...
	    'String','  Use MHA output for playback.');
  uicontrol('Style','listbox','position',[40+wx 120 wx 130],...
	    'Tag','calib_use_output_channel_select',...
	    'Callback',@use_output_channel_select,...
	    'Fontsize',14,...
	    'String',mha_basic_cfg.names.out,...
	    'Value',min(mha_basic_cfg.nch.out,...
			configdb.get_mhaconfig(mha_basic_cfg.mha,'last_selection.mha_channel_for_calib_out',1)));
  use_output_channel_select;

function use_output_channel_select(varargin)
  global mha_basic_cfg;
  libconfigdb();
  b_use = get(findobj('tag','calib_use_output_channel'),'Value');
  h_sel = findobj('tag','calib_use_output_channel_select');
  ch = get(h_sel,'Value');
  configdb.set_mhaconfig(mha_basic_cfg.mha,'last_selection.use_mha_for_calib_out',b_use);
  configdb.set_mhaconfig(mha_basic_cfg.mha,'last_selection.mha_channel_for_calib_out',ch)
  cfg = struct;
  if b_use
    cfg.mode = 'on';
    set(h_sel,'Enable','on');
  else
    cfg.mode = 'off';
    set(h_sel,'Enable','off');
  end
  cfg.level = 80;
  cfg.channels = ch-1;
  mha_set(mha_basic_cfg.mha,[mha_basic_cfg.base.transducers,'.calib_out.speechnoise'],cfg);
  
  
function mhagui_calib_in_next( sData )
  global mha_basic_cfg;
  libconfigdb();
  sTauName = [mha_basic_cfg.base.transducers,'.calib_in.tau_level'];
  tc = mha_get(mha_basic_cfg.mha,sTauName);
  mha_set(mha_basic_cfg.mha,sTauName,2);
  pause(2);
  vLevel = mha_get(mha_basic_cfg.mha,[mha_basic_cfg.base.transducers,'.calib_in.rmslevel']);
  mha_set(mha_basic_cfg.mha,sTauName,tc);
  vPeakLevel = mha_get(mha_basic_cfg.mha,...
		       [mha_basic_cfg.base.transducers, ...
		    '.calib_in.peaklevel']);
  peaklevel = configdb.get_mhaconfig(mha_basic_cfg.mha,'calib_editcal.cfg.calib_in.peaklevel',vPeakLevel);
  peaklevel(sData.channel) = vPeakLevel(sData.channel)-vLevel(sData.channel)+80;
  configdb.set_mhaconfig(mha_basic_cfg.mha,'calib_editcal.cfg.calib_in.peaklevel',peaklevel);
  hinsert = findobj('Tag','calib_copy_input_to_output');
  b_skout = 1-get(findobj('Tag','calib_skip_output'),'Value');
  if sData.insert
    configdb.set_mhaconfig(mha_basic_cfg.mha,'calib_editcal.cfg.calib_out.peaklevel',peaklevel);
  end
  cfg = struct;
  cfg.mode = 'off';
  cfg.channels = [];
  mha_set(mha_basic_cfg.mha,[mha_basic_cfg.base.transducers,'.calib_out.speechnoise'],cfg);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% End of wizardry.
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
    
function calib_select_from_list(varargin)
  global mha_basic_cfg;
  libconfigdb();
  h = findobj('Tag','mhagui_calib_listbox');
  if prod(size(h)) ~= 1
    error('invalid gui handle');
  end
  v = get(h,'Value');
  csCalIDs = get(h,'String');
  configdb.set_mhaconfig(mha_basic_cfg.mha,'calib_current',csCalIDs{v});
  calib_upload;
  
function sCalib = calib_current_calib
  global mha_basic_cfg;
  libconfigdb();
  cCalibDB = get_calib_db;
  sCalID = configdb.get_mhaconfig(mha_basic_cfg.mha,'calib_current',cCalibDB{1,1});
  sCalib = calib_get_calib( sCalID );
  configdb.set_mhaconfig(mha_basic_cfg.mha,'calib_current',sCalib.id);
  
function calib_upload( sCalib )
  global mha_basic_cfg;
  if nargin < 1
    sCalib = calib_current_calib;
  end
  if isempty(sCalib.cfg.calib_in.peaklevel) || ...
	isempty(sCalib.cfg.calib_out.peaklevel)
    sCalib = calib_default;
  end
  if length(sCalib.cfg.calib_in.peaklevel) ~= ...
	mha_basic_cfg.nch.in
    sCalib.cfg.calib_in.peaklevel = ...
	repmat(sCalib.cfg.calib_in.peaklevel(1),[1,mha_basic_cfg.nch.in]);
  end
  if length(sCalib.cfg.calib_out.peaklevel) ~= ...
	mha_basic_cfg.nch.out
    sCalib.cfg.calib_out.peaklevel = ...
	repmat(sCalib.cfg.calib_out.peaklevel(1),[1,mha_basic_cfg.nch.out]);
  end
  cfg_spnoise_off = struct;
  cfg_spnoise_off.calib_in.speechnoise.mode = 'off';
  cfg_spnoise_off.calib_in.speechnoise.channels = [];
  cfg_spnoise_off.calib_out.speechnoise.mode = 'off';
  cfg_spnoise_off.calib_out.speechnoise.channels = [];
  mha_set(mha_basic_cfg.mha,mha_basic_cfg.base.transducers,cfg_spnoise_off);
  mha_set(mha_basic_cfg.mha,mha_basic_cfg.base.transducers,sCalib.cfg);
  set(findobj('Tag',[mha_basic_cfg.base.transducers,'::current_calib']),'String', ...
		    sCalib.id);
  if ~isempty(findobj('tag','mhagui_calib_window'))
    update_current_calib_gui(sCalib);
  end
  
function update_current_calib_gui( sCalib )
  global mha_basic_cfg;
  if nargin < 1
    sCalib = calib_current_calib;
  end
  for k=1:mha_basic_cfg.nch.in
    uih = findobj('Tag',sprintf('mhagui_calib_peaklevel_in_text_%d',k));
    val = sCalib.cfg.calib_in.peaklevel(k);
    set(uih,'String',sprintf('%1.1f',val));
  end
  for k=1:mha_basic_cfg.nch.out
    uih = findobj('Tag',sprintf('mhagui_calib_peaklevel_out_text_%d',k));
    val = sCalib.cfg.calib_out.peaklevel(k);
    set(uih,'String',sprintf('%1.1f',val));
    ph = findobj('tag',sprintf('mhagui_calib_recresp_pl%d',k));
    if ~isempty(ph)
      irs = sCalib.cfg.calib_out.fir';
      if isfield(sCalib,'fs')
	fs = sCalib.fs;
      else
	fs = 44100;
      end
      if isempty(irs)
	irs = 1;
      end
      if size(irs,2) == 1
	irs = repmat(irs,[1,mha_basic_cfg.nch.out]);
      end
      H = 20*log10(abs(realfft(zeropad(irs(:,k),round(fs)))));
      f = [1:length(H)]'-1;
      set(ph,'XData',f,'YData',H);
    end
  end
  
function [cCalDB,csCalibs] = get_calib_db
  global mha_basic_cfg;
  libconfigdb();
  cCalDB = configdb.get_mhaconfig(mha_basic_cfg.mha,'calib_db',cell([2, ...
		    0]));
  cal = calib_default;
  cCalDB = configdb.smap_set(cCalDB,cal.id,cal);
  csCalibs = cCalDB(1,:);
  
function set_calib_db( cCalDB )
  global mha_basic_cfg;
  libconfigdb();
  configdb.set_mhaconfig(mha_basic_cfg.mha,'calib_db',cCalDB);
  
function sCalib = calib_get_calib( sCalID )
  libconfigdb();
  cCalDB = get_calib_db;
  [sCalib,idx] = configdb.smap_get( cCalDB, sCalID );
  if isempty(idx)
    sCalib = calib_default;
  end
  
function calib_add_calib( sCalib )
  global mha_basic_cfg;
  libconfigdb();
  cCalDB = get_calib_db;
  [tmp,idx] = configdb.smap_get( cCalDB, sCalib.id );
  if ~isempty(idx)
    error(sprintf('A calibration of name ''%s'' already exists',sCalib.id));
  end
  cCalDB = configdb.smap_set( cCalDB, sCalib.id, sCalib );
  set_calib_db(cCalDB);
  configdb.set_mhaconfig(mha_basic_cfg.mha,'calib_current',sCalib.id);
  calib_upload;
 
function s = calib_default
  global mha_basic_cfg;
  if nargin < 3
    pl_in = 120;
  end
  if nargin < 4
    pl_out = 137.2;
  end
  if length(pl_in) ~= mha_basic_cfg.nch.in
    pl_in = repmat(pl_in(1),[1 mha_basic_cfg.nch.in]);
  end
  if length(pl_out) ~= mha_basic_cfg.nch.out
    pl_out = repmat(pl_out(1),[1 mha_basic_cfg.nch.out]);
  end
  s = struct;
  s.id = 'default (flat, 0 dB FS = 120 dB SPL)';
  s.cfg = struct;
  s.cfg.calib_in.peaklevel = repmat(120,[1,mha_basic_cfg.nch.in]);
  s.cfg.calib_in.fir = [];
  s.cfg.calib_out.peaklevel = repmat(120,[1,mha_basic_cfg.nch.out]);
  s.cfg.calib_out.fir = [];
  s.output.responses = {};
  s.writable = 0;

function c = col_bg
  c = 0.7*ones(1,3);