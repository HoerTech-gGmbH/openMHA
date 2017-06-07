function sAudProf = audprof_acalos_generate( sAudProf )
% generates loudness data from audiogram
% sAudProf : Auditory profile
  audiogram.frequencies = [sAudProf.l.htl_ac.data.f];
  audiogram.l.htl = [sAudProf.l.htl_ac.data.hl];
  audiogram.l.ucl = PascoeUCL(audiogram.l.htl);
  audiogram.r.htl = [sAudProf.r.htl_ac.data.hl];
  audiogram.r.ucl = PascoeUCL(audiogram.r.htl);
  audiogram.name = sAudProf.client_id;
  hfd = gt_Subject_new_from_audiogram(audiogram, audiogram.frequencies);
  for side='lr'
    acalos = [];
    for k=1:numel(hfd.(side))
      acalos = ...
	  audprof_acalos_entry_add( acalos, ...
				    hfd.(side)(k).frequency, ...
				    hfd.(side)(k).mlow, ...
				    hfd.(side)(k).mhigh, ...
				    hfd.(side)(k).lcut, ...
				    hfd.(side)(k).measured_data );
    end
    sAudProf.(side).acalos = acalos;
  end
  
