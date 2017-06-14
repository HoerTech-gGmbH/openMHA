function s = sd_mergepar( s, vCol )
% merge multiple parameter columns
%
% s : struct data
% vCol : number of clumns or cell array of column names
  ;
  if iscellstr( vCol )
    v = zeros(size(vCol));
    for k=1:numel(vCol)
      v(k) = strmatch(vCol{k},s.fields,'exact');
    end
    vCol = v;
  end
  fn = '';
  for kPar=vCol
    fn = [fn,s.fields{kPar},','];
    if isnumeric(s.values{kPar})
      cs = {};
      for kVal=1:numel(s.values{kPar})
	cs{end+1} = num2str(s.values{kPar}(kVal));
      end
      s.values{kPar} = cs;
    end
  end
  fn(end) = '';
  csVal = cell(1,size(s.data,1));
  for k=1:size(s.data,1)
    csVal{k} = '';
    for kPar=vCol
      csVal{k} = [csVal{k},s.values{kPar}{s.data(k,kPar)},','];
    end
    csVal{k}(end) = '';
  end
  s.fields(vCol) = [];
  s.fields = [{fn},s.fields];
  s.data(:,vCol) = [];
  s.data = [[1:size(s.data,1)]',s.data];
  s.values(vCol) = [];
  s.values = [{csVal},s.values];
  s = sd_compactval( s );