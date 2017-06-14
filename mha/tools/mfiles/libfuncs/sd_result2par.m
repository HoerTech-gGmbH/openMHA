function s = sd_result2par( s, kField )
% convert a result column into a parameter column.
%
% s      : Data structure.
% kField : Field number or name to be converted.
%
% Author: Giso Grimm
% Date: 12/2008
  ;
  if ischar(kField)
    kField = strmatch(kField,s.fields,'exact');
  end
  if prod(size(kField)) ~= 1
    error('Invalid dimension of field number (e.g., invalid field name).');
  end
  if kField > size(s.data,2)
    error('Invalid field number (to large).');
  end
  nPars = length(s.values);
  if kField <= length(nPars)
    error('Field is already a parameter column.');
  end
  nData = size(s.data,2)-nPars;
  idx_n = setdiff(nPars+[1:nData],kField);
  
  s.values{end+1} = unique(s.data(:,kField));
  for k=1:size(s.data,1)
    s.data(k,kField) = find(s.values{end}==s.data(k,kField));
  end
  s.data = [s.data(:,1:nPars),s.data(:,kField),s.data(:,idx_n)];
  s.fields = [s.fields(1:nPars),s.fields(kField),s.fields(idx_n)];
