function handle = exp_threshold(src,event)
handle = guidata(src);
mha = handle.mha;
exp = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold');
edit_tag = get(gcbo,'tag');
switch edit_tag
    case 'edit_l_250Hz'
        exp_str = get(handle.exp_left_edit1,'string');
        new_exp = strsplit(exp_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_left_edit1,'string',[new_exp{1} ' dB']);
                set(handle.exp_right_edit1,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_left_edit1,'string',[exp_str ' dB']);
                set(handle.exp_right_edit1,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [str2double(new_exp{1}) exp(2) exp(3) exp(4) exp(5) str2double(new_exp{1}) exp(7) exp(8) exp(9) exp(10)]);
        else
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_left_edit1,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_left_edit1,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [str2double(new_exp{1}) exp(2) exp(3) exp(4) exp(5) exp(6) exp(7) exp(8) exp(9) exp(10)]);
        end
    case 'edit_l_500Hz'
        exp_str = get(handle.exp_left_edit2,'string');
        new_exp = strsplit(exp_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_left_edit2,'string',[new_exp{1} ' dB']);
                set(handle.exp_right_edit2,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_left_edit2,'string',[exp_str ' dB']);
                set(handle.exp_right_edit2,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [exp(1) str2double(new_exp{1}) exp(3) exp(4) exp(5) exp(6) str2double(new_exp{1}) exp(8) exp(9) exp(10)]);
        else
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_left_edit2,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_left_edit2,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [exp(1) str2double(new_exp{1}) exp(3) exp(4) exp(5) exp(6) exp(7) exp(8) exp(9) exp(10)]);
        end
    case 'edit_l_1kHz'
        exp_str = get(handle.exp_left_edit3,'string');
        new_exp = strsplit(exp_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_left_edit3,'string',[new_exp{1} ' dB']);
                set(handle.exp_right_edit3,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_left_edit3,'string',[exp_str ' dB']);
                set(handle.exp_right_edit3,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [exp(1) exp(2) str2double(new_exp{1}) exp(4) exp(5) exp(6) exp(7) str2double(new_exp{1}) exp(9) exp(10)]);
        else
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_left_edit3,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_left_edit3,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [exp(1) exp(2) str2double(new_exp{1}) exp(4) exp(5) exp(6) exp(7) exp(8) exp(9) exp(10)]);
        end
    case 'edit_l_2kHz'
        exp_str = get(handle.exp_left_edit4,'string');
        new_exp = strsplit(exp_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_left_edit4,'string',[new_exp{1} ' dB']);
                set(handle.exp_right_edit4,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_left_edit4,'string',[exp_str ' dB']);
                set(handle.exp_right_edit4,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [exp(1) exp(2) exp(3) str2double(new_exp{1}) exp(5) exp(6) exp(7) exp(8) str2double(new_exp{1}) exp(10)]);
        else
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_left_edit4,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_left_edit4,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [exp(1) exp(2) exp(3) str2double(new_exp{1}) exp(5) exp(6) exp(7) exp(8) exp(9) exp(10)]);
        end
    case 'edit_l_4kHz'
        exp_str = get(handle.exp_left_edit5,'string');
        new_exp = strsplit(exp_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_left_edit5,'string',[new_exp{1} ' dB']);
                set(handle.exp_right_edit5,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_left_edit5,'string',[exp_str ' dB']);
                set(handle.exp_right_edit5,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [exp(1) exp(2) exp(3) exp(4) str2double(new_exp{1}) exp(6) exp(7) exp(8) exp(9) str2double(new_exp{1})]);
        else
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_left_edit5,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_left_edit5,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [exp(1) exp(2) exp(3) exp(4) str2double(new_exp{1}) exp(6) exp(7) exp(8) exp(9) exp(10)]);
        end
    case 'edit_r_250Hz'
        exp_str = get(handle.exp_right_edit1,'string');
        new_exp = strsplit(exp_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_right_edit1,'string',[new_exp{1} ' dB']);
                set(handle.exp_left_edit1,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_left_edit1,'string',[exp_str ' dB']);
                set(handle.exp_right_edit1,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [str2double(new_exp{1}) exp(2) exp(3) exp(4) exp(5) str2double(new_exp{1}) exp(7) exp(8) exp(9) exp(10)]);
        else
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_right_edit1,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_right_edit1,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [exp(1) exp(2) exp(3) exp(4) exp(5) str2double(new_exp{1}) exp(7) exp(8) exp(9) exp(10)]);
        end
    case 'edit_r_500Hz'
        exp_str = get(handle.exp_right_edit2,'string');
        new_exp = strsplit(exp_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_right_edit2,'string',[new_exp{1} ' dB']);
                set(handle.exp_left_edit2,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_right_edit2,'string',[exp_str ' dB']);
                set(handle.exp_left_edit2,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [exp(1) str2double(new_exp{1}) exp(3) exp(4) exp(5) exp(6) str2double(new_exp{1}) exp(8) exp(9) exp(10)]);
        else
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_right_edit2,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_right_edit2,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [exp(1) exp(2) exp(3) exp(4) exp(5) exp(6) str2double(new_exp{1}) exp(8) exp(9) exp(10)]);
        end
    case 'edit_r_1kHz'
        exp_str = get(handle.exp_right_edit3,'string');
        new_exp = strsplit(exp_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_right_edit3,'string',[new_exp{1} ' dB']);
                set(handle.exp_left_edit3,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_right_edit3,'string',[exp_str ' dB']);
                set(handle.exp_left_edit3,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [exp(1) exp(2) str2double(new_exp{1}) exp(4) exp(5) exp(6) exp(7) str2double(new_exp{1}) exp(9) exp(10)]);
        else
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_right_edit3,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_right_edit3,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [exp(1) exp(2) exp(3) exp(4) exp(5) exp(6) exp(7) str2double(new_exp{1}) exp(9) exp(10)]);
        end
    case 'edit_r_2kHz'
        exp_str = get(handle.exp_right_edit4,'string');
        new_exp = strsplit(exp_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_right_edit4,'string',[new_exp{1} ' dB']);
                set(handle.exp_left_edit4,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_right_edit4,'string',[exp_str ' dB']);
                set(handle.exp_left_edit4,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [exp(1) exp(2) exp(3) str2double(new_exp{1}) exp(5) exp(6) exp(7) exp(8) str2double(new_exp{1}) exp(10)]);
        else
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_right_edit4,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_right_edit4,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [exp(1) exp(2) exp(3) exp(4) exp(5) exp(6) exp(7) exp(8) str2double(new_exp{1}) exp(10)]);
        end
    case 'edit_r_4kHz'
        exp_str = get(handle.exp_right_edit5,'string');
        new_exp = strsplit(exp_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_right_edit5,'string',[new_exp{1} ' dB']);
                set(handle.exp_left_edit5,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_right_edit5,'string',[exp_str ' dB']);
                set(handle.exp_left_edit5,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [exp(1) exp(2) exp(3) exp(4) str2double(new_exp{1}) exp(6) exp(7) exp(8) exp(9) str2double(new_exp{1})]);
        else
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_right_edit5,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_right_edit5,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [exp(1) exp(2) exp(3) exp(4) exp(5) exp(6) exp(7) exp(8) exp(9) str2double(new_exp{1})]);
        end
    case 'all_right'
        exp_str = get(handle.exp_all_edit_right,'string');
        new_exp = strsplit(exp_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_all_edit_right,'string',[new_exp{1} ' dB']);
                set(handle.exp_right_edit1,'string',[new_exp{1} ' dB']);
                set(handle.exp_right_edit2,'string',[new_exp{1} ' dB']);
                set(handle.exp_right_edit3,'string',[new_exp{1} ' dB']);
                set(handle.exp_right_edit4,'string',[new_exp{1} ' dB']);
                set(handle.exp_right_edit5,'string',[new_exp{1} ' dB']);
                set(handle.exp_all_edit_left,'string',[new_exp{1} ' dB']);
                set(handle.exp_left_edit1,'string',[new_exp{1} ' dB']);
                set(handle.exp_left_edit2,'string',[new_exp{1} ' dB']);
                set(handle.exp_left_edit3,'string',[new_exp{1} ' dB']);
                set(handle.exp_left_edit4,'string',[new_exp{1} ' dB']);
                set(handle.exp_left_edit5,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_all_edit_right,'string',[exp_str ' dB']);
                set(handle.exp_right_edit1,'string',[exp_str ' dB']);
                set(handle.exp_right_edit2,'string',[exp_str ' dB']);
                set(handle.exp_right_edit3,'string',[exp_str ' dB']);
                set(handle.exp_right_edit4,'string',[exp_str ' dB']);
                set(handle.exp_right_edit5,'string',[exp_str ' dB']);
                set(handle.exp_all_edit_left,'string',[exp_str ' dB']);
                set(handle.exp_left_edit1,'string',[exp_str ' dB']);
                set(handle.exp_left_edit2,'string',[exp_str ' dB']);
                set(handle.exp_left_edit3,'string',[exp_str ' dB']);
                set(handle.exp_left_edit4,'string',[exp_str ' dB']);
                set(handle.exp_left_edit5,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                repmat(str2double(new_exp{1}),1,10));
        else
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_all_edit_right,'string',[new_exp{1} ' dB']);
                set(handle.exp_right_edit1,'string',[new_exp{1} ' dB']);
                set(handle.exp_right_edit2,'string',[new_exp{1} ' dB']);
                set(handle.exp_right_edit3,'string',[new_exp{1} ' dB']);
                set(handle.exp_right_edit4,'string',[new_exp{1} ' dB']);
                set(handle.exp_right_edit5,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_all_edit_right,'string',[exp_str ' dB']);
                set(handle.exp_right_edit1,'string',[exp_str ' dB']);
                set(handle.exp_right_edit2,'string',[exp_str ' dB']);
                set(handle.exp_right_edit3,'string',[exp_str ' dB']);
                set(handle.exp_right_edit4,'string',[exp_str ' dB']);
                set(handle.exp_right_edit5,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [exp(1:5) repmat(str2double(new_exp{1}),1,5)]);
        end
    case 'all_left'
        exp_str = get(handle.exp_all_edit_left,'string');
        new_exp = strsplit(exp_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_all_edit_right,'string',[new_exp{1} ' dB']);
                set(handle.exp_right_edit1,'string',[new_exp{1} ' dB']);
                set(handle.exp_right_edit2,'string',[new_exp{1} ' dB']);
                set(handle.exp_right_edit3,'string',[new_exp{1} ' dB']);
                set(handle.exp_right_edit4,'string',[new_exp{1} ' dB']);
                set(handle.exp_right_edit5,'string',[new_exp{1} ' dB']);
                set(handle.exp_all_edit_left,'string',[new_exp{1} ' dB']);
                set(handle.exp_left_edit1,'string',[new_exp{1} ' dB']);
                set(handle.exp_left_edit2,'string',[new_exp{1} ' dB']);
                set(handle.exp_left_edit3,'string',[new_exp{1} ' dB']);
                set(handle.exp_left_edit4,'string',[new_exp{1} ' dB']);
                set(handle.exp_left_edit5,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_all_edit_right,'string',[exp_str ' dB']);
                set(handle.exp_right_edit1,'string',[exp_str ' dB']);
                set(handle.exp_right_edit2,'string',[exp_str ' dB']);
                set(handle.exp_right_edit3,'string',[exp_str ' dB']);
                set(handle.exp_right_edit4,'string',[exp_str ' dB']);
                set(handle.exp_right_edit5,'string',[exp_str ' dB']);
                set(handle.exp_all_edit_left,'string',[exp_str ' dB']);
                set(handle.exp_left_edit1,'string',[exp_str ' dB']);
                set(handle.exp_left_edit2,'string',[exp_str ' dB']);
                set(handle.exp_left_edit3,'string',[exp_str ' dB']);
                set(handle.exp_left_edit4,'string',[exp_str ' dB']);
                set(handle.exp_left_edit5,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                repmat(str2double(new_exp{1}),1,10));
        else
            if str2double(new_exp(1)) < 0
                new_exp{1} = '0';
                set(handle.exp_all_edit_left,'string',[new_exp{1} ' dB']);
                set(handle.exp_left_edit1,'string',[new_exp{1} ' dB']);
                set(handle.exp_left_edit2,'string',[new_exp{1} ' dB']);
                set(handle.exp_left_edit3,'string',[new_exp{1} ' dB']);
                set(handle.exp_left_edit4,'string',[new_exp{1} ' dB']);
                set(handle.exp_left_edit5,'string',[new_exp{1} ' dB']);
            else
                set(handle.exp_all_edit_left,'string',[exp_str ' dB']);
                set(handle.exp_left_edit1,'string',[exp_str ' dB']);
                set(handle.exp_left_edit2,'string',[exp_str ' dB']);
                set(handle.exp_left_edit3,'string',[exp_str ' dB']);
                set(handle.exp_left_edit4,'string',[exp_str ' dB']);
                set(handle.exp_left_edit5,'string',[exp_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',...
                [repmat(str2double(new_exp{1}),1,5) exp(6:10)]);
        end
end
handle = plot_data_right(handle);
handle = plot_data_left(handle);
end
