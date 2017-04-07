function s = sd_fun( s, kCol, fun )
% FUN - apply a function to a data column and add result as new
% column
%
% Usage:
% s = sdlib.fun( s, kCol, fun )
%
% s: struct data
% kCol: column number or name
% fun: function handle (must return numeric value)
%
% Author: Giso Grimm, 9/2010
  ;
  if ischar(kCol)
    kCol = strmatch(kCol,s.fields,'exact');
  end
  bPar = (kCol <= numel(s.values));
  if bPar
    vVal = s.values{kCol}(s.data(:,kCol));
  else
    vVal = s.data(:,kCol);
  end
  vRes = fun(vVal);
  s.fields{end+1} = func2str(fun);
  s.data(:,end+1) = vRes;