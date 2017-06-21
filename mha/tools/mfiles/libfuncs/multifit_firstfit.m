function sPlug = multifit_firstfit( sPlug, sGainrule, sAud, sSide )
% fill gaintable structure
%
% sPlug     : plugin handle
% sGainrule : name of gainrule
% sAud      : auditory profile
% sSide     : side to fit (l, r, lr, rl)
  switch sSide
   case {'l','r','lr','rl'}
   otherwise
    error(['invalid side "',sSide,'"']);
  end
  sPlug.audprof = sAud;
  % update the fitting model (i.e., hearing aid model):
  %sPlug.fitmodel = feval(sPlug.mha2fitmodel,sPlug.plugincfg);
  sPlug.fitmodel.side = sSide;
  % apply gain rule and create gain table:
  sPlug.gaintable = feval(['gainrule_',sGainrule],sAud,sPlug.fitmodel);
  sPlug.gaintable = merge_structs(sPlug.fitmodel,sPlug.gaintable);
  sPlug.gainrule = sGainrule;
