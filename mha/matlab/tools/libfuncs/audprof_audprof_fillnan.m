function sAud = audprof_audprof_fillnan( sAud )
  for side='rl'
    if ~isfield(sAud,side)
      sAud.(side) = struct;
    end
    for type={'htl_ac','htl_bc','ucl'}
      stype = type{:};
      vf = unique([1000*2.^[-3:3],1500*2.^[-1:2]])';
      if ~isfield(sAud.(side),stype)
	sAud.(side).(stype) = audprof_threshold_new();
      end
      sAud.(side).(stype) = ...
	  audprof_threshold_entry_addmissing(sAud.(side).(stype),...
					     vf,nan(size(vf)));
    end
  end