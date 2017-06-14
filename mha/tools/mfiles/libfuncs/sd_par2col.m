function s = sd_par2col( s, kPar )
% convert a parameter field into additional data columns
% s  : data structure
% kPar : number or name of parameter column
%
%Example:
% sIn.fields = {'p','numeric','a','b'};
% sIn.values = {{'X','Y'},[1:3]};
% sIn.data = [...
%  1, 1, 0.81, 0.27;...
%  2, 1, 0.90, 0.54;...
%  1, 2, 0.12, 0.95;...
%  2, 2, 0.91, 0.96;...
%  1, 3, 0.63, 0.15;...
%  2, 3, 0.09, 0.97;...
% ];
% sOut = sd.par2col( sIn, 'numeric' );
%
%Note:
% This operation can not be inverted by col2par. The fields 
% need to be reordered first.
%
%Author: 
% Giso Grimm, 12/2008
  if ischar(kPar)
    kPar = strmatch(kPar,s.fields,'exact');
  end
  if prod(size(kPar))~= 1
    error('invalid parameter');
  end
  s = sd_compactval( s );
  % constants:
  nPar = length(s.values);
  nFields = length(s.fields);
  nData = nFields-nPar;
  vVal = s.values{kPar};
  % remove control column from parameter block:
  par_out = s.data(:,1:nPar);
  par_out(:,kPar) = [];
  par_out = unique(par_out,'rows');
  nEntrNew = size(par_out,1);
  % check the dimensions:
  if nEntrNew * numel(s.values{kPar}) ~= size(s.data,1)
    disp('s.fields');
    s.fields
    sd_getdim(s)
    disp('size(s.data,1)');
    disp(size(s.data,1));
    nEntrNew
    disp('s.values{kPar}');
    disp(s.values{kPar});
    warning('Dimension mismatch');
  end
  data_out = zeros(nEntrNew,nData*length(vVal));
  data_out(:) = nan;
  sFields = {};
  for k=1:length(vVal)
    % create new field names:
    if iscell(vVal)
      sVal = vVal{k};
    else
      sVal = num2str(vVal(k));
    end
    for kf=1:nData
      sFields{end+1} = sprintf('%s, %s',s.fields{kf+nPar},sVal);
    end
    % select data subset:
    sTmp = sd_restrict(s, kPar, k );
    % remove control field from parameter set:
    mpar = sTmp.data(:,1:nPar);
    mpar(:,kPar) = [];
    if size(unique(mpar,'rows'),1) ~= size(mpar,1)
      error(['duplicate parameters for field ',...
	     s.fields{kPar},'=',sVal]);
    end
    %
    for kd=1:size(mpar,1)
      idx = find(ismember(par_out,mpar(kd,:),'rows'));
      data_out(idx,[1:nData]+(k-1)*nData) = sTmp.data(kd,nPar+1:end);
    end
  end
  s.fields = s.fields(1:nPar);
  s.fields(kPar) = [];
  s.values(kPar) = [];
  s.fields = [s.fields,sFields];
  s.data = [par_out,data_out];