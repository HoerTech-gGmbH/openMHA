function handle = tau_cllb(src,event)
handle = guidata(src);
mha = handle.mha;

edit_tag = get(gcbo,'tag');
switch edit_tag
    case 'edit_r_AT'
        AT = get(handle.AT_right_edit,'string');
        if str2double(AT) < 0
            AT = num2str(0);
        elseif str2double(AT) >= 1
            AT = num2str(0.999);
        end
        set(handle.AT_right_edit,'string',[AT ' s']);
        set(handle.AT_left_edit,'string',[AT ' s']);
        mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.tau_attack',str2double(AT));
    case 'edit_r_RT'
        RT = get(handle.RT_right_edit,'string');
        if str2double(RT) < 0
            RT = num2str(0);
        elseif str2double(RT) >= 1
            RT = num2str(0.999);
        end
        set(handle.RT_right_edit,'string',[RT ' s']);
        set(handle.RT_left_edit,'string',[RT ' s']);
        mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.tau_decay',str2double(RT));
    case 'edit_l_AT'
        AT = get(handle.AT_left_edit,'string');
        if str2double(AT) < 0
            AT = num2str(0);
        elseif str2double(AT) >= 1
            AT = num2str(0.999);
        end
        set(handle.AT_right_edit,'string',[AT ' s']);
        set(handle.AT_left_edit,'string',[AT ' s']);
        mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.tau_attack',str2double(AT));
    case 'edit_l_RT'
        RT = get(handle.RT_left_edit,'string');
        if str2double(RT) < 0
            RT = num2str(0);
        elseif str2double(RT) >= 1
            RT = num2str(0.999);
        end
        set(handle.RT_right_edit,'string',[RT ' s']);
        set(handle.RT_left_edit,'string',[RT ' s']);
        mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.tau_decay',str2double(RT));
end
end
