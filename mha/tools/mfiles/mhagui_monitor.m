function h = mhagui_monitor(handle, var, pos)
% Create a generic MHA monitor display field.
%
% h = mhagui_monitor(handle, var, pos)
%
% Input arguments:
% - var      : Node name of the MHA variable to be shown, can 
%              to be of any type.
% - pos      : Position vector ([x y] or [x y w]) of left bottom corner.
% - hostname : Name of the host running the MHA server.
%              (default: 'localhost')
% - port     : Port number on which the MHA server is listening.
%              (default: 33337)
%
% Return value:
% - h        : Uicontrol handle of edit field panel.
%
% (c) 2005 Universitaet Oldenburg, Germany
%          Author: Giso Grimm
%
  x = pos(1);
  y = pos(2);
  handle.var = var;
  title = var;
  tmp = findstr(title,'.');
  if ~isempty(tmp)
    title(1:max(tmp)) = [];
  end
  info = mha_getinfo(handle,var);
  h = uicontrol('Style','text',...
		'position',[x+160 y+5 275 30],...
		'BackgroundColor',[0.9 1 0.9],...
		'String','',...
		'HorizontalAlignment','left',...
		'ToolTipString',info.help);
  handle.button = h;
  set(h,'UserData',handle);
  uicontrol('Style','Pushbutton',...
	    'Position',[x+440 y+5 55 30],...
	    'Callback',@gui_edit_cb,...
	    'String','Reload',...
	    'UserData',handle,...
		'ToolTipString',info.help);
  d = get(h,'UserData');
  v = mhactl_wrapper(d, [d.var '?val']);
  set(h,'String', v);
  uicontrol('Style','text',...
	    'FontWeight','bold',...
	    'String',title,...
	    'ToolTipString',info.help,...
	    'ForegroundColor',[0 0.4 0],...
	    'Position',[x+2 y+5 156 26]);
  %uicontrol('Style','text',...
  %	    'String',info.type,...
  %	    'Position',[x+2 y+1 156 19]);

  
function gui_edit_cb(varargin)
  h = gcbo;
  d = get(h,'UserData');
  v = mhactl_wrapper(d, [d.var '?val']);
  set(d.button,'String', v);
