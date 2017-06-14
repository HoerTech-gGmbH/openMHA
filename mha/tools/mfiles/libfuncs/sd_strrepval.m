function s = sd_strrepval( s, field, pattern, replacement )
% replace strings in value list of one or more fields
%
% s: struct data
% field: column numbers or cellstr with field names, or string with
% field name
  
  if iscellstr( field )
    for sF = field
      s = sd_strrepval( s, sF{:}, pattern, replacement );
    end
    return
  end
  if ischar( field )
    field = strmatch(field,s.fields,'exact');
  end
  if numel(field) > 1
    for sF = field
      s = sd_strrepval( s, sF, pattern, replacement );
    end
    return
  end
  s.values{field} = strrep(s.values{field},pattern,replacement);
  