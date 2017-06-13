function [val, idx] = configdb_smap_get( cValues, sName, defval )
% Get an entry from a cell list and its index
  ;
  if nargin < 3
    defval = [];
  end
  idx = strmatch(sName,cValues(1,:),'exact');
  if isempty(idx)
    val = defval;
  else
    val = cValues{2,idx(1)};
  end