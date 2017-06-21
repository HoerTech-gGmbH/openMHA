function h = mhagui_edit(handle, var, pos)
% Create a generic MHA variable edit field. The character
% representation of the variable can be edited.
%
% h = mhagui_edit( handle, var, pos )
%
% Input arguments:
% - handle   : MHA handle
% - var      : Node name of the MHA variable to be shown, can 
%              to be of any type.
% - pos      : Position vector ([x y] or [x y w]) of left bottom corner.
%
% Return value:
% - h        : Uicontrol handle of edit field panel.
%
% (c) 2005 Universitaet Oldenburg, Germany
%          Author: Giso Grimm
%
  x = pos(1);
  y = pos(2);
  w = 80;
  if length(pos) > 2
    w = pos(3);
  end
  handle.var = var;
  title = var;
  tmp = findstr(title,'.');
  if ~isempty(tmp)
    title(1:max(tmp)) = [];
  end
  info = mha_getinfo(handle,var);
  if strcmp(info.perm,'writable')
    bEnabled = 'on';
  else
    bEnabled = 'off';
  end
  h = uicontrol('Style','Edit',...
		'position',[x+160 y+5 w 30],...
		'Callback',@gui_edit_cb,...
		'BackgroundColor',ones(1,3),...
		'String',info.val,...
		'Tag',var,...
		'Enable',bEnabled,...
		'HorizontalAlignment','left',...
		'ToolTipString',info.help);
  set(h,'UserData',handle);
  uicontrol('Style','text',...
	    'FontWeight','bold',...
	    'String',title,...
	    'Tag',['label:',var],...
	    'Position',[x+2 y+5 156 26],...
	    'ToolTipString',info.help);
  %uicontrol('Style','text',...
  %	    'String',info.type,...
  %	    'Position',[x+2 y+1 156 19]);

  
function gui_edit_cb(varargin)
  h = gcbo;
  d = get(h,'UserData');
  try
    mha_set(d, d.var, get(h,'String'));
  catch
    errordlg(lasterr,'MHA Error');
  end
  val = mhactl_wrapper(d,[d.var '?val']);
  val = val;
  set(h,'String', val);
