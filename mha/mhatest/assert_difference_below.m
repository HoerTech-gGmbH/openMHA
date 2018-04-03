function assert_difference_below(expected, actual, epsilon, error_message)
% assert_difference_below(expected, actual, epsilon [, error_message])
%
% Use this assertion to confirm that all absolute differences between
% corresponding entries in the arrays expected and actual are below
% epsilon.

% This file is part of MatlabUnit.  
% Copyright (C) 2003-2005 Medizinische Physik, Universität Oldenburg
% Author: Tobias Herzke
% File Version: $Id: assert_difference_below.m,v 1.6 2005/04/13 16:17:09 tobiasl Exp $

if (nargin < 4)
  error_message = '';
end

if ~isequal(size(expected),size(actual))
   error_message = sprintf('%s: SIZE DIFFERS (expected [%s]; actual [%s]):',...
                           error_message, ...
                           num2str(size(expected)), ...
                           num2str(size(actual)));
   unittest_assert(0, error_message);
end

result = abs(expected - actual) <= epsilon;

if any(~result(:))
    if (length(expected) > 1)
        indices = find(~result(:));
          error_message = ...
              sprintf(['%s: VALUES DIFFER TOO MUCH, first difference ', ...
                       'at index (%s): %.17g~=%.17g, difference %.17g '],...
                      error_message, ...
                      nice_index(indices(1),size(result)), ...
                      expected(indices(1)), actual(indices(1)), ...
                      abs(expected(indices(1)) - actual(indices(1))));
    else
          error_message = ...
              sprintf(['%s: VALUES DIFFER TOO MUCH: expected: %.17g, ', ...
                       'actual: %.17g, difference %.17g '], ...
                      error_message, expected, actual, abs(expected - actual));
    end
end

unittest_assert(all(result(:)), error_message);

function nice = nice_index(index, size)
  if any(size == 1)
      nice = num2str(index);
  else
      dummy = zeros(size);
      dummy(index) = 1;
      [i,j] = find(dummy);
      nice = sprintf('(%d,%d)',[i;j]);
  end