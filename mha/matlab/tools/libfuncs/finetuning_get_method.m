function sMethod = finetuning_get_method( ch )
  cf_children = get(gcf,'children');
  h = cf_children(strcmp(get(cf_children,'tag'),['finetuning_method_',ch]));
  if ~isempty(h)
    csMethods = get(h,'String');
    sMethod = csMethods{get(h,'Value')};
  else 
    sMethod = '';
  end
