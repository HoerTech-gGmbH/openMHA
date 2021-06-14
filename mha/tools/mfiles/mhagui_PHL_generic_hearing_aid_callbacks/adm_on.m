function handle = adm_on(src,event)
handle = guidata(src);
mha = handle.mha;
if handle.rb.adm_on.Value == 0
    set(handle.rb.adm_off,'value',1)
    mha_set(mha,'mha.transducers.mhachain.split.bte.adm.bypass',1)
    mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.decomb.select','identity')
else
    set(handle.rb.adm_off,'value',0)
    mha_set(mha,'mha.transducers.mhachain.split.bte.adm.bypass',0)
    mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.decomb.select','equalize')
end
end
