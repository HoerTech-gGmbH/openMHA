function [s, csFields] = sd_average( s, vkField, cFunc )
% calculate average across specified fields
%
% Usage:
% [s, csFields] = sd_average( s, vkField [, cFunc ] )
%
% Input arguments:
% s       : data structure.
% vkField : field numbers to calculate the average.
% cFunc   : cell array of function handles (optional).
%           Functions may take a matrix of values as input and must
%           return a row vector (e.g., '{@mean}').
%
% Return values:
% s  : modified data structure with extra data fields 'N' (number
%      of fields) and the processed value of each data column for 
%      each specified function (e.g., 'mean(Y)').
% csFields : names of processed fields.
%
% Author: Giso Grimm
% Date: 11/2008
  ;
  if nargin < 3
    cFunc = {@mean,@std};
  end
  if ischar(vkField)
    vkField = strmatch(vkField,s.fields,'exact');
  end
  % get lengths:
  nParIn = length(s.values);
  nFieldsIn = length(s.fields);
  nData = nFieldsIn-nParIn;
  %
  csFields = s.fields(vkField);
  ridx = setdiff(1:nParIn,vkField);
  mKeep = unique(s.data(:,ridx),'rows');
  % create field names:
  sOut.fields = [s.fields(ridx),'N'];
  sOut.values = s.values(ridx);
  for kFun=1:length(cFunc)
    for kData=1:nData
      sOut.fields{end+1} = sprintf('%s(%s)',func2str(cFunc{kFun}),s.fields{nParIn+kData});
    end
  end
  % select data and calc functions:
  sOut.data = zeros(0,size(mKeep,2)+1+length(cFunc)*nData);
  for kKeep=1:size(mKeep,1)
    idx = find(ismember(s.data(:,ridx),mKeep(kKeep,:),'rows'));
    mSubData = s.data(idx,(nParIn+1):end);
    mRes = length(idx);
    for kFun=1:length(cFunc)
      mRes = [mRes,cFunc{kFun}(mSubData)];
    end
    sOut.data(end+1,:) = [mKeep(kKeep,:),mRes];
  end
  s = sOut;