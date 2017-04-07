function sFT = finetuning_updategui( sFT )
  global finetuning_sFT;
  if nargin >= 1
    if ~isequal(finetuning_sFT.f,sFT.f)
      error('not yet implemented');
    end
    finetuning_sFT = sFT;
  else
    sFT = finetuning_sFT;
  end
  for ch='lr'
    for k=1:length(finetuning_sFT.f)
      h = findobj('tag',sprintf('finetuning_val_%s_%d',ch,k));
      set(h,'String',sprintf('%g',finetuning_get_val(ch,k)));
    end
  end
