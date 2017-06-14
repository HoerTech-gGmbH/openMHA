function csRules = multifit_list_gainrules
  [pathstr,name,ext] = fileparts(mfilename('fullpath'));
  d1 = dir([pathstr,filesep,'gainrule_*.m']);
  d2 = dir([pathstr,filesep,'gainrule_*.',mexext]);
  d = [d1(:);d2(:)];
  csRules = cell(length(d),1);
  for k=1:length(d)
    [pathstr,name,ext] = fileparts(d(k).name);
    csRules{k} = name(10:end);
  end
