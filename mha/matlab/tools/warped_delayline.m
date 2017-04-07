function y = warped_delayline( x, len, rho )
  x = x(:,1);
  y = zeros(size(x,1),len);
  B = [-rho 1];
  A = [1 -rho];
  y(:,1) = x;
  for k=2:len
    y(:,k) = filter(B,A,y(:,k-1));
  end
