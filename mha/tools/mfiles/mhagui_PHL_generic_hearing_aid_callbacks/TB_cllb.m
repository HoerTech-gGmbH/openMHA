function handle = TB_cllb(src,event)
handle = guidata(src);
tb_tag = get(gcbo,'tag');
switch tb_tag
    case 'LA_left'
        if handle.TB1_left.Value == 1
            set(handle.TB2_left,'value',0);
        else
            set(handle.TB2_left,'value',1);
        end
%         handles = plot_data_right(handles,mha);
%         handles = plot_data_left(handles,mha);
    case 'V_left'
        if handle.TB2_left.Value == 1
            set(handle.TB1_left,'value',0);
        else
            set(handle.TB1_left,'value',1);
        end
    case 'LA_right'
        if handle.TB1_right.Value == 1
            set(handle.TB2_right,'value',0);
        else
            set(handle.TB2_right,'value',1);
        end

    case 'V_right'
        if handle.TB2_right.Value == 1
            set(handle.TB1_right,'value',0);
        else
            set(handle.TB1_right,'value',1);
        end
end

plot_data_left(handle);
plot_data_right(handle);

end
