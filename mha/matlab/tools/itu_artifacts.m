function q = itu_artifacts( task, labels, ylim )
% ITU_ARTIFACTS - display an absolute rating GUI and return user response
%
% Usage:
% q = itu_artifacts( task, labels, ylim )
%
% Date: 2006, 6/2012
% Author: Giso Grimm
  mhagui = libmhagui();
  if nargin < 1
    task = 'Beurteilen Sie die Artefakte des nachfolgenden Signals.';
  end
  if nargin < 2
    labels = {'nicht wahrnehmbar',...
	      ['wahrnehmbar, aber',10,'nicht störend'], ...
	      'leicht störend',...
	      'störend',...
	      'sehr störend', ...
	      ''};
    % 228 246 252 äöü
  end
  if nargin < 3
    ylim = [0 length(labels)-1];
  end
  global itu_artifacts_ok;
  itu_artifacts_ok = 0;
  headh = 70;
  ax_size = [340 420];
  buttonh = 50;
  ax_dist = 20;
  wsize = [ax_size(1) 2*ax_dist+headh+buttonh+ax_size(2)];
  fh = findobj('tag','itu_artifacts');
  if isempty(fh)
    fh = mhagui.figure('ITU artifacts','itu_artifacts',wsize,...
		       'CloseRequestFcn',@itu_cancel);
  else
    close(fh(2:end));
    delete(get(fh,'Children'));
  end
  figure(fh);
  drawnow;
  ax = mhagui.rating_axes(labels,...
			  'Units','pixel',...
			  'position',[1 buttonh+ax_dist ax_size]);
  %q = ylim(1)-ylim(2);
  uicontrol(fh,'style','pushbutton','String','Ok',...
	    'callback',@itu_ok,...
	    'Position',[0 0 ax_size(1) buttonh])
  uicontrol(fh,'style','frame',...
	    'BackgroundColor',[0.7 0.9 0.7],...
	    'Position',[1 buttonh+2*ax_dist+ax_size(2) ax_size(1)-1 headh])
  uicontrol(fh,'style','text','String',task,...
	    'Fontsize',14,'Fontweight','bold',...
	    'BackgroundColor',[0.7 0.9 0.7],...
	    'Position',[2 buttonh+2*ax_dist+ax_size(2)+1 ax_size(1)-3 ...
		    headh-10])
  while ~itu_artifacts_ok
    pause(0.1);
  end
  if itu_artifacts_ok == 1
    sData = get(ax,'UserData');
    q = sData.rating;
    q = q/(numel(labels)-1)*diff(ylim)+ylim(1);
  else
    q = nan;
  end
  
function itu_ok( varargin )
  global itu_artifacts_ok;
  itu_artifacts_ok = 1;

function itu_cancel( varargin )
  global itu_artifacts_ok;
  itu_artifacts_ok = 2;
  closereq();