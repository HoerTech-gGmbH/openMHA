function s = sd_compactval( s )
% remove unused entries from value vectors
%
% Author: Giso Grimm
% Date: 11/2008
  ;
  for kp=1:length(s.values)
    % first remove unused values:
    iidx = zeros(length(s.values{kp}),1);
    idx1 = unique(s.data(:,kp));
    idx = 1:length(idx1);
    iidx(idx1) = idx;
    s.values{kp} = s.values{kp}(idx1);
    s.data(:,kp) = iidx(s.data(:,kp));
    % now remove duplicates (without resorting):
    [s.values{kp},idx] = unique_unsorted_(s.values{kp});
    s.data(:,kp) = idx(s.data(:,kp));
  end

function [dout,idx] = unique_unsorted_( din )
  dout = din([]);
  idx = zeros(size(din));
  for k=1:numel(din)
    [ism,loc] = ismember(din(k),dout);
    if ~ism
      dout(end+1) = din(k);
      idx(k) = numel(dout);
    else
      idx(k) = loc;
    end
  end
  