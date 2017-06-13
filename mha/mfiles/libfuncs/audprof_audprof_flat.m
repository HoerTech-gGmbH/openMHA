function sAud = audprof_audprof_flat( HTL )
% return a flat auditory profile
  sAud = audprof_audprof_new();
  sAud.id = sprintf('flat audiogram %g dB',HTL);
  vf = unique([1000*2.^[-3:3],1500*2.^[-1:2]]);
  for side='lr'
    sAud.(side).htl_ac = ...
	audprof_threshold_entry_add([],vf,HTL*ones(size(vf)));
  end
