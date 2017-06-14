function s = sd_renamefield( s, field, newname )
% rename a data field
%
% s       : data structure
% field   : field name or number
% newname : new name
  if ischar( field )
    field = strmatch( field, s.fields, 'exact' );
  end
  s.fields{field} = newname;