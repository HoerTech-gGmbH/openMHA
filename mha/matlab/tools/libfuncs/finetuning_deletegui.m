function dummy = finetuning_deletegui
  vObj = findobj('type','uicontrol');
  delete(vObj(strmatch('finetuning_',get(vObj,'tag'))));
  dummy = 0;
