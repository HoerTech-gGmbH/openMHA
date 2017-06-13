function sd_latex_tabular( s, format )
% sd_latex_tabular - print structured data as LaTeX
% tabular 
%
% Usage:
% sd_latex_tabular( s [, format ] )
%
% s : structured data
% format: optional format string (default: %g)
%
% Author: Giso Grimm
% Date: 12/2008
  ;
  if nargin < 2
    format = '%g';
  end
  data = s;
  s = '\begin{tabular}{';
  for f=1:length(data.values)
    s = sprintf('%sl',s);
  end
  for f=length(data.values)+1:length(data.fields)
    s = sprintf('%sc',s);
  end
  s = [s,'}'];
  disp(s);
  disp('\hline');
  s = '';
  for f=1:length(data.fields)
    s = sprintf('%s& %s ',s,data.fields{f});
  end
  s(1:2) = [];
  s = [s,'\\'];
  disp(s);
  disp('\hline');
  for k=1:size(data.data,1)
    s = '';
    for f=1:length(data.values)
      if isnumeric(data.values{f})
	s = sprintf(['%s & ',format],s,data.values{f}(data.data(k,f)));
      else
	s = sprintf('%s & %s',s,data.values{f}{data.data(k,f)});
      end
    end
    for f=length(data.values)+1:size(data.data,2)
      s = sprintf(['%s & ',format],s,data.data(k,f));
    end
    s(1:3) = [];
    s = [s,' \\'];
    disp(s);
  end
  disp('\hline');
  disp('\end{tabular}');