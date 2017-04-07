function sFT = finetuning_interp_f( sFT, f )
  interpf = [log(min(min(sFT.f),min(f)))-1,...
	     log(sFT.f(:)'),...
	     log(max(max(sFT.f),max(f)))+1];
  for ch='lr'
    for fn=fieldnames(sFT.(ch))'
      data = sFT.(ch).(fn{:})(:)';
      data = [data(1),data,data(end)];
      sFT.(ch).(fn{:}) = ...
	  interp1(interpf,data,log(f),...
		  'linear','extrap');
    end
  end
  sFT.f = f;
