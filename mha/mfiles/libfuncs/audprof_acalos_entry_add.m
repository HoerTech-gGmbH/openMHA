function vs = audprof_acalos_entry_add( vs, f, mlow, mhigh, lcut, data )
% add an ACALOS entry to a acalos structure
% vs    : ACALOS structure
% f     : frequency
% mlow  : slope at low levels
% mhigh : slope at high levels
% lcut  : level of 25 cu
% data  : measured (raw) data
  ;
  if isempty(vs)
    vs = struct('f',[],'mlow',[],'mhigh',[],'lcut',[],'data',[]);
    vs = vs([]);
  end
  vs(end+1) = struct('f',f,'mlow',mlow,'mhigh',mhigh,...
		     'lcut',lcut,'data',data);
  
