function [sAud,sAcalos] = audprof2aud( sAudProf )
% AUDPROF2AUD - convert an auditory profile into an audiogram structure
%
% Usage:
%   [sAud,sAcalos] = audprof2aud( sAudProf )
  sAcalos = struct([]);
  sAud = struct('id',sAudProf.id,'client_id',sAudProf.client_id);
  sAud.frequencies = unique([1000*2.^[-3:3],1500*2.^[-1:2]]);
  sTmp = struct;
  for side='lr'
    sTmp.(side).f = [];
    sTmp.(side).htl = [];
    sTmp.(side).fu = [];
    sTmp.(side).ucl = [];
    if isfield(sAudProf,side) && isfield(sAudProf.(side),'htl_ac')
      sTmp.(side).f = [sAudProf.(side).htl_ac.data.f];
      sTmp.(side).htl = [sAudProf.(side).htl_ac.data.hl];
    end
    if isfield(sAudProf,side) && isfield(sAudProf.(side),'ucl')
      sTmp.(side).fu = [sAudProf.(side).ucl.data.f];
      sTmp.(side).ucl = [sAudProf.(side).ucl.data.hl];
    end
  end
  for side='lr'
    sAud.frequencies = unique([sAud.frequencies,sTmp.(side).f,sTmp.(side).fu]);
    sAud.(side).htl = NaN * ones(size(sAud.frequencies));
    sAud.(side).ucl = NaN * ones(size(sAud.frequencies));
  end
  for side='lr'
    for kaud=1:numel(sAud.frequencies)
      f = sAud.frequencies(kaud);
      k = find(sTmp.(side).f==f);
      if ~isempty(k)
	sAud.(side).htl(kaud) = sTmp.(side).htl(k);
      end
      k = find(sTmp.(side).fu==f);
      if ~isempty(k)
	sAud.(side).ucl(kaud) = sTmp.(side).ucl(k);
      end
    end
  end
  for side='lr'
    if isfield(sAudProf,side) && isfield(sAudProf.(side),'acalos')
      if isempty(sAcalos)
	sAcalos = struct;
	sAcalos.cliend_id = sAudProf.client_id;
	sAcalos.id = sAudProf.id;
      end
      for k=1:numel(sAudProf.(side).acalos)
	sAc = sAudProf.(side).acalos(k);
	sAc.measured_data = sAc.data;
	sAc = rmfield(sAc,'data');
	if ~isfield(sAcalos,side)
	  sAcalos.(side) = sAc;
	else
	  sAcalos.(side)(end+1) = sAc;
	end
      end
    end
  end
