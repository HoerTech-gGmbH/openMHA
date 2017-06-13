function [csList,sCur,Idx] = configdb_fit_preset_list( mha )
  libmultifit();
  try
    sPlug = configdb_fit_get_current(mha);
    csPresets = configdb_clientconfig_get( mha,[sPlug.addr,'.presets'],cell([2 0]));
    vValidPresets = multifit.validate_fits(csPresets(2,:),sPlug,mha);
    idx_invalid = any(1-vValidPresets);
    csPresets(:,idx_invalid) = [];
    csList = csPresets(1,:);
    sCur = configdb_clientconfig_get( mha,[sPlug.addr,'.current_preset'],'');
    if isempty(sCur) && ~isempty(csList)
      sCur = csList{1};
    end
    Idx = strmatch(sCur,csList,'exact');
    if isempty(Idx)
      idx = 1;
    end
  catch
    disp_err_rethrow;
  end
