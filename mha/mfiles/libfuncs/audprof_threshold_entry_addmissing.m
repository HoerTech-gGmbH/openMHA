function vs = audprof_threshold_entry_addmissing( vs, f, hl )
% add one or more entries to a threshold structure (htl_ac, htl_bc, ucl)
% vs  : Threshold structure
% f   : frequency (scalar or vector)
% hl  : threshold (same dimension as f)
  ;
  if isempty(vs)
    vs = audprof_threshold_new();
  end
  for k=1:numel(f)
    idx = find([vs.data.f]==f(k));
    if isempty(idx)
      vs.data(end+1) = struct('f',f(k),'hl',hl(k));
    end
  end
  [tmp,idx] = sort([vs.data.f]);
  vs.data = vs.data(idx);
