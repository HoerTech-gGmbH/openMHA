function h = mhagui_scalar(handle, var, pos, user_range)
% Create a generic MHA variable slider field for scalar types (int,
% float).
%
% h = mhagui_scalar(handle, var, pos)
%
% Input arguments:
% - handle   : Handle of the running MHA server.
% - var      : Node name of the MHA variable to be shown, can 
%              to be of type 'int' or 'float'.
% - pos      : Position vector ([x y]) of left bottom corner.
%
% Return value:
% - h        : Uicontrol handle of slider field panel.
%
% (c) 2005 Universitaet Oldenburg, Germany
%          Author: Giso Grimm
%
  x = pos(1);
  y = pos(2);
  if length(pos) > 2
    gui_w = pos(3);
  else
    gui_w = 340;
  end
  title = var;
  handle.var = var;
  tmp = findstr(title,'.');
  if ~isempty(tmp)
    title(1:max(tmp)) = [];
  end
  if nargin < 4
    user_range = [];
  end
  info = mha_getinfo(handle,var);
  if strcmp(info.type,'float')
    cb = @gui_value_callback;
    cb_edit = @gui_edit_value_callback;
  elseif strcmp(info.type,'int')
    cb = @gui_value_int_callback;
    cb_edit = @gui_edit_value_int_callback;
  else
    error(sprintf('Invalid type of variable "%s": "%s"',...
		  var,info.type));
  end
  info.val = str2num(info.val);
  [v_min, v_max] = mhagui_rangestr2range( info.range, info.val );
  if ~isempty(user_range)
    v_min = user_range(1);
    v_max = user_range(2);
  end
  if strcmp(info.type,'float')
    stepsize = [0.01 0.1];
  else
    stepsize = [min(1,1/(v_max-v_min)) min(1,10/(v_max-v_min))];
  end
  if strcmp(info.perm,'writable')
    bEnabled = 'on';
  else
    bEnabled = 'off';
  end
  h = uicontrol('Style','slider',...
		'position',[x+160 y+3 gui_w 16],...
		'Callback',cb,...
		'Tag',var,...
		'ToolTipString',info.help,...
		'SliderStep',stepsize,...
		'Enable',bEnabled,...
		'Min',v_min,'Max',v_max);
  guidata(h,handle);
  set(h,'Value',min(max(info.val,v_min),v_max));
  uicontrol('Style','text',...
	    'FontWeight','bold',...
	    'Tag',['label:',var],...
	    'String',title,...
	    'ToolTipString',info.help,...
	    'Position',[x+2 y+5 156 26]);
  uicontrol('Style','edit',...
	    'String',sprintf('%g',info.val),...
	    'Position',[x+160 y+19 gui_w 20],...
	    'Callback',cb_edit,...
	    'BackgroundColor',ones(1,3),...
	    'UserData',handle,...
	    'Enable',bEnabled,...
	    'ToolTipString',info.help,...
	    'Tag',[var '=str']);

function gui_value_callback(varargin)
  h = gcbo;
  d = guidata(h);
  var = get(h,'Tag');
  val = get(h,'Value');
  try
    mha_set(d,var,val);
  catch
    errordlg(lasterr,'MHA Error');
  end
  h = findobj(gcbf,'Tag',[var '=str']);
  set(h,'String',sprintf('%g',val));

function gui_value_int_callback(varargin)
  h = gcbo;
  d = guidata(h);
  var = get(h,'Tag');
  val = round(get(h,'Value'));
  set(h,'Value',val);
  try
    mha_set(d,var,val);
  catch
    errordlg(lasterr,'MHA Error');
  end
  h = findobj(gcbf,'Tag',[var '=str']);
  set(h,'String',sprintf('%g',val));

function gui_edit_value_callback(varargin)
  h = gcbo;
  d = get(h,'UserData');
  val = str2num(get(h,'String'));
  try
    mha_set(d,d.var,val);
  catch
    errordlg(lasterr,'MHA Error');
  end
  val = mha_get(d,d.var);
  set(h,'String',sprintf('%g',val));
  h = findobj(gcbf,'Tag',d.var);
  set_val_in_range( h, val );

function gui_edit_value_int_callback(varargin)
  h = gcbo;
  d = get(h,'UserData');
  val = str2num(get(h,'String'));
  set(h,'Value',val);
  try
    mha_set(d,d.var,val);
  catch
    errordlg(lasterr,'MHA Error');
  end
  val = mha_get(d,d.var);
  set(h,'String',sprintf('%g',val));
  h = findobj(gcbf,'Tag',d.var);
  set_val_in_range( h, val );
  stepsize = [min(1,1/(get(h,'max')-get(h,'min'))) min(1,10/(get(h,'max')-get(h,'min')))];
  set(h,'SliderStep',stepsize);
  
function set_val_in_range( h, val )
  vmin = get(h,'Min');  
  vmax = get(h,'Max');
  if val > vmax
    set(h,'Max',val);
  end
  if val < vmin
    set(h,'Min',val);
  end
  set(h,'value',val);
