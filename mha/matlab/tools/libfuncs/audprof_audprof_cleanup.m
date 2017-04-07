function sAud = audprof_audprof_cleanup( sAud )
% Remove NaN and Inf entries from thresholds and remove empty
% auditory profile data.
  for side='lr'
    if isfield(sAud,side)
      for type={'htl_ac','htl_bc','ucl'}
	stype = type{:};
	if isfield( sAud.(side),stype )
	  sAud.(side).(stype) = ...
	      audprof_threshold_cleanup( sAud.(side).(stype) );
	  if audprof_threshold_isempty( sAud.(side).(stype) )
	    sAud.(side) = rmfield(sAud.(side),stype);
	  end
	end
      end
      if isempty(fieldnames(sAud.(side)))
	sAud = rmfield(sAud,side);
      end
    end
  end
  if ~isfield(sAud,'id')
    sAud.id = '';
  end
  if ~isfield(sAud,'client_id')
    sAud.client_id = '';
  end