function ax = mhagui_rating_axes( csLabel, varargin )
  ylim = [0,length(csLabel)-1];
  dq = 0.03*diff(ylim);
  ax = axes('Visible','on','NextPlot','ReplaceChildren',...
	    'Clipping','off',...
	    'XLim',[0 5],'Ylim',ylim-[dq -dq],...
	    'Box','on','XTick',[],'YTick',[],...
	    'Color',0.9*ones(1,3),...
	    varargin{:});
    q = ylim(1)-ylim(2);
  vhsel = [];
  vhsel(end+1) = ...
      patch([0.05 0.05 1.05 1.05],[ylim(1) ylim(2) ylim(2) ylim(1)],ones(1,3));
  hold on;
  for k=1:length(csLabel)
    %y = (k)/(length(csLabel)+1)*diff(ylim);
    y = (k-1)/(length(csLabel)-1)*diff(ylim);
    vhsel(end+1) = ...
	text(1.4,y,csLabel{k},'HorizontalAlignment','left',...
	     'VerticalAlignment','middle',...
	     'Fontsize',14,'Fontweight','bold');
    if (y < ylim(2)) && (y > ylim(1))
      vhsel(end+1) = ...
	  plot([0.05 1],[y y],'-','Color',0.6*ones(1,3),'linewidth', ...
	       2);
    end
    for kmin=1:3
      y = (k-1+kmin/4)/(length(csLabel)-1)*diff(ylim);
      if (y < ylim(2)) && (y > ylim(1))
	vhsel(end+1) = ...
	    plot([0.05 1],[y y],'--','Color',0.6*ones(1,3),'linewidth', ...
		 1);
      end
    end
  end
  vhsel(end+1) = ax;
  vhsel(end+1) = ...
      plot([0 1 0.5 1 0.5],[q q q+dq q q-dq],...
	   'k-','linewidth',3);
  set(vhsel,'ButtonDownFcn',@mhagui_rating_axes_select);
  set(ax,'UserData',struct('rating',nan,'arrow',vhsel(end),'ylim',ylim));
  
function mhagui_rating_axes_select( varargin )
  ax = gca();
  axp = get(ax,'CurrentPoint');
  sData = get(ax,'UserData');
  ylim = sData.ylim;
  q = axp(1,2);
  %%if (q<ylim(1))||(q>ylim(2))
  %%  return
  %%end
  q = min(max(q,ylim(1)),ylim(2));
  h = sData.arrow;
  q0 = get(h,'YData');
  set(h,'YData',q0-q0(1)+q);
  sData.rating = q;
  set(ax,'UserData',sData);
  if isfield(sData,'callback')
    sData.callback();
  end
  