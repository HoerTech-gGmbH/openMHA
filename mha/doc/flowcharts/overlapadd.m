function overlapadd
  addpath('../../matlab/tools');
  global fontsize;
  fontsize = 18;
  fh = figure('Name','Overlap-add');
  axlab_high = 0.06;
  hborder = 0.02;
  ax1 = axes('position',[hborder 0.5+axlab_high 1-2*hborder 0.5-axlab_high],'Box','on');
  ax2 = axes('position',[hborder axlab_high 1-2*hborder 0.5-axlab_high],'Box','on');
  nfft = 1024;
  nwnd = 600;
  nshift = 300;
  pad = floor((nfft-nwnd)/2);
  pad = [pad nfft-pad-nwnd];
  x1 = [1:nfft]-pad(1)-nwnd;
  x2 = [1:nfft]-1;
  y1 = [zeros(1,pad(1)) hanning(nwnd)' zeros(1,pad(2))];
  ramplen = round(0.7 * pad);
  ramp1 = hanning(2*ramplen(1))';
  ramp2 = hanning(2*ramplen(2))';
  y2 = [ramp1(1:ramplen(1)) ones(1,pad(1)-ramplen(1)) ...
	ones(1,nwnd) ones(1,pad(2)-ramplen(2)) ...
	ramp2(end-ramplen(2)+1:end)];
  y0 = 1.11;
  dy = 0.15;
  axes(ax1);
  hold on;
  %rectangle('position',[-nshift 0 nshift 1]);
  plot(minmax(x1),[1 1],'k-');
  plot([-nwnd -nwnd],[0 1],'k-');
  patch([-nshift -nshift 0 0],[0 1 1 0],[0.7 0.7 0.7]);
  %patch([-2*nshift -2*nshift -nshift -nshift],[0 1 1 0],[1 1 1]);
  %patch([0 0 pad(2) pad(2)],[0 1 1 0],[1 1 1]);
  %patch([-nwnd-pad(1) -nwnd-pad(1) -nwnd -nwnd],[0 1 1 0],[1 1 1]);
  text(pad(2)/2,0.87,'zeros','horizontalalignment','center','fontsize',fontsize);
  text(-nshift/2+30,0.87,'from input','horizontalalignment','center','fontsize',fontsize);
  text(-1.5*nshift-30,0.87,'from history','horizontalalignment','center','fontsize',fontsize);
  text(-nwnd-0.5*pad(1),0.87,'zeros','horizontalalignment','center','fontsize',fontsize);
  text_range(-nshift,0,y0,'fragment size',210);
  text_range(-nwnd,0,y0+dy,'window length',231);
  text_range(-nfft+pad(1),pad(2),y0+2*dy,'FFT length',193);
  plot(x1,y1,'k-','linewidth',2.5);
  axes(ax2);
  hold on;
  patch([0 0 nshift nshift],[0 1 1 0],[0.7 0.7 0.7]);
  text(nshift/2+30,0.7,'to output','horizontalalignment','center','fontsize',fontsize);
  text_range(0,nshift,y0+dy,'fragment size',210);
  text_range(0,ramplen(1),y0,'ramps',116);
  text_range(nfft-ramplen(2),nfft,y0,'ramps',116);
  plot(x2,y2,'k-','linewidth',2.5);
  plot(x2-nshift,y2,'k:','linewidth',2);
  plot(x2-2*nshift,y2,'k:','linewidth',2);
  plot(x2-3*nshift,y2,'k:','linewidth',2);
  set([ax1;ax2],'ylim',[0 1.5],...
		'ytickmode','manual','ytick',[],...
		'xtickmode','manual',...
		'fontsize',fontsize);
  set(ax1,'xlim',minmax(x1),...
	  'xtick',[-2*nshift:nshift:0], ...
	  'xticklabel',{'','','t=0'});
  set(ax2,'xlim',minmax(x2),...
	  'xtick',[0 pad(1) pad(1)+nshift pad(1)+nwnd],...
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
  
