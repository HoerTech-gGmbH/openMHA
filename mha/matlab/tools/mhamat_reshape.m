function x = mhamat_reshape(x, channels )
% MHAMAT_RESHAPE - reshape matrix from MHA acsave to original
% dimension
%
% Usage:
% y = mhamat_reshape(x, channels );
%
% x        : input matrix (from acsave)
% channels : original number of channels
% y        : reshaped output matrix
%
  len = size(x,1);
  fragsize = size(x,2)/channels;
  x = reshape(x',[channels,len*fragsize])';