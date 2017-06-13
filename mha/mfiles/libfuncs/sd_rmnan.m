function s = sd_rmnan( s )
% RMNAN - remove rows with NaN values in data section
%
% Usage:
% s = sd.rmnan(s);
  ;
  subdata = s.data(:,(numel(s.values)+1):numel(s.fields));
  idx = find(~any(isnan(subdata),2));
  s.data = s.data(idx,:);