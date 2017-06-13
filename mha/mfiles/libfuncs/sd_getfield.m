function [v,field] = sd_getfield( s, field )
% return name and number of a field
% field : field number or field name
%
% Returns field name "v" and field number "field".
  ;
  if ischar( field )
    field = strmatch(field,s.fields,'exact');
  end
  if isempty(field)
    v = '';
  else
    v = s.fields{field};
  end
