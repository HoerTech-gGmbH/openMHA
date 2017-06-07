function sResults = loctest( sExperiment, sSubject, sTag )
% LOCTEST - localization test method
%
% Usage:
% loctest( sExperiment, sSubject, sTag )
%
% - sExperiment : configuration script (e.g., 'loctest_example')
%
% - sSubject    : subject name (optional if set by 
%                 configuration script)
% - sTag        : tag for data logging (optional, can be set in
%                 configuration script)
%
% Author: Giso Grimm
% Date: Aug 2012

  ;
  % do not change code in this file - use one of the following
  % files instead:
  %
  % - the experiment configuration (e.g., loctest_example.m)
  % - the sound prepare/play/release file, as configured in the
  %   experiment configuration file (e.g., loctest_sound_play)
  hSD = libsd();
  eval( sExperiment );

  guipos = get(0,'ScreenSize');
  %sz = (0.75*min(guipos(3:4)))/guipos(3:4); 
  sz = round([1,1.1]*(0.7*min(guipos(3:4))));
  pos = round(0.5*(guipos(3:4)-sz));
  
  guipos = [pos,sz];
  sCfg = defv( sCfg, 'sound', struct );
  sCfg.sound = defv( sCfg.sound, 'prepare', @nofun );
  sCfg.sound = defv( sCfg.sound, 'release', @nofun );
  sCfg.sound = defv( sCfg.sound, 'play', @nofun );
  sCfg = defv( sCfg, 'repetitions', 1 );
  sCfg = defv( sCfg, 'randomized', true );
  sCfg = defv(sCfg,'gui',struct);
  sCfg.gui = defv( sCfg.gui, 'az', [0:45:315] );
  sCfg.gui = defv( sCfg.gui, 'r', 1 );
  sCfg.gui = defv( sCfg.gui, 'label', {'1','2','3','4','5','6','7','8'} );
  sCfg.gui = defv( sCfg.gui, 'pos', guipos );
  sCfg.gui = defv( sCfg.gui, 'axpos', [0,0.1,1,0.9] );
  sCfg.gui = defv( sCfg.gui, 'buttonpos', [0,0,1,0.1] );
  sCfg.gui = defv( sCfg.gui, 'marker',{});
  sCfg.gui = defv( sCfg.gui, 'linewidth',2);
  sCfg.gui = defv( sCfg.gui, 'markersize',24);
  sCfg.gui = defv( sCfg.gui, 'timeout',3600);
  sCfg.gui = defv( sCfg.gui, 'xlim',[-1.4,1.4]);
  sCfg.gui = defv( sCfg.gui, 'ylim',[-1.4,1.4]);
  sCfg.gui = defv( sCfg.gui, 'headsize',1);
  sCfg.gui = defv( sCfg.gui, 'headthickness',1);
  sCfg.gui = defv( sCfg.gui, 'statusdisplay',true);
  if nargin >= 2
    sCfg = defv( sCfg, 'subject', sSubject );
  end
  if nargin >= 3
    sCfg = defv( sCfg, 'tag', sTag );
  end
  sCfg.startdate = datestr(now,30);
  sCfg.figurehandle = ...
      figure('Name',...
	     ['localization test ',sCfg.startdate,' ',...
	      sCfg.subject,' ',sCfg.tag],...
	     'NumberTitle','off','menubar','none',...
	     'Position',sCfg.gui.pos);
  
  disp(['loctest_',sCfg.startdate,'_',sCfg.subject,'_',sCfg.tag]);
  sPar = struct();
  sPar.fields = fieldnames(sCfg.par)';
  sPar.fields{end+1} = 'resp_az';
  sPar.fields{end+1} = 'resp_r';
  sPar.fields{end+1} = 'resp_time';
  sPar.values = struct2cell(sCfg.par)';
  
  % prepare audio system:
  sCfg.sound.prepare( sCfg );
  % wait for click:
  hbutt = uicontrol( 'style','pushbutton','Units','normalized', ...
		     'position',[0.2,0.2,0.6,0.6],...
		     'String','Start','callback','delete(gcbo);',...
		     'FontSize',30);
  waitfor(hbutt);
  sResults = hSD.eval( sPar, @loctest_eval, ...
		       'display',sCfg.gui.statusdisplay,...
		       'nrep',sCfg.repetitions, ...
		       'brand', sCfg.randomized,...
		       'structarg',true,...
		       'param',sCfg);
  %sResults = hSD.rmnan(sResults);
  % release audio system:
  sCfg.sound.release( sCfg );
  sResults.config = sCfg;
  struct2mfile( sResults, ['loctest_',sCfg.startdate,'_',sCfg.subject,'_',sCfg.tag]);
  close(sCfg.figurehandle);
  
function [az,r,t] = loctest_gui( sCfg )
  az = nan;
  r = nan;
  figure(sCfg.figurehandle);
  ax = axes('Position',sCfg.gui.axpos,'XTick',[],'YTick',[],...
	    'XLim',sCfg.gui.xlim,'YLim',sCfg.gui.ylim,'DataAspectRatio',[1,1,1],...
	    'ButtonDownFcn',@loctest_click,'UserData',[az,r], ...
	    'NextPlot','replacechildren');
  if isfield( sCfg.gui,'bitmap')
    img = imread(sCfg.gui.bitmap);
    xl = get(ax,'XLim');
    yl = get(ax,'YLim');
    vX = linspace(xl(1),xl(2),size(img,1));
    vY = linspace(yl(2),yl(1),size(img,2));
    image(vX,vY,img,'ButtonDownFcn',@loctest_click);
    hold on;
  end
  hp = [];
  % draw dummy head:
  if sCfg.gui.headsize > 0
    vAz = [0:pi/100:2*pi];
    z = 0.05*exp(i*vAz).*(5-cos(2*vAz));
    hp(end+1) = plot(sCfg.gui.headsize*z,'k-','linewidth', ...
		     sCfg.gui.headthickness);
    hold on;
    % nose
    hp(end+1) = plot(sCfg.gui.headsize*[-0.03,0,0.03],...
		     sCfg.gui.headsize*[0.30,0.38,0.3],'k-','linewidth', ...
		     sCfg.gui.headthickness);
    % ears
    hp(end+1) = plot(sCfg.gui.headsize*[0.22,0.18,0.22],...
		     sCfg.gui.headsize*[-0.036,0,0.036],'k-','linewidth', ...
		     sCfg.gui.headthickness);
    hp(end+1) = plot(-sCfg.gui.headsize*[0.22,0.18,0.22],...
		     sCfg.gui.headsize*[-0.036,0,0.036],'k-','linewidth', ...
		     sCfg.gui.headthickness);
  end
  hp(end+1) = plot([-20,-20],[-20,20],'r-',...
		   'color',[0.5,0,0],'linewidth',5,'tag','respmarker');
  hold on;
  hp(end+1) = plot([-20,-20],[-20,20],'r-',...
		   'color',[0.5,0,0],'linewidth',5,'tag','respmarkerarr');
  labr = sCfg.gui.r;
  if numel(sCfg.gui.r) < numel(sCfg.gui.label)
    labr(end+1:numel(sCfg.gui.label)) = labr(end);
  end
  for k = 1:numel(sCfg.gui.az)
    % add label:
    if numel(sCfg.gui.label) >= k
      ht = text(-labr(k)*sin(pi*sCfg.gui.az(k)/180),labr(k)*cos(pi*sCfg.gui.az(k)/180),...
		sCfg.gui.label{k},...
		'VerticalAlignment','middle','HorizontalAlignment','center',...
		'ButtonDownFcn',@loctest_click);
      if isfield( sCfg.gui,'fontsize')
	set(ht,'fontsize',sCfg.gui.fontsize);
      end
    end
    % add marker:
    if (numel(sCfg.gui.marker) >= k) && ...
	  (~isempty(sCfg.gui.marker{k}))
      hp(end+1) = plot(-labr(k)*sin(pi*sCfg.gui.az(k)/180),...
		       labr(k)*cos(pi*sCfg.gui.az(k)/180),...
		       sCfg.gui.marker{k},...
		       'markersize',sCfg.gui.markersize,...
		       'linewidth',sCfg.gui.linewidth);
    end
  end
  set(hp,'ButtonDownFcn',@loctest_click);
  hbutt = uicontrol( 'style','pushbutton','Units','normalized', ...
		     'position',sCfg.gui.buttonpos,...
		     'String','ok','callback','delete(gcbo);',...
		     'fontsize',sCfg.gui.fontsize);
  drawnow;
  tic;
  while ishandle(hbutt)
    pause(0.01);
    if toc > sCfg.gui.timeout
      delete(hbutt);
    end
  end
  t = toc;
  if ~ishandle(sCfg.figurehandle)
    warning( 'Window closed by user - measurement may be corrupted' );
  else
    ud = get(ax,'UserData');
    az = ud(1);
    r = ud(2);
    delete( get(sCfg.figurehandle,'children'));
    drawnow();
  end
  
function err = loctest_click( ax, b )
  xp = get(gca,'CurrentPoint');
  az = atan2(-xp(1,1),xp(1,2))/pi*180;
  r = sqrt(xp(1,1)^2+xp(1,2)^2);
  pdata = ([20,r]'*[cos(az*pi/180),sin(az*pi/180)])';
  h = findobj(gca,'tag','respmarker');
  har = findobj(gca,'tag','respmarkerarr');
  set(h,'XData',-pdata(2,:),'YData',pdata(1,:));
  pdata_arr = [0.2,0,0.2].*exp(i*(az+[45,0,-45])*pi/180);
  set(har,'XData',-imag(pdata_arr)-pdata(2,2),...
	  'YData',real(pdata_arr)+pdata(1,2));
  set(gca,'UserData',[az,r]);
  
function [az, r, t] = loctest_eval( sPar, sCfg )
  az = nan;
  r = nan;
  t = nan;
  if sCfg.sound.play( sPar, sCfg )
    [az,r,t] = loctest_gui( sCfg );
  end
  
function x = nofun( varargin )
  x = true;
  
function s = defv( s, n, v )
  if ~isfield(s,n)
    s.(n) = v;
  end
  
