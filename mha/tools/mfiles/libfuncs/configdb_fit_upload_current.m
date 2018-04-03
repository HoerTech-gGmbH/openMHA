function configdb_fit_upload_current(mha, sFT )
  sPlug = configdb_fit_get_current( mha );
  if nargin >= 2
    sPlug.finetuning = sFT;
  end
  if isfield(sPlug,'gaintable')
    libmultifit();
    sPlug.final_gaintable = ...
	multifit.upload(sPlug,mha);
  end
  configdb_fit_set_current(mha,sPlug);
