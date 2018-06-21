function data = mhagui_waitfor( fh, callback )
% wait for control button or window close, and return figure UserData
% fh : figure handle (optional)
% data : user data, or empty if Cancel or window closed
  
  if nargin < 1
    fh = gcf;
  end
  data = [];
  pos = get(fh,'Position');
  pos = pos(3:4);
  bsize = [80,32];
  uicontrol('Style','PushButton','String','Cancel',...
	    'Position',[pos(1)-200,20,bsize],...
	    'tag','waitfor:cancel',...
	    'Callback',@mhagui_waitfor_cancel);
  h = uicontrol('Style','PushButton','String','Ok',...
		'tag','waitfor:ok',...
		'Callback','uiresume(gcbf)',...
		'Position',[pos(1)-100,20,bsize]);
  if nargin > 1
    callback(fh);
  end
  uiwait(fh);
  if ishandle(fh)
    data = get(fh,'UserData');
    close(fh);
  end
  
function mhagui_waitfor_cancel( varargin )
  close(gcbf);