function val = finetuning_get_val( ch, band )
  global finetuning_sFT;
  sMethod = finetuning_get_method(ch);
  if ~isempty(sMethod)
    if nargin == 2
      val = finetuning_sFT.(ch).(sMethod)(band);
    else
      val = finetuning_sFT.(ch).(sMethod);
    end
  else
    val = [];
  end
