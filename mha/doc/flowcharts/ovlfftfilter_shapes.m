function draw_filtershapes
  x = -1.2:0.002:1.2;
  figure('position',[180 300 880 300]);
  set(gcf,'PaperPosition',[0.25 0.25 10 3]);
  p = 0.4;
  h = plot(x,filter_w(x+1,p),'r-',...
	   x,filter_w(x-1,p),'g-',...
	   x,filter_w(x,p),'b-'...
	   );
  hold on;
  set(h(3),'linewidth',2);
  plot([-0.5*p -0.5*p],[0 1],'k:');
  plot([0.5*p 0.5*p],[0 1],'k:');
  h = text(-0.5*p,-0.08,'-plateau/2');
  set(h,'HorizontalAlignment','center');
  h = text(0.5*p,-0.08,'plateau/2');
  set(h,'HorizontalAlignment','center');
  h = text(0,1.14,'cf_0');
  set(h,'HorizontalAlignment','center');
  h = text(-1,1.14,'cf_{-1}');
  set(h,'HorizontalAlignment','center');
  h = text(1,1.14,'cf_1');
  set(h,'HorizontalAlignment','center');
  set(gca,'Xlim',[min(x) max(x)],'Ylim',[0 1.1]);
  set(gca,'XTick',[-1:0.5:1],'YTick',[0 1]);
  saveas(gcf,mfilename,'epsc');
  

function y = filter_w( x, plateau )
  y = zeros(size(x));
  y(find(abs( x ) > 1-0.5*plateau)) = 0;
  y(find(abs( x ) < 0.5*plateau)) = 1;
  idx = find((-1+0.5*plateau <= x) & (-0.5*plateau >= x));
  y(idx) = 0.5*(1-cos(pi*(x(idx)+1-0.5*plateau)/(1-plateau)));
  idx = find((1-0.5*plateau >= x) & (0.5*plateau <= x));
  y(idx) = 0.5*(1-cos(pi*(x(idx)-1+0.5*plateau)/(1-plateau)));
  return
