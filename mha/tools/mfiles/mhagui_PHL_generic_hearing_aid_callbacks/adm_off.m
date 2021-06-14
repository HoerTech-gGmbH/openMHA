function handle = adm_off(src,event)
handle = guidata(src);
mha = handle.mha;
if handle.rb.adm_off.Value == 0
    set(handle.rb.adm_on,'value',1)
    mha_set(mha,'mha.transducers.mhachain.split.bte.adm.bypass',0)
    mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.decomb.select','equalize')
else
    set(handle.rb.adm_on,'value',0)
    mha_set(mha,'mha.transducers.mhachain.split.bte.adm.bypass',1)
    mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.decomb.select','identity')
end
end
