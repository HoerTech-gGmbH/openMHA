function sAudProf = aud2audprof( sAud )
% AUD2AUDPROF - convert an audiogram structure into a auditory
% profile structure
%
% Usage:
% sAudProf = aud2audprof( sAud )
  libaudprof();
  sAudProf = struct;
  sAudProf.id = sAud.id;
  sAudProf.client_id = sAud.client_id;
  for side='lr'
    HTL = sAud.(side).htl;
    idx = find(isfinite(HTL));
    if ~isempty(idx)
      sAudProf.(side).htl_ac = ...
	  audprof.threshold_entry_add( [], ...
				       sAud.frequencies(idx),...
				       HTL(idx));
    end
  end