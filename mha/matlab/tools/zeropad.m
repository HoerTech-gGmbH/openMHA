function y = zeropad( x, len )
% ZEROPAD - pad with zeros up to length len
%
% x: input column vector
% len: new length
  ;
  y = zeros(len,size(x,2));
  y(1:min(len,size(x,1)),:) = x(1:min(len,size(x,1)),:);
