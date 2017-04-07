function vValid = multifit_validate_fits( csFits, sFit, mha )
  sMHACfg = mha_get(mha,sFit.addr);
  vValid = ones(size(csFits));
  for k=1:length(csFits)
    try
      sFitL = csFits{k};
      sCurrentFitmodel = merge_structs(sFitL.mha2fitmodel(sMHACfg),sFitL.fitmodel);
      if ~isequal(sCurrentFitmodel,sFitL.fitmodel)
	vValid(k) = 0;
      end
    catch
      vValid(k) = 0;
    end
  end
