function str = format_backtrace(format, bt)
% str = format_backtrace([format [, bt]])
% Returns a formatted backtrace in a string.
% 
% Parameters:
% format: A format string for each backtrace stack frame, containing %s
%         for mfilename, %s for inner function name, %d for line number,
%         in this order. Default is '> In %s (%s):%d\n'
% bt:     The backtrace to forman, as returned by the backtrace function.
%         Default is the current backtrace without the format_backtrace 
%         function itself.

% This file is part of MatlabUnit.  
% Copyright (C) 2003-2005 Medizinische Physik, Universität Oldenburg
% Author: Tobias Herzke
% File Version: $Id: format_backtrace.m,v 1.3 2005/04/13 16:17:09 tobiasl Exp $

if (nargin < 1)
    format = '> In %s (%s):%d\n';
end
if nargin < 2
    bt = backtrace;
    bt = bt(2:end); % cut format_backtrace function frame from bt
end

% concatenate formatted strings for all backtrace frames.
str = '';
for x = bt
    str = [str, sprintf(format, x.file, x.inner_function, x.line)];
    if ~isequal(str(end), sprintf('\n'))
        str = [str, sprintf('\n')];
    end
end
