function handle = mpo_sel_all(src,event)
handle = guidata(src);
edit_tag = get(gcbo,'tag');
switch edit_tag
    case 'right'
        if handle.sel_all_mpo_right.Value == 0
            set([handle.mpo_right_slider1 handle.mpo_right_slider2 handle.mpo_right_slider3 handle.mpo_right_slider4 handle.mpo_right_slider5],...
                'enable','on','backgroundcolor',[1 .5 .5]);
            set([handle.mpo_right_edit1 handle.mpo_right_edit2 handle.mpo_right_edit3 handle.mpo_right_edit4 handle.mpo_right_edit5],...
                'enable','on');
            set(handle.mpo_all_edit_right,'enable','off');
            set(handle.mpo_all_slider_right ,'enable','off','backgroundcolor',[.5 .5 .5]);
        elseif handle.sel_all_mpo_right.Value == 1
            set([handle.mpo_right_slider1 handle.mpo_right_slider2 handle.mpo_right_slider3 handle.mpo_right_slider4 handle.mpo_right_slider5],...
                'enable','off','backgroundcolor',[.5 .5 .5]);
            set([handle.mpo_right_edit1 handle.mpo_right_edit2 handle.mpo_right_edit3 handle.mpo_right_edit4 handle.mpo_right_edit5],...
                'enable','off');
            set(handle.mpo_all_slider_right,'enable','on','backgroundcolor',[1 .5 .5]);
            set(handle.mpo_all_edit_right,'enable','on');
        end
    case 'left'
        if handle.sel_all_mpo_left.Value == 0
            set([handle.mpo_left_slider1 handle.mpo_left_slider2 handle.mpo_left_slider3 handle.mpo_left_slider4 handle.mpo_left_slider5],...
                'enable','on','backgroundcolor',[.5 .5 1]);
            set([handle.mpo_left_edit1 handle.mpo_left_edit2 handle.mpo_left_edit3 handle.mpo_left_edit4 handle.mpo_left_edit5],...
                'enable','on');
            set(handle.mpo_all_edit_left,'enable','off');
            set(handle.mpo_all_slider_left ,'enable','off','backgroundcolor',[.5 .5 .5]);
        elseif handle.sel_all_mpo_left.Value == 1
            set([handle.mpo_left_slider1 handle.mpo_left_slider2 handle.mpo_left_slider3 handle.mpo_left_slider4 handle.mpo_left_slider5],...
                'enable','off','backgroundcolor',[.5 .5 .5]);
            set([handle.mpo_left_edit1 handle.mpo_left_edit2 handle.mpo_left_edit3 handle.mpo_left_edit4 handle.mpo_left_edit5],...
                'enable','off');
            set(handle.mpo_all_slider_left,'enable','on','backgroundcolor',[.5 .5 1]);
            set(handle.mpo_all_edit_left,'enable','on');
        end
end
