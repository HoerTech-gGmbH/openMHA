function h = mhagui_keyword( handle, var, pos, callback )
% Create a generic MHA selection box.
%
% h = mhagui_keyword(handle, var, pos)
%
% Input arguments:
% - handle   : MHA connection handle struct, containing the fields
%              'host' and 'port'
% - var      : Node name of the MHA variable to be shown, can 
%              to be of type 'keyword_list'.
% - pos      : Position vector ([x y] or [x y w]) of left bottom
%              corner.
%
% Optional argument:
% - callback : Callback function handle to be called (with same
%              argument as mha_set function)
%
% Return value:
% - h        : Uicontrol handle of selection box.
%
% (c) 2005 Universitaet Oldenburg, Germany
%          Author: Giso Grimm
%
  if length(pos) < 3
    pos(3) = 80;
  end
  if nargin >= 4
    handle.gui_callback = callback;
  end
  x = pos(1);
  y = pos(2);
  w = pos(3);
  info = mha_getinfo(handle,var);
  if ~strcmp(info.type,'keyword_list')
    error(sprintf('The tye of variable %s is %s, not keyword_list.',...
		  var,info.type));
  end
  if strcmp(info.perm,'writable')
    bEnabled = 'on';
  else
    bEnabled = 'off';
  end
  info.range = mha_mha2matlab('vector<string>',info.range);
  h = uicontrol('Style','popupmenu',...
		'Position',[x+160 y+5 w 30],...
		'Tag',var,...
		'String',info.range,...
		'Value',strmatch(info.val,info.range,'exact'),...
		'Enable',bEnabled,...
		'Callback',@select_chain,...
		'ToolTipString',info.help);
  set(h,'UserData',handle);
  pos = get(h,'Position');
  title = var;
  tmp = findstr(title,'.');
  if ~isempty(tmp)
    title(1:max(tmp)) = [];
  end
  uicontrol('Style','text',...
	    'Fontweight','bold',...
	    'Tag',['label:',var],...
	    'String',title,...
	    'ToolTipString',info.help,...
	    'position',[x+2 y+5 156 26]);
  if isfield( handle, 'gui_callback' )
    feval( handle.gui_callback, var, info.val );
  end

function select_chain(varargin)
  h = gcbo;
  d = get(h,'UserData');
  chains = get(h,'String');
  chain = chains{get(h,'Value')};
  var = get(h,'Tag');
  try
    mha_set(d,var,chain);
    if isfield( d, 'gui_callback' )
      feval( d.gui_callback, var, chain );
    end
  catch
    errordlg(lasterr,'MHA Error');
  end
