function s = merge_structs( s1, s2 )
% MERGE_STRUCTS - merge two structures into one
%
% Usage: s = merge_structs( s1, s2 )
%
% The return value s contains all fields of s1 and s2. If a field
% is available in both structures, the contents of s1 will be
% used, except for sub-structures, which are merged recursively.
%

 if nargin ~= 2
    error('two input arguments expected');
  end
  if ~isstruct( s1 )
    error('first input argument is not a structure');
  end
  if ~isstruct( s2 )
    error('second input argument is not a structure');
  end
  fn1 = fieldnames(s1);
  fn2 = fieldnames(s2);
  s = struct;
  for fn=fn1'
    if strmatch(fn{:}, fn2, 'exact')
      if isstruct(getfield(s1,fn{:})) && isstruct(getfield(s2,fn{:}))
	s = setfield(s, fn{:}, merge_structs( getfield(s1,fn{:}), ...
					      getfield(s2,fn{:})));
      else
	s = setfield(s, fn{:}, getfield(s1,fn{:}));
      end
    else
      s = setfield(s, fn{:}, getfield(s1,fn{:}));
    end
  end
  for fn=fn2'
    if isempty(strmatch(fn{:}, fn1, 'exact'))
      s = setfield(s, fn{:}, getfield(s2,fn{:}));
    end
  end
