function handle = maxgain_sel_all(src,event)
handle = guidata(src);
mha = handle.mha;

maxgain = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain');
edit_tag = get(gcbo,'tag');
switch edit_tag
    case 'all_right'
        if handle.sel_all_maxgain_right.Value == 0
            set([handle.maxgain_right_edit1 handle.maxgain_right_edit2 handle.maxgain_right_edit3 handle.maxgain_right_edit4 handle.maxgain_right_edit5],...
                'visible','on');
            set([handle.cf1_right handle.cf2_right handle.cf3_right handle.cf4_right handle.cf5_right],...
                'visible','on');
            set([handle.maxgain_all_edit_right handle.bb_right],'visible','off');
            set(handle.sel_all_maxgain_right,'position',[0.44 0.85 0.1 0.2]);
        elseif handle.sel_all_maxgain_right.Value == 1
            set([handle.maxgain_right_edit1 handle.maxgain_right_edit2 handle.maxgain_right_edit3 handle.maxgain_right_edit4 handle.maxgain_right_edit5],...
                'visible','off');
            set([handle.cf1_right handle.cf2_right handle.cf3_right handle.cf4_right handle.cf5_right],...
                'visible','off');
            set([handle.maxgain_all_edit_right handle.bb_right],'visible','on');
            set(handle.sel_all_maxgain_right,'position',[0.3 0.85 0.1 0.2]);
            if isequal(maxgain(6:10),repmat(maxgain(6),1,5))
                set(handle.maxgain_all_edit_right,'string',[num2str(maxgain(6)) ' dB']);
            else
                set(handle.maxgain_all_edit_right,'string','');
            end
        end
    case 'all_left'
        if handle.sel_all_maxgain_left.Value == 0
            set([handle.maxgain_left_edit1 handle.maxgain_left_edit2 handle.maxgain_left_edit3 handle.maxgain_left_edit4 handle.maxgain_left_edit5],...
                'visible','on');
            set([handle.cf1_left handle.cf2_left handle.cf3_left handle.cf4_left handle.cf5_left],...
                'visible','on');
            set([handle.maxgain_all_edit_left handle.bb_left],'visible','off');
            set(handle.sel_all_maxgain_left,'position',[0.45 0.85 0.1 0.2]);
        elseif handle.sel_all_maxgain_left.Value == 1
           set([handle.maxgain_left_edit1 handle.maxgain_left_edit2 handle.maxgain_left_edit3 handle.maxgain_left_edit4 handle.maxgain_left_edit5],...
                'visible','off');
            set([handle.cf1_left handle.cf2_left handle.cf3_left handle.cf4_left handle.cf5_left],...
                'visible','off');
            set([handle.maxgain_all_edit_left handle.bb_left],'visible','on');
            set(handle.sel_all_maxgain_left,'position',[0.3 0.85 0.1 0.2]);
            if isequal(maxgain(1:5),repmat(maxgain(1),1,5))
                set(handle.maxgain_all_edit_left,'string',[num2str(maxgain(1)) ' dB']);
            else
                set(handle.maxgain_all_edit_left,'string','');
            end
        end
end
