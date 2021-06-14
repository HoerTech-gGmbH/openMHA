function [handle,dat] = mbc_on(src,event)
handle = guidata(src);
mha = handle.mha;
if handle.rb.mbc_on.Value == 0
    set(handle.rb.mbc_off,'value',1);
    set(handle.gain_right.Children(6:end-2),'enable','off');
    set(handle.mpo_right.Children,'enable','off');
    set(handle.exp_right.Children,'enable','off');
    set(handle.gain_left.Children(6:end-2),'enable','off');
    set(handle.mpo_left.Children,'enable','off');
    set(handle.exp_left.Children,'enable','off');
    mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.select','identity')
else
    set(handle.rb.mbc_off,'value',0);
    set(handle.gain_right.Children(6:end-2),'enable','on');
    if handle.sel_all_G50_right.Value == 0
        set(handle.gain_right.Children(45),'enable','off');
    else
        set(handle.gain_right.Children(48:57),'enable','off');
    end
     if handle.sel_all_G80_right.Value == 0
        set(handle.gain_right.Children(59),'enable','off');
    else
        set(handle.gain_right.Children(62:71),'enable','off');
    end
    set(handle.mpo_right.Children,'enable','on');
    if handle.sel_all_mpo_right.Value == 0
        set(handle.mpo_right.Children(1:2),'enable','off');
    else
        set(handle.mpo_right.Children(3:13),'enable','off');
    end
    set(handle.exp_right.Children,'enable','on');
    if handle.sel_all_exp_right.Value == 0
        set(handle.exp_right.Children(2),'enable','off');
        else
        set(handle.exp_right.Children(4:8),'enable','off');
    end
    set(handle.gain_left.Children(6:end-2),'enable','on');
    if handle.sel_all_G50_left.Value == 0
        set(handle.gain_left.Children(45),'enable','off');
    else
        set([handle.gain_left.Children(48:57)],'enable','off');
    end
    if handle.sel_all_G80_left.Value == 0
        set(handle.gain_left.Children(59),'enable','off');
    else
        set([handle.gain_left.Children(62:71)],'enable','off');
    end
    set(handle.mpo_left.Children,'enable','on');
    if handle.sel_all_mpo_left.Value == 0
        set(handle.mpo_left.Children(1:2),'enable','off');
    else
        set(handle.mpo_left.Children(3:13),'enable','off');
    end
    set(handle.exp_left.Children,'enable','on');
    if handle.sel_all_exp_left.Value == 0
        set(handle.exp_left.Children(2),'enable','off');
    else
        set(handle.exp_left.Children(4:8),'enable','off');
    end
    mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.select','multibandcompressor')
    [handle] = maxGainCheck(handle);
end
end
