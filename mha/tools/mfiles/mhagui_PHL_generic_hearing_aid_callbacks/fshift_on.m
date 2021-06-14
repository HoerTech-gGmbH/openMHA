function handle = fshift_on(src,event)
handle = guidata(src);
mha = handle.mha;
if handle.rb.fshift_on.Value == 0
    set(handle.rb.fshift_off,'value',1)
    mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.fshift.select','identity')
else
    set(handle.rb.fshift_off,'value',0)
    mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.fshift.select','fshift')
end
end
