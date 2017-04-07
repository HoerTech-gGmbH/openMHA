function audprof_audprof_print( sAud )
  libmhagui();
  fh = figure('MenuBar','none','PaperUnits','centimeters',...
	      'PaperPosition',[2,10,17,15]);
  side = 'lr';
  csSide = {'left','right'};
  for k=1:2
    ax = subplot(2,2,3-k);
    oside = side(3-k);
    sAudTmp = sAud;
    if isfield(sAud,oside)
      sAudTmp = rmfield(sAudTmp,oside);
    end
    set(findobj(fh,'tag','audiogram_axes'),'tag','');
    set(ax,'tag','audiogram_axes');
    mhagui.audprof_plot( sAudTmp );
    title(sprintf('%s - %s',[sAud.client_id,' ',sAud.id],...
		  csSide{k}),...
	  'Interpreter','none');
    
    sAudTmp = audprof_audprof_fillnan( sAudTmp );
    ax = subplot(2,2,5-k);
    ldatestr = '';
    if k==2
      if isfield(sAud,'date')
	ldatestr = sAud.date;
      end
    elseif k==1
      ldatestr = sprintf('Printed: %s',datestr(now));
    end
    ktype = 0;
    for type={'htl_ac','htl_bc','ucl'}
      slab = type{:};
      slab = strrep(slab,'htl','HTL');
      slab = strrep(slab,'ucl','UCL');
      ktype = ktype+1;
      sA = sAudTmp.(side(k)).(type{:});
      vf = [sA.data.f];
      vhl = [sA.data.hl];
      text(ktype,0,slab,'fontweight','bold','horizontalAlignment','center',...
	   'Interpreter','none');
      csYLab = {''};
      for kf=1:numel(vf)
	if isfinite(vhl(kf))
	  text(ktype,kf,sprintf('%4.1f dB',vhl(kf)),...
				'horizontalAlignment','center');
	else
	  text(ktype,kf,'-',...
	       'horizontalAlignment','center');
	end
	csYLab{end+1} = num2str(vf(kf));
      end
    end
    set(ax,'YLim',[-0.5 numel(vf)+0.5],...
	   'YTick',0:numel(vf),'YTickLabel',csYLab,...
	   'XLim',[0.2 3.8],'XTick',[2],...
	   'XTickLabel',{ldatestr},...
	   'YDir','reverse','Box','on','NextPlot','add');
    plot([0 4],[0.5 0.5],'k-');
    title(sprintf('%s - %s',[sAud.client_id,' ',sAud.id],csSide{k}),...
	  'Interpreter','none');
  end
  printdlg(fh);
  close(fh);
