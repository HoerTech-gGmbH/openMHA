function sGt = finetuning_apply( sFT, sGt )
% apply finetuning to a gain table
  sFT = finetuning_interp_f( sFT, sGt.frequencies );
  for ch='lr'
    sGt.(ch) = sGt.(ch) + repmat(sFT.(ch).gain,[size(sGt.(ch),1),1]);
    sGt.(ch) = min(sGt.(ch),repmat(sFT.(ch).maxgain,[size(sGt.(ch),1),1]));
  end
