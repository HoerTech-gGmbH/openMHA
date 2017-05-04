function overlapadd_nozero
  addpath('../../matlab/tools');
  global fontsize;
  fontsize = 18;
  fh = figure('Name','Overlap-add');
  axlab_high = 0.06;
  hborder = 0.02;
  ax1 = axes('position',[hborder 0.5+axlab_high 1-2*hborder 0.5-axlab_high],'Box','on');
  ax2 = axes('position',[hborder axlab_high 1-2*hborder 0.5-axlab_high],'Box','on');
  nplot = 1024;
  nfft = 600;
  nwnd = 600;
  nshift = 300;
  x1 = [1:nwnd]-nwnd;
  x2 = [1:nwnd]-1;
  y1 = (hanning(nwnd)').^0.75;
  y2 = (hanning(nwnd)').^0.25;
  y0 = 1.11;
  dy = 0.16;
  axes(ax1);
  hold on;
  %rectangle('position',[-nshift 0 nshift 1]);
  plot(minmax(x1),[1 1],'k-');
  plot([-nwnd -nwnd],[0 1],'k-');
  patch([-nshift -nshift 0 0],[0 1 1 0],[0.7 0.7 0.7]);
  text(-nshift/2+35,0.87,'from input','horizontalalignment','center','fontsize',fontsize);
  text(-1.5*nshift-42,0.87,'from history','horizontalalignment','center','fontsize',fontsize);
  text_range(-nshift,0,y0,'fragment size',210);
  text_range(-nfft,0,y0+dy,'FFT length = window length',430);
  plot(x1,y1,'k-','linewidth',2.5);
  axes(ax2);
  hold on;
  plot([0 nwnd nwnd],[1 1 0],'k-');
  patch([0 0 nshift nshift],[0 1 1 0],[0.7 0.7 0.7]);
  text(nshift/2,0.5,'to output','horizontalalignment','center','fontsize',fontsize);
  text_range(0,nshift,y0,'fragment size',210);
  plot(x2,y2,'k-','linewidth',2.5);
  plot(x2-nshift,y2,'k:','linewidth',2);
  plot(x2-2*nshift,y2,'k:','linewidth',2);
  plot(x2-3*nshift,y2,'k:','linewidth',2);
  set([ax1;ax2],'ylim',[0 1.5],...
		'ytickmode','manual','ytick',[],...
		'xtickmode','manual',...
		'fontsize',fontsize);
  set(ax1,'xlim',[-nwnd nplot-nwnd],...
	  'xtick',[-2*nshift:nshift:0], ...
	  'xticklabel',{'','','t=0'});
  set(ax2,'xlim',[0 nplot],...
	  'xtick',[0 nshift nwnd],...
	  'xticklabel',{'t=0','','',''});
  saveas(fh,mfilename,'eps');

function text_range(x1,x2,y,t,tl)
  global fontsize;
  dx = 15;
  dy = 0.05;
  tx = mean([x1 x2]);
  th = text(tx,y,t,...
	    'HorizontalAlignment','center',...
	    'VerticalAlignment','middle',...
	    'fontsize',fontsize);
  lim = get(th,'Extent');
  plot([x1 tx-tl/2],[y y],'k-');
  plot([tx+tl/2 x2],[y y],'k-');
  %plot([x1 x2],[y y],'k-');
  plot([x1 x1],[y-dy y+dy],'k-');
  plot([x2 x2],[y-dy y+dy],'k-');
  
