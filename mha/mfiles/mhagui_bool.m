function h = mhagui_bool(handle, var, pos)
% Create a generic MHA bool variable field.
%
% h = mhagui_scalar(var, pos, hostname, port)
%
% Input arguments:
% - var      : Node name of the MHA variable to be shown, can 
%              to be of type 'bool'.
% - pos      : Position vector ([x y]) of left bottom corner.
% - hostname : Name of the host running the MHA server.
%              (default: 'localhost')
% - port     : Port number on which the MHA server is listening.
%              (default: 33337)
%
% Return value:
% - h        : Uicontrol handle of bool field panel.
%
% (c) 2005 Universitaet Oldenburg, Germany
%          Author: Giso Grimm
%
  x = pos(1);
  y = pos(2);
  if length(pos) > 2
    gui_w = pos(3);
  else
    gui_w = 180;
  end
  handle.var = var;
  title = var;
  tmp = findstr(title,'.');
  if ~isempty(tmp)
    title(1:max(tmp)) = [];
  end
  info = mha_getinfo(handle,var);
  if ~strcmp(info.type,'bool')
    error(sprintf('Variable %s is not bool.',var));
  end
  h = uicontrol('Style','ToggleButton',...
		'position',[x+160 y+5 gui_w 30],...
		'Callback',@gui_bool_cb,...
		'String',info.val,...
		'Value',strcmp(info.val,'yes'),...
		'ToolTipString',info.help);
  set(h,'UserData',handle);
  uicontrol('Style','text',...
	    'FontWeight','bold',...
	    'String',title,...
		'ToolTipString',info.help,...
	    'Position',[x+2 y+5 156 26]);
  %%uicontrol('Style','text',...
  %%	    'String',info.type,...
  %%		'ToolTipString',info.help,...
  %%	    'Position',[x+2 y+1 156 19]);

  
function gui_bool_cb(varargin)
  h = gcbo;
  d = get(h,'UserData');
  mha_set(d,d.var,get(h,'Value'));
  val = mha_get(d,d.var);
  set(h,'Value',val);
  if val
    set(h,'String','yes');
  else
    set(h,'String','no');
  end
