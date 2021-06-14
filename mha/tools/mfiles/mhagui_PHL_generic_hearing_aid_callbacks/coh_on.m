function handle = coh_on(src,event)
handle = guidata(src);
mha = handle.mha;
if handle.rb.coh_on.Value == 0
    set(handle.rb.coh_off,'value',1)
    mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.coh.select','identity')
else
    set(handle.rb.coh_off,'value',0)
    mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.coh.select','coherence')
end
end
