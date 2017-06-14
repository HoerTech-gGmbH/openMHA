function [P,T,STATS,TERMS] = sd_anovan( sData, DataCol, vParCol, varargin )
% anovan version of struct data - see anovan for help
  Y = sData.data(:,DataCol);
  GROUP = {};
  for k=vParCol
    GROUP{end+1} = sData.values{k}(sData.data(:,k))';
  end
  [P,T,STATS,TERMS] = anovan(Y,GROUP,'varnames',sData.fields(vParCol), varargin{:});
