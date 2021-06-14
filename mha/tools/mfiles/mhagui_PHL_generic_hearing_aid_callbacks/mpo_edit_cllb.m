function handle = mpo_edit_cllb(src,event)
handle = guidata(src);
mha = handle.mha;
mpo = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold');
edit_tag = get(gcbo,'tag');
switch edit_tag
    case 'edit_l_250Hz'
        mpo_str = get(handle.mpo_left_edit1,'string');
        new_mpo = strsplit(mpo_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_mpo(1)) < get(handle.mpo_left_slider1,'min')
                new_mpo{1} = num2str(get(handle.mpo_left_slider1,'min'));
                set(handle.mpo_left_edit1,'string',[num2str(get(handle.mpo_left_slider1,'min')) ' dB']);
                set(handle.mpo_right_edit1,'string',[num2str(get(handle.mpo_left_slider1,'min')) ' dB']);
            elseif str2double(new_mpo(1)) > get(handle.mpo_left_slider1,'max')
                new_mpo{1} = num2str(get(handle.mpo_left_slider1,'max'));
                set(handle.mpo_left_edit1,'string',[num2str(get(handle.mpo_left_slider1,'max')) ' dB']);
                set(handle.mpo_right_edit1,'string',[num2str(get(handle.mpo_left_slider1,'max')) ' dB']);
            else
                set(handle.mpo_left_edit1,'string',[mpo_str ' dB']);
                set(handle.mpo_right_edit1,'string',[mpo_str ' dB']);
            end

            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [str2double(new_mpo(1)) mpo(2:5) str2double(new_mpo(1)) mpo(7:10)]);
            set(handle.mpo_left_slider1,'value',str2double(new_mpo(1)));
            set(handle.mpo_right_slider1,'value',str2double(new_mpo(1)));
        else
            if str2double(new_mpo(1)) < get(handle.mpo_left_slider1,'min')
                new_mpo{1} = num2str(get(handle.mpo_left_slider1,'min'));
                set(handle.mpo_left_edit1,'string',[num2str(get(handle.mpo_left_slider1,'min')) ' dB']);

            elseif str2double(new_mpo(1)) > get(handle.mpo_left_slider1,'max')
                new_mpo{1} = num2str(get(handle.mpo_left_slider1,'max'));
                set(handle.mpo_left_edit1,'string',[num2str(get(handle.mpo_left_slider1,'max')) ' dB']);
            else
                set(handle.mpo_left_edit1,'string',[mpo_str ' dB']);
            end

            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [str2double(new_mpo(1)) mpo(2) mpo(3) mpo(4) mpo(5) mpo(6) mpo(7) mpo(8) mpo(9) mpo(10)]);
            set(handle.mpo_left_slider1,'value',str2double(new_mpo(1)));
        end
    case 'edit_l_500Hz'
        mpo_str = get(handle.mpo_left_edit2,'string');
        new_mpo = strsplit(mpo_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_mpo(1)) < get(handle.mpo_left_slider2,'min')
                new_mpo{1} = num2str(get(handle.mpo_left_slider2,'min'));
                set(handle.mpo_left_edit2,'string',[num2str(get(handle.mpo_left_slider2,'min')) ' dB']);
                set(handle.mpo_right_edit2,'string',[num2str(get(handle.mpo_left_slider2,'min')) ' dB']);
            elseif str2double(new_mpo(1)) > get(handle.mpo_left_slider2,'max')
                new_mpo{1} = num2str(get(handle.mpo_left_slider2,'max'));
                set(handle.mpo_left_edit2,'string',[num2str(get(handle.mpo_left_slider2,'max')) ' dB']);
                set(handle.mpo_right_edit2,'string',[num2str(get(handle.mpo_left_slider2,'max')) ' dB']);
            else
                set(handle.mpo_left_edit2,'string',[mpo_str ' dB']);
                set(handle.mpo_right_edit2,'string',[mpo_str ' dB']);
            end

            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) str2double(new_mpo(1)) mpo(3:6) str2double(new_mpo(1)) mpo(8:10)]);
            set(handle.mpo_left_slider2,'value',str2double(new_mpo(1)));
            set(handle.mpo_right_slider2,'value',str2double(new_mpo(1)));
        else
            if str2double(new_mpo(1)) < get(handle.mpo_left_slider2,'min')
                new_mpo{1} = num2str(get(handle.mpo_left_slider2,'min'));
                set(handle.mpo_left_edit2,'string',[num2str(get(handle.mpo_left_slider2,'min')) ' dB']);

            elseif str2double(new_mpo(1)) > get(handle.mpo_left_slider2,'max')
                new_mpo{1} = num2str(get(handle.mpo_left_slider2,'max'));
                set(handle.mpo_left_edit2,'string',[num2str(get(handle.mpo_left_slider2,'max')) ' dB']);
            else
                set(handle.mpo_left_edit2,'string',[mpo_str ' dB']);
            end

            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) str2double(new_mpo(1)) mpo(3) mpo(4) mpo(5) mpo(6) mpo(7) mpo(8) mpo(9) mpo(10)]);
            set(handle.mpo_left_slider2,'value',str2double(new_mpo(1)));
        end
    case 'edit_l_1kHz'
        mpo_str = get(handle.mpo_left_edit3,'string');
        new_mpo = strsplit(mpo_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_mpo(1)) < get(handle.mpo_left_slider3,'min')
                new_mpo{1} = num2str(get(handle.mpo_left_slider3,'min'));
                set(handle.mpo_left_edit3,'string',[num2str(get(handle.mpo_left_slider3,'min')) ' dB']);
                set(handle.mpo_right_edit3,'string',[num2str(get(handle.mpo_left_slider3,'min')) ' dB']);
            elseif str2double(new_mpo(1)) > get(handle.mpo_left_slider3,'max')
                new_mpo{1} = num2str(get(handle.mpo_left_slider3,'max'));
                set(handle.mpo_left_edit3,'string',[num2str(get(handle.mpo_left_slider3,'max')) ' dB']);
                set(handle.mpo_right_edit3,'string',[num2str(get(handle.mpo_left_slider3,'max')) ' dB']);
            else
                set(handle.mpo_left_edit3,'string',[mpo_str ' dB']);
                set(handle.mpo_right_edit3,'string',[mpo_str ' dB']);
            end

            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1:2) str2double(new_mpo(1)) mpo(4:7) str2double(new_mpo(1)) mpo(9:10)]);
            set(handle.mpo_left_slider3,'value',str2double(new_mpo(1)));
            set(handle.mpo_right_slider3,'value',str2double(new_mpo(1)));
        else
            if str2double(new_mpo(1)) < get(handle.mpo_left_slider3,'min')
                new_mpo{1} = num2str(get(handle.mpo_left_slider3,'min'));
                set(handle.mpo_left_edit3,'string',[num2str(get(handle.mpo_left_slider3,'min')) ' dB']);
            elseif str2double(new_mpo(1)) > get(handle.mpo_left_slider3,'max')
                new_mpo{1} = num2str(get(handle.mpo_left_slider3,'max'));
                set(handle.mpo_left_edit3,'string',[num2str(get(handle.mpo_left_slider3,'max')) ' dB']);
            else
                set(handle.mpo_left_edit3,'string',[mpo_str ' dB']);
            end

            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) str2double(new_mpo(1)) mpo(4) mpo(5) mpo(6) mpo(7) mpo(8) mpo(9) mpo(10)]);
            set(handle.mpo_left_slider3,'value',str2double(new_mpo(1)));
        end
        case 'edit_l_2kHz'
        mpo_str = get(handle.mpo_left_edit4,'string');
        new_mpo = strsplit(mpo_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_mpo(1)) < get(handle.mpo_left_slider4,'min')
                new_mpo{1} = num2str(get(handle.mpo_left_slider4,'min'));
                set(handle.mpo_left_edit4,'string',[num2str(get(handle.mpo_left_slider4,'min')) ' dB']);
                set(handle.mpo_right_edit4,'string',[num2str(get(handle.mpo_left_slider4,'min')) ' dB']);
            elseif str2double(new_mpo(1)) > get(handle.mpo_left_slider4,'max')
                new_mpo{1} = num2str(get(handle.mpo_left_slider4,'max'));
                set(handle.mpo_left_edit4,'string',[num2str(get(handle.mpo_left_slider4,'max')) ' dB']);
                set(handle.mpo_right_edit4,'string',[num2str(get(handle.mpo_left_slider4,'max')) ' dB']);
            else
                set(handle.mpo_left_edit4,'string',[mpo_str ' dB']);
                set(handle.mpo_right_edit4,'string',[mpo_str ' dB']);
            end

            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1:3) str2double(new_mpo(1)) mpo(5:8) str2double(new_mpo(1)) mpo(10)]);
            set(handle.mpo_left_slider4,'value',str2double(new_mpo(1)));
            set(handle.mpo_right_slider4,'value',str2double(new_mpo(1)));
        else
            if str2double(new_mpo(1)) < get(handle.mpo_left_slider4,'min')
                new_mpo{1} = num2str(get(handle.mpo_left_slider4,'min'));
                set(handle.mpo_left_edit4,'string',[num2str(get(handle.mpo_left_slider4,'min')) ' dB']);

            elseif str2double(new_mpo(1)) > get(handle.mpo_left_slider4,'max')
                new_mpo{1} = num2str(get(handle.mpo_left_slider4,'max'));
                set(handle.mpo_left_edit4,'string',[num2str(get(handle.mpo_left_slider4,'max')) ' dB']);
            else
                set(handle.mpo_left_edit4,'string',[mpo_str ' dB']);
            end

            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) mpo(3) str2double(new_mpo(1)) mpo(5) mpo(6) mpo(7) mpo(8) mpo(9) mpo(10)]);
            set(handle.mpo_left_slider4,'value',str2double(new_mpo(1)));
        end
    case 'edit_l_4kHz'
        mpo_str = get(handle.mpo_left_edit5,'string');
        new_mpo = strsplit(mpo_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_mpo(1)) < get(handle.mpo_left_slider5,'min')
                new_mpo{1} = num2str(get(handle.mpo_left_slider5,'min'));
                set(handle.mpo_left_edit5,'string',[num2str(get(handle.mpo_left_slider5,'min')) ' dB']);
                set(handle.mpo_right_edit5,'string',[num2str(get(handle.mpo_left_slider5,'min')) ' dB']);
            elseif str2double(new_mpo(1)) > get(handle.mpo_left_slider5,'max')
                new_mpo{1} = num2str(get(handle.mpo_left_slider5,'max'));
                set(handle.mpo_left_edit5,'string',[num2str(get(handle.mpo_left_slider5,'max')) ' dB']);
                set(handle.mpo_right_edit5,'string',[num2str(get(handle.mpo_left_slider5,'max')) ' dB']);
            else 
                set(handle.mpo_left_edit5,'string',[mpo_str ' dB']);
                set(handle.mpo_right_edit5,'string',[mpo_str ' dB']);
            end

            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1:4) str2double(new_mpo(1)) mpo(6:9) str2double(new_mpo(1))]);
            set(handle.mpo_left_slider5,'value',str2double(new_mpo(1)));
            set(handle.mpo_right_slider5,'value',str2double(new_mpo(1)));
        else
            if str2double(new_mpo(1)) < get(handle.mpo_left_slider5,'min')
                new_mpo{1} = num2str(get(handle.mpo_left_slider5,'min'));
                set(handle.mpo_left_edit5,'string',[num2str(get(handle.mpo_left_slider5,'min')) ' dB']);

            elseif str2double(new_mpo(1)) > get(handle.mpo_left_slider5,'max')
                new_mpo{1} = num2str(get(handle.mpo_left_slider5,'max'));
                set(handle.mpo_left_edit5,'string',[num2str(get(handle.mpo_left_slider5,'max')) ' dB']);
            else 
                set(handle.mpo_left_edit5,'string',[mpo_str ' dB']);
            end

            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) mpo(3) mpo(4) str2double(new_mpo(1)) mpo(6) mpo(7) mpo(8) mpo(9) mpo(10)]);
            set(handle.mpo_left_slider5,'value',str2double(new_mpo(1)));
        end
    case 'edit_r_250Hz'
        mpo_str = get(handle.mpo_right_edit1,'string');
        new_mpo = strsplit(mpo_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_mpo(1)) < get(handle.mpo_right_slider1,'min')
                new_mpo{1} = num2str(get(handle.mpo_right_slider1,'min'));
                set(handle.mpo_right_edit1,'string',[num2str(get(handle.mpo_right_slider1,'min')) ' dB']);
                set(handle.mpo_left_edit1,'string',[num2str(get(handle.mpo_right_slider1,'min')) ' dB']);
            elseif str2double(new_mpo(1)) > get(handle.mpo_right_slider1,'max')
                new_mpo{1} = num2str(get(handle.mpo_right_slider1,'max'));
                set(handle.mpo_right_edit1,'string',[num2str(get(handle.mpo_right_slider1,'max')) ' dB']);
                set(handle.mpo_left_edit1,'string',[num2str(get(handle.mpo_right_slider1,'max')) ' dB']);
            else
                set(handle.mpo_right_edit1,'string',[mpo_str ' dB']);
                set(handle.mpo_left_edit1,'string',[mpo_str ' dB']);
            end

            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [str2double(new_mpo(1)) mpo(2:5) str2double(new_mpo(1)) mpo(7:10)]);
            set(handle.mpo_right_slider1,'value',str2double(new_mpo(1)));
            set(handle.mpo_left_slider1,'value',str2double(new_mpo(1)));
        else
            if str2double(new_mpo(1)) < get(handle.mpo_right_slider1,'min')
                new_mpo{1} = num2str(get(handle.mpo_right_slider1,'min'));
                set(handle.mpo_right_edit1,'string',[num2str(get(handle.mpo_right_slider1,'min')) ' dB']);

            elseif str2double(new_mpo(1)) > get(handle.mpo_right_slider1,'max')
                new_mpo{1} = num2str(get(handle.mpo_right_slider1,'max'));
                set(handle.mpo_right_edit1,'string',[num2str(get(handle.mpo_right_slider1,'max')) ' dB']);
            else
                set(handle.mpo_right_edit1,'string',[mpo_str ' dB']);
            end

            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) mpo(3) mpo(4) mpo(5) str2double(new_mpo(1)) mpo(7) mpo(8) mpo(9) mpo(10)]);
            set(handle.mpo_right_slider1,'value',str2double(new_mpo(1)));
        end
    case 'edit_r_500Hz'
        mpo_str = get(handle.mpo_right_edit2,'string');
        new_mpo = strsplit(mpo_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_mpo(1)) < get(handle.mpo_right_slider2,'min')
                new_mpo{1} = num2str(get(handle.mpo_right_slider2,'min'));
                set(handle.mpo_right_edit2,'string',[num2str(get(handle.mpo_right_slider2,'min')) ' dB']);
                set(handle.mpo_left_edit2,'string',[num2str(get(handle.mpo_right_slider2,'min')) ' dB']);
            elseif str2double(new_mpo(1)) > get(handle.mpo_right_slider2,'max')
                new_mpo{1} = num2str(get(handle.mpo_right_slider2,'max'));
                set(handle.mpo_right_edit2,'string',[num2str(get(handle.mpo_right_slider2,'max')) ' dB']);
                set(handle.mpo_left_edit2,'string',[num2str(get(handle.mpo_right_slider2,'max')) ' dB']);
            else
                set(handle.mpo_right_edit2,'string',[mpo_str ' dB']);
                set(handle.mpo_left_edit2,'string',[mpo_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) str2double(new_mpo(1)) mpo(3:6) str2double(new_mpo(1)) mpo(8:10)]);
            set(handle.mpo_right_slider2,'value',str2double(new_mpo(1)));
            set(handle.mpo_left_slider2,'value',str2double(new_mpo(1)));
        else
            if str2double(new_mpo(1)) < get(handle.mpo_right_slider2,'min')
                new_mpo{1} = num2str(get(handle.mpo_right_slider2,'min'));
                set(handle.mpo_right_edit2,'string',[num2str(get(handle.mpo_right_slider2,'min')) ' dB']);

            elseif str2double(new_mpo(1)) > get(handle.mpo_right_slider2,'max')
                new_mpo{1} = num2str(get(handle.mpo_right_slider2,'max'));
                set(handle.mpo_right_edit2,'string',[num2str(get(handle.mpo_right_slider2,'max')) ' dB']);
            else
                set(handle.mpo_right_edit2,'string',[mpo_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) mpo(3) mpo(4) mpo(5) mpo(6) str2double(new_mpo(1)) mpo(8) mpo(9) mpo(10)]);
            set(handle.mpo_right_slider2,'value',str2double(new_mpo(1)));
        end
        case 'edit_r_1kHz'
        mpo_str = get(handle.mpo_right_edit3,'string');
        new_mpo = strsplit(mpo_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_mpo(1)) < get(handle.mpo_right_slider3,'min')
                new_mpo{1} = num2str(get(handle.mpo_right_slider3,'min'));
                set(handle.mpo_right_edit3,'string',[num2str(get(handle.mpo_right_slider3,'min')) ' dB']);
                set(handle.mpo_left_edit3,'string',[num2str(get(handle.mpo_right_slider3,'min')) ' dB']);
            elseif str2double(new_mpo(1)) > get(handle.mpo_right_slider3,'max')
                new_mpo{1} = num2str(get(handle.mpo_right_slider3,'max'));
                set(handle.mpo_right_edit3,'string',[num2str(get(handle.mpo_right_slider3,'max')) ' dB']);
                set(handle.mpo_left_edit3,'string',[num2str(get(handle.mpo_right_slider3,'max')) ' dB']);
            else
                set(handle.mpo_right_edit3,'string',[mpo_str ' dB']);
                set(handle.mpo_left_edit3,'string',[mpo_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1:2) str2double(new_mpo(1)) mpo(4:7) str2double(new_mpo(1)) mpo(9:10)]);
            set(handle.mpo_right_slider3,'value',str2double(new_mpo(1)));
            set(handle.mpo_left_slider3,'value',str2double(new_mpo(1)));
        else
            if str2double(new_mpo(1)) < get(handle.mpo_right_slider3,'min')
                new_mpo{1} = num2str(get(handle.mpo_right_slider3,'min'));
                set(handle.mpo_right_edit3,'string',[num2str(get(handle.mpo_right_slider3,'min')) ' dB']);

            elseif str2double(new_mpo(1)) > get(handle.mpo_right_slider3,'max')
                new_mpo{1} = num2str(get(handle.mpo_right_slider3,'max'));
                set(handle.mpo_right_edit3,'string',[num2str(get(handle.mpo_right_slider3,'max')) ' dB']);
            else
                set(handle.mpo_right_edit3,'string',[mpo_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) mpo(3) mpo(4) mpo(5) mpo(6) mpo(7) str2double(new_mpo(1)) mpo(9) mpo(10)]);
            set(handle.mpo_right_slider3,'value',str2double(new_mpo(1)));
        end
    case 'edit_r_2kHz'
        mpo_str = get(handle.mpo_right_edit4,'string');
        new_mpo = strsplit(mpo_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_mpo(1)) < get(handle.mpo_right_slider4,'min')
                new_mpo{1} = num2str(get(handle.mpo_right_slider4,'min'));
                set(handle.mpo_right_edit4,'string',[num2str(get(handle.mpo_right_slider4,'min')) ' dB']);
                set(handle.mpo_left_edit4,'string',[num2str(get(handle.mpo_right_slider4,'min')) ' dB']);
            elseif str2double(new_mpo(1)) > get(handle.mpo_right_slider4,'max')
                new_mpo{1} = num2str(get(handle.mpo_right_slider4,'max'));
                set(handle.mpo_right_edit4,'string',[num2str(get(handle.mpo_right_slider4,'max')) ' dB']);
                set(handle.mpo_left_edit4,'string',[num2str(get(handle.mpo_right_slider4,'max')) ' dB']);
            else
                set(handle.mpo_right_edit4,'string',[mpo_str ' dB']);
                set(handle.mpo_left_edit4,'string',[mpo_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1:3) str2double(new_mpo(1)) mpo(5:8) str2double(new_mpo(1)) mpo(10)]);
            set(handle.mpo_right_slider4,'value',str2double(new_mpo(1)));
            set(handle.mpo_left_slider4,'value',str2double(new_mpo(1)));
        else
            if str2double(new_mpo(1)) < get(handle.mpo_right_slider4,'min')
                new_mpo{1} = num2str(get(handle.mpo_right_slider4,'min'));
                set(handle.mpo_right_edit4,'string',[num2str(get(handle.mpo_right_slider4,'min')) ' dB']);

            elseif str2double(new_mpo(1)) > get(handle.mpo_right_slider4,'max')
                new_mpo{1} = num2str(get(handle.mpo_right_slider4,'max'));
                set(handle.mpo_right_edit4,'string',[num2str(get(handle.mpo_right_slider4,'max')) ' dB']);
            else
                set(handle.mpo_right_edit4,'string',[mpo_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) mpo(3) mpo(4) mpo(5) mpo(6) mpo(7) mpo(8) str2double(new_mpo(1)) mpo(10)]);
            set(handle.mpo_right_slider4,'value',str2double(new_mpo(1)));
        end
    case 'edit_r_4kHz'
        mpo_str = get(handle.mpo_right_edit5,'string');
        new_mpo = strsplit(mpo_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_mpo(1)) < get(handle.mpo_right_slider5,'min')
                new_mpo{1} = num2str(get(handle.mpo_right_slider5,'min'));
                set(handle.mpo_right_edit5,'string',[num2str(get(handle.mpo_right_slider5,'min')) ' dB']);
                set(handle.mpo_left_edit5,'string',[num2str(get(handle.mpo_right_slider5,'min')) ' dB']);
            elseif str2double(new_mpo(1)) > get(handle.mpo_right_slider5,'max')
                new_mpo{1} = num2str(get(handle.mpo_right_slider5,'max'));
                set(handle.mpo_right_edit5,'string',[num2str(get(handle.mpo_right_slider5,'max')) ' dB']);
                set(handle.mpo_left_edit5,'string',[num2str(get(handle.mpo_right_slider5,'max')) ' dB']);
            else
                set(handle.mpo_right_edit5,'string',[mpo_str ' dB']);
                set(handle.mpo_left_edit5,'string',[mpo_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1:4) str2double(new_mpo(1)) mpo(6:9) str2double(new_mpo(1))]);
            set(handle.mpo_right_slider5,'value',str2double(new_mpo(1)));
            set(handle.mpo_left_slider5,'value',str2double(new_mpo(1)));
        else
            if str2double(new_mpo(1)) < get(handle.mpo_right_slider5,'min')
                new_mpo{1} = num2str(get(handle.mpo_right_slider5,'min'));
                set(handle.mpo_right_edit5,'string',[num2str(get(handle.mpo_right_slider5,'min')) ' dB']);
            elseif str2double(new_mpo(1)) > get(handle.mpo_right_slider5,'max')
                new_mpo{1} = num2str(get(handle.mpo_right_slider5,'max'));
                set(handle.mpo_right_edit5,'string',[num2str(get(handle.mpo_right_slider5,'max')) ' dB']);
            else
                set(handle.mpo_right_edit5,'string',[mpo_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) mpo(3) mpo(4) mpo(5) mpo(6) mpo(7) mpo(8) mpo(9) str2double(new_mpo(1))]);
            set(handle.mpo_right_slider5,'value',str2double(new_mpo(1)));
        end
    case 'all_right'
        mpo_str = get(handle.mpo_all_edit_right,'string');
        new_mpo = strsplit(mpo_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_mpo(1)) < get(handle.mpo_all_slider_right,'min')
                new_mpo{1} = num2str(get(handle.mpo_all_slider_right,'min'));
                set(handle.mpo_all_edit_right,'string',[num2str(get(handle.mpo_all_slider_right,'min')) ' dB']);
                set(handle.mpo_right_edit1,'string',[num2str(get(handle.mpo_all_slider_right,'min')) ' dB']);
                set(handle.mpo_right_edit2,'string',[num2str(get(handle.mpo_all_slider_right,'min')) ' dB']);
                set(handle.mpo_right_edit3,'string',[num2str(get(handle.mpo_all_slider_right,'min')) ' dB']);
                set(handle.mpo_right_edit4,'string',[num2str(get(handle.mpo_all_slider_right,'min')) ' dB']);
                set(handle.mpo_right_edit5,'string',[num2str(get(handle.mpo_all_slider_right,'min')) ' dB']);
                
                set(handle.mpo_all_edit_left,'string',[num2str(get(handle.mpo_all_slider_left,'min')) ' dB']);
                set(handle.mpo_left_edit1,'string',[num2str(get(handle.mpo_all_slider_left,'min')) ' dB']);
                set(handle.mpo_left_edit2,'string',[num2str(get(handle.mpo_all_slider_left,'min')) ' dB']);
                set(handle.mpo_left_edit3,'string',[num2str(get(handle.mpo_all_slider_left,'min')) ' dB']);
                set(handle.mpo_left_edit4,'string',[num2str(get(handle.mpo_all_slider_left,'min')) ' dB']);
                set(handle.mpo_left_edit5,'string',[num2str(get(handle.mpo_all_slider_left,'min')) ' dB']);
            elseif str2double(new_mpo(1)) > get(handle.mpo_all_slider_left,'max')
                new_mpo{1} = num2str(get(handle.mpo_all_slider_right,'max'));
                set(handle.mpo_all_edit_right,'string',[num2str(get(handle.mpo_all_slider_right,'max')) ' dB']);
                set(handle.mpo_right_edit1,'string',[num2str(get(handle.mpo_all_slider_right,'max')) ' dB']);
                set(handle.mpo_right_edit2,'string',[num2str(get(handle.mpo_all_slider_right,'max')) ' dB']);
                set(handle.mpo_right_edit3,'string',[num2str(get(handle.mpo_all_slider_right,'max')) ' dB']);
                set(handle.mpo_right_edit4,'string',[num2str(get(handle.mpo_all_slider_right,'max')) ' dB']);
                set(handle.mpo_right_edit5,'string',[num2str(get(handle.mpo_all_slider_right,'max')) ' dB']);
                
                set(handle.mpo_all_edit_left,'string',[num2str(get(handle.mpo_all_slider_left,'max')) ' dB']);
                set(handle.mpo_left_edit1,'string',[num2str(get(handle.mpo_all_slider_left,'max')) ' dB']);
                set(handle.mpo_left_edit2,'string',[num2str(get(handle.mpo_all_slider_left,'max')) ' dB']);
                set(handle.mpo_left_edit3,'string',[num2str(get(handle.mpo_all_slider_left,'max')) ' dB']);
                set(handle.mpo_left_edit4,'string',[num2str(get(handle.mpo_all_slider_left,'max')) ' dB']);
                set(handle.mpo_left_edit5,'string',[num2str(get(handle.mpo_all_slider_left,'max')) ' dB']);
            else
                set(handle.mpo_all_edit_right,'string',[mpo_str ' dB']);
                set(handle.mpo_right_edit1,'string',[mpo_str ' dB']);
                set(handle.mpo_right_edit2,'string',[mpo_str ' dB']);
                set(handle.mpo_right_edit3,'string',[mpo_str ' dB']);
                set(handle.mpo_right_edit4,'string',[mpo_str ' dB']);
                set(handle.mpo_right_edit5,'string',[mpo_str ' dB']);
                
                set(handle.mpo_all_edit_left,'string',[mpo_str ' dB']);
                set(handle.mpo_left_edit1,'string',[mpo_str ' dB']);
                set(handle.mpo_left_edit2,'string',[mpo_str ' dB']);
                set(handle.mpo_left_edit3,'string',[mpo_str ' dB']);
                set(handle.mpo_left_edit4,'string',[mpo_str ' dB']);
                set(handle.mpo_left_edit5,'string',[mpo_str ' dB']);
            end

            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
            repmat(str2double(new_mpo(1)),1,10));
            set(handle.mpo_all_slider_right,'value',str2double(new_mpo(1)));
            set(handle.mpo_right_slider1,'value',str2double(new_mpo(1)));
            set(handle.mpo_right_slider2,'value',str2double(new_mpo(1)));
            set(handle.mpo_right_slider3,'value',str2double(new_mpo(1)));
            set(handle.mpo_right_slider4,'value',str2double(new_mpo(1)));
            set(handle.mpo_right_slider5,'value',str2double(new_mpo(1)));
            
            set(handle.mpo_all_slider_left,'value',str2double(new_mpo(1)));
            set(handle.mpo_left_slider1,'value',str2double(new_mpo(1)));
            set(handle.mpo_left_slider2,'value',str2double(new_mpo(1)));
            set(handle.mpo_left_slider3,'value',str2double(new_mpo(1)));
            set(handle.mpo_left_slider4,'value',str2double(new_mpo(1)));
            set(handle.mpo_left_slider5,'value',str2double(new_mpo(1)));
        else
            if str2double(new_mpo(1)) < get(handle.mpo_all_slider_right,'min')
                new_mpo{1} = num2str(get(handle.mpo_all_slider_right,'min'));
                set(handle.mpo_all_edit_right,'string',[num2str(get(handle.mpo_all_slider_right,'min')) ' dB']);
                set(handle.mpo_right_edit1,'string',[num2str(get(handle.mpo_all_slider_right,'min')) ' dB']);
                set(handle.mpo_right_edit2,'string',[num2str(get(handle.mpo_all_slider_right,'min')) ' dB']);
                set(handle.mpo_right_edit3,'string',[num2str(get(handle.mpo_all_slider_right,'min')) ' dB']);
                set(handle.mpo_right_edit4,'string',[num2str(get(handle.mpo_all_slider_right,'min')) ' dB']);
                set(handle.mpo_right_edit5,'string',[num2str(get(handle.mpo_all_slider_right,'min')) ' dB']);
            elseif str2double(new_mpo(1)) > get(handle.mpo_all_slider_left,'max')
                new_mpo{1} = num2str(get(handle.mpo_all_slider_right,'max'));
                set(handle.mpo_all_edit_right,'string',[num2str(get(handle.mpo_all_slider_right,'max')) ' dB']);
                set(handle.mpo_right_edit1,'string',[num2str(get(handle.mpo_all_slider_right,'max')) ' dB']);
                set(handle.mpo_right_edit2,'string',[num2str(get(handle.mpo_all_slider_right,'max')) ' dB']);
                set(handle.mpo_right_edit3,'string',[num2str(get(handle.mpo_all_slider_right,'max')) ' dB']);
                set(handle.mpo_right_edit4,'string',[num2str(get(handle.mpo_all_slider_right,'max')) ' dB']);
                set(handle.mpo_right_edit5,'string',[num2str(get(handle.mpo_all_slider_right,'max')) ' dB']);
            else
                set(handle.mpo_all_edit_right,'string',[mpo_str ' dB']);
                set(handle.mpo_right_edit1,'string',[mpo_str ' dB']);
                set(handle.mpo_right_edit2,'string',[mpo_str ' dB']);
                set(handle.mpo_right_edit3,'string',[mpo_str ' dB']);
                set(handle.mpo_right_edit4,'string',[mpo_str ' dB']);
                set(handle.mpo_right_edit5,'string',[mpo_str ' dB']);
            end

            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
            [mpo(1) mpo(2) mpo(3) mpo(4) mpo(5) repmat(str2double(new_mpo(1)),1,5)]);
            set(handle.mpo_all_slider_right,'value',str2double(new_mpo(1)));
            set(handle.mpo_right_slider1,'value',str2double(new_mpo(1)));
            set(handle.mpo_right_slider2,'value',str2double(new_mpo(1)));
            set(handle.mpo_right_slider3,'value',str2double(new_mpo(1)));
            set(handle.mpo_right_slider4,'value',str2double(new_mpo(1)));
            set(handle.mpo_right_slider5,'value',str2double(new_mpo(1)));
        end
        
    case 'all_left'
        mpo_str = get(handle.mpo_all_edit_left,'string');
        new_mpo = strsplit(mpo_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(new_mpo(1)) < get(handle.mpo_all_slider_left,'min')
                new_mpo{1} = num2str(get(handle.mpo_all_slider_left,'min'));
                set(handle.mpo_all_edit_left,'string',[num2str(get(handle.mpo_all_slider_left,'min')) ' dB']);
                set(handle.mpo_left_edit1,'string',[num2str(get(handle.mpo_all_slider_left,'min')) ' dB']);
                set(handle.mpo_left_edit2,'string',[num2str(get(handle.mpo_all_slider_left,'min')) ' dB']);
                set(handle.mpo_left_edit3,'string',[num2str(get(handle.mpo_all_slider_left,'min')) ' dB']);
                set(handle.mpo_left_edit4,'string',[num2str(get(handle.mpo_all_slider_left,'min')) ' dB']);
                set(handle.mpo_left_edit5,'string',[num2str(get(handle.mpo_all_slider_left,'min')) ' dB']);
                
                set(handle.mpo_all_edit_right,'string',[num2str(get(handle.mpo_all_slider_right,'min')) ' dB']);
                set(handle.mpo_right_edit1,'string',[num2str(get(handle.mpo_all_slider_right,'min')) ' dB']);
                set(handle.mpo_right_edit2,'string',[num2str(get(handle.mpo_all_slider_right,'min')) ' dB']);
                set(handle.mpo_right_edit3,'string',[num2str(get(handle.mpo_all_slider_right,'min')) ' dB']);
                set(handle.mpo_right_edit4,'string',[num2str(get(handle.mpo_all_slider_right,'min')) ' dB']);
                set(handle.mpo_right_edit5,'string',[num2str(get(handle.mpo_all_slider_right,'min')) ' dB']);
            elseif str2double(new_mpo(1)) > get(handle.mpo_all_slider_left,'max')
                new_mpo{1} = num2str(get(handle.mpo_all_slider_left,'max'));
                set(handle.mpo_all_edit_left,'string',[num2str(get(handle.mpo_all_slider_left,'max')) ' dB']);
                set(handle.mpo_left_edit1,'string',[num2str(get(handle.mpo_all_slider_left,'max')) ' dB']);
                set(handle.mpo_left_edit2,'string',[num2str(get(handle.mpo_all_slider_left,'max')) ' dB']);
                set(handle.mpo_left_edit3,'string',[num2str(get(handle.mpo_all_slider_left,'max')) ' dB']);
                set(handle.mpo_left_edit4,'string',[num2str(get(handle.mpo_all_slider_left,'max')) ' dB']);
                set(handle.mpo_left_edit5,'string',[num2str(get(handle.mpo_all_slider_left,'max')) ' dB']);
                
                set(handle.mpo_all_edit_right,'string',[num2str(get(handle.mpo_all_slider_right,'max')) ' dB']);
                set(handle.mpo_right_edit1,'string',[num2str(get(handle.mpo_all_slider_right,'max')) ' dB']);
                set(handle.mpo_right_edit2,'string',[num2str(get(handle.mpo_all_slider_right,'max')) ' dB']);
                set(handle.mpo_right_edit3,'string',[num2str(get(handle.mpo_all_slider_right,'max')) ' dB']);
                set(handle.mpo_right_edit4,'string',[num2str(get(handle.mpo_all_slider_right,'max')) ' dB']);
                set(handle.mpo_right_edit5,'string',[num2str(get(handle.mpo_all_slider_right,'max')) ' dB']);
            else
                set(handle.mpo_all_edit_left,'string',[mpo_str ' dB']);
                set(handle.mpo_left_edit1,'string',[mpo_str ' dB']);
                set(handle.mpo_left_edit2,'string',[mpo_str ' dB']);
                set(handle.mpo_left_edit3,'string',[mpo_str ' dB']);
                set(handle.mpo_left_edit4,'string',[mpo_str ' dB']);
                set(handle.mpo_left_edit5,'string',[mpo_str ' dB']);
                
                set(handle.mpo_all_edit_right,'string',[mpo_str ' dB']);
                set(handle.mpo_right_edit1,'string',[mpo_str ' dB']);
                set(handle.mpo_right_edit2,'string',[mpo_str ' dB']);
                set(handle.mpo_right_edit3,'string',[mpo_str ' dB']);
                set(handle.mpo_right_edit4,'string',[mpo_str ' dB']);
                set(handle.mpo_right_edit5,'string',[mpo_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                repmat(str2double(new_mpo(1)),1,10));
            set(handle.mpo_all_slider_left,'value',str2double(new_mpo(1)));
            set(handle.mpo_left_slider1,'value',str2double(new_mpo(1)));
            set(handle.mpo_left_slider2,'value',str2double(new_mpo(1)));
            set(handle.mpo_left_slider3,'value',str2double(new_mpo(1)));
            set(handle.mpo_left_slider4,'value',str2double(new_mpo(1)));
            set(handle.mpo_left_slider5,'value',str2double(new_mpo(1)));
            
            set(handle.mpo_all_slider_right,'value',str2double(new_mpo(1)));
            set(handle.mpo_right_slider1,'value',str2double(new_mpo(1)));
            set(handle.mpo_right_slider2,'value',str2double(new_mpo(1)));
            set(handle.mpo_right_slider3,'value',str2double(new_mpo(1)));
            set(handle.mpo_right_slider4,'value',str2double(new_mpo(1)));
            set(handle.mpo_right_slider5,'value',str2double(new_mpo(1)));
        else
            if str2double(new_mpo(1)) < get(handle.mpo_all_slider_left,'min')
                new_mpo{1} = num2str(get(handle.mpo_all_slider_left,'min'));
                set(handle.mpo_all_edit_left,'string',[num2str(get(handle.mpo_all_slider_left,'min')) ' dB']);
                set(handle.mpo_left_edit1,'string',[num2str(get(handle.mpo_all_slider_left,'min')) ' dB']);
                set(handle.mpo_left_edit2,'string',[num2str(get(handle.mpo_all_slider_left,'min')) ' dB']);
                set(handle.mpo_left_edit3,'string',[num2str(get(handle.mpo_all_slider_left,'min')) ' dB']);
                set(handle.mpo_left_edit4,'string',[num2str(get(handle.mpo_all_slider_left,'min')) ' dB']);
                set(handle.mpo_left_edit5,'string',[num2str(get(handle.mpo_all_slider_left,'min')) ' dB']);
            elseif str2double(new_mpo(1)) > get(handle.mpo_all_slider_left,'max')
                new_mpo{1} = num2str(get(handle.mpo_all_slider_left,'max'));
                set(handle.mpo_all_edit_left,'string',[num2str(get(handle.mpo_all_slider_left,'max')) ' dB']);
                set(handle.mpo_left_edit1,'string',[num2str(get(handle.mpo_all_slider_left,'max')) ' dB']);
                set(handle.mpo_left_edit2,'string',[num2str(get(handle.mpo_all_slider_left,'max')) ' dB']);
                set(handle.mpo_left_edit3,'string',[num2str(get(handle.mpo_all_slider_left,'max')) ' dB']);
                set(handle.mpo_left_edit4,'string',[num2str(get(handle.mpo_all_slider_left,'max')) ' dB']);
                set(handle.mpo_left_edit5,'string',[num2str(get(handle.mpo_all_slider_left,'max')) ' dB']);
            else
                set(handle.mpo_all_edit_left,'string',[mpo_str ' dB']);
                set(handle.mpo_left_edit1,'string',[mpo_str ' dB']);
                set(handle.mpo_left_edit2,'string',[mpo_str ' dB']);
                set(handle.mpo_left_edit3,'string',[mpo_str ' dB']);
                set(handle.mpo_left_edit4,'string',[mpo_str ' dB']);
                set(handle.mpo_left_edit5,'string',[mpo_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [repmat(str2double(new_mpo(1)),1,5) mpo(6) mpo(7) mpo(8) mpo(9) mpo(10)]);
            
            set(handle.mpo_all_slider_left,'value',str2double(new_mpo(1)));
            set(handle.mpo_left_slider1,'value',str2double(new_mpo(1)));
            set(handle.mpo_left_slider2,'value',str2double(new_mpo(1)));
            set(handle.mpo_left_slider3,'value',str2double(new_mpo(1)));
            set(handle.mpo_left_slider4,'value',str2double(new_mpo(1)));
            set(handle.mpo_left_slider5,'value',str2double(new_mpo(1)));
        end
handle = plot_data_right(handle,mha);
handle = plot_data_left(handle,mha);

guidata(src, handle);
end
