function fh = mhagui_calibration_dialog( mode, mha )
% mhagui_calibration_dialog - calibration interface for 'transducers' plugin
%
% Create calibration wizard GUI:
% mhagui_calibration_dialog( 'gui', mha )
%
% Upload calibration data:
% mhagui_calibration_dialog( 'upload', mha )
%
% If mha network handle is not provided, the default handle
% (localhost at port 33337) is assumed. The default operation is to
% open the GUI.
  
  if nargin < 1
    mode = 'gui';
  end
  if nargin < 2
    mha = struct('host','localhost','port',33337);
  end
  % analyse MHA connection and create global handle variable:
  mha_get_basic_cfg_network( mha );
  switch mode
    case 'upload'
      calib_upload;
      fh = [];
    case 'gui'
      fh = calib_gui;
    otherwise
      error(sprintf('Invalid calibration mode ''%s''.',mode));
  end

function fh = calib_gui()
  global mha_basic_cfg;
  libconfigdb();
  libmhagui();
  % re-use figure if possible:
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
  % initialize current calibration:
  calib_current_calib();
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
                'String',wordwrap(msg));
  if ~isempty(ver('Octave'))
    set(h,'VerticalAlignment','top');
  end


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

function mhagui_calib_finish_draw()
  global mha_basic_cfg;
  libconfigdb();
  h = infomessage(['Review the calibration values, then press finish to ',...
                   'save and upload your calibration.']);
  pos = get(h,'Position');
  pos = pos + [0 300 0 -300];
  set(h,'Position',pos);
  sCalib = configdb.get_mhaconfig(mha_basic_cfg.mha,'calib_editcal',calib_default);
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

function mhagui_calib_finish_next()
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

function mhagui_calib_start_draw()
  global mha_basic_cfg;
  sCalib = calib_current_calib();
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
            'HorizontalAlignment','left',...
            'Fontweight','bold');
  uicontrol('Style','edit','position',[20 210 wpos(3)-40 35],...
            'Tag','calib_wizard_edit_newname',...
            'BackgroundColor',[1 1 1],...
            'Fontsize',14,...
            'HorizontalAlignment','left',...
            'String',[datestr(now,'yyyy-mm-dd'),' ',sCalib.id]);
  uicontrol('Style','text','position',[20 185 wpos(3)-40 18],...
            'String','input calibration mode:',...
            'HorizontalAlignment','left',...
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

function mhagui_calib_start_next()
  global mha_basic_cfg;
  libconfigdb();
  sCalib = calib_current_calib();
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
    error(sprintf(['Please enter an other calibration ID, ' ...
                     '''%s'' already exists in list'],sCalib.id));
  end
  % end verification
  sCalib.writable = 1;
  configdb.set_mhaconfig(mha_basic_cfg.mha,'calib_editcal',sCalib);
  calib_upload(sCalib);


function [cc,ctype] = get_coupler_corr()
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

function sCalib = calib_current_calib()
  global mha_basic_cfg;
  libconfigdb();
  cCalibDB = get_calib_db();
  sCalID = configdb.get_mhaconfig(mha_basic_cfg.mha,'calib_current',cCalibDB{1,1});
  sCalib = calib_get_calib( sCalID );
%configdb.set_mhaconfig(mha_basic_cfg.mha,'calib_current',sCalib.id);

function calib_upload( sCalib )
  global mha_basic_cfg;
  if nargin < 1
    sCalib = calib_current_calib();
  end
  if ~isfield(sCalib,'cfg')
    % We have no calibration to upload. Do not alter existing settings.
    return;
  end
  if isempty(sCalib.cfg.calib_in.peaklevel) || ...
        isempty(sCalib.cfg.calib_out.peaklevel)
    return; % do not alter calibration settings if we have no calibration
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
    sCalib = calib_current_calib();
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
      irs_zeropadded = irs(:,k);
      irs_zeropadded(end+1:round(fs),:) = 0;
      irs_zeropadded(round(fs)+1:end,:) = [];
      H = 20*log10(abs(realfft(irs_zeropadded)));
      f = [1:length(H)]'-1;
      set(ph,'XData',f,'YData',H);
    end
  end

function [cCalDB,csCalibs] = get_calib_db()
  global mha_basic_cfg;
  libconfigdb();
  cCalDB = configdb.get_mhaconfig(mha_basic_cfg.mha,'calib_db',cell([2, ...
                      0]));
  %cal = struct('id','default'); %calib_default;
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

function s = calib_default()
  global mha_basic_cfg;
  s = struct;
  s.id = 'default (flat, 0 dB FS = 120 dB SPL)';
  s.cfg = struct;
  s.cfg.calib_in.peaklevel = repmat(120,[1,mha_basic_cfg.nch.in]);
  s.cfg.calib_in.fir = [];
  s.cfg.calib_out.peaklevel = repmat(120,[1,mha_basic_cfg.nch.out]);
  s.cfg.calib_out.fir = [];
  s.output.responses = {};
  s.writable = 0;

function c = col_bg()
  c = 0.7*ones(1,3);

function y = realfft( x )
% REALFFT - FFT transform of pure real data
%
% Usage: y = realfft( x )
%
% Returns positive frequencies of fft(x), assuming that x is pure
% real data. Each column of x is transformed separately.
  ;
  fftlen = size(x,1);

  y = fft(x);
  y = y([1:floor(fftlen/2)+1],:);


function mhagui_wizard_framework( csCfg, fh )
  if nargin < 2
    fh = figure;
  else
    figure(fh);
  end
  p = get(fh,'Position');
  delete(get(fh,'Children'));
  delete(findobj('Tag','mhagui_wizard_cancel'));
  delete(findobj('Tag','mhagui_wizard_next'));
  drawnow;
  uicontrol(fh,'Style','Pushbutton','String','Cancel',...
            'Position',[20 20 80 30],...
            'tag','mhagui_wizard_cancel',...
            'Callback',@mhagui_wizard_cancel);
  uicontrol(fh,'Style','Pushbutton','String','Start',...
            'Position',[p(3)-100 20 80 30],...
            'tag','mhagui_wizard_next',...
            'Callback',@mhagui_wizard_next);
  uicontrol(fh,'Style','Text','String','',...
            'Position',[20 p(4)-40 p(3)-40 30],...
            'tag','mhagui_wizard_title',...
            'FontSize',14,...
            'FontWeight','bold',...
            'BackgroundColor',[0.8 0.8 0.8],...
            'HorizontalAlignment','left');
  sCfg = struct;
  sCfg.cfg = csCfg;
  sCfg.current = 1;
  sCfg.skip = [];
  sPanel = sCfg.cfg{1};
  if isfield(sPanel,'draw')
    try
      if isfield(sPanel,'drawdata')
        feval(sPanel.draw,sPanel.drawdata);
      else
        feval(sPanel.draw);
      end
    catch
      disp_err
    end
  end
  if isfield(sPanel,'title')
    set(findobj('Tag','mhagui_wizard_title'),'String', ...
                      sPanel.title);
  else
    set(findobj('Tag','mhagui_wizard_title'),'String','');
  end      
  set(findobj('Tag','mhagui_wizard_next'),'UserData',sCfg);
  drawnow;
  uiwait(fh);

function mhagui_wizard_cancel(varargin)
  fh = gcbf;
  delete(get(fh,'Children'));
  uiresume(fh);

function mhagui_wizard_next(varargin)
  try
    fh = gcbf;
    sCfg = get(findobj('Tag','mhagui_wizard_next'),'UserData');
    set(findobj('Tag','mhagui_wizard_next'),'Enable','off');
    sPanel = sCfg.cfg{sCfg.current};
    % first, execute handler function
    if isfield(sPanel,'callback')
      try
        if isfield(sPanel,'callbackdata')
          feval(sPanel.callback,sPanel.callbackdata);
        else
          feval(sPanel.callback);
        end
      catch
        disp_err
        set(findobj('Tag','mhagui_wizard_next'),'Enable','on');
        return;
      end
    end
    sCfg = get(findobj('Tag','mhagui_wizard_next'),'UserData');
    if sCfg.current >= length(sCfg.cfg)
      delete(get(fh,'Children'));
      uiresume(fh);
      return;
    end
    % cleanup figure:
    vCh = get(fh,'Children');
    for k=1:length(vCh)
      if ~strncmp(get(vCh(k),'Tag'),'mhagui_wizard_',14)
        delete(vCh(k));
      end
    end
    % execute screen function of next step:
    sCfg.current = sCfg.current + 1;
    while any(sCfg.current==sCfg.skip)
      sCfg.current = sCfg.current + 1;
    end
    sCfg.current = min(sCfg.current,length(sCfg.cfg));
    % setup 'next'-button string:
    if sCfg.current == length(sCfg.cfg)
      set(findobj('Tag','mhagui_wizard_next'),'String','Finish');
    elseif sCfg.current ~= 1
      set(findobj('Tag','mhagui_wizard_next'),'String','Next > ');
    end
    if sCfg.current <= length(sCfg.cfg)
      sPanel = sCfg.cfg{sCfg.current};
      if isfield(sPanel,'title')
        set(findobj('Tag','mhagui_wizard_title'),...
            'BackgroundColor',[0.8 0.8 0.8],...
            'String', sPanel.title);
      else
        set(findobj('Tag','mhagui_wizard_title'),...
            'BackgroundColor',[0.8 0.8 0.8],...
            'String','');
      end      
      if isfield(sPanel,'draw')
        try
          if isfield(sPanel,'drawdata')
            feval(sPanel.draw,sPanel.drawdata);
          else
            feval(sPanel.draw);
          end
        catch
          disp_err;
        end
      end
    end
    set(findobj('Tag','mhagui_wizard_next'),'UserData',sCfg);
    set(findobj('Tag','mhagui_wizard_next'),'Enable','on');
    drawnow;
  catch
    disp_err_rethrow;
  end

function disp_err()
  err = lasterror;
  disp(err.message);
  for k=1:length(err.stack)
    disp(sprintf('%s:%d %s',...
                 err.stack(k).file,...
                 err.stack(k).line,...
                 err.stack(k).name));
  end
  uiwait(errordlg(err.message));

function mhagui_wizard_skip_page( vPage, bSkip )
  if nargin < 2
    bSkip = 1;
  end
  uih = findobj('Tag','mhagui_wizard_next');
  if isempty(uih)
    error('No UI control found');
  end
  sCfg = get(uih,'UserData');
  if ~isfield(sCfg,'skip')
    sCfg.skip = [];
  end
  if ischar(vPage)
    tag = vPage;
    vPage = [];
    for k=1:length(sCfg.cfg)
      cf = sCfg.cfg{k};
      if isfield(cf,'tag')
        if strcmp(cf.tag,tag)
          vPage(end+1) = k;
        end
      end
    end
  end
  if bSkip
    for page=vPage
      sCfg.skip(end+1) = page;
      sCfg.skip = unique(sCfg.skip);
    end
  else
    for page=vPage
      sCfg.skip(find(sCfg.skip==page)) = [];
    end
  end
  set(uih,'UserData',sCfg);

function cRDB = mha_get_response_db( mha, mode )
  cfdb = libconfigdb();
  cRDB = cfdb.get_mhaconfig( mha, 'response_db', cell([2,0]) );
  sResp = flat_response;
  cRDB = cfdb.smap_set( cRDB, sResp.id, sResp );
  sResp = hp6db_response;
  cRDB = cfdb.smap_set( cRDB, sResp.id, sResp );
  sResp = hda200_response;
  cRDB = cfdb.smap_set( cRDB, sResp.id, sResp );
  if (nargin > 1) && ischar(mode) &&  strcmp(mode,'list')
    cRDB = cRDB(1,:);
  end

function s = flat_response()
  s = struct;
  s.f = [10 20000];
  s.g = [0 0];
  s.id = 'flat';

function s = hp6db_response()
  s = struct;
  s.f = [125 250 250*sqrt(2) 500 600 1000 2000 10000];
  s.g = [-12 -6 -3 0 0 0 0 0];
  s.id = '6dB/oct high pass, 500 Hz';

function s = hda200_response()
  s = struct;
  s.f = ([1:769]-1)/768*22050;
  s.g = [0.1306 0.1308 0.1311 0.1315 0.1320 0.1326 0.1331 0.1336 ...
         0.1341 0.1344 0.1347 0.1349 0.1352 0.1356 0.1362 0.1369 ...
         0.1382 0.1398 0.1417 0.1443 0.1473 0.1507 0.1548 0.1593 ...
         0.1642 0.1696 0.1753 0.1813 0.1876 0.1939 0.2004 0.2069 ...
         0.2134 0.2200 0.2266 0.2333 0.2403 0.2481 0.2566 0.2658 ...
         0.2770 0.2896 0.3034 0.3201 0.3381 0.3576 0.3793 0.4016 ...
         0.4246 0.4477 0.4701 0.4919 0.5112 0.5287 0.5445 0.5562 ...
         0.5659 0.5735 0.5772 0.5792 0.5797 0.5776 0.5747 0.5709 ...
         0.5659 0.5606 0.5549 0.5486 0.5422 0.5355 0.5284 0.5210 ...
         0.5134 0.5052 0.4969 0.4884 0.4796 0.4709 0.4622 0.4537 ...
         0.4456 0.4378 0.4308 0.4244 0.4185 0.4138 0.4098 0.4064 ...
         0.4043 0.4027 0.4018 0.4018 0.4022 0.4030 0.4043 0.4058 ...
         0.4074 0.4092 0.4109 0.4128 0.4146 0.4165 0.4184 0.4206 ...
         0.4229 0.4253 0.4283 0.4316 0.4351 0.4394 0.4440 0.4489 ...
         0.4545 0.4604 0.4667 0.4734 0.4803 0.4875 0.4949 0.5024 ...
         0.5101 0.5178 0.5256 0.5333 0.5411 0.5489 0.5566 0.5643 ...
         0.5720 0.5796 0.5871 0.5946 0.6020 0.6093 0.6166 0.6239 ...
         0.6311 0.6383 0.6455 0.6528 0.6602 0.6676 0.6752 0.6829 ...
         0.6908 0.6989 0.7072 0.7155 0.7241 0.7327 0.7414 0.7501 ...
         0.7589 0.7677 0.7764 0.7852 0.7939 0.8027 0.8115 0.8202 ...
         0.8290 0.8378 0.8465 0.8552 0.8638 0.8722 0.8804 0.8882 ...
         0.8959 0.9028 0.9094 0.9157 0.9211 0.9260 0.9306 0.9343 ...
         0.9377 0.9408 0.9432 0.9455 0.9475 0.9493 0.9509 0.9525 ...
         0.9540 0.9556 0.9571 0.9586 0.9601 0.9617 0.9632 0.9648 ...
         0.9664 0.9679 0.9694 0.9710 0.9725 0.9740 0.9755 0.9771 ...
         0.9786 0.9802 0.9817 0.9833 0.9849 0.9864 0.9880 0.9895 ...
         0.9910 0.9925 0.9939 0.9953 0.9965 0.9976 0.9985 0.9992 ...
         0.9998 0.9999 0.9999 0.9996 0.9988 0.9979 0.9968 0.9952 ...
         0.9936 0.9918 0.9898 0.9878 0.9857 0.9835 0.9813 0.9791 ...
         0.9769 0.9747 0.9724 0.9702 0.9680 0.9658 0.9635 0.9613 ...
         0.9590 0.9568 0.9546 0.9524 0.9502 0.9480 0.9458 0.9436 ...
         0.9414 0.9392 0.9369 0.9347 0.9325 0.9302 0.9280 0.9258 ...
         0.9236 0.9214 0.9192 0.9169 0.9147 0.9125 0.9103 0.9081 ...
         0.9059 0.9036 0.9014 0.8992 0.8970 0.8948 0.8927 0.8906 ...
         0.8887 0.8868 0.8851 0.8835 0.8820 0.8808 0.8796 0.8787 ...
         0.8779 0.8773 0.8768 0.8765 0.8762 0.8761 0.8760 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 0.8759 ...
         0.8759];
  s.g = 20*log10(s.g);
  s.f(1) = [];
  s.g(1) = [];
  s.id = 'HDA200';

function [irs,fs,sResp] = mha_get_response( mha, id, inout, b_plot )
  if (nargin < 1) || isempty(mha)
    mha = struct('host','localhost','port',33337);
  end
  if nargin < 2
    id = [];
  end
  if nargin < 3
    inout = 'out';
  end
  if nargin < 4
    b_plot = 0;
  end
  if isempty(id)
    id = 'flat';
  end
  global mha_basic_cfg;
  mha_get_basic_cfg_network( mha );
  if ~isfield(mha_basic_cfg.base,'transducers')
    error('No plugin ''transducers'' found, please check your MHA configuration');
  end
  cRDB = mha_get_response_db(mha);
  cfdb = libconfigdb();
  [sResp,idx] = cfdb.smap_get( cRDB, id );
  if isempty(idx)
    error(sprintf('Response ''%s'' not found.',id));
  end
  fs = ...
      mha_get(mha,...
              [mha_basic_cfg.base.transducers,...
               '.calib_',inout,'.config.srate']);
  fragsize = ...
      mha_get(mha,...
              [mha_basic_cfg.base.transducers,...
               '.calib_',inout,'.config.fragsize']);
  if std(sResp.g) == 0
    irs = 10.^(0.05*mean(sResp.g));
  else
    vFreqHz = 0:round(fs/2);
    vGaindB = zeros(size(vFreqHz));
    fc = 100;
    idx_log = fc+1:length(vFreqHz);
    idx_lin = 1:fc;
    vfr = sResp.f;
    vgr = sResp.g;
    for k=1:5
      vfr = [vfr(1)/2,vfr];
      vgr = [vgr(1)-(vgr(2)-vgr(1)),vgr];
      vfr(end+1) = vfr(end)*2;
      vgr(end+1) = vgr(end);
    end
    intp_mode = 'spline';
    f_lin = [fc sResp.f(1)];
    g_lin = [interp1(log(vfr),vgr,log(fc),intp_mode,'extrap'),...
             sResp.g(1)];
    vGaindB(idx_log) = ...
        interp1(log(vfr),vgr,log(vFreqHz(idx_log)),...
                intp_mode,'extrap');
    vGaindB(idx_lin) = ...
        interp1(f_lin,g_lin,vFreqHz(idx_lin),...
                intp_mode,'extrap');
    vGain = 10.^(0.05*vGaindB');
    irs = smoothspec( vGain, fragsize+1 );
  end
  if b_plot
    ax = findobj('tag','response_ax');
    if isempty(ax)
      ax = axes('tag','response_ax');
    end
    axes(ax);
    h = plot(sResp.f,sResp.g,'r--o',...
             vFreqHz,20*log10(vGain),'b-',...
             vFreqHz,20*log10(abs(realfft(zeropad(irs,fs)))),'k-');
    set(h(3),'linewidth',2);
    set(gca,'XLim',[50 fs/2],'XScale','log');
    title(id);
    grid on
  end

function irs = smoothspec( spec, n )
  numbins = size(spec,1);
  if ~bitand( numbins, 1 )
    error(['This function works only for even FFT lengths\n',...
           '(odd number of bins in spectrum)']);
  end
  fftlen = 2*(numbins-1);
  spec = abs(spec);
  phase = -imag(myhilbert(log(max(1e-10,spec))));
  irs = realifft(spec .* exp(i*phase));
  wnd = cos(0.5*[0:(n-1)]'*pi/n);
  irs = irs(1:n,:) .* repmat(wnd,[1,size(spec,2)]);

function x = myhilbert( xr )
  if prod(size(xr)) == max(size(xr))
    xr = xr(:);
  end
  X = fft(xr);
  fftlen = size(X,1);
  nyquist_bin = floor(fftlen/2)+1;
  X(nyquist_bin+1:end,:) = 0;
  X(1,:) = 0.5*X(1,:);
  if ~mod(fftlen,2)
    X(nyquist_bin,:) = 0.5*X(nyquist_bin,:);
  end
  x = ifft(2*X);
  
function x = realifft( y )
% REALIFFT - inverse FFT of positive frequencies in y
%
% Usage: x = realifft( y )
%
% Returns inverse FFT or half-complex spectrum. Each column of y is
% taken as a spectrum.
  ;
  channels = size(y,2);
  nbins = size(y,1);
  x = zeros(2*(nbins-1),channels);
  for ch=1:channels
    ytmp = y(:,ch);
    ytmp(1) = real(ytmp(1));
    ytmp(nbins) = real(ytmp(nbins));
    ytmp2 = [ytmp; conj(ytmp(nbins-1:-1:2))];
    x(:,ch) = real(ifft(ytmp2));
  end
  
function s = wordwrap( s )
  width = 50;
  lastbreak = 0;
  for k=1:numel(s)
    if (s(k) == 10)
      lastbreak = k;
    end
    if (s(k) == ' ') && (k-lastbreak > width)
      s(k) = 10;
      lastbreak = k;
    end
  end
  
  % Local Variables:
  % indent-tabs-mode: nil
  % matlab-indent-level: 2
  % coding: utf-8-unix
  % End:
