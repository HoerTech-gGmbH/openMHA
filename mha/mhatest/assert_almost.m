function assert_almost(expected, actual, epsilon, error_message)
% assert_almost(expected, actual, times_epsilon [, error_message])
%
% Use this assertion to confirm that all relative differences between
% corresponding entries in the arrays expected and actual
%
%      relative_differences = abs(actual ./ expected - 1)
%
% are below epsilon. Make sure expected does not contain 0.

% This file is part of MatlabUnit.  
% Copyright (C) 2003-2005 Medizinische Physik, Universit�t Oldenburg
% Author: Tobias Herzke
% File Version: $Id: assert_almost.m,v 1.6 2010/10/05 15:27:11 tobias Exp $

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

relative_differences = abs(actual ./ expected - 1);

result = (relative_differences <= epsilon) | (actual == expected);

if any(~result(:))
      if (numel(expected) > 1)
        indices = find(~result(:));
          error_message = ...
              sprintf(['%s: VALUES DIFFER TOO MUCH, first difference ', ...
                       'at index (%s): %.17g+%.17gi~=%.17g+%.17gi, ', ...
                       'relative difference %.17g '], ...
                      error_message, ...
                      nice_index(indices(1),size(result)), ...
                      real(expected(indices(1))), ...
                      imag(expected(indices(1))), ...
                      real(actual(indices(1))), ...
                      imag(actual(indices(1))), ...
                      relative_differences(indices(1)));
    else
          error_message = ...
              sprintf(['%s: VALUES DIFFER TOO MUCH: expected: %.17g+%.17gi, ', ...
                       'actual: %.17g+%.17gi, relative difference %.17g '], ...
                      error_message, real(expected), imag(expected), ...
                      real(actual), imag(actual), relative_differences);
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
