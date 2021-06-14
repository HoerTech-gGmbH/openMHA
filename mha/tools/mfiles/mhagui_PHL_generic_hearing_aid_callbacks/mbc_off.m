function handle = mbc_off(src,event)
handle = guidata(src);
mha = handle.mha;
if handle.rb.mbc_off.Value == 0
    set(handle.rb.mbc_on,'value',1)
    set(handle.gain_right.Children(6:end-2),'enable','on');
    set(handle.mpo_right.Children,'enable','on');
    set(handle.exp_right.Children,'enable','on');
    set(handle.gain_left.Children(6:end-2),'enable','on');
    set(handle.mpo_left.Children,'enable','on');
    set(handle.exp_left.Children,'enable','on');
    mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.select','multibandcompressor')
else
    set(handle.rb.mbc_on,'value',0)
    set(handle.rb.mbc_off,'value',1);
    set(handle.gain_right.Children(6:end-2),'enable','off');
    set(handle.mpo_right.Children,'enable','off');
    set(handle.exp_right.Children,'enable','off');
    set(handle.gain_left.Children(6:end-2),'enable','off');
    set(handle.mpo_left.Children,'enable','off');
    set(handle.exp_left.Children,'enable','off');
    mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.select','identity')
end
end
% isequal(handles.gain_right.Children(28).BackgroundColor,[.5 .5 .5])
