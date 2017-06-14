function h = mhagui_vfloatwrap(handle, var, pos)
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
  sinfo = mha_getinfo(handle, var );
  if ~strcmp(sinfo.type,'vector<float>')
    error(sprintf('Invalid type of variable "%s": "%s"',...
		  var,sinfo.type));
  end
  h = uicontrol('Style','PushButton',...
		'position',[x+160 y+5 220 30],...
		'Callback',@gui_parser_cb,...
		'String','-> open vector<float> control',...
		'ToolTipString',sinfo.help);
  set(h,'UserData',handle);
  uicontrol('Style','text',...
	    'FontWeight','bold',...
	    'String',title,...
	    'Position',[x+2 y+5 156 26]);
  %uicontrol('Style','text',...
  %	    'String',sinfo.type,...
  %	    'Position',[x+2 y+1 156 19]);
  
  

function gui_parser_cb(varargin)
  h = gcbo;
  d = get(h,'UserData');
  opos = [];
  if ishandle( d.client )
    opos = get(d.client,'Position');
    close(d.client);
  end
  d.client = mhagui_vfloat(d.var,d);
  if ~isempty(opos)
    npos = get(d.client,'Position'); 
    set(d.client,'Position',[opos(1) opos(2) npos(3) npos(4)]);
  end
  set(h,'UserData',d);
