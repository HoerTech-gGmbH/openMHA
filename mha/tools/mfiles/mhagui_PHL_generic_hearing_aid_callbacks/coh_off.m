function handle = coh_off(src,event)
handle = guidata(src);
mha = handle.mha;
if handle.rb.coh_off.Value == 0
    set(handle.rb_right.coh_on,'value',1)
    mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.coh.select','coherence')
else
    set(handle.rb.coh_on,'value',0)
    mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.coh.select','identity')
end
end
