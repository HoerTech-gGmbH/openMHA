function handle = bbc_off(src,event)
handle = guidata(src);
mha = handle.mha;
if handle.rb.bbc_off.Value == 0
    set(handle.rb.bbc_on,'value',1)
    mha_set(mha,'mha.transducers.calib_out.do_clipping',1)
else
    set(handle.rb.bbc_on,'value',0)
    mha_set(mha,'mha.transducers.calib_out.do_clipping',0)
end
end
