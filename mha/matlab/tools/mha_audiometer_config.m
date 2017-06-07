function sCfg = mha_audiometer_config( sCfg )
  try
    libmhagui();
    libaudprof();
    if nargin < 1
      sCfg = struct;
    end
    sCfg = default_val(sCfg,'aud',audprof.audprof_new());
    sCfg = default_val(sCfg,'mha',mha_ensure_mhahandle);
    sCfg = default_val(sCfg,'base',-1);
    if sCfg.base == -1
      scfgtmp = mha_get_basic_cfg_network( sCfg.mha );
      sCfg.base = scfgtmp.base.audiometerbackend;
    end
    sCfg = default_val(sCfg,'stepsize',5);
    sCfg.orig_cfg = mha_get(sCfg.mha,sCfg.base,'writable');
    sCfg.sigtype = mha_get(sCfg.mha,[sCfg.base,'.sigtype']);
    sCfg.vf = stdaud_freq;
    fh = mhagui.figure('Audiometer setup',...
		       'mhagui.audiometer.setup',...
		       [610,420]);
    sCfg.fh = fh;
    set(fh,'UserData',sCfg);
    add_label([240,390,150,20],'Frequency / Hz:');
    uicontrol('style','edit','tag','add_freq','callback',@add_freq,...
	      'Position',[240,360,150,30]);
    uicontrol('style','listbox','tag','select_freq','string', ...
	      sCfg.vf,'callback',@select_freq, ...
	      'FontSize',12,...
	      'Position',[240,120,150,240]);
    uicontrol('style','pushbutton','String','remove freq.',...
	      'callback',@remove_freq,...
	      'Position',[410,360,110,30]);
    uicontrol('style','pushbutton','String','clear all',...
	      'callback',@remove_allfreq,...
	      'Position',[410,320,110,30]);
    uicontrol('style','pushbutton','String','standard',...
	      'callback',@addstd_freq,...
	      'Position',[410,280,110,30]);
    uicontrol('style','text','horizontalAlignment','left',...
	      'Position',[410,180,180,90],...
	      'Tag','text:missingcal','ForegroundColor',[0.6,0,0],...
	      'Fontsize',8,'FontWeight','bold',...
	      'BackgroundColor',get(fh,'Color'));
    uicontrol('style','pushbutton','String','Review calibration',...
	      'callback',@review_calib,...
	      'Position',[410,120,150,50]);
    
    add_label([20,390,150,20],'Stimulus type:');
    h = mhagui_keyword(sCfg.mha,[sCfg.base,'.sigtype'],[0,0],@update_sigtype);
    set(h,'style','listbox','Position',[20,270,200,120],...
	  'FontSize',12);
    delete(findobj(fh,'tag',['label:',sCfg.base,'.sigtype']));

    add_label([20,240,150,20],'Step size / dB:');
    uicontrol('style','edit','string',sCfg.stepsize,...
	      'callback',@cb_update,...
	      'Position',[20,210,200,30],'tag','ed:stepsize');
    
    add_label([20,180,200,20],'Threshold type:');
    uicontrol('style','listbox','tag','list:threshold',...
	      'string',{'htl_ac','htl_bc','ucl'},...
	      'callback',@cb_update,...
	      'Position',[20,100,200,80],'FontSize',12);

    add_label([20,70,150,20],'Start with:');
    uicontrol('style','listbox','string',{'selected audiogram','empty audiogram'},...
	      'Position',[20,20,200,50],'tag','list:start_threshold',...
	      'Fontsize',12,...
	      'Callback',@set_start_threshold);
    
    % some GUI polishing:
    set(findobj(fh,'style','edit'),'BackgroundColor',ones(1,3),...
		      'HorizontalAlignment','left');
    sCfg = mhagui.waitfor(fh,@update_state);
  catch
    disp_err_rethrow
  end

function vf = stdaud_freq
  vf = unique([1000*2.^[-3:3],1500*2.^[-1:2]])';
  
function update_sigtype( var, val )
  sCfg = get(gcf,'UserData');
  sCfg.sigtype = val;
  set(gcf,'UserData',sCfg);
  check_aud_calib(gcf);
  
function check_aud_calib( fh )
  try
    sCfg = get(fh,'UserData');
    addr = sCfg.base;
    addr(1:max(find(addr=='.'))) = [];
    cfgpath = ...
	['aud_calib.',addr,'.',sCfg.sigtype];
    libconfigdb();
    calib = configdb.get_mhaconfig(sCfg.mha,...
				   cfgpath,...
				   struct('f',[],'corr',[],'corrfun','freefield'));
    sCfg.calib = calib;
    nocal = [];
    for f=sCfg.vf(:)'
      if isempty(find(f==calib.f))
	nocal(end+1) = f;
      end
    end
    %nocal
    sNocal = sprintf('%g, ',nocal);
    bCanstart = 'off';
    if ~isempty(nocal)
      sNocal(end-1:end) = '';
      sNocal = sprintf('Calibration data missing for:\n%s',sNocal);
      vcol = [0.4,0,0];
    else
      sNocal = 'Calibration available!';
      vcol = [0,0.4,0];
      bCanstart = 'on';
    end
    if isempty(sCfg.vf)
      sNocal = '';
      bCanstart = 'off';
    end
    set(findobj(sCfg.fh,...
		'tag','waitfor:ok'),'Enable',bCanstart);
    set(findobj(sCfg.fh,...
		'Tag','text:missingcal'),'String',sNocal,...
	'ForegroundColor',vcol);
    set(sCfg.fh,'UserData',sCfg);
  catch
    disp_err_rethrow
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
    disp_err_rethrow
  end


function add_freq( varargin )
  try
    v = str2num(get(gcbo,'String'));
    if isfinite(v) && (numel(v)==1)
      select_freqv( v );
    end
  catch
    disp_err_rethrow
  end

function select_freqv( f )
  sCfg = get(gcf,'UserData');
  if ~isempty(f)
    idx = find(sCfg.vf==f);
    if isempty(idx)
      sCfg.vf(end+1) = f;
      sCfg.vf = unique(sCfg.vf);
      idx = find(sCfg.vf==f);
      set(sCfg.fh,'UserData',sCfg);
    end
    hList = findobj(sCfg.fh,'tag','select_freq');
    set(hList,'String',sCfg.vf,'Value',idx);
    hEd = findobj(sCfg.fh,'tag','add_freq');
    set(hEd,'String',f);
  end
  check_aud_calib(gcf);
  
  
function remove_freq( varargin )
  try
    sCfg = get(gcbf,'UserData');
    hList = findobj(sCfg.fh,'tag','select_freq');
    idx = get(hList,'Value');
    if (~isempty(idx)) && (idx>0)
      sCfg.vf(idx) = [];
      sCfg.vf = unique(sCfg.vf);
      set(gcbf,'UserData',sCfg);
      idx = min(1,numel(sCfg.vf));
      if isempty(sCfg.vf)
	f = [];
      else
	f = sCfg.vf(1);
      end
      set(hList,'String',sCfg.vf,'Value',idx);
      hEd = findobj(sCfg.fh,'tag','add_freq');
      set(hEd,'String',f);
    end
    check_aud_calib(gcbf);
  catch
    disp_err_rethrow
  end
  
function remove_allfreq( varargin )
  sCfg = get(gcbf,'UserData');
  sCfg.vf = [];
  set(gcbf,'UserData',sCfg);
  hList = findobj(sCfg.fh,'tag','select_freq');
  set(hList,'String',sCfg.vf,'Value',0);
  hEd = findobj(sCfg.fh,'tag','add_freq');
  set(hEd,'String','');
  check_aud_calib(gcbf);
  
function addstd_freq( varargin )
  sCfg = get(gcbf,'UserData');
  sCfg.vf = stdaud_freq;
  set(gcbf,'UserData',sCfg);
  select_freqv( sCfg.vf(1) );
  
function review_calib( varargin )
  try
    sCfg = get(gcbf,'UserData');
    audiometercalib( sCfg.mha, sCfg.base );
    check_aud_calib(gcbf);
  catch
    disp_err_rethrow
  end

function add_label(pos,s)
  uicontrol('style','text','Position',pos,'String',s,...
	    'HorizontalAlignment','left',...
	    'FontWeight','bold',...
	    'BackgroundColor',get(gcf,'Color'));
  
function set_start_threshold( varargin )
  update_state(gcbf);
  
function update_state( fh )
  libaudprof();
  check_aud_calib(fh);
  sCfg = get(fh,'UserData');
  csThreshold = get(findobj(fh,'tag','list:threshold'),'String');
  idx = get(findobj(fh,'tag','list:threshold'),'Value');
  sThreshold = csThreshold{idx};
  sCfg.threshold_type = sThreshold;
  % define start audiogram:
  if get(findobj(fh,'tag','list:start_threshold'),'Value')==1
    % selected audiogram
    
    aud = sCfg.aud;
  else
    % empty audiogram
    aud = audprof.audprof_new();
  end
  for side='lr'
    th = audprof.threshold_new();
    if isfield(aud,side) && isfield(aud.(side),sThreshold)
      th = aud.(side).(sThreshold);
    end
    sCfg.start_threshold.(side) = ...
	audprof.threshold_fill_intersect( th, sCfg.vf, 0*sCfg.vf);
  end
  % start at median frequency:
  sCfg.start_idx = floor(numel(sCfg.vf)/2);
  if isfield(sCfg.calib,'spl2hl')
    sCfg.spl2hl = interp1(sCfg.calib.f',...
			  sCfg.calib.spl2hl',...
			  sCfg.vf', ...
			  'nearest')';
  end
  stepsize = ...
      str2num(get(findobj(sCfg.fh,'tag','ed:stepsize'), ...
		  'String'));
  if isempty(stepsize) || (~isfinite(stepsize)) || (stepsize <= 0)
  	error('inavlid step size');
  end
  sCfg.stepsize = stepsize;
  set(fh,'UserData',sCfg);

function cb_update( varargin )
  update_state(gcbf);
  
function sCfg = default_val( sCfg, field, value )
  if ~isfield(sCfg,field)
    sCfg.(field) = value;
  end
  