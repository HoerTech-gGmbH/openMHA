function s = sd_plotwizzard( s )
% PLOT_STRUCT_DATA_WIZZARD - Interactively create data plots based
% on structured data input.
%
% Usage:
% s = plot_sd_wizzard( s )
%
% s : structure containing the data
%
% Author: Giso Grimm
% Date: 11/2006
  fh = figure('NumberTitle','off','MenuBar','none',...
	      'Position',[250 400 740 620],...
	      'Name','Structured data plotting wizzard');
  set(fh,'UserData',s);
  %movegui(fh,'center');
  uicontrol('style','text','fontweight','bold',...
	    'string','Y fields',...
	    'position',[40 580 180 20]);
  uicontrol('style','text','fontweight','bold',...
	    'string','Y axis',...
	    'position',[270 580 180 20]);
  uicontrol('style','text','fontweight','bold',...
	    'string','Available fields',...
	    'position',[40 460 180 20]);
  uicontrol('style','text','fontweight','bold',...
	    'string','X axis',...
	    'position',[270 460 180 20]);
  uicontrol('style','text','fontweight','bold',...
	    'string','Average model',...
	    'position',[500 580 180 20]);
  uicontrol('style','text','fontweight','bold',...
	    'string','Parameter',...
	    'position',[270 370 180 20]);
  uicontrol('style','text','fontweight','bold',...
	    'string','Restrictions',...
	    'position',[270 280 180 20]);
  uicontrol('style','listbox','string',s.fields(length(s.values)+1:end),...
	    'Position',[40 500 180 80],'tag','avail_fields_y',...
	    'value',1);
  uicontrol('style','listbox','string',s.fields(1:length(s.values)),...
	    'Position',[40 110 180 350],'tag','avail_fields',...
	    'value',1);
  uicontrol('style','listbox','string',{},...
	    'Position',[270 500 180 80],'tag','ydata_selection');
  uicontrol('style','text','string','',...
	    'Position',[270 410 180 50],'tag','xdata_selection',...
	    'HorizontalAlignment','left');
  uicontrol('style','text','string','',...
	    'Position',[270 320 180 50],'tag','pdata_selection',...
	    'HorizontalAlignment','left');
  uicontrol('style','listbox','string',{},...
	    'Position',[270 200 180 80],'tag','restrictions',...
	    'Callback',@select_restriction);
  uicontrol('style','listbox','string',{},...
	    'Position',[270 110 180 80],'tag','restriction_vals',...
	    'Callback',@select_restriction_val);
  uicontrol('style','listbox','string',{'mean','median'},...
	    'Position',[500 500 180 80],'tag','average_model');
  uicontrol('style','PushButton','string','>>',...
	    'Position',[230 560 30 20],'callback',@add_y_data);
  uicontrol('style','PushButton','string','<<',...
	    'Position',[230 530 30 20],'callback',@rm_y_data);
  uicontrol('style','PushButton','string','>>',...
	    'Position',[230 440 30 20],'callback',@add_x_data);
  uicontrol('style','PushButton','string','<<',...
	    'Position',[230 410 30 20],'callback',@rm_x_data);
  uicontrol('style','PushButton','string','>>',...
	    'Position',[230 350 30 20],'callback',@add_p_data);
  uicontrol('style','PushButton','string','<<',...
	    'Position',[230 320 30 20],'callback',@rm_p_data);
  uicontrol('style','PushButton','string','>>',...
	    'Position',[230 260 30 20],'callback',@add_restriction);
  uicontrol('style','PushButton','string','<<',...
	    'Position',[230 230 30 20],'callback',@rm_restriction);
  uicontrol('style','PushButton','string','plot',...
	    'Position',[40 40 130 40],'callback',@plot_this);
  uicontrol('style','PushButton','string','quit',...
	    'Position',[240 40 130 40],'callback','uiresume(gcf)');
  %% plot style controls:
  uicontrol('style','frame','position',[470 100 260 280],...
	    'tag','plotpar');
  create_plotpar_ui( 'fontsize', 1, @validate_positive, 14 );
  create_plotpar_ui( 'markersize', 2, @validate_positive, 3 );
  create_plotpar_ui( 'linewidth', 3, @validate_positive, 1 );
  create_plotpar_ui( 'linestyles', 4, @validate_string_or_cellarray, '''''' );
  create_plotpar_ui( 'gridfreq', 5, @validate_positive, 1 );
  create_plotpar_ui( 'firstgrid', 6, @validate_positive, 1 );
  create_plotpar_ui( 'colors', 7, @validate_string_or_cellarray, '''''' );
  create_plotpar_ui( 'markers', 8, @validate_string_or_cellarray, '''''' );
  if isfield( s, 'datapars' )
    set_all_datapars( s.datapars, fh );
  end
  if isfield( s, 'plotpars' )
    set_all_plotpar_svals( s.plotpars, fh );
  end
  uiwait(fh);
  if ishandle(fh)
    s.datapars = get_data_pars( fh );
    plotpars = struct;
    for fn={'fontsize','markersize','linewidth','linestyles', ...
	    'gridfreq','firstgrid','colors','markers'}
      name = fn{:};
      plotpars = get_plotpar_sval( plotpars, name, fh );
    end
    s.plotpars = plotpars;
    close(fh);
  end
  if nargout == 0
    clear s
  end
  
  
  
function add_y_data(varargin)
  h_fields = findobj(gcf,'tag','avail_fields_y');
  h_ydata = findobj(gcf,'tag','ydata_selection');
  csfields = get(h_fields,'string');
  if length(csfields)
    val = get(h_fields,'value');
    field = csfields{val};
    csfields(val) = [];
    set(h_fields,'value',length(csfields),'string',csfields);
    csfields =  get(h_ydata,'string');
    csfields{end+1} = field;
    set(h_ydata,'string',csfields,'value',length(csfields));
  end
  
function rm_y_data(varargin)
  h_fields = findobj(gcf,'tag','avail_fields_y');
  h_ydata = findobj(gcf,'tag','ydata_selection');
  csfields = get(h_ydata,'string');
  if length(csfields)
    val = get(h_ydata,'value');
    field = csfields{val};
    csfields(val) = [];
    set(h_ydata,'value',length(csfields),'string',csfields);
    csfields =  get(h_fields,'string');
    csfields{end+1} = field;
    set(h_fields,'string',csfields,'value',length(csfields));
  end
    
function add_x_data(varargin)
  h_fields = findobj(gcf,'tag','avail_fields');
  h_xdata = findobj(gcf,'tag','xdata_selection');
  csfields = get(h_fields,'string');
  if length(csfields)
    if length(get(h_xdata,'String')) == 0
      val = get(h_fields,'value');
      field = csfields{val};
      csfields(val) = [];
      set(h_fields,'value',length(csfields),'string',csfields);
      set(h_xdata,'string',field);
    end
  end
  
function rm_x_data(varargin)
  h_fields = findobj(gcf,'tag','avail_fields');
  h_xdata = findobj(gcf,'tag','xdata_selection');
  field = get(h_xdata,'string');
  if length(field)
    set(h_xdata,'string','');
    csfields =  get(h_fields,'string');
    csfields{end+1} = field;
    set(h_fields,'string',csfields,'value',length(csfields));
  end
  
function add_p_data(varargin)
  h_fields = findobj(gcf,'tag','avail_fields');
  h_xdata = findobj(gcf,'tag','pdata_selection');
  csfields = get(h_fields,'string');
  if length(csfields)
    if length(get(h_xdata,'String')) == 0
      val = get(h_fields,'value');
      field = csfields{val};
      csfields(val) = [];
      set(h_fields,'value',length(csfields),'string',csfields);
      set(h_xdata,'string',field);
    end
  end
  
function rm_p_data(varargin)
  h_fields = findobj(gcf,'tag','avail_fields');
  h_xdata = findobj(gcf,'tag','pdata_selection');
  field = get(h_xdata,'string');
  if length(field)
    set(h_xdata,'string','');
    csfields =  get(h_fields,'string');
    csfields{end+1} = field;
    set(h_fields,'string',csfields,'value',length(csfields));
  end
  
function add_restriction(varargin)
  h_fields = findobj(gcf,'tag','avail_fields');
  h_ydata = findobj(gcf,'tag','restrictions');
  csfields = get(h_fields,'string');
  if length(csfields)
    val = get(h_fields,'value');
    field = csfields{val};
    csfields(val) = [];
    set(h_fields,'value',min(1,length(csfields)),'string',csfields);
    csfields =  get(h_ydata,'string');
    csfields{end+1} = field;
    set(h_ydata,'string',csfields,'value',length(csfields));
    select_restriction;
  end
  
function rm_restriction(varargin)
  h_fields = findobj(gcf,'tag','avail_fields');
  h_ydata = findobj(gcf,'tag','restrictions');
  csfields = get(h_ydata,'string');
  if length(csfields)
    val = get(h_ydata,'value');
    field = csfields{val};
    csfields(val) = [];
    set(h_ydata,'value',min(1,length(csfields)),'string',csfields);
    csfields =  get(h_fields,'string');
    csfields{end+1} = field;
    set(h_fields,'string',csfields,'value',length(csfields));
    select_restriction;
    k = get_restriction_idx( field );
    s = get(gcf,'UserData');
    s.restrictions(k) = [];
    set(gcf,'UserData',s);
  end
  
function k = get_restriction_idx( sRes )
  s = get(gcf,'UserData');
  kRes = strmatch(sRes,s.fields,'exact');
  if ~isfield(s,'restrictions')
    s.restrictions = {};
  end
  k = 0;
  for kResL=1:length(s.restrictions)
    cRes = s.restrictions{kResL};
    if cRes{1} == kRes
      k = kResL;
    end
  end
  if ~k
    s.restrictions{end+1} = {kRes,1};
    k = length(s.restrictions);
  end
  set(gcf,'UserData',s);
  return

function select_restriction(varargin)
  h_restr = findobj(gcf,'tag','restrictions');
  h_rval = findobj(gcf,'tag','restriction_vals');
  csRes = get(h_restr,'String');
  if length(csRes)
    kRes = get(h_restr,'Value');
    kRes = get_restriction_idx( csRes{kRes} );
    s = get(gcf,'UserData');
    set(h_rval,'String',s.values{s.restrictions{kRes}{1}},...
	       'Value',s.restrictions{kRes}{2});
  else
    set(h_rval,'String',{},'Value',0);
  end
  
function select_restriction_val(varargin)
  h_restr = findobj(gcf,'tag','restrictions');
  h_rval = findobj(gcf,'tag','restriction_vals');
  csRes = get(h_restr,'String');
  kRes = get(h_restr,'Value');
  kRes = get_restriction_idx( csRes{kRes} );
  s = get(gcf,'UserData');
  val = get(h_rval,'Value');
  s.restrictions{kRes}{2} = val;
  set(gcf,'UserData',s);
  
function sDataPar = get_data_pars( fh )
  sDataPar = struct;
  s = get(fh,'UserData');
  if ~isfield(s,'restrictions')
    s.restrictions = {};
  end
  h_ydata = findobj(fh,'tag','ydata_selection');
  h_xdata = findobj(fh,'tag','xdata_selection');
  h_pdata = findobj(fh,'tag','pdata_selection');
  ydata_name = get(h_ydata,'String');
  xdata_name = get(h_xdata,'String');
  pdata_name = get(h_pdata,'String');
  k_ydata = [];
  for sYdata=ydata_name'
    k_ydata(end+1) = strmatch(sYdata{:},s.fields,'exact');
  end
  k_xdata = strmatch(xdata_name,s.fields,'exact');
  k_pdata = strmatch(pdata_name,s.fields,'exact');
  sDataPar.y_data = k_ydata;
  sDataPar.x_data = k_xdata;
  sDataPar.par_data = k_pdata;
  sDataPar.restrictions = s.restrictions;


function plot_this(varargin)
  sDataPar = get_data_pars( gcf )
  s = get(gcf,'UserData');
  if ~isfield(s,'restrictions')
    s.restrictions = {};
  end
  h_ydata = findobj(gcf,'tag','ydata_selection');
  h_xdata = findobj(gcf,'tag','xdata_selection');
  h_pdata = findobj(gcf,'tag','pdata_selection');
  ydata_name = get(h_ydata,'String');
  xdata_name = get(h_xdata,'String');
  pdata_name = get(h_pdata,'String');
  k_ydata = [];
  for sYdata=ydata_name'
    k_ydata(end+1) = strmatch(sYdata{:},s.fields,'exact');
  end
  k_xdata = strmatch(xdata_name,s.fields,'exact');
  k_pdata = strmatch(pdata_name,s.fields,'exact');
  s_ydata = sprintf('%d ',k_ydata);
  s_ydata = s_ydata(1:end-1);
  sPlotPars = get_plot_params( gcf )
  s_restr = '';
  for k=1:length(s.restrictions)
    s_restr = sprintf('%s{%d,%d},',s_restr,s.restrictions{k}{1},s.restrictions{k}{2});
  end
  s_restr = s_restr(1:end-1);
  %pcmd = sprintf('plot_struct_data(s,%d,[%s],%d,{%s})',...
  %		 k_xdata,s_ydata, ...
  %		 k_pdata,...
  %		 s_restr);
  %  disp(pcmd);
  h_avg = findobj(gcf,'tag','average_model');
  avg_model = get(h_avg,'String');
  avg_model = avg_model{get(h_avg,'Value')};
  try
    sd_plot(s,k_xdata,k_ydata,sPlotPars,...
		     'parameter',k_pdata,...
		     'restrictions',s.restrictions,...
		     'average',avg_model);
  catch
    err = lasterror;
    disp(err.message);
    for k=1:length(err.stack)
      disp(sprintf('%s:%d %s\n',...
		   err.stack(k).file,...
		   err.stack(k).line,...
		   err.stack(k).name));
    end
    rethrow err;
  end
  
function sPlotPar = get_plot_params( fh )
  sPlotPar = struct;
  sPlotPar = set_plotpar_field( sPlotPar, 'fontsize', fh );
  sPlotPar = set_plotpar_field( sPlotPar, 'markersize', fh );
  sPlotPar = set_plotpar_field( sPlotPar, 'linewidth', fh );
  sPlotPar = set_plotpar_field( sPlotPar, 'gridfreq', fh );
  sPlotPar = set_plotpar_field( sPlotPar, 'firstgrid', fh );
  sPlotPar = set_plotpar_field( sPlotPar, 'colors', fh );
  sPlotPar = set_plotpar_field( sPlotPar, 'linestyles', fh );
  sPlotPar = set_plotpar_field( sPlotPar, 'markers', fh );
  
function sPlotParS = get_plotpar_sval( sPlotParS, name, fh )
  h = findall(fh,'tag',sprintf('plotpar_%s',name));
  sPlotParS.(name) = get(h,'String');
  
function set_all_plotpar_svals( sPlotParS, fh )
  for fn = fieldnames(sPlotParS)'
    name = fn{:};
    h = findall(fh,'tag',sprintf('plotpar_%s',name));
    set(h,'String',sPlotParS.(name));
  end
  
function sPlotPar = set_plotpar_field( sPlotPar, name, fh )
  h = findall(fh,'tag',sprintf('plotpar_%s',name));
  s = get(h,'String');
  if ~isempty(s)
    eval(sprintf('val=%s;',s));
    if ~isempty(val)
      sPlotPar.(name) = val;
    end
  end
  
  
function err = validate_string_or_cellarray( bo, varargin )
  s = get(bo, 'String' );
  if isempty( s )
    s = '''''';
  elseif strcmp([s(1) s(end)],'''''')
    s = s;
  elseif strcmp([s(1) s(end)],'{}')
    s = s;
  else
    s = ['''' s ''''];
  end
  set(bo, 'String', s );
  
function err = validate_positive( bo, varargin )
  s = get(bo, 'String' );
  v_def = get(bo,'UserData');
  try
    val = str2num(s);
    if isempty(val)
      val = v_def;
    end
  catch
    val = v_def;
  end
  if val < eps
    val = eps;
  end
  s = num2str(val);
  set(bo, 'String', s );

function create_plotpar_ui( name, idx, validator, default )
  pos = get(findall(gcf,'tag','plotpar'),'Position');
  x = pos(1)+10;
  y = pos(2)+pos(4)-10;
  uicontrol('style','text','string',name,...
	    'position',[x y-25*idx-2 85 20],...
	    'fontweight','bold','horizontalalignment','right');
  h = uicontrol('tag',sprintf('plotpar_%s',name),...
		'style','edit',...
		'position',[x+90 y-25*idx 150 20],...
		'callback',validator,...
		'userdata',default,...
		'BackgroundColor',[1 1 1]);
  validator(h);

function set_all_datapars( sPars, fh )
  s = get(fh,'UserData');
  % y axis:
  idx_yavail = 1:length(s.fields);
  idx_yavail(sPars.y_data) = [];
  idx_yavail(find(idx_yavail<=length(s.values))) = [];
  h_fields = findobj(fh,'tag','avail_fields_y');
  h_ydata = findobj(fh,'tag','ydata_selection');
  set(h_fields,'String',s.fields(idx_yavail),'Value',length(idx_yavail));
  set(h_ydata,'String',s.fields(sPars.y_data));
  % x axis:
  h_xdata = findobj(fh,'tag','xdata_selection');
  set(h_xdata,'String',s.fields(sPars.x_data));
  % p axis:
  h_pdata = findobj(fh,'tag','pdata_selection');
  set(h_pdata,'String',s.fields(sPars.par_data));
  % restrictions:
  s.restrictions = sPars.restrictions;
  h_rdata = findobj(fh,'tag','restrictions');
  vRestr = [];
  for k=1:length(s.restrictions)
    vRestr(end+1) = s.restrictions{k}{1};
  end
  set(h_rdata,'String',s.fields(vRestr));
  set(fh,'UserData',s);
  select_restriction;
  % remove used fields:
  idx_xavail = 1:length(s.fields);
  idx_xavail(find(idx_xavail==sPars.x_data)) = [];
  idx_xavail(find(idx_xavail==sPars.par_data)) = [];
  for k=1:length(vRestr)
    idx_xavail(find(idx_xavail==vRestr(k))) = [];
  end
  idx_xavail(find(idx_xavail>length(s.values))) = [];
  h_fields = findobj(fh,'tag','avail_fields');
  set(h_fields,'String',s.fields(idx_xavail),'Value',length(idx_xavail));
