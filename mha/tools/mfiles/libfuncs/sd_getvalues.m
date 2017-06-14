function v = sd_getvalues( s, field )
% return value set of a given parameter field
% field : parameter field number or field name
  ;
  if ischar( field )
    field = strmatch(field,s.fields,'exact');
  end
  v = s.values{field};
