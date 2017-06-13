function csPlugs = multifit_uploadallfirstfit( mha, sGainrule, sAud, sSide )
% upload First Fit to all fittable plugins
%
% mha       : MHA handle
% sGainrule : name of gain prescription rule, or empty to
%             autodetect by plugin name
% sAud      : auditory profile
% sSide     : side to fit (l, r, lr, rl)
  
  sGRuleLocal = sGainrule;
  csPlugs = multifit_query(mha);
  for k=1:length(csPlugs)
    sFit = csPlugs{k};
    if isempty( sGainrule )
      sGRuleLocal = sFit.addr;
      sGRuleLocal(1:max(find(sGRuleLocal=='.'))) = [];
      if ~exist(['gainrule_',sGRuleLocal])
	msg = ['Gainrule ''',sGRuleLocal,''' does not exist.'];
	errordlg(msg);
	error(msg);
      end
    end
    sFit = multifit_firstfit( sFit, sGRuleLocal, sAud, sSide );
    multifit_upload( sFit, mha );
    csPlugs{k} = sFit;
  end
