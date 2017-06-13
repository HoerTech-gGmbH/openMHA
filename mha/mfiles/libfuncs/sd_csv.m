function sd_csv( data, fname )
% sd_csv - export data structure as CSV file
%
% Usage:
% sd_csv( data, fname )
%
  ;
  fh = fopen(fname,'w');
  s = '';
  for f=1:length(data.fields)
    s = sprintf('%s,"%s"',s,data.fields{f});
  end
  s(1) = [];
  fprintf(fh,'%s\n',s);
  for k=1:size(data.data,1)
    s = '';
    for f=1:length(data.values)
      if isnumeric(data.values{f})
	s = sprintf('%s,%g',s,data.values{f}(data.data(k,f)));
      else
	s = sprintf('%s,"%s"',s,data.values{f}{data.data(k,f)});
      end
    end
    for f=length(data.values)+1:size(data.data,2)
      s = sprintf('%s,%g',s,data.data(k,f));
    end
    s(1) = [];
    fprintf(fh,'%s\n',s);
  end
  fclose(fh);
  
