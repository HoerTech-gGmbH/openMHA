function assert_not_equal(not_expected, actual, error_message)
% Gfb_assert_not_equal(not_expected, actual [, error_message])
%
% prüft zwei Skalare, Vektoren, Matrizen, oder Structs auf Verschiedenheit.
% Es ist ein Fehler, wenn die Werte gleich sind.
%
% Parameter
% not_expected  Wert, der nicht erwartet wird
% actual        zu testender Wert
% error_message Fehlermeldung für den Fall der Gleichheit, kann weggelassen
%               werden

% This file is part of MatlabUnit.  
% Copyright (C) 2003-2005 Medizinische Physik, Universität Oldenburg
% Author: Tobias Herzke
% File Version: $Id: assert_not_equal.m,v 1.3 2005/04/13 16:17:09 tobiasl Exp $

if (nargin < 3)
  error_message = '';
end

unittest_assert(~isequal(not_expected, actual), error_message);
