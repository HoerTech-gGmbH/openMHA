function fh = mhagui_figure( name, tag, wsize, varargin )
% create a figure handle, remove menu bar and set position
%
% name  : window name
% tag   : window tag (used for identification in position database)
% wsize : window size (position is taken from database, or centered
%         on screen if no entry is found)
%
% Author: Giso Grimm, 3/2011
%
  ScreenSize = get(0,'ScreenSize');
  %cfdb = libconfigdb();
  %bmp = cfdb.readfile('mha_ini.mat','bitmap.logo_uol_htch',ones(0,0,0));
  %bm_y = size(bmp,1);
  %bm_x = size(bmp,2);
  %if bm_y > 0
  %  wsize(2) = wsize(2) + bm_y;
  %  bmp = [repmat(bmp(:,1,:),[1,max(1,wsize(1)-bm_x),1]),bmp];
  %end
  wsize = round([ScreenSize(3:4)/2-wsize/2,wsize]);

  fh = figure('Name',name,...
	      'tag',tag,...
	      'NumberTitle','off','Menubar','none',...
	      'Position',wsize,...
	      'DeleteFcn',@mhagui_store_windowpos,...
	      varargin{:});
  mhagui_restore_windowpos(fh);
  %if bm_y > 0
  %  image(bmp);
  %  set(gca,'Units','normalized','visible','off','Position',[0,(wsize(4)-bm_y)/wsize(4),1,bm_y/wsize(4)]);
  %  %set(gca,'Position',[0,wsize(2)-bm_y,wsize(1),bm_y],...
  %  %	    'visible','on');
  %end