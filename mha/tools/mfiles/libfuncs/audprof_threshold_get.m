function sTh = audprof_threshold_get( sAud, side, stype )
  sTh = audprof_threshold_new();
  if isfield(sAud,side) && isfield(sAud.(side),stype)
    sTh = sAud.(side).(stype);
  end