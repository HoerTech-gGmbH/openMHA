function s = audprof_threshold_new
  s = struct;
  s.id = '';
  s.client_id = '';
  s.datestr = datestr(now);
  s.data = struct('f',[],'hl',[]);
  s.data = s.data([]);