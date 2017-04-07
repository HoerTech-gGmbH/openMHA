function ax = mhagui_audprof_plot( sAudProf, ax )
% auditory profile plotting tool
%
% sAudProf : Auditory profile to plot
% ax       : Plot axes
%
% If an axes object with the tag 'audiogram_axes' is found in the
% current figure, then this is used (and returned), otherwise new axes
% are created.
   
  
  libaudprof();
  sAudProf = audprof.audprof_cleanup(sAudProf);
  vFreqMajor = 125*2.^[0:6];
  vFreqMinor = 750*2.^[0:3];
  vHL = [-10:10:120];
  vXLim = [100 10000];
  vYLim = [-15 115];
  if nargin < 2
    ax = findall(gcf,'Tag','audiogram_axes');
    if isempty(ax)
      ax = axes('Tag','audiogram_axes');
    end
    ax = ax(1);
  end
  axes(ax);
  set(ax,'NextPlot','ReplaceChildren');
  vH = ...
      plot(sort(repmat(vFreqMajor',[3,1])),...
	   repmat([vYLim,inf]',[7,1]),'k-',...
	   sort(repmat(vFreqMinor',[3,1])),...
	   repmat([vYLim,inf]',[4,1]),'k:',...
	   sort(repmat(vFreqMinor',[3,1])),...
	   repmat([vYLim,inf]',[4,1]),'k:',...
	   repmat([vXLim,inf]',[length(vHL),1]),...
	   sort(repmat(vHL',[3,1])),'k-');
  set(vH,'color',0.7*ones(1,3));
  set(ax,'NextPlot','Add');
  plot(vXLim,[0 0],'k-',[1000 1000],vYLim,'k-');
  sCol = struct;
  sCol.l.c = [0,0,0.7];
  sCol.l.htl_ac.m = 'x';
  sCol.l.htl_bc.m = '';
  sCol.l.htl_bc.t = '>';
  sCol.l.ucl.m = '^';
  sCol.l.df = 1.19;
  sCol.r.c = [0.7,0,0];
  sCol.r.htl_ac.m = 'o';
  sCol.r.htl_bc.m = '';
  sCol.r.htl_bc.t = '<';
  sCol.r.ucl.m = '^';
  sCol.r.df = 1/sCol.l.df;
  for side='lr'
    if isfield(sAudProf,side)
      if isfield(sAudProf.(side),'acalos')
	
	for k=1:numel(sAudProf.(side).acalos)
	  sAc = sAudProf.(side).acalos(k);
	  L0 = sAc.lcut - 25/sAc.mlow;
	  L25 = sAc.lcut;
	  L50 = sAc.lcut + 25/sAc.mhigh;
	  vf1 = [sAc.f;sAc.f;sAc.f*sqrt(sCol.(side).df)];
	  vd1 = [L0;L25;L25];
	  vf2 = [sAc.f;sAc.f*sqrt(sCol.(side).df);sAc.f*sCol.(side).df;sAc.f];
	  vd2 = [L25;L25;L50;L50];
	  patch(vf1,vd1,0.3*sCol.(side).c+0.7*ones(1,3),...
		'EdgeColor',sCol.(side).c);
	  patch(vf2,vd2,0.3*sCol.(side).c+0.7*ones(1,3),...
		'EdgeColor',sCol.(side).c);
	end
      end
    end
  end
  for side='lr'
    if isfield(sAudProf,side)
      for type={'htl_ac','htl_bc','ucl'}
	stype = type{:};
	if isfield(sAudProf.(side),stype)
	  vf = [sAudProf.(side).(stype).data.f];
	  htl = [sAudProf.(side).(stype).data.hl];
	  plot(vf,htl,...
	       ['-',sCol.(side).(stype).m],'linewidth',2,'MarkerSize',10,...
	       'Color',sCol.(side).c);
	  if isfield(sCol.(side).(stype),'t')
	    for kf=1:numel(vf)
	      if isfinite(htl(kf))
		text(vf(kf),htl(kf),sCol.(side).(stype).t,...
		     'FontUnits','normalized','FontSize',0.1,...
		     'FontWeight','bold',...
		     'VerticalAlignment','middle',...
		     'HorizontalAlignment','center',...
		     'Color',sCol.(side).c);
	      end
	    end
	  end
	  idx = find(~isfinite(htl));
	  for kf=idx
	    text(vf(kf),100+floor(sCol.(side).df)*10,'v',...
		 'FontUnits','normalized','FontSize',0.1,...
		 'VerticalAlignment','baseline',...
		 'HorizontalAlignment','center',...
		 'Color',sCol.(side).c);
	    text(vf(kf),100+floor(sCol.(side).df)*10,'I',...
		 'FontUnits','normalized','FontSize',0.1,...
		 'VerticalAlignment','baseline',...
		 'HorizontalAlignment','center',...
		 'Color',sCol.(side).c);
	  end
	end
      end
    end
  end
  csLab = {'125','250','500','1k','2k','4k','8k'};
  set(ax,'XGrid','off','YGrid','off',...
	 'XLim',vXLim,...
	 'XScale','log',...
	 'XTick',vFreqMajor,...
	 'XTickLabel',csLab,...
	 'YDir','reverse',...
	 'YLim',vYLim,...
	 'YTick',vHL,...
	 'Box','on',...
	 'NextPlot','replacechildren');
  xlabel('Frequency / Hz');
  ylabel('Threshold / dB HL');
  title([sAudProf.client_id,', ',sAudProf.id],'interpreter','none');
  drawnow;
