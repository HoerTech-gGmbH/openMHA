function [COMPARISON,MEANS,H,GNAMES] = sd_posthoc( sData, DataCol,vParCol)
% call multcompare for each factor
  [p,t,stats,terms] = sd_anovan(sData,DataCol,vParCol);
  %,'display','off'
  for k=1:numel(vParCol)
    figure
    [COMPARISON,MEANS,H,GNAMES] = ...
	multcompare(stats,'ctype','lsd','dimension',k,'estimate','anovan');
  end
  