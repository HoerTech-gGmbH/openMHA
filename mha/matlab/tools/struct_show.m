function o = struct_show( s, pref )
  
  if nargin < 2
    pref = '';
  end
  o = '';
  if isstruct( s )
    fn = fieldnames( s );
    o = '';
    for fld=fn'
      if ~strcmp(pref,'')
	npref = sprintf('%s.%s',pref,fld{:});
      else
	npref = fld{:};
      end
      res = struct_show( getfield( s, fld{:} ), npref );
      if length(res) > 0
	o = sprintf('%s%s',o,res);
      end
    end
  else
    if iscellstr( s )
      o = '{';
      k = 0;
      for subs=s
	if k==0
	  sep = '';
	else
	  sep = ',';
	end
	k = k+1;
	o = sprintf('%s%s''%s''',o,sep,subs{:});
      end
      o = [o '}'];
    elseif isnumeric( s )
      if prod(size(s)) == 1
	o = num2str(s);
      elseif (size(s,1) == 1) & (size(s,2) < 20) & (ndims(s) == 2)
	o = sprintf('[%s]',num2str(s,'%1.4g '));
      elseif (size(s,2) == 1) & (size(s,1) < 20) & (ndims(s) == 2)
	o = sprintf('[%s]',num2str(s','%1.4g;'));
      else
	o = sprintf('[%s numeric]',num2str(size(s),'%1dx'));
      end
    elseif ischar( s )
      o = sprintf('''%s''',s);
    end
    o = sprintf('%s = %s\n',pref,o);
  end
