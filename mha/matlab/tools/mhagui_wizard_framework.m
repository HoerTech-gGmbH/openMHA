function mhagui_wizard_framework( csCfg, fh )
  if nargin < 2
    fh = figure;
  else
    figure(fh);
  end
  p = get(fh,'Position');
  delete(get(fh,'Children'));
  delete(findobj('Tag','mhagui_wizard_cancel'));
  delete(findobj('Tag','mhagui_wizard_next'));
  uicontrol(fh,'Style','Pushbutton','String','Cancel',...
	    'Position',[20 20 80 30],...
	    'tag','mhagui_wizard_cancel',...
	    'Callback',@mhagui_wizard_cancel);
  uicontrol(fh,'Style','Pushbutton','String','Start',...
	    'Position',[p(3)-100 20 80 30],...
	    'tag','mhagui_wizard_next',...
	    'Callback',@mhagui_wizard_next);
  uicontrol(fh,'Style','Text','String','',...
	    'Position',[20 p(4)-40 p(3)-40 30],...
	    'tag','mhagui_wizard_title',...
	    'FontSize',14,...
	    'FontWeight','bold',...
	    'BackgroundColor',[0.8 0.8 0.8],...
	    'HorizontalAlignment','left');
  sCfg = struct;
  sCfg.cfg = csCfg;
  sCfg.current = 1;
  sCfg.skip = [];
  sPanel = sCfg.cfg{1};
  if isfield(sPanel,'draw')
    try
      if isfield(sPanel,'drawdata')
	feval(sPanel.draw,sPanel.drawdata);
      else
	feval(sPanel.draw);
      end
    catch
      disp_err
    end
  end
  if isfield(sPanel,'title')
    set(findobj('Tag','mhagui_wizard_title'),'String', ...
		      sPanel.title);
  else
    set(findobj('Tag','mhagui_wizard_title'),'String','');
  end      
  set(findobj('Tag','mhagui_wizard_next'),'UserData',sCfg);
  uiwait(fh);
  
function mhagui_wizard_cancel(varargin)
  fh = gcbf;
  delete(get(fh,'Children'));
  uiresume(fh);

function mhagui_wizard_next(varargin)
  try
  fh = gcbf;
  sCfg = get(findobj('Tag','mhagui_wizard_next'),'UserData');
  set(findobj('Tag','mhagui_wizard_next'),'Enable','off');
  sPanel = sCfg.cfg{sCfg.current};
  % first, execute handler function
  if isfield(sPanel,'callback')
    try
      if isfield(sPanel,'callbackdata')
	feval(sPanel.callback,sPanel.callbackdata);
      else
	feval(sPanel.callback);
      end
    catch
      disp_err
      set(findobj('Tag','mhagui_wizard_next'),'Enable','on');
      return;
    end
  end
  sCfg = get(findobj('Tag','mhagui_wizard_next'),'UserData');
  if sCfg.current >= length(sCfg.cfg)
    delete(get(fh,'Children'));
    uiresume(fh);
    return;
  end
  % cleanup figure:
  vCh = get(fh,'Children');
  for k=1:length(vCh)
    if ~strncmp(get(vCh(k),'Tag'),'mhagui_wizard_',14)
      delete(vCh(k));
    end
  end
  % execute screen function of next step:
  sCfg.current = sCfg.current + 1;
  while any(sCfg.current==sCfg.skip)
    sCfg.current = sCfg.current + 1;
  end
  sCfg.current = min(sCfg.current,length(sCfg.cfg));
  % setup 'next'-button string:
  if sCfg.current == length(sCfg.cfg)
    set(findobj('Tag','mhagui_wizard_next'),'String','Finish');
  elseif sCfg.current ~= 1
    set(findobj('Tag','mhagui_wizard_next'),'String','Next > ');
  end
  if sCfg.current <= length(sCfg.cfg)
    sPanel = sCfg.cfg{sCfg.current};
    if isfield(sPanel,'title')
      set(findobj('Tag','mhagui_wizard_title'),...
	  'BackgroundColor',[0.8 0.8 0.8],...
	  'String', sPanel.title);
    else
      set(findobj('Tag','mhagui_wizard_title'),...
	  'BackgroundColor',[0.8 0.8 0.8],...
	  'String','');
    end      
    if isfield(sPanel,'draw')
      try
	if isfield(sPanel,'drawdata')
	  feval(sPanel.draw,sPanel.drawdata);
	else
	  feval(sPanel.draw);
	end
      catch
	disp_err;
      end
    end
  end
  set(findobj('Tag','mhagui_wizard_next'),'UserData',sCfg);
  set(findobj('Tag','mhagui_wizard_next'),'Enable','on');
  drawnow;
  catch
    disp_err_rethrow;
  end

function disp_err
  err = lasterror;
  disp(err.message);
  for k=1:length(err.stack)
    disp(sprintf('%s:%d %s',...
		 err.stack(k).file,...
		 err.stack(k).line,...
		 err.stack(k).name));
  end
  uiwait(errordlg(err.message));

