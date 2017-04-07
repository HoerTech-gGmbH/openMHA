function sMethod = finetuning_get_method( ch )
  h = findobj('tag',['finetuning_method_',ch]);
  if ~isempty(h)
    csMethods = get(h,'String');
    sMethod = csMethods{get(h,'Value')};
  else 
    sMethod = '';
  end
