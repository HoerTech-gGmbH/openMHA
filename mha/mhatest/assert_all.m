function assert_all(bools, error_message)
% assert_all(bools [, error_message])
%
% assertion that all entries in bools evaluate to true
  
% This file is part of MatlabUnit.  
% Copyright (C) 2003-2005 Medizinische Physik, Universität Oldenburg
% Author: Tobias Herzke
% File Version: $Id: assert_all.m,v 1.4 2005/04/13 16:17:09 tobiasl Exp $

if (nargin < 2)
  error_message = '';
end

if any(~bools)
    indices = find(~bools);
    error_message = ...
        sprintf(['%s: SOME PREDICATES ARE NOT TRUE, first false ', ...
                       'at index (%s)'],...
                      error_message, nice_index(indices(1),size(bools)));
end

unittest_assert(all(bools), error_message);

function nice = nice_index(index, size)
  if any(size == 1)
      nice = num2str(index);
  else
      dummy = zeros(size);
      dummy(index) = 1;
      [i,j] = find(dummy);
      nice = sprintf('(%d,%d)',[i;j]);
  end
  