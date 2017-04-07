function [mhah, fh] = mha_transport_gui( mhah, fh, pos )
% [mhah, fh] = mha_transport_gui( mhah, fh, pos )
%
% Show a transport GUI front-end for an MHA. If no argument is given,
% then a connection to a local MHA on port 33337 is opened, otherwise
% 'mhah' is expected to be a valid MHA handle (as returned my mhactl,
% i.e. 'tcp.cmd.handle' from a mha_setup-connection).
%
% The MHA handle 'mhah' and the figure handle 'fh' are returned.
%
% See also 'mha_setup'.
  ;
  mha_transport_gui_data = struct;
  if nargin < 1
    mha_transport_gui_data.h = struct;
    mha_transport_gui_data.h.host = 'localhost';
    mha_transport_gui_data.h.port = 33337;
  else
    mha_transport_gui_data.h = mhah;
  end
  if nargin < 2
    fh = figure('Name','MHA Transport Control',...
		'Position',[100 400 320 30],...
		'ToolBar','none',...
		'MenuBar','none',...
		'Numbertitle','off');
  end
  if nargin < 3
    pos = [0 0];
  end
  pos = [pos(1) pos(2) 0 0];
  mha_transport_gui_data.col1 = [0.8 0.8 0.8];
  mha_transport_gui_data.col2 = [0.7 0.9 0.7];
  mha_transport_gui_data.fh = fh;
  mha_transport_gui_data.c_prepare = ...
      uicontrol(mha_transport_gui_data.fh,'Style','pushbutton',...
		'String','Prepare',...
		'Position',pos+[0 0 60 30],...
		'Callback',@prepare_cb);
  mha_transport_gui_data.c_start = ...
      uicontrol(mha_transport_gui_data.fh,'Style','pushbutton',...
		'String','Start',...
		'Position',pos+[60 0 60 30],...
		'Callback',@start_cb);
  mha_transport_gui_data.c_stop = ...
      uicontrol(mha_transport_gui_data.fh,'Style','pushbutton',...
		'String','Stop',...
		'Position',pos+[120 0 60 30],...
		'Callback',@stop_cb);
  mha_transport_gui_data.c_release = ...
      uicontrol(mha_transport_gui_data.fh,'Style','pushbutton',...
		'String','Release',...
		'Position',pos+[180 0 60 30],...
		'Callback',@release_cb);
  mha_transport_gui_data.c_quit = ...
      uicontrol(mha_transport_gui_data.fh,'Style','pushbutton',...
		'String','kill MHA',...
		'Position',pos+[250 0 70 30],...
		'Callback',@quit_cb);
  %set(findobj(mha_transport_gui_data.fh,'type','uicontrol'),...
  %    'BackgroundColor',mha_transport_gui_data.col1);
  fh = mha_transport_gui_data.fh;
  set(findobj(fh,'type','uicontrol'),'UserData',mha_transport_gui_data);
  get_state( mha_transport_gui_data.c_prepare );
  
function prepare_cb( obj, tmp )
  mha_transport_gui_data = get(obj,'UserData');
  try
    mha_set(mha_transport_gui_data.h,'cmd','prepare');
  catch
    err = lasterror;
    errordlg(err.message,'Error');
  end
  get_state( obj );

function start_cb( obj, tmp )
  mha_transport_gui_data = get(obj,'UserData');
  try
    mha_set(mha_transport_gui_data.h,'cmd','start');
  catch
    err = lasterror;
    errordlg(err.message,'Error');
  end
  get_state( obj );
  
function stop_cb( obj, tmp )
  mha_transport_gui_data = get(obj,'UserData');
  try
    mha_set(mha_transport_gui_data.h,'cmd','stop');
  catch
    err = lasterror;
    errordlg(err.message,'Error');
  end
  get_state( obj );
  
function release_cb( obj, tmp )
  mha_transport_gui_data = get(obj,'UserData');
  try
    mha_set(mha_transport_gui_data.h,'cmd','release');
  catch
    err = lasterror;
    errordlg(err.message,'Error');
  end
  get_state( obj );
  
function quit_cb( obj, tmp )
  mha_transport_gui_data = get(obj,'UserData');
  try
    mha_set(mha_transport_gui_data.h,'cmd','quit');
    close(gcbf);
  catch
    err = lasterror;
    errordlg(err.message,'Error');
  end

function get_state( obj )
  mha_transport_gui_data = get(obj,'UserData');
  try
    state = mha_get(mha_transport_gui_data.h,'state');
    if strcmp( state, 'unprepared' )
      set(mha_transport_gui_data.c_prepare,...
	  'BackgroundColor',mha_transport_gui_data.col1);
    else
      set(mha_transport_gui_data.c_prepare,...
	  'BackgroundColor',mha_transport_gui_data.col2);
    end
    if strcmp( state, 'starting' )|strcmp( state, 'running' )
      set(mha_transport_gui_data.c_start,...
	  'BackgroundColor',mha_transport_gui_data.col2);
    else
      set(mha_transport_gui_data.c_start,...
	  'BackgroundColor',mha_transport_gui_data.col1);
    end
  catch
    errordlg(lasterr,'Error');
  end
