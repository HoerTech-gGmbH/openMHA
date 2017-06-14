function csHelp = multifit_gainrule_doc
  csRules = multifit_list_gainrules;
  csHelp = [csRules,csRules];
  for k=1:size(csRules,1)
    csHelp{k,2} = help(['gainrule_',csRules{k}]);
  end
