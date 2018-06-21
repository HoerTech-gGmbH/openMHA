function random = repeatable_rand(rows, colums, rand_state)
% random = repeatable_rand(rows, colums, rand_state)
%
% invokes the random number generator with given rows and columns
% arguments, after setting the state of the random number generator to
% rand_state. 
%
% The random number generator is restored to its previous state after
% generating the desired random number matrix.
  
% This file is part of MatlabUnit.  
% Copyright (C) 2003-2005 Medizinische Physik, Universität Oldenburg
% Author: Tobias Herzke

% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2004 2005 2006 2013 2018 HörTech gGmbH
%
% openMHA is free software: you can redistribute it and/or modify
% it under the terms of the GNU Affero General Public License as published by
% the Free Software Foundation, version 3 of the License.
%
% openMHA is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU Affero General Public License, version 3 for more details.
%
% You should have received a copy of the GNU Affero General Public License, 
% version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

old_rand_state =  rand('state');
rand('state', rand_state);
random = rand(rows, colums);
rand('state', old_rand_state);

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
