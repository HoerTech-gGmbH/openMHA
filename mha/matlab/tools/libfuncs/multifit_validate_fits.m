function vValid = multifit_validate_fits( csFits, sFit, mha )
  lfa=libfitadaptor();
  sMHACfg = mha_get(mha,sFit.addr);
  vValid = ones(size(csFits));
  for k=1:length(csFits)
    try
      sFitL = csFits{k};
      % This is ugly and relies on consinstent naming in libfitadaptor(): remove
      % 'fitadaptor_' to get the function you want to call
      % This hack replaces to use of function handles which cannot be saved by octave 
      sCurrentFitmodel = merge_structs(eval(['lfa.' sFitL.mha2fitmodel(12:end) '(sMHACfg)']),sFitL.fitmodel);
      % This is the old solution which needs a function handle
      % sCurrentFitmodel = merge_structs(sFitL.mha2fitmodel(sMHACfg),sFitL.fitmodel);
      if ~isequal(sCurrentFitmodel,sFitL.fitmodel)
	vValid(k) = 0;
      end
    catch
      vValid(k) = 0;
    end
  end
