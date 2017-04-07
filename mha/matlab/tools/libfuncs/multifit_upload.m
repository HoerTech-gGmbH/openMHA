function sGt = multifit_upload( sFit, mha )

  sGt = sFit.gaintable;
  if isfield(sFit,'finetuning')
    libfinetuning();
    sGt = finetuning.apply(sFit.finetuning,sGt);
  end
  % create noisegate:
  sGt = multifit_apply_noisegate( sGt );
  sGt.fit = sFit;
  % create MHA configuration and upload:
  
  % TODO: a temporary hack -----------------------
  % finds the function_handle in the library by value
  lfau = libfitadaptor();
  fields = fieldnames(lfau);
  for i = 1:numel(fields)
    if ~isstruct(lfau.(fields{i}))
      if isequal(func2str(sFit.gaintable2mha),func2str(lfau.(fields{i})))
        sFit.gaintable2mha = lfau.(fields{i});
        break;
      end
     end 
  end
  %------------------------------------------------
  mhacfg = sFit.gaintable2mha(sGt,sFit.plugincfg);
  mha_set(mha,sFit.addr,mhacfg);

