function s = sd_rminf( s )
% remove rows with inf values in data section
  ;
  subdata = s.data(:,(numel(s.values)+1):numel(s.fields));
  idx = find(~any(~isfinite(subdata),2));
  s.data = s.data(idx,:);