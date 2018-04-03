function sFit = multifit_verify( sFit, mha )
  ;
  % verify that current plugin setting is compatible with fit model:
  sCurrentFitmodel = merge_structs(sFit.mha2fitmodel(mha_get(mha,sFit.addr)),sFit.fitmodel);
  if ~isequal(sCurrentFitmodel,sFit.fitmodel)
    error('The current plugin settings are incompatible with the fit.');
  end
