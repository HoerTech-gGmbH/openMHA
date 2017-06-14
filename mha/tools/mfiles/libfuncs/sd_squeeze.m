function [s,csFields,cValues] = sd_squeeze( s )
% remove singleton dimensions (parameter and data fields)
%
% Author: Giso Grimm
% Date: 11/2008
  ;
  vDim = sd_getdim( s );
  idx = find(vDim(1:length(s.values))==1);
  csFields = s.fields(idx);
  s.fields(idx) = [];
  cValues = {};
  for k=1:length(idx)
    val = s.values{idx(k)};
    if ~iscell(val)
      cValues{k} = val(s.data(1,idx(k)));
    else
      cValues{k} = val{s.data(1,idx(k))};
    end
  end
  s.values(idx) = [];
  s.data(:,idx) = [];
  