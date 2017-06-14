function struct2mfile( s, fname )
% STRUCT2MFILE - Convert variables of type 'struct' into Matlab
% function scripts with the same return value.
%  
% Syntax:
% struct2mfile( s, fname )
%
% Parameters:
%   s     : structure to be converted
%   fname : file name to store the data
%
% Author: Giso Grimm
% Date: 11/2006
  ;
  [pathstr,name,ext] = fileparts( fname );
  if isempty(ext)
    ext = '.m';
  end
  fh = fopen(fullfile(pathstr,[name ext]),'w');
  fprintf(fh,'function s = %s\n',name);
  fprintf(fh,'%% data file created by struct2mfile\n');
  fprintf(fh,'%% date: %s\n',datestr(now));
  fprintf(fh,'%% user: %s\n',getenv('USER'));
  fprintf(fh,'%% cwd: %s\n',pwd);
  fprintf(fh,'s = struct;\n');
  fprintf(fh,'%s',struct2string( s, 's' ));
  fclose(fh);
  clear(name);

function o = struct2string( s, pref, show_eq )
  if nargin < 2
    pref = '';
  end
  if nargin < 3
    show_eq = 1;
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
      res = struct2string( getfield( s, fld{:} ), npref );
      if length(res) > 0
	o = sprintf('%s%s',o,res);
      end
    end
  else
    if iscell( s )
      o = '{';
      if ndims(s) > 2
	error('Only 2-dim cells supported');
      end
      for k1 = 1:size(s,1)
	for k2 = 1:size(s,2)
	  if k2<size(s,2)
	    sep2 = ',';
	  else
	    sep2 = '';
	  end
	  o = sprintf('%s%s%s',o,struct2string(s{k1,k2},'',0),sep2);
	end
	if k1 < size(s,1)
	  o = sprintf('%s;',o);
	end
      end
      o = [o '}'];
    elseif isnumeric( s )
      o = num2str_local( s );
    elseif ischar( s )
      so = '';
      for ks=1:length(s)
	if s(ks) ~= ''''
	  so(end+1) = s(ks);
	else
	  so(end+1) = s(ks);
	  so(end+1) = s(ks);
	end
      end
      o = sprintf('''%s''',so);
    elseif islogical( s )
      if( s )
	o = 'true';
      else
	o = 'false';
      end
    elseif strcmp(class(s),'function_handle')
      o = ['@',char(s)];
    end
    if show_eq
      o = sprintf('%s = %s;\n',pref,o);
    end
  end

function o = num2str_local( s )
  o = '';
  if prod(size(s)) == 1
    o = sprintf('%1.20g',s);
  elseif prod(size(s)) == 0
    if max(size(s)) == 0
      o = '[]';
    else
      o = sprintf('zeros(%d,%d)',size(s,1),size(s,2));
    end
  elseif (min(size(s)) == 1) & (ndims(s) == 2)
    o = sprintf('[%s]',num2str_row(s));
  elseif (ndims(s) == 2)
    o = sprintf('%s[...\n',o);
    for k=1:size(s,1)
      o = sprintf('%s%s;...\n',o,num2str_row(s(k,:)));
    end
    o = sprintf('%s]',o);
  else
    error('unable to handle matrixes with more than 2 dimensions.');
  end
  
function o = num2str_row( s )
  o = '';
  if (prod(size(s))>1) & (min(size(s))==1)
    if size(s,1) > size(s,2)
      delim = sprintf(';\t');
    else
      delim = sprintf('\t');
    end
    for k=1:length(s)
      if k==length(s)
	delim = '';
      end
      o = sprintf('%s%1.20g%s',o,s(k),delim);
    end
  else
    error('only vectors supported.');
  end
  
