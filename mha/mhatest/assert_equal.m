function assert_equal(expected, actual, error_message)
% Gfb_assert_equal(expected, actual [, error_message])
%
% prüft zwei Skalare, Vektoren, Matrizen, oder Structs auf Gleichheit.
% Es ist ein Fehler,
% wenn die Werte nicht gleich sind.
%
% Parameter
% expected      erwarteter Wert
% actual        zu testender Wert
% error_message Sprechende Fehlermeldung für den Fall der Ungleichheit, kann
%               weggelassen werden

% This file is part of MatlabUnit.  
% Copyright (C) 2003-2005 Medizinische Physik, Universität Oldenburg
% Author: Tobias Herzke
% File Version: $Id: assert_equal.m,v 1.5 2005/08/29 07:29:10 tobiasl Exp $

res = isequal(expected, actual);

if (nargin < 3)
  error_message = '';
end
if ~res
      if ischar(expected) 
           if all([length(expected(:)), length(actual(:))] < 80)
               unittest_assert(res, ...
			       sprintf('%s: expected "%s", got "%s"', ...
				       error_message, expected, actual));
           elseif length(expected(:)) < 80
               unittest_assert(res, ...
			       sprintf('%s: expected "%s", got "%s[...]"', ...
				       error_message, expected, actual(1:80)));
           end
      elseif isstruct(expected)
        if isstruct(actual)
          error_message = sprintf('%s: Structs differ', error_message);
        else
          error_message = sprintf('%s: Expected struct, got %s', ...
				  error_message, class(actual));
        end
      elseif ~isequal(size(expected),size(actual))
          error_message = sprintf('%s: SIZE DIFFERS (expected [%s]; actual [%s]):',...
                                  error_message, ...
                                  num2str(size(expected)), ...
                                  num2str(size(actual)));
      elseif length(expected) > 1
          indices = find(expected ~= actual);
          error_message = ...
              sprintf('%s: VALUES DIFFER, first difference at index %d: %f~=%f ',...
                      error_message, ...
                      indices(1), expected(indices(1)), actual(indices(1)));
      else
          is_octave = exist('OCTAVE_VERSION', 'builtin') ~= 0;
          if (is_octave)
            expected_str = disp(expected);
            actual_str = disp(actual);
          else
            expected_str = evalc('disp(expected)');
            actual_str = evalc('disp(actual)');
          end
          error_message = ...
              sprintf('%s: VALUES DIFFER: expected: %s, actual: %s ', ...
                      error_message, ...
                      strip_string(expected_str), ...
                      strip_string(actual_str));
      end
end


unittest_assert(res, error_message);

function string = strip_string(string)
% strips leading and trailing white space from string
  finished = 0;
  while ~finished
    if isempty(string)
      string = '';
      finished = 1;
    elseif isspace(string(1))
      string = string(2:end);
    elseif isspace(string(end))
      string = string(1:end-1);
    else
      finished = 1;
    end
  end
