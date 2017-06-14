function cValues = configdb_smap_set( cValues, name, val )
% add an entry to a cell list
%
% Existing entries of the same name will be replaced.
  ; 
  [valt,idx] = configdb_smap_get(cValues,name);
  if isempty(idx)
    cValues(:,end+1) = {name;val};
  else
    cValues{2,idx} = val;
  end