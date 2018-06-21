function sFT = finetuning_triggerguicallback
  global finetuning_sFT;
  sFT = finetuning_sFT;
  vh = findobj('tag','finetuning_frame');
  if ~isempty(vh)
    for h=vh(:)'
      ud = get(h,'UserData');
      if ~isempty(ud)
	if isstruct(ud)
	  if isfield(ud,'callback')
	    ud.callback(finetuning_sFT,ud.callback_data{:});
	    return;
	  end
	end
      end
    end
  end
