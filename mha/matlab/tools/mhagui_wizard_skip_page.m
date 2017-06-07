function mhagui_wizard_skip_page( vPage, bSkip )
  if nargin < 2
    bSkip = 1;
  end
  uih = findobj('Tag','mhagui_wizard_next');
  if isempty(uih)
    error('No UI control found');
  end
  sCfg = get(uih,'UserData');
  if ~isfield(sCfg,'skip')
    sCfg.skip = [];
  end
  if ischar(vPage)
    tag = vPage;
    vPage = [];
    for k=1:length(sCfg.cfg)
      cf = sCfg.cfg{k};
      if isfield(cf,'tag')
	if strcmp(cf.tag,tag)
	  vPage(end+1) = k;
	end
      end
    end
  end
  if bSkip
    for page=vPage
      sCfg.skip(end+1) = page;
      sCfg.skip = unique(sCfg.skip);
    end
  else
    for page=vPage
      sCfg.skip(find(sCfg.skip==page)) = [];
    end
  end
  set(uih,'UserData',sCfg);
