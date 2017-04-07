function [sOut,mFields] = sd_col2par( sIn, sParName, cValues, csDataNames )
% convert multiple data columns into a new parameter column
% sParName : new parameter column name
% cValues : values of new parameter column
% csDataNames : names of resulting data columns
%
%Note: 
% If field order is {'a1','a2','a3','b1','b2','b3'} then use
%
% sOut = sd.col2par(sIn,'numeric',{'1','2','3'},{'a','b'});
%
% or
%
% sOut = sd.col2par(sIn,'numeric',1:3,{'a','b'});
%
% This means that the new parameter field must be in interleaved
% order in the input data fields.
%
% Check consistency of mFields to avoid unexpected results.
  ;
  % get number of fields:
  nParIn = length(sIn.values);
  nDataIn = size(sIn.data,2)-nParIn;
  nElem = size(sIn.data,1);
  nParElem = length(cValues);
  nDataOut = length(csDataNames);
  if( length(csDataNames) * length(cValues) ~= nDataIn )
    error(sprintf(['Number of values (%d) times number of remaining data' ...
		   ' fields (%d) must be number of input data fields' ...
		   ' (%d).'],length(csDataNames),length(cValues),nDataIn));
  end
  mParBlock = repmat([sIn.data(:,1:nParIn),zeros(nElem,1)],[nParElem,1]);
  mDataIn = sIn.data(:,nParIn+[1:nDataIn]);
  for k=1:nParElem
    mParBlock([1:nElem]+(k-1)*nElem,end) = k;
  end
  mDataOut = reshape(mDataIn,[nElem*nParElem,nDataOut]);
  sOut = struct;
  for k=1:length(csDataNames)
    if strcmp(class(csDataNames{k}),'function_handle')
      csDataNames{k} = func2str(csDataNames{k});
    end
  end
  sOut.fields = [sIn.fields(1:nParIn),{sParName},csDataNames];
  sOut.values = [sIn.values,{cValues}];
  sOut.data = [mParBlock,mDataOut];
  idx = find(~any(~isnan(mDataOut),2));
  sOut.data(idx,:) = [];
  mFields = reshape(sIn.fields(nParIn+[1:nDataIn]),[nParElem,nDataOut]);