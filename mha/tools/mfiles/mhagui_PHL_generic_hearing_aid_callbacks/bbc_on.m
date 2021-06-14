function handle = bbc_on(src,event)
handle = guidata(src);
mha = handle.mha;
if handle.rb.bbc_on.Value == 0
    set(handle.rb.bbc_off,'value',1)
    mha_set(mha,'mha.transducers.calib_out.do_clipping',0)
else
    set(handle.rb.bbc_off,'value',0)
    mha_set(mha,'mha.transducers.calib_out.do_clipping',1)
end
end
