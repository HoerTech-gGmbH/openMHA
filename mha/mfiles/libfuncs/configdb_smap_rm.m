function cValues = configdb_smap_rm( cValues, name )
% remove an entry from a cell list
%
% No error will be reported if name is not in list.
  ;
  [val,idx] = configdb_smap_get(cValues,name);
  if ~isempty(idx)
    cValues = [cValues(:,1:(idx(1)-1)),cValues(:,(idx(1)+1):end)];
  end