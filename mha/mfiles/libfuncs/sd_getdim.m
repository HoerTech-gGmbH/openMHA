function vDim = sd_getdim( s )
% STRUCT_DATA_GETDIM - return dimension of data
%
% Usage:
% vDim = sd_getdim( s )
%
% Author: Giso Grimm
% Date: 11/2008
  ;
  vDim = zeros(1,size(s.data,2));
  for k=1:length(vDim)
    vDim(k) = length(unique(s.data(:,k)));
  end
