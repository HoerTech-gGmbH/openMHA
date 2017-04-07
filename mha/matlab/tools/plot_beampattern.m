function plot_beampattern( sData )
  h = libsd();
  csMethod = sData.values{2};
  for k=1:numel(csMethod)
    fh = figure('Name',csMethod{k});
    plot_data(sData,csMethod{k},h);
  end
  
function plot_data(sData,method,hSD)
  vAz = sData.values{1};
  vF = sData.values{3};
  sData = hSD.restrict(sData,'tag',{{method}});
  sData = hSD.par2col(sData,'frequency');
  data = sData.data(:,3:end);
  for k=1:numel(vF)
    data(:,k) = data(:,k) - max(data(:,k));
  end
  my_polar(vAz,data(:,1:2:end),40,vF(1:2:end));
  
function vph = my_polar( a, h, href, vLeg, dx )
  if nargin < 3
    href = 35;
  end
  if nargin < 4
    vLeg = [1:size(h,2)];
  end
  if nargin < 5
    dx = 0;
  end
  dx = dx*2.8*(href);
  vGrid = pi*[0:360]'/180;
  vHGrid = ones(361,1)*[0:-10:-href+1]+href;
  vHMinGrid = ones(361,1)*[-5:-10:-href+1]+href;
  x = sin(vGrid).*vHGrid(:,1);
  y = cos(vGrid).*vHGrid(:,1);
  plot(dx+x,y,'k-');
  hold on;
  for k=1:size(vHMinGrid,2)
    x = sin(vGrid).*vHMinGrid(:,k);
    y = cos(vGrid).*vHMinGrid(:,k);
    plot(dx+x,y,'k:','linewidth',0.1);
  end
  for k=1:size(vHGrid,2)
    x = sin(vGrid).*vHGrid(:,k);
    y = cos(vGrid).*vHGrid(:,k);
    plot(dx+x,y,'k--','linewidth',0.1);
    text(dx,vHGrid(1,k),sprintf('%g ',vHGrid(1,k)-href),...
         'HorizontalAlignment','right',...
         'VerticalAlignment','bottom',...
         'FontSize',10);
  end
  for a0=0:30:330
    x = (href)*sin(pi*a0/180);
    y = (href)*cos(pi*a0/180);
    if a0 > 180
      a0 = a0 - 360;
    end
    plot(dx+[0,x],[0,y],'k-','linewidth',0.1);
    text(dx+1.1*x,1.1*y,num2str(a0),...
         'fontsize',14,...
         'HorizontalAlignment','center');
  end
  h = max(0,href+h);
  vph = zeros(size(h,2),1);;
  a = pi*a(:)/180;
  h = [h;h(1,:)];
  a = [a;a(1)];
  for k=1:size(h,2)
    hk = h(:,k);
    x = sin(a).*hk;
    y = cos(a).*hk;
    vph(k) = plot(dx+x,y);
    hold on;
  end
  hold off;
  set(gca,'DataAspectRatio',[1,1,1],...
          'XLim',(href)*[-1.2,1.5],'yLim',(href)*1.2*[-1,1],...
          'XTick',[],'YTick',[]);
  
  map = colormap;
  map = interp1([0:size(map,1)-1]/(size(map,1)-1),...
                map,[0:size(vph,1)-1]/(size(vph,1)-1));
  for k=1:length(vph)
    set(vph(k),'Color',map(k,:),'linewidth',2);
  end
  h = legend(vph,num2str(vLeg(:)),'Location','NorthEast');
  set(h,'fontsize',14);
