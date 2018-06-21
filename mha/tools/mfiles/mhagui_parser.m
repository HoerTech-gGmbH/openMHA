function h = mhagui_parser(handle, var, pos)
% Show a slider GUI of a scalar component
%
% mhagui_scalar(var, pos [, hostname [, port ] ])
%
%
%
%
  x = pos(1);
  y = pos(2);
  handle.client = [];
  handle.var = var;
  title = var;
  tmp = findstr(title,'.');
  if ~isempty(tmp)
    title(1:max(tmp)) = [];
  end
  info = mha_getinfo(handle, var);
  if ~strcmp(info.type,'parser')
    error(sprintf('Invalid type of variable "%s": "%s"',...
		  var,info.type));
  end
  h = uicontrol('Style','PushButton',...
		'position',[x+160 y+5 220 30],...
		'Callback',@gui_parser_cb,...
		'String','-> open sub-parser',...
		'ToolTipString',info.help);
  set(h,'UserData',handle);
  uicontrol('Style','text',...
	    'FontWeight','bold',...
	    'String',title,...
	    'ToolTipString',info.help,...
	    'Position',[x+2 y+5 156 26]);
  %uicontrol('Style','text',...
%	    'String',info.type,...
%	    'Position',[x+2 y+5 156 26]);
  
  

function gui_parser_cb(varargin)
  h = gcbo;
  d = get(h,'UserData');
  opos = [];
  if ishandle( d.client )
    opos = get(d.client,'Position');
    close(d.client);
  end
  d.client = mhagui_generic(d,d.var);
  if ~isempty(opos)
    npos = get(d.client,'Position'); 
    set(d.client,'Position',[opos(1) opos(2) npos(3) npos(4)]);
  end
  set(h,'UserData',d);
  
