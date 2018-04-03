function assignments = struct2mhacfg( mha, filename, prefix )
% assignments = struct2mhacfg( mha [, filename ])
%
% Converts Matlab structures into MHA compatible configuration
% assignments. If an optional filename is given, then the result is
% stored into a MHA configuration file.
  
  if nargin < 2
    filename = '';
  end
  if nargin < 3
    prefix = '';
  end
  assignments = mha_assignments(mha, prefix);
  dotcount = zeros(1,length(assignments));
  for i = 1:length(dotcount)
    dotcount(i) = sum(strtok(assignments{i},'=') == '.');
    % make sort stable
    dotcount(i) = dotcount(i) + 1e-5 * i;

    if strmatch('mhalib',assignments{i})
      dotcount(i) = dotcount(i) + 1e-3;
    elseif strmatch('iolib',assignments{i})
      dotcount(i) = dotcount(i) + 1e-3;
    end
  end
  [dummy, sort_permutation] = sort(dotcount);
  assignments = assignments(sort_permutation);
  
  if ~isempty(filename)
    if ~ischar(filename)
      error('Invalid file name argument');
    end
    [fd, msg] = fopen(filename,'w');
    if fd == -1
      error(msg);
    end
    fprintf(fd,'# This file was created by "%s".\n',mfilename);
    fprintf(fd,'# (%s)\n',which(mfilename));
    fprintf(fd,'#   user: %s\n',my_getenv('USER'));
    fprintf(fd,'#   host: %s\n',my_getenv('HOSTNAME'));
    fprintf(fd,'#    cwd: %s\n',pwd);
    fprintf(fd,'# matlab: %s (%s)\n',version,computer);
    fprintf(fd,'#   date: %s\n',date);
    for str=assignments
      fprintf(fd,'%s\n',str{:});
    end
    fclose(fd);
  end


function e = mha_assignments(sVal, prefix)
  if nargin < 2
    prefix = '';
  end
  if ~isstruct( sVal )
    e = {sprintf('%s=%s', prefix, ...
                 mha_value(sVal))};
    return;
  end
  e = {};
  for fieldname = fieldnames(sVal)'
    if ~isempty(prefix)
      lprefix = [prefix '.'];
    else
      lprefix = prefix;
    end
    fieldname = fieldname{1};
    e = [e, mha_assignments(sVal.(fieldname), ...
                            sprintf('%s%s', lprefix, fieldname))];
  end

function v = mha_value(val)
  if isstruct(val)
    error(sprintf('Called with struct:\n%s', format_backtrace));
  end
  if ischar(val)
    v = val;
  elseif islogical(val)
    if val
      v = 'yes';
    else
      v = 'no';
    end
  elseif iscellstr(val)
    v = '[';
    for i = 1:length(val)
      v = sprintf('%s %s', v, val{i});
    end
    v = [v, ' ]'];
  elseif iscell(val)
    v = '[';
    for i = 1:length(val)
      v = [v, mha_value(val{i}),';'];
    end
    v(end) = ']';
  elseif isnumeric(val)
    if isreal(val)
      if length(val) == 1
        v = sprintf('%.17g',val);
      elseif size(val,1) == 1
        v = ['[ ', sprintf('%.17g ', val), ']'];
      else
        v = '[ ';
        for k=1:size(val,1)
        v = [v '[ ', sprintf('%.17g ', val(k,:)), ']; '];
        end
        v = [v ']'];
      end
    else
      if length(val) == 1
        v = sprintf('(%.17g%+.17gi)', real(val), imag(val));
      elseif size(val,1) == 1
        v = '[';
        for i = 1:size(val,2)
          v = [v, ' ', mha_value(val(i))];
        end
        v = [v, ']'];
      else
        v = '[ ';
        for k=1:size(val,1)
          if size(val,2) == 1
            v = [v, '[', mha_value(val(k,:)), '];'];
          else
            v = [v, mha_value(val(k,:)), ';'];
          end
        end
        v = [v, ']'];
      end
    end
  else
    error(sprintf('Unrecognized data type: %s\n%s', ...
                  class(val), format_backtrace));
  end

function str = format_backtrace(format, bt)
  if (nargin < 1)
    format = '> In %s (%s):%d\n';
  end
  if nargin < 2
    bt = backtrace;
    bt = bt(2:end);
  end
  str = '';
  for x = bt
    str = [str, sprintf(format, x.file, x.inner_function, x.line)];
    if ~isequal(str(end), sprintf('\n'))
      str = [str, sprintf('\n')];
    end
  end

function bt = backtrace
% returns a backtrace structure array of stack frames with the following
% fields:
% - file:           m-filename with complete path
% - inner_function: the current inner function in that m-file
% - line:           the current line number in that file
  
  matlab_release = str2double(version('-release'));

  if (matlab_release <= 13)
    dbt = dbstack;
    dbt = dbt(2:end);
    bt = struct('file',{},'inner_function',{},'line',{});
    for index = 1:length(dbt)
      [file, inner_function] = m_file_and_inner_function(dbt(index).name);
      bt(index) = struct('file', file, 'inner_function', inner_function, ...
                         'line', dbt(index).line);
    end
  else
    dbt = dbstack('-completenames');
    dbt = dbt(2:end);
    bt = struct('file', {dbt.file}, ...
                'inner_function', {dbt.name}, ...
                'line', {dbt.line});
  end


function [m_file, inner_function] = m_file_and_inner_function(bt_line_start)
  if isequal(bt_line_start(end), ')')
    index = strfind(bt_line_start, '(');
    index = index(end);
    inner_function = bt_line_start((index+1):end-1);
    m_file = bt_line_start(1:(index-2));
  else
    m_file = bt_line_start;
    [dummy, inner_function] = fileparts(m_file);
  end

% Local Variables:
% mode: octave
% indent-tabs-mode: nil
% End:
