function sAud = mha_audiometer( sCfg, sAud, client_id )
% MHA_AUDIOMETER - Audiometer interface for MHA plugin 'audiometerbackend'.
%
% Usage:
% sAud = mha_audiometer( sCfg [, sAud] );
%
% sCfg is a control structure with these fields (all fields
% optional):
%
% - mha         : handle of MHA network connection
% - base         : configuration base name of 'addsndfile' plugin
% - start_idx    : number of file to start with
% - client_id    : client identifier string (for display only)
%
% Author: Giso Grimm
% (c) 2007, 2011 Universitaet Oldenburg
%
  ;
  if nargin < 1
    sCfg = struct;
  end
  if nargin < 2
    libaudprof();
    sAud = audprof.audprof_new();
  end
  if nargin < 3
    if isfield(sAud,'client_id')
      client_id = sAud.client_id;
    else
      client_id = '';
    end
  end
  %
  % fill default values:
  %
  sCfg = default_val(sCfg,'client_id',client_id);
  sCfg.aud = sAud;
  %
  sCfg = mha_audiometer_config( sCfg );
  %error stop
  if isempty(sCfg)
    sAud = [];
    return
  end
  %
  % Setup control data handle:
  %
  sData = struct;
  sData.cur_side = 2;
  sData.xdata = 1:length(sCfg.vf);
  sData.htl_current = ...
      [[sCfg.start_threshold.l.data.hl];...
       [sCfg.start_threshold.r.data.hl]];
  sData.htl_current(find(~isfinite(sData.htl_current))) = 0;
  sData.htl_final = nan(size(sData.htl_current));
  sData.htl_backup = zeros(size(sData.htl_current));
  sData.cfg = sCfg;
  sData.b_sig = 0;
  sData.cur_idx = repmat(sCfg.start_idx,[1 2]);
  %
  % setup GUI:
  %
  fh = figure('Name',sprintf('MHA audiometer %s',sCfg.client_id),...
              'NumberTitle','off',...
              'MenuBar','none',...
              'Position',[100 200 900 520],...
              'PaperUnits','inch',...
              'PaperType','A4',...
              'PaperPosition',[1 5 6 2],...
              'CloseRequestFcn',@cb_cancel);
  ax1 = axes('Units','Pixels','Position',[60 215 300 240]);
  ax2 = axes('Units','Pixels','Position',[580 215 300 240]);
  vAx = [ax2;ax1];
  vCol = ['br'];
  switch sData.cfg.threshold_type
   case 'htl_ac'
    vMark = 'xo';
   case 'htl_bc'
    vMark = '><';
   case 'ucl'
    vMark = '^^';
  end
  csSide = {'Left','Right'};
  sData.inf_marker = [];
  for kSide = 1:2
    axes(vAx(kSide));
    sData.ph_htl(kSide) = plot(sData.xdata,...
                               sData.htl_current(kSide,sData.xdata),...
                               [vCol(kSide) '-']);
    hold on;
    sData.ph_final(kSide) = plot(sData.xdata,...
                                 sData.htl_final(kSide,sData.xdata),...
                                 [vCol(kSide) vMark(kSide)]);
    sData.ph_marker(kSide) = ...
        plot(sData.cur_idx(kSide),...
             sData.htl_current(kSide,sData.cur_idx(kSide)),...
             'k+',...
             'MarkerSize',20);
    ylabel('Hearing Threshold / dB HL');
    title(sprintf('%s - %s',sCfg.client_id,csSide{kSide}),...
          'Interpreter','none');
    for kf=1:numel(sCfg.vf)
      sData.inf_marker(kSide,kf) = ...
          text(kf,100,'V','HorizontalAlignment','center',...
               'FontSize',18,'Visible','off');
    end
  end
  set(sData.ph_htl,'Linewidth',1);
  set(sData.ph_final,'Linewidth',2,'MarkerSize',13);
  set(vAx,'XLim',[0.5 max(sData.xdata)+0.5],...
          'YDir','reverse',...
          'YLim',[-10 120],'XTick',sData.xdata,...
          'XGrid','on',...
          'YTick',[0:10:110],...
          'YGrid','on',...
          'XTickLabel',vf2lab(sCfg.vf));
  uicontrol('Style','frame','Position',[390 215 120 240]);
  uih = uicontrol('Style','text','Position',[391 216 118 238],...
                  'Tag','mha_htl_signal_indicator',...
                  'FontWeight','bold','FontSize',14);
  mha_htl_update_plot(sData);
  upload_sound_setall(sData);
  sz_small = [0 0 30 30];
  sz_big = [0 0 560 65];
  sz_medium = [0 0 90 30];
  sz_medium2 = [0 0 90 90];
  sData.accelerator_map = {...
      'uparrow',@cb_threshold_down,'^','Decrese level by 2 dB',sz_small+[435 155 0 0],[];...
      'downarrow',@cb_threshold_up,'v','Increase level by 2 dB',sz_small+[435 85 0 0],[];...
      'leftarrow',@cb_signal_left,'<','Select previous test signal',sz_small+[400 120 0 0],[];...
      'rightarrow',@cb_signal_right,'>','Select next test signal',sz_small+[470 120 0 0],[];...
      'return',@cb_threshold_accept,'Ok','Accept data point',sz_small+[435 120 0 0],[];...
      'space',@cb_toggle_sound,'Sound on/off','Toggle playback of sound',sz_big+[170 10 0 0],[];...
      'r',@cb_select_right,'R','Select Right ear',sz_medium2+[170 85 0 0],[];...
      'l',@cb_select_left,'L','Select Left ear',sz_medium2+[640 85 0 0],[];...
      'control-p',@cb_print,'Print','Print audiogram',sz_medium+[200 490 0 0],[];...
      'escape',@cb_cancel,'Cancel','Close audiometer without saving data',sz_medium+[100 490 0 0],[];...
      'q',@cb_quit,'Quit','Save data and close audiometer',sz_medium+[0 490 0 0],[];...
      'x',@cb_toggle_inf,'not noticed','The test tone was not noticed',sz_medium+[520 85 0 0],[];...
                   };
  sData = setup_uicontrol(sData);
  set(fh,'KeyPressFcn',@mha_htl_key,...
         'UserData',sData,...
         'Tag','mha_htl_main_ctl');
  clear('sAud_mha_audiometer');
  global sAud_mha_audiometer;
  sAud_mha_audiometer = data_to_aud(sData);
  uiwait(fh);
  mha_set(sCfg.mha,sCfg.base,sCfg.orig_cfg);
  sAud = sAud_mha_audiometer;
  clear('sAud_mha_audiometer');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Helper functions
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function sData = setup_uicontrol(sData)
  am = sData.accelerator_map;
  for k=1:size(am,1)
    am{k,6} = ...
        uicontrol('Style','Pushbutton','String',am{k,3},...
                  'Position',am{k,5},'CallBack',am{k,2},...
                  'Tooltip',sprintf('Key: ''%s''\n%s',am{k,1},am{k,4}));
    ;
    %'KeypressFcn',@mha_htl_key
  end
  sData.accelerator_map = am;
  sData.default_ui = uicontrol('KeypressFcn',@mha_htl_key,...
                               'Position',[0 0 1 1],...
                               'BusyAction','cancel');

function sAud = data_to_aud( sData )
  libaudprof();
  sAud = sData.cfg.aud;
  k=1;
  for side='lr'
    sAud.(side).(sData.cfg.threshold_type) = ...
        audprof.threshold_entry_add([],sData.cfg.vf, ...
                                    sData.htl_final(k,:));
    sAud.(side).(sData.cfg.threshold_type).datestr = datestr(now);
    sAud.(side).(sData.cfg.threshold_type).id = ...
        [sData.cfg.calib.retSPL_fun,', ',sData.cfg.sigtype];
    k = k+1;
  end
  sAud = audprof.audprof_cleanup( sAud );

function sCfg = default_val( sCfg, field, value )
  if ~isfield(sCfg,field)
    sCfg.(field) = value;
  end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Keyboard handler
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function mha_htl_key( bo, key )
  if isempty(key.Modifier)
    lkey = key.Key;
  else
    lkey = '';
    for mod=key.Modifier
      lkey = [lkey mod{:} '-'];
    end
    lkey = [lkey key.Key];
  end
  sData = get(gcf, 'UserData');
  accel_keys = sData.accelerator_map(:,1)';
  idx = strmatch(lkey,accel_keys,'exact');
  if ~isempty(idx)
    am = sData.accelerator_map(idx(1),:);
    feval(am{2});
  end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Implementation of callbacks (mouse/keyboard)
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function cb_signal_left( varargin )
  sData = get(gcf, 'UserData');
  if sData.cur_idx(sData.cur_side) > 1
    sData.cur_idx(sData.cur_side) = sData.cur_idx(sData.cur_side)-1;
    upload_sound_set(sData);
  end
  set(gcf,'UserData',sData);
  mha_htl_update_plot(sData);

function cb_signal_right( varargin )
  sData = get(gcf, 'UserData');
  if sData.cur_idx(sData.cur_side) < length(sData.xdata)
    sData.cur_idx(sData.cur_side) = sData.cur_idx(sData.cur_side)+1;
    upload_sound_set(sData);
  end
  set(gcf,'UserData',sData);
  mha_htl_update_plot(sData);

function cb_threshold_up( varargin )
  sData = get(gcf, 'UserData');
  if sData.htl_current(sData.cur_side,sData.cur_idx(sData.cur_side)) < 100
    sData.htl_current(sData.cur_side,sData.cur_idx(sData.cur_side)) = ...
        sData.htl_current(sData.cur_side,sData.cur_idx(sData.cur_side))+sData.cfg.stepsize;
    upload_sound_level(sData);
  end
  set(gcf,'UserData',sData);
  mha_htl_update_plot(sData);

function cb_threshold_down( varargin )
  sData = get(gcf, 'UserData');
  if sData.htl_current(sData.cur_side,sData.cur_idx(sData.cur_side)) > -20
    sData.htl_current(sData.cur_side,sData.cur_idx(sData.cur_side)) = ...
        sData.htl_current(sData.cur_side,sData.cur_idx(sData.cur_side))-sData.cfg.stepsize;
    upload_sound_level(sData);
  end
  set(gcf,'UserData',sData);
  mha_htl_update_plot(sData);

function cb_threshold_accept( varargin )
  sData = get(gcf, 'UserData');
  sData.htl_final(sData.cur_side,sData.cur_idx(sData.cur_side)) = ...
      sData.htl_current(sData.cur_side,sData.cur_idx(sData.cur_side));
  sData.date = datestr(now);
  set(gcf,'UserData',sData);
  mha_htl_update_plot(sData);

function cb_select_left( varargin )
  sData = get(gcf, 'UserData');
  if sData.cur_side ~= 1
    sData.cur_side = 1;
    sData.b_sig = 0;
    upload_sound_set(sData);
  end
  set(gcf,'UserData',sData);
  mha_htl_update_plot(sData);

function cb_select_right( varargin )
  sData = get(gcf, 'UserData');
  if sData.cur_side ~= 2
    sData.cur_side = 2;
    sData.b_sig = 0;
    upload_sound_set(sData);
  end
  set(gcf,'UserData',sData);
  mha_htl_update_plot(sData);

function cb_toggle_sound( varargin )
  sData = get(gcf, 'UserData');
  sData.b_sig = 1-sData.b_sig;
  upload_sound_set(sData);
  set(gcf,'UserData',sData);
  mha_htl_update_plot(sData);

function cb_cancel( varargin )
  sData = get(gcf, 'UserData');
  answ = questdlg(['Do you really want to close the audiometer' ...
                   ' and discard the current measurement?'],...
                  'Closing audiometer','Yes','No','No');
  if strcmp(answ,'Yes')
    global sAud_mha_audiometer;
    sAud_mha_audiometer = [];
    delete(gcf);
    return;
  end

function cb_quit( varargin )
  sData = get(gcf, 'UserData');
  global sAud_mha_audiometer;
  sAud_mha_audiometer = data_to_aud(sData);
  delete(gcf);
  return;

function cb_print( varargin )
  sData = get(gcf, 'UserData');
  print_data(sData);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
% Implementation of audiometer functionality
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function mha_htl_update_plot(sData)
  for k=1:2
    val = sData.htl_current(k,sData.cur_idx(k));
    if ~isfinite(val)
      if ~isfinite(sData.htl_backup(k,sData.cur_idx(k)))
        val = 0;
      else
        val = sData.htl_backup(k,sData.cur_idx(k));
      end
    end
    set(sData.ph_marker(k),...
        'XData',sData.cur_idx(k),...
        'YData',val,...
        'MarkerSize',eps+(k==sData.cur_side)*40,...
        'LineWidth',eps+(k==sData.cur_side)*2);
    set(sData.ph_htl(k),...
        'YData',sData.htl_current(k,sData.xdata));
    set(sData.ph_final(k),...
        'YData',sData.htl_final(k,sData.xdata));
  end
  kSide = sData.cur_side;
  kF = sData.cur_idx(sData.cur_side);
  val = sData.htl_current(kSide,kF);
  if isfinite(val) || isnan(val)
    set(sData.inf_marker(kSide,kF),'Visible','off');
  else
    set(sData.inf_marker(kSide,kF),'Visible','on');
  end
  h = findall(gcf,'Tag','mha_htl_signal_indicator');
  if sData.cur_side == 1
    col = [0 0 1];
  else
    col = [1 0 0];
  end
  if ~sData.b_sig
    col = 0.05*col+0.7;
  else
    col = 0.3*col+0.5;
  end
  sLabel = sData.cfg.vf(sData.cur_idx(sData.cur_side));
  if iscell(sLabel)
    sLabel = sLabel{:};
  end
  csSide = {'Left','Right'};
  csState = {'Silence','Playing'};
  set(h,'BackgroundColor',col,'Value',sData.b_sig,...
        'String',sprintf('\n\n%s\n\n%s\n\n%g dB\n\n%s',csSide{sData.cur_side},num2str(sLabel),sData.htl_current(sData.cur_side,sData.cur_idx(sData.cur_side)),csState{sData.b_sig+1}));
  drawnow;
  %set(gcf,'CurrentObject',findall(gcf,'Tag','mha_htl_signal_indicator'));
  if isfield(sData,'default_ui')
    bo = gcbo;
    if ~isempty(bo)
      if bo ~= sData.default_ui
        uicontrol(sData.default_ui);
      end
    end
  end

function l = getlevel( sData )
  l = sData.htl_current(sData.cur_side,sData.cur_idx(sData.cur_side)) + ...
      sData.cfg.calib.spl2hl(sData.cur_side,sData.cur_idx(sData.cur_side));

function upload_sound_level(sData)
  level = getlevel( sData );
  if sData.b_sig && isfinite(level)
    mha_set_basev(sData,'level',level);
  else
    mha_set_basev(sData,'level',-60);
  end

function mha_set_basev(sData,name,val)
  mha_set(sData.cfg.mha,[sData.cfg.base '.' name],val);

function upload_sound_set(sData)
  mha_set_basev(sData,'level',-60);
  pause(0.1);
  if iscell(sData.cfg.vf)
    mha_set_basev(sData,'freq', ...
                  sData.cfg.vf{sData.cur_idx(sData.cur_side)});
  else
    mha_set_basev(sData,'freq', ...
                  sData.cfg.vf(sData.cur_idx(sData.cur_side)));
  end
  csSide = {'left','right'};
  mha_set_basev(sData,'mode',csSide{sData.cur_side});
  if sData.b_sig
    mha_set_basev(sData,'level',getlevel(sData));
  end

function upload_sound_setall(sData)
  cfg = struct;
  if iscell(sData.cfg.vf)
    cfg.freq = sData.cfg.vf{sData.cur_idx(sData.cur_side)};
  else
    cfg.freq = sData.cfg.vf(sData.cur_idx(sData.cur_side));
  end
  %cfg.loop = 1;

  if sData.b_sig
    cfg.level = getlevel(sData);
  else
    cfg.level = -60;
  end
  %cfg.levelmode = 'rms';
  cfg.mode = 'mute';
  cfg.ramplen = 0.125;
  mha_set(sData.cfg.mha,sData.cfg.base,cfg);

function cb_toggle_inf( varargin )
  sData = get(gcf, 'UserData');
  idx = sData.cur_idx(sData.cur_side);
  if isfinite(sData.htl_current(sData.cur_side,sData.cur_idx(sData.cur_side)))
    sData.htl_backup(sData.cur_side,idx) = ...
        sData.htl_current(sData.cur_side,idx);
    sData.htl_current(sData.cur_side,idx) = inf;
    sData.htl_final(sData.cur_side,idx) = inf;
  else
    sData.htl_current(sData.cur_side,idx) = ...
        sData.htl_backup(sData.cur_side,idx);
  end
  set(gcf,'UserData',sData);
  mha_htl_update_plot(sData);

function cs = vf2lab( vf )
  cs = {};
  for k=1:numel(vf)
    f = vf(k);
    if f < 1000
      cs{end+1} = sprintf('%g',f);
    else
      cs{end+1} = sprintf('%gk',f/1000);
    end
  end
