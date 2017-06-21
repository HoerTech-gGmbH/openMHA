function s = sd_rmcol( s, vField )
% remove columns from a data structure
%
% s      : data structure
% vField : column numbers or column name to delete (can be cell
%          string to delete multiple columns by name)
  ;
  if iscell( vField )
    for sF = vField
      s = sd_rmcol( s, sF{:} );
    end
    return
  end
  if ischar( vField )
    vField = strmatch(vField,s.fields,'exact');
  end
  nPar = length(s.values);
  vFieldPar = vField(find(vField<=nPar));
  s.fields(vField) = [];
  s.values(vFieldPar) = [];
  s.data(:,vField ) = [];