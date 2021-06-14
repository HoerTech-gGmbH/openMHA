function handle = exp_slope(src,event)
handle = guidata(src);
mha = handle.mha;
edit_tag = get(gcbo,'tag');
if isequal(edit_tag,'right')
    slope_str = get(handle.slope_right_edit,'string');
    if str2double(slope_str) <= 0
        slope_str = num2str(0.1);
    elseif str2double(slope_str) > 10
        slope_str = num2str(10);
    end
    set(handle.slope_left_edit,'string',slope_str);
    set(handle.slope_right_edit,'string',slope_str);
elseif isequal(edit_tag,'left')
    slope_str = get(handle.slope_left_edit,'string');
    if str2double(slope_str) <= 0
        slope_str = num2str(0.1);
    elseif str2double(slope_str) > 10
        slope_str = num2str(10);
    end
    set(handle.slope_right_edit,'string',slope_str);
    set(handle.slope_left_edit,'string',slope_str);
end
newslope =  str2double(slope_str);
mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_slope',newslope);
handle = plot_data_right(handle);
handle = plot_data_left(handle);

guidata(src, handle);
end
