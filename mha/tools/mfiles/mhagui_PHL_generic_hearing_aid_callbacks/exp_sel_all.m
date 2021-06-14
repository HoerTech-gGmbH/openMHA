function handle = exp_sel_all(src,event)
handle = guidata(src);
edit_tag = get(gcbo,'tag');
switch edit_tag
    case 'all_right'
        if handle.sel_all_exp_right.Value == 0
            set([handle.exp_right_edit1 handle.exp_right_edit2 handle.exp_right_edit3 handle.exp_right_edit4 handle.exp_right_edit5],...
                'enable','on');
            set(handle.exp_all_edit_right,'enable','off');
        elseif handle.sel_all_exp_right.Value == 1
            set([handle.exp_right_edit1 handle.exp_right_edit2 handle.exp_right_edit3 handle.exp_right_edit4 handle.exp_right_edit5],...
                'enable','off');
            set(handle.exp_all_edit_right,'enable','on');
        end
    case 'all_left'
        if handle.sel_all_exp_left.Value == 0
            set([handle.exp_left_edit1 handle.exp_left_edit2 handle.exp_left_edit3 handle.exp_left_edit4 handle.exp_left_edit5],...
                'enable','on');
            set(handle.exp_all_edit_left,'enable','off');
        elseif handle.sel_all_exp_left.Value == 1
            set([handle.exp_left_edit1 handle.exp_left_edit2 handle.exp_left_edit3 handle.exp_left_edit4 handle.exp_left_edit5],...
                'enable','off');
            set(handle.exp_all_edit_left,'enable','on');
        end
end
