function r = range_intersection(r1,r2)
% r, r1, r2 are ranges, i.e. 2 element-vectors where the elements
% constitute the lower bound and the upper bound of an interval.
% Computed is the intersection of the ranges.
% If the ranges do not overlap, an empty range is returned.

r = r1;
if r2(1) > r(1)
    r(1) = r2(1);
end
if r2(2) < r(2)
    r(2) = r2(2);
end
if r(2) < r(1)
    r(2) = r(1);
end
