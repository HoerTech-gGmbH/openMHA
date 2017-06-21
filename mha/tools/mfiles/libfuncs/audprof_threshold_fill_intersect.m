function vs = audprof_threshold_fill_intersect( vs, f, hl )
% fill a threshold structure, remove entries not present in f
% vs  : Threshold structure
% f   : frequency (scalar or vector)
% hl  : threshold (same dimension as f)
  ;
  if isempty(vs)
    vs = audprof_threshold_new();
  end
  [tmp,idx] = intersect([vs.data.f],f);
  vs.data = vs.data(idx);
  for k=1:numel(f)
    idx = find([vs.data.f]==f(k));
    if isempty(idx)
      vs.data(end+1) = struct('f',f(k),'hl',hl(k));
    end
  end
  [tmp,idx] = sort([vs.data.f]);
  vs.data = vs.data(idx);
