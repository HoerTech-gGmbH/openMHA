function [s1, s2] = sd_uniquepar( s1, kField1, s2, kField2, func )
% sd_unify_param - combine parameter values and use common
% index
%
% Usage:
% [s1, s2] = sd_unify_param( s1, kField1, s2, kField2 [, func ] )
%
% s1     : first data structure
% kField1: field number in first structure to be merged
% s2     : second data structure
% kField2: field number in second structure to be merged
% func   : Merge function to be used (optional)
%          Typically '@intersect' (default) or '@union'.
%
% Author: Giso Grimm
% Date: 11/2008
  ;
  if nargin < 5
    % union or intersect
    func = @intersect;
  end
  vVal1 = s1.values{kField1};
  vVal2 = s2.values{kField2};
  if ~isequal(class(vVal1),class(vVal2))
    error('Parameter values are of different classes');
  end
  % get combined values and transform indicees:
  vVal = func(vVal1,vVal2);
  for k=1:size(s1.data,1)
    idx = strmatch( vVal1{s1.data(k,kField1)}, vVal, 'exact' );
    if isempty(idx)
      idx = nan;
    end
    s1.data(k,kField1) = idx;
  end
  for k=1:size(s2.data,1)
    idx = strmatch( vVal2{s2.data(k,kField2)}, vVal, 'exact' );
    if isempty(idx)
      idx = nan;
    end
    s2.data(k,kField2) = idx;
  end
  s1.data(find(~isfinite(s1.data(:,kField1))),:) = [];
  s2.data(find(~isfinite(s2.data(:,kField2))),:) = [];
  s1.values{kField1} = vVal;  
  s2.values{kField2} = vVal;
  
