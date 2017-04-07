function y = dirac_delta( len )
  if nargin < 1
    len = 1;
  end
  y = zeros(len,1);
  y(1) = 1;
