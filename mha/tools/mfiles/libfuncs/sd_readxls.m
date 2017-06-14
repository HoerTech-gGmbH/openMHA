function s = sd_readxls( fname, labelrow, datarows, parcols, datacols )
% STRUCT_DATA_READXLS - read an xls spreadsheet and convert to structdata
%
% Usage:
% s = sd_readxls( fname, labelrow, datarows, parcols, datacols )
%
% Author: Giso Grimm, 9/2010
  ;
  [num,txt,raw] = xlsread(fname,1,'','basic');
  s = struct;
  s.fields = txt(labelrow,[parcols,datacols]);
  s.values = {};
  s.data = zeros(numel(datarows),numel(parcols)+numel(datacols));
  for k=1:numel(parcols)
    %vNumPar = num(datarows,parcols(k));
    %vTxtPar = txt(datarows,parcols(k));
    vRaw = raw(datarows,parcols(k));
    if iscellstr(vRaw)
      s.values{k} = vRaw;
    else
      s.values{k} = cell2mat(vRaw);
    end
    s.data(:,k) = 1:numel(datarows);
  end
  for k=1:numel(datacols)
    s.data(:,numel(parcols)+k) = cell2mat(raw(datarows,datacols(k)));
  end
  s = sd_compactval( s );