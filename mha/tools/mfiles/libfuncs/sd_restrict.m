function s = sd_restrict( s, kField, vValueIdx, bInvert )
% select a data subset by applying restrictions
%
% s         : data structure
% kField    : array of field number indices, or
%             single field name
% vValueIdx : array of value indices, or 
%             cell array of arrays of values
% bInvert   : invert restrictions (keep unmatched values)
%
% Author: Giso Grimm, 2007
  if ischar(kField)
    sName = kField;
    kField = strmatch(sName,s.fields,'exact');
    if isempty(kField)
      error(sprintf('No field ''%s''.',sName));
    end
  end
  if ischar(vValueIdx)
    vValueIdx = {vValueIdx};
  end
  if nargin < 4
    bInvert = false;
  end
  if iscell(vValueIdx)
    cValues = vValueIdx;
    vValueIdx = [];
    for k=1:length(kField)
      cAvailVal = s.values{kField(k)};
      if iscell(cAvailVal)
	idx = strmatch(cValues{k},cAvailVal,'exact');
      else
	idx = find(cValues{k}==cAvailVal);
      end
      idx(end+1) = 0;
      vValueIdx(end+1) = idx(1);
    end
  end
  idx = find(ismember(s.data(:,kField),vValueIdx,'rows'));
  if bInvert
    idx = setdiff(1:size(s.data,1),idx);
  end
  s.data = s.data(idx,:);