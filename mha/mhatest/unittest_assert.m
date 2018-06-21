function assertions = unittest_assert(bool, error_message)
% unittest_assert(bool, error_message)
%
% The main assertion function for unittests.
% if the bool is not true, then the assertion fails, and the
% error_message + backtrace info is thrown in a matlab error (matlab error
% id = 'unittest:failure').
%
% Tests will mostly invoke this function only indirectly through one of
% the "assert_*.m" functions. Tests should not evaluate the return value
% of unittest_assert.

% This file is part of MatlabUnit.  
% Copyright (C) 2003-2005 Medizinische Physik, Universität Oldenburg
% Author: Tobias Herzke
% File Version: $Id: unittest_assert.m,v 1.3 2005/04/13 16:17:09 tobiasl Exp $

% global counter for assertion calls
persistent assertion_count;

if (nargin < 2)
    error_message = '';
end

if isequal('unittest: reset assertion count', bool)
    % special parameter for resetting global assertion count.
    assertion_count = 0;
else
    assertion_count = assertion_count + 1;
    if ~bool
        bt = backtrace;
        bt = bt(2:end);  % cut off unittest_assert stack frame
        error('unittest:failure', 'Assertion failed: %s\n%s', ...
              error_message, format_backtrace('> In %s (%s):%d', bt));
    end
end
assertions = assertion_count;
