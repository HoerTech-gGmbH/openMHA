function s = audprof_threshold_cleanup( s )
  if ~isfield(s,'data')
    if isfield(s,'f') && isfield(s,'hl')
      tmp = s;
      s = audprof_threshold_new();
      s.data = tmp;
    else
      s = audprof_threshold_new();
    end
  end
  s.data = s.data(find(~isnan([s.data.hl])));
