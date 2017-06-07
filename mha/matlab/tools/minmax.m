function pr=minmax(p)
%MINMAX Ranges of matrix rows.
%
%  Syntax
%
%    pr = minmax(p)
%
%  Description
%
%    MINMAX(P) takes one argument,
%      P - RxQ matrix.
%    and returns the Rx2 matrix PR of minimum and maximum values
%    for each row of P.
%
%    Alternately, P can be an MxN cell array of matrices.  Each matrix
%    P{i,j} should Ri rows and Q columns.  In this case, MINMAX returns
%    an Mx1 cell array where the mth matrix is an Rix2 matrix of the
%    minimum and maximum values of elements for the matrics on the
%    ith row of P.
%
%  Examples
%
%    p = [0 1 2; -1 -2 -0.5]
%    pr = minmax(p)
%
%    p = {[0 1; -1 -2] [2 3 -2; 8 0 2]; [1 -2] [9 7 3]};
%    pr = minmax(p)

% Mark Beale, 11-31-97
% Copyright 1992-2002 The MathWorks, Inc.
% $Revision: 1.1 $


if iscell(p)
  [m,n] = size(p);
  pr = cell(m,1);
  for i=1:m
    pr{i} = minmax([p{i,:}]);
  end
elseif isa(p,'double')
  pr = [min(p,[],2) max(p,[],2)];
else
  error('Argument has illegal type.')
end
