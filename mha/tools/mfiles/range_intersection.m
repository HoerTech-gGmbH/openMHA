function r = range_intersection(r1,r2)
% r = range_intersection(r1,r2)
% r, r1, r2 are ranges, i.e. 2 element-vectors where the element at index 1
% constitutes the inclusive lower bound and the element at index 2 is the 
% exclusive upper bound of a range of real numbers.
% If the lower bound value of one range is equal to its upper bound value,
% then this range is considered empty.
% This function computes the intersection of both input ranges.
% If the ranges do not overlap, then an empty range where lower and upper bound
% are equal is returned.  This is done so that the extent of the returned range
% can be computed with diff(r).
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2009 2013 2017 2019 2020 HörTech gGmbH

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

% Ensure arguments are pairs of real numbers
assert(all(size(r1) == [1,2]));
assert(all(size(r2) == [1,2]));
assert(all(~isnan([r1,r2])));
assert(isreal([r1,r2]));

% Start with result = 1st argument, then reduce its extent if needed.
r = r1;
if r2(1) > r(1)
    r(1) = r2(1);
end
if r2(2) < r(2)
    r(2) = r2(2);
end

% Disallow negative widths
if r(2) < r(1)
    r(2) = r(1);
end
