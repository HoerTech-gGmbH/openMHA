function bt = backtrace
% returns a backtrace structure array of stack frames with the following
% fields:
% - file:           m-filename with complete path
% - inner_function: the current inner function in that m-file
% - line:           the current line number in that file

% This file is part of MatlabUnit.  
% Copyright (C) 2003-2005 Medizinische Physik, Universität Oldenburg
% Author: Tobias Herzke
% File Version: $Id: backtrace.m,v 1.5 2005/04/13 16:17:09 tobiasl Exp $

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


function [m_file, inner_function] = m_file_and_inner_function(name)
% Matlab R13 has mfile name and inner function name mangled into one
% string: 'mfilewithpath.m (inner_function)'. If the main function is
% executing, then the ' (inner function)' part is not present. This
% function demangles that string.
if isequal(name(end), ')')
    index = strfind(name, '(');
    index = index(end);
    inner_function = name((index+1):end-1);
    m_file = name(1:(index-2));
else
    m_file = name;
    [dummy, inner_function] = fileparts(m_file);
end
