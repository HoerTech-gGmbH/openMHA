function testbox( mha )
% TESTBOX - Virtual test box for MHA compressors
%
% This modul implements a virtual test box with the ISMADHA
% procedure of percentile level estimation.
%
% Author: Giso Grimm
% Date: 6/2009
  ;
  
  if nargin < 1
    mha = [];
  end
  %
  % scan MHA:
  %
  global mha_basic_cfg;
  mha = mha_ensure_mhahandle(mha)
  mha.timeout = 400;
  mha_get_basic_cfg_network( mha );
  mha_basic_cfg.mha.timeout = mha.timeout;
  if ~isfield(mha_basic_cfg.base,'testbox')
    error('No testbox plugin configured.');
  end
  if strcmp(mha_get(mha,'state'),'running')==0
    msg = ['MHA is not running: ',mha_get(mha,'asyncerror')];
    error(msg);
  end

  sParam = mha_get(mha_basic_cfg.mha,mha_basic_cfg.base.testbox, ...
	      'writable');
  sParam = rmfield(sParam,'wait');
  nChannels = mha_get(mha_basic_cfg.mha,...
		      [mha_basic_cfg.base.testbox,'.mhaconfig_in.channels']);
  for fn=fieldnames(sParam)'
    if (~isnumeric(sParam.(fn{:}))) || (prod(size(sParam.(fn{:}))) ~= 1)
      sParam = rmfield(sParam,fn{:});
    end
  end
  
  %
  % setup GUI:
  %
  p = get(0,'ScreenSize');
  w = [560,260+77];
  p = round(0.5*(p(3:4)-w));
  uih = round(0.2*w(2)-10);
  close(findobj('tag','mhagui_testbox'));
  libmhagui();
  fh = mhagui.figure('MHA Test Box','mhagui_testbox',w);

  uicontrol('style','frame','Position',[20,20+57,520,240]);

  fns = fieldnames(sParam)';
  for k=1:length(fns)
    mhagui_scalar(mha,[mha_basic_cfg.base.testbox,'.',fns{k}],[30,250-k*40+57]);
  end
  %uicontrol('style','frame','Position',[560,20+57,140,240]);
  %for kch=1:nChannels
  %  uicontrol('style','checkbox','string',sprintf('channel %d',kch),...
  %	      'Position',[570,250-30*kch+57,120,20],...
  %	      'tag',sprintf('testbox:channel_%d',kch),...
  %	      'Value',1);
  %end
  %
  % Create control buttons (start/save/close...):
  %
  h = uicontrol('style','edit','String','testbox.mat',...
		'BackGroundColor',ones(1,3),...
		'Position',[20,23,130,31]);
  uicontrol('style','pushbutton','String','Save',...
	    'callback',@cb_save_mat,...
	    'UserData',h,...
	    'Position',[170,20,110,37]);
  uicontrol('style','pushbutton','String','Start',...
	    'callback',@cb_start,...
	    'Position',[430,20,110,37]);
  uicontrol('style','pushbutton','String','Close',...
	    'callback','close(gcf);',...
	    'Position',[300,20,110,37]);
  
function cb_save_mat( varargin )
  global mha_basic_cfg;
  fname = get(get(gcbo,'UserData'),'String');
  mha_set(mha_basic_cfg.mha,[mha_basic_cfg.base.testbox, ...
		    '.savemat4'],fname);
  
function cb_start(varargin)
  global mha_basic_cfg;
  bgcol = get(gcbo,'BackgroundColor');
  set(gcbo,'Value',1,'Enable','off',...
	   'BackgroundColor',[0.8,0.2,0.2]);
  drawnow;
  mha_set(mha_basic_cfg.mha,[mha_basic_cfg.base.testbox, ...
		    '.quantiles'],get_quantile_conf);
  mha_set(mha_basic_cfg.mha,[mha_basic_cfg.base.testbox, ...
		    '.start_rec'],'commit');
  mha_set(mha_basic_cfg.mha,[mha_basic_cfg.base.testbox, ...
		    '.wait'],1);
  mL = mha_get(mha_basic_cfg.mha,[mha_basic_cfg.base.testbox, ...
		    '.PercentileLevel']);
  mG = mha_get(mha_basic_cfg.mha,[mha_basic_cfg.base.testbox, ...
		    '.PercentileGain']);
  cf = mha_get(mha_basic_cfg.mha,[mha_basic_cfg.base.testbox, ...
		    '.AnalyzerCf']);
  mSTL = mha_get(mha_basic_cfg.mha,[mha_basic_cfg.base.testbox, ...
		    '.ShorttimeLevel']);
  %sISMADHA = struct;
  %mL = sISMADHA.PercentileLevel';
  %mG = sISMADHA.PercentileGain';
  %cf = sISMADHA.cf;
  %mSTL = sISMADHA.ShorttimeLevel';
  %sISMADHA.ShorttimeBBLevel = mSTL
  idxBands = [3,6,9,12,15]+4;
  nBands = length(cf);
  nChannels = size(mG,1)/nBands;
  p0 = get(0,'ScreenSize');
  sz = [300*nChannels,900];
  pos = round([(p0(3:4)-sz)/2,sz]);
  figure('Position',pos,'NumberTitle','off','MenuBar','none', ...
	 'Name','testbox result','PaperType','A4',...
	 'PaperPosition',[2,2,3*nChannels,9]);
  uicontrol('style','edit','position',[10,10,160,30],...
	    'String','testbox_result','tag','testbox:saveed',...
	    'BackgroundColor',ones(1,3),'HorizontalAlignment','left');
  uicontrol('style','pushbutton',...
	    'Position',[170,10,120,30],...
	    'callback',@save_and_close,...
	    'String','Save EPS & close');
  for kch=1:nChannels
    name = sprintf('channel %d',kch);
    %figure('Name',['Levels ',name]);
    ax = subplot(3,nChannels,kch);
    plot_percentile_level( ax, cf, ...
			   mL([1:nBands]+(kch+nChannels-1)*nBands,:)', ...
			   mL([1:nBands]+(kch-1)*nBands,:)');
    title(['Levels ',name]);
    %figure('Name',['Gains ',name]);
    
    ax = prepare_axes_pg(subplot(3,nChannels,kch+nChannels));
    h = plot(cf,mG([1:nBands]+(kch-1)*nBands,5:-1:1)','k-','linewidth',2.5);
    [qc,sCol] = get_quantile_conf;
    for k=1:4
      set(h(k+(k>2)),'Color',sCol.o.face(k,:));
    end
    title(['Gains ',name]);
    hl = legend(h,num2str(100*qc(5:-1:1)','%g%%\n'),'Location','NorthWest');
    %hl = legend(h,num2str(100*qc(1:5)','%g%%\n'));
    set(hl,'Fontsize',6);
    %figure('Name',['IO ',name]);
    subplot(3,nChannels,kch+2*nChannels);
    plot([-10,120],[-10,120],'k-');
    hold on;
    mSTLin = mSTL(idxBands+(kch+nChannels-1)*nBands,:)';
    mSTLout = mSTL(idxBands+(kch-1)*nBands,:)';
    h = plot(mSTLin,mSTLout,'.');
    vMarker = '*oxdv^';
    for k=1:length(h)
      set(h(k),'Marker',vMarker(k),'MarkerSize',3);
    end
    hl = legend(h,num2str(cf(idxBands)'),'Location','SouthEast');
    set(hl,'Fontsize',6);
    xlim([0,100]);
    ylim([0,100]);
    set(gca,'XTick',[0:20:100],'YTick',[0:20:100]);
    grid on;
    xlabel('Input level / dB SPL');
    ylabel('Output level / dB SPL');
  end
  set(gcbo,'Value',0,'Enable','on','BackgroundColor',bgcol);

function save_and_close( varargin )
  h = findobj(gcbf,'tag','testbox:saveed'); ###?
  name = get(h,'String');
  delete(h);
  set(gcbo,'visible','off');
  drawnow;
  saveas(gcbf,name,'epsc');
  close(gcbf);
  
function [q,sCol] = get_quantile_conf
  q = [0.3,0.5,0.65,0.95,0.99,0.01];
  sCol = struct;
  sCol.o.face = [0,255,119;0,187,153;0,119,187;0,51,221]/255;
  sCol.i.face = 0.5*(sCol.o.face + ones(size(sCol.o.face)));
  sCol.o.pen = 0.5*sCol.o.face;
  sCol.i.pen = 0.5*sCol.i.face;
  
  
function plot_percentile_level( ax,cf, pLin, pLout )
  
%[ax1,ax2] = prepare_axes;
%axes(ax1);
  ax = prepare_axes_pl(ax);
  [q,sCol] = get_quantile_conf;
  fp = [cf,cf(end:-1:1)];
  sQ = struct;
  %for ch='io'
  %  sQ.(ch) = quantile(sTestBox.(ch).L,q);
  %end
  sQ.i = pLin;
  sQ.o = pLout;
  for ch='io'
    if ch=='i'
      sLS = '--';
    else
      sLS = '-';
    end
    %sTestBox.(ch).f
    %sQ.(ch)(6,:)
    plot(cf,sQ.(ch)(6,:),sLS,'linewidth',1+2*(ch=='o'),...
	 'Color',[0,0,0.4]);
    vphl = [];
    for k=1:4
      patch(fp,[sQ.(ch)(k,:),sQ.(ch)(k+1,end:-1:1)],sCol.(ch).face(5-k,:),'LineStyle','none');
      vphl(end+1) = plot(0,0,'-','LineWidth',5,'Color',sCol.(ch).face(5-k,:));
    end
  end
  for ch='io'
    if ch=='i'
      sLS = '--';
    else
      sLS = '-';
    end
    for k=1:4
      plot(cf,sQ.(ch)(k+(k>2),:),sLS,'Color',sCol.(ch).pen(5-k,:));
    end
    plot(cf,sQ.(ch)(3,:),['k',sLS]);
  end
  csLeg = {};
  for k=1:4
    csLeg{k} = sprintf('%g-%g%%',100*q(k),100*q(k+1));
  end
  lh = legend(vphl(end:-1:1),csLeg(end:-1:1),'Location','NorthWest');
  set(lh,'FontSize',6);
  %%plot(data.f,data.Lout(3,:),'c-','linewidth',2);
  %vX = [4300,4300,6000,6000];
  %vY = [0,4,4,0];
  %patch([4000,4000,15000,15000],[100,123,123,100],ones(1,3));
  %for k=1:4
  %  y = 100+5*k-3;
  %  patch(vX,vY+y,sCol.o.face(5-k,:));
  %  text(max(vX)*1.05,y+2.5,sprintf('%g-%g%%',100*q(k),100*q(k+1)));
  %end

function [vIsoThrDB, vsF] = isothr(vsDesF);
% [vIsoThrDB, vsF] = isothr(vsDesF);
%
% author: Jens-E. Appell
%
% values from 20 Hz to 12500 Hz are taken from ISO 226 (1985-05-01)
% values at 14000 Hz and 15000 Hz are taken from ISO-Threshold-table
% in Klaus Bethges thesis.
% values at 0 and 20000 Hz are not taken from ISO Threshold contour !!
  vThr = [ 80    74.3 65.0  56.3   48.4 41.7 35.5  29.8 25.1 20.7 ...
           16.8 13.8 11.2 8.9   7.2 6.0 5.0  4.4 4.2 3.8  2.6 1.0 ...
           -1.2 -3.6 -3.9 -1.1 6.6 15.3 16.4 11.6    16.0 24.1    70.0]';
  vsF  =1000*[0.0    0.02 0.025 0.0315 0.04 0.05 0.063 0.08 0.1 ...
              0.125 0.16 0.2  0.25 0.315 0.4 0.5 0.63 0.8 1.0 1.25 ...
              1.6 2.0 2.5  3.15 4.0  5.0  6.3 8.0  10.  12.5    14.0 ...
              15.0    20.0]';
  if( ~isempty( find( vsDesF < 50 ) ) )
    warning('frequency values below 50 Hz set to 50 Hz');
    vsDesF(find( vsDesF < 50 )) = 50;
  end
  if nargin > 0,
    vIsoThrDB = interp1(vsF,vThr,vsDesF,'linear','extrap');
    vsF = vsDesF;
  else,
    vIsoThrDB = vThr;
  end;

function ax = prepare_axes_pl( ax )
  if nargin < 1
    ax = axes;
  end
  hold off;
  f = 1000*2.^[-4:1/3:4];
  patch([f(1),f,f(end)],...
	[-10,isothr(f),-10],0.7*ones(1,3));
  hold on;
  fx = [[100,200,500],1000*2.^[0:3]];
  set(ax,'tag','testbox_axes','XScale','log',...
	 'XLim',minmax(f),...
	 'XTick',fx,...
	 'XTickLabel',0.001*fx,...
	 'YLim',[-10 105]);
  xlabel('frequency / kHz');
  ylabel('target signal level / dB SPL');
  grid on;
  %%

function ax = prepare_axes_pg( ax )
  if nargin < 1
    ax = axes;
  end
  f = 1000*2.^[-4:1/3:4];
  hold off;
  plot(minmax(f),[0,0],'k-');
  hold on;
  fx = [[100,200,500],1000*2.^[0:3]];
  set(ax,'tag','testbox_axes','XScale','log',...
	 'XLim',minmax(f),...
	 'XTick',fx,...
	 'XTickLabel',0.001*fx);
  xlabel('frequency / kHz');
  ylabel('target signal gain / dB');
  grid on;
  %%

function mm = minmax(x)
  mm = [min(x),max(x)];