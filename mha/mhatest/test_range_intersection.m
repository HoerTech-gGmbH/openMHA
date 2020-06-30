function test_range_intersection
% Tests of our helper function range_intersection
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2020 HörTech gGmbH
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

  % Intersection of 2 identical ranges is identical to input
  assert_equal([0,1], range_intersection([0,1],[0,1]));

  % Intersection of 1 large range and a short completely contained range is the
  % short contained range
  assert_equal([0,1], range_intersection([0,1],[-9,9]));
  assert_equal([0,1], range_intersection([-9,9],[0,1]));

  % same if the short contained range is at the very beginning or very end
  assert_equal([0,1], range_intersection([0,1],[-9,1]));
  assert_equal([0,1], range_intersection([-9,1],[0,1]));
  assert_equal([0,1], range_intersection([0,1],[0,9]));
  assert_equal([0,1], range_intersection([0,9],[0,1]));
  
  % Two large ranges that intersect only for a short interval where 1 begins
  % and the other ends
  assert_equal([0,1], range_intersection([0,9],[-9,1]));
  assert_equal([0,1], range_intersection([-9,1],[0,9]));
 
  % Empty intersection: lower bound is equal to upper bound, diff zero
  assert_equal(0, diff(range_intersection([1,9],[-9,-1])));
  assert_equal(0, diff(range_intersection([-9,-1],[1,9])));
  
  % These overlaps are also empty because of exclusive upper bounds:
  assert_equal(0, diff(range_intersection([0,9],[-9,0])));
  assert_equal(0, diff(range_intersection([-9,0],[0,9])));

  % Inf is handled correctly
  assert_equal([0,1], range_intersection([0,1],[-Inf,Inf]));
  assert_equal([0,1], range_intersection([-Inf,Inf],[0,1]));
