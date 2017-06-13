function s = sd_permval( s, field, vPerm )
% permute value order in a given column
%
% s: struct data
% field: name or number of column
% vPerm: permutation vector
%
% Author: Giso Grimm 10/2010
  if ischar(field)
    field = strmatch(field,s.fields,'exact');
  end
  if numel(vPerm) ~= numel(s.values{field})
    error('dimension mismatch');
  end
  [tmp,idx] = sort(vPerm);
  s.values{field} = s.values{field}(vPerm);
  s.data(:,field) = idx(s.data(:,field));
  