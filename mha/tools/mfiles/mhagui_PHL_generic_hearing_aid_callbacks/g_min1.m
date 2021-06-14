function handle = g_min1(src,event)
handle = guidata(src);
mha = handle.mha;
% data.g1.max_right = max([handles.G80_right_slider1.Max-handles.G80_right_slider1.Value; handles.G50_right_slider1.Max-handles.G50_right_slider1.Value ]);
% data.g1.min_right = -max([handles.G80_right_slider1.Value-handles.G80_right_slider1.Min; handles.G80_right_slider1.Value-handles.G80_right_slider1.Min]);

G80 = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80');
G50 = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50');
pb_tag = get(gcbo,'tag');
switch pb_tag
    case '250 Hz_right'
        g50_str = get(handle.G50_right_edit1,'string');
        g50_val = strsplit(g50_str,' ');
        g80_str = get(handle.G80_right_edit1,'string');
        g80_val = strsplit(g80_str,' ');
        if str2double(g50_val(1))-1 <= handle.G50_right_slider1.Min && str2double(g80_val(1))-1 <= handle.G80_right_slider1.Min
            set(handle.G50_right_edit1,'string',[num2str(handle.G50_right_slider1.Min) ' dB']);
            set(handle.G50_right_slider1,'value',handle.G50_right_slider1.Min);
            set(handle.G80_right_edit1,'string',[num2str(handle.G80_right_slider1.Min) ' dB']);
            set(handle.G80_right_slider1,'value',handle.G80_right_slider1.Min);
            set([handle.all_gain_rightm31 handle.all_gain_rightm11],'enable','off');

        elseif str2double(g50_val(1))-1 <= handle.G50_right_slider1.Min
            set(handle.G50_right_edit1,'string',[num2str(handle.G50_right_slider1.Min) ' dB']);
            set(handle.G50_right_slider1,'value',handle.G50_right_slider1.Min);
            set(handle.G80_right_edit1,'string',[num2str(str2double(g80_val(1))-1) ' dB']);
            set(handle.G80_right_slider1,'value',str2double(g80_val(1))-1);
        elseif str2double(g80_val(1))-1 <= handle.G80_right_slider1.Min
            set(handle.G80_right_edit1,'string',[num2str(handle.G80_right_slider1.Min) ' dB']);
            set(handle.G80_right_slider1,'value',handle.G80_right_slider1.Min);
            set(handle.G50_right_edit1,'string',[num2str(str2double(g50_val(1))-1) ' dB']);
            set(handle.G50_right_slider1,'value',str2double(g50_val(1))-1);
        else
            set(handle.G50_right_edit1,'string',[num2str(str2double(g50_val(1))-1) ' dB']);
            set(handle.G50_right_slider1,'value',str2double(g50_val(1))-1);
            set(handle.G80_right_edit1,'string',[num2str(str2double(g80_val(1))-1) ' dB']);
            set(handle.G80_right_slider1,'value',str2double(g80_val(1))-1);
        end
        set([handle.all_gain_rightp31 handle.all_gain_rightp11],'enable','on');
            
        if isequal(get(handle.link,'tag'),'link')
            g50_str2 = get(handle.G50_left_edit1,'string');
            g50_val2 = strsplit(g50_str2,' ');
            g80_str2 = get(handle.G80_left_edit1,'string');
            g80_val2 = strsplit(g80_str2,' ');
            if str2double(g50_val2(1))-1 <= handle.G50_left_slider1.Min && str2double(g80_val2(1))-1 <= handle.G80_left_slider1.Min
                set(handle.G50_left_edit1,'string',[num2str(handle.G50_left_slider1.Min) ' dB']);
                set(handle.G50_left_slider1,'value',handle.G50_left_slider1.Min);
                set(handle.G80_left_edit1,'string',[num2str(handle.G80_left_slider1.Min) ' dB']);
                set(handle.G80_left_slider1,'value',handle.G80_left_slider1.Min);
                set([handle.all_gain_leftm31 handle.all_gain_leftm11],'enable','off');

            elseif str2double(g50_val2(1))-1 <= handle.G50_left_slider1.Min
                set(handle.G50_left_edit1,'string',[num2str(handle.G50_left_slider1.Min) ' dB']);
                set(handle.G50_left_slider1,'value',handle.G50_left_slider1.Min);
                set(handle.G80_left_edit1,'string',[num2str(str2double(g80_val2(1))-1) ' dB']);
                set(handle.G80_left_slider1,'value',str2double(g80_val2(1))-1);
            elseif str2double(g80_val2(1))-1 <= handle.G80_left_slider1.Min
                set(handle.G80_left_edit1,'string',[num2str(handle.G80_left_slider1.Min) ' dB']);
                set(handle.G80_left_slider1,'value',handle.G80_left_slider1.Min);
                set(handle.G50_left_edit1,'string',[num2str(str2double(g50_val2(1))-1) ' dB']);
                set(handle.G50_left_slider1,'value',str2double(g50_val2(1))-1);
            else
                set(handle.G50_left_edit1,'string',[num2str(str2double(g50_val2(1))-1) ' dB']);
                set(handle.G50_left_slider1,'value',str2double(g50_val2(1))-1);
                set(handle.G80_left_edit1,'string',[num2str(str2double(g80_val2(1))-1) ' dB']);
                set(handle.G80_left_slider1,'value',str2double(g80_val2(1))-1);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [handle.G50_left_slider1.Value G50(2:5) handle.G50_right_slider1.Value G50(7:10)]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [handle.G80_left_slider1.Value G80(2:5) handle.G80_right_slider1.Value G80(7:10)]);
            set([handle.all_gain_leftp31 handle.all_gain_leftp11],'enable','on');
        else
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1:5) handle.G50_right_slider1.Value G50(7:10)]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1:5) handle.G80_right_slider1.Value G80(7:10)]);
        end
    case '500 Hz_right'
        g50_str = get(handle.G50_right_edit2,'string');
        g50_val = strsplit(g50_str,' ');
        g80_str = get(handle.G80_right_edit2,'string');
        g80_val = strsplit(g80_str,' ');
        if str2double(g50_val(1))-1 <= handle.G50_right_slider2.Min && str2double(g80_val(1))-1 <= handle.G80_right_slider2.Min
            set(handle.G50_right_edit2,'string',[num2str(handle.G50_right_slider2.Min) ' dB']);
            set(handle.G50_right_slider2,'value',handle.G50_right_slider2.Min);
            set(handle.G80_right_edit2,'string',[num2str(handle.G80_right_slider2.Min) ' dB']);
            set(handle.G80_right_slider2,'value',handle.G80_right_slider2.Min);
            set([handle.all_gain_rightm32 handle.all_gain_rightm12],'enable','off');

        elseif str2double(g50_val(1))-1 <= handle.G50_right_slider2.Min
            set(handle.G50_right_edit2,'string',[num2str(handle.G50_right_slider2.Min) ' dB']);
            set(handle.G50_right_slider2,'value',handle.G50_right_slider2.Min);
            set(handle.G80_right_edit2,'string',[num2str(str2double(g80_val(1))-1) ' dB']);
            set(handle.G80_right_slider2,'value',str2double(g80_val(1))-1);
        elseif str2double(g80_val(1))-1 <= handle.G80_right_slider2.Min
            set(handle.G80_right_edit2,'string',[num2str(handle.G80_right_slider2.Min) ' dB']);
            set(handle.G80_right_slider2,'value',handle.G80_right_slider2.Min);
            set(handle.G50_right_edit2,'string',[num2str(str2double(g50_val(1))-1) ' dB']);
            set(handle.G50_right_slider2,'value',str2double(g50_val(1))-1);
        else
            set(handle.G50_right_edit2,'string',[num2str(str2double(g50_val(1))-1) ' dB']);
            set(handle.G50_right_slider2,'value',str2double(g50_val(1))-1);
            set(handle.G80_right_edit2,'string',[num2str(str2double(g80_val(1))-1) ' dB']);
            set(handle.G80_right_slider2,'value',str2double(g80_val(1))-1);
        end
        set([handle.all_gain_rightp32 handle.all_gain_rightp12],'enable','on');
            
        if isequal(get(handle.link,'tag'),'link')
            g50_str2 = get(handle.G50_left_edit2,'string');
            g50_val2 = strsplit(g50_str2,' ');
            g80_str2 = get(handle.G80_left_edit2,'string');
            g80_val2 = strsplit(g80_str2,' ');
            if str2double(g50_val2(1))-1 <= handle.G50_left_slider2.Min && str2double(g80_val2(1))-1 <= handle.G80_left_slider2.Min
                set(handle.G50_left_edit2,'string',[num2str(handle.G50_left_slider2.Min) ' dB']);
                set(handle.G50_left_slider2,'value',handle.G50_left_slider2.Min);
                set(handle.G80_left_edit2,'string',[num2str(handle.G80_left_slider2.Min) ' dB']);
                set(handle.G80_left_slider2,'value',handle.G80_left_slider2.Min);
                set([handle.all_gain_leftm32 handle.all_gain_leftm12],'enable','off');

            elseif str2double(g50_val2(1))-1 <= handle.G50_left_slider2.Min
                set(handle.G50_left_edit2,'string',[num2str(handle.G50_left_slider2.Min) ' dB']);
                set(handle.G50_left_slider2,'value',handle.G50_left_slider2.Min);
                set(handle.G80_left_edit2,'string',[num2str(str2double(g80_val2(1))-1) ' dB']);
                set(handle.G80_left_slider2,'value',str2double(g80_val2(1))-1);
            elseif str2double(g80_val2(1))-1 <= handle.G80_left_slider2.Min
                set(handle.G80_left_edit2,'string',[num2str(handle.G80_left_slider2.Min) ' dB']);
                set(handle.G80_left_slider2,'value',handle.G80_left_slider2.Min);
                set(handle.G50_left_edit2,'string',[num2str(str2double(g50_val2(1))-1) ' dB']);
                set(handle.G50_left_slider2,'value',str2double(g50_val2(1))-1);
            else
                set(handle.G50_left_edit2,'string',[num2str(str2double(g50_val2(1))-1) ' dB']);
                set(handle.G50_left_slider2,'value',str2double(g50_val2(1))-1);
                set(handle.G80_left_edit2,'string',[num2str(str2double(g80_val2(1))-1) ' dB']);
                set(handle.G80_left_slider2,'value',str2double(g80_val2(1))-1);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) handle.G50_left_slider2.Value G50(3:6) handle.G50_right_slider2.Value G50(8:10)]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) handle.G80_left_slider2.Value G80(3:6) handle.G80_right_slider2.Value G80(8:10)]);
            set([handle.all_gain_leftp32 handle.all_gain_leftp12],'enable','on');
        else
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1:6) handle.G50_right_slider2.Value G50(8:10)]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1:6) handle.G80_right_slider2.Value G80(8:10)]);
        end
    case '1 kHz_right'
        g50_str = get(handle.G50_right_edit3,'string');
        g50_val = strsplit(g50_str,' ');
        g80_str = get(handle.G80_right_edit3,'string');
        g80_val = strsplit(g80_str,' ');
        if str2double(g50_val(1))-1 <= handle.G50_right_slider3.Min && str2double(g80_val(1))-1 <= handle.G80_right_slider3.Min
            set(handle.G50_right_edit3,'string',[num2str(handle.G50_right_slider3.Min) ' dB']);
            set(handle.G50_right_slider3,'value',handle.G50_right_slider3.Min);
            set(handle.G80_right_edit3,'string',[num2str(handle.G80_right_slider3.Min) ' dB']);
            set(handle.G80_right_slider3,'value',handle.G80_right_slider3.Min);
            set([handle.all_gain_rightm33 handle.all_gain_rightm13],'enable','off');

        elseif str2double(g50_val(1))-1 <= handle.G50_right_slider3.Min
            set(handle.G50_right_edit3,'string',[num2str(handle.G50_right_slider3.Min) ' dB']);
            set(handle.G50_right_slider3,'value',handle.G50_right_slider3.Min);
            set(handle.G80_right_edit3,'string',[num2str(str2double(g80_val(1))-1) ' dB']);
            set(handle.G80_right_slider3,'value',str2double(g80_val(1))-1);
        elseif str2double(g80_val(1))-1 <= handle.G80_right_slider3.Min
            set(handle.G80_right_edit3,'string',[num2str(handle.G80_right_slider3.Min) ' dB']);
            set(handle.G80_right_slider3,'value',handle.G80_right_slider3.Min);
            set(handle.G50_right_edit3,'string',[num2str(str2double(g50_val(1))-1) ' dB']);
            set(handle.G50_right_slider3,'value',str2double(g50_val(1))-1);
        else
            set(handle.G50_right_edit3,'string',[num2str(str2double(g50_val(1))-1) ' dB']);
            set(handle.G50_right_slider3,'value',str2double(g50_val(1))-1);
            set(handle.G80_right_edit3,'string',[num2str(str2double(g80_val(1))-1) ' dB']);
            set(handle.G80_right_slider3,'value',str2double(g80_val(1))-1);
        end
        set([handle.all_gain_rightp33 handle.all_gain_rightp13],'enable','on');
            
        if isequal(get(handle.link,'tag'),'link')
            g50_str2 = get(handle.G50_left_edit3,'string');
            g50_val2 = strsplit(g50_str2,' ');
            g80_str2 = get(handle.G80_left_edit3,'string');
            g80_val2 = strsplit(g80_str2,' ');
            if str2double(g50_val2(1))-1 <= handle.G50_left_slider3.Min && str2double(g80_val2(1))-1 <= handle.G80_left_slider3.Min
                set(handle.G50_left_edit3,'string',[num2str(handle.G50_left_slider3.Min) ' dB']);
                set(handle.G50_left_slider3,'value',handle.G50_left_slider3.Min);
                set(handle.G80_left_edit3,'string',[num2str(handle.G80_left_slider3.Min) ' dB']);
                set(handle.G80_left_slider3,'value',handle.G80_left_slider3.Min);
                set([handle.all_gain_leftm33 handle.all_gain_leftm13],'enable','off');

            elseif str2double(g50_val2(1))-1 <= handle.G50_left_slider3.Min
                set(handle.G50_left_edit3,'string',[num2str(handle.G50_left_slider3.Min) ' dB']);
                set(handle.G50_left_slider3,'value',handle.G50_left_slider3.Min);
                set(handle.G80_left_edit3,'string',[num2str(str2double(g80_val2(1))-1) ' dB']);
                set(handle.G80_left_slider3,'value',str2double(g80_val2(1))-1);
            elseif str2double(g80_val2(1))-1 <= handle.G80_left_slider3.Min
                set(handle.G80_left_edit3,'string',[num2str(handle.G80_left_slider3.Min) ' dB']);
                set(handle.G80_left_slider3,'value',handle.G80_left_slider3.Min);
                set(handle.G50_left_edit3,'string',[num2str(str2double(g50_val2(1))-1) ' dB']);
                set(handle.G50_left_slider3,'value',str2double(g50_val2(1))-1);
            else
                set(handle.G50_left_edit3,'string',[num2str(str2double(g50_val2(1))-1) ' dB']);
                set(handle.G50_left_slider3,'value',str2double(g50_val2(1))-1);
                set(handle.G80_left_edit3,'string',[num2str(str2double(g80_val2(1))-1) ' dB']);
                set(handle.G80_left_slider3,'value',str2double(g80_val2(1))-1);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1:2) handle.G50_left_slider3.Value G50(4:7) handle.G50_right_slider3.Value G50(9:10)]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1:2) handle.G80_left_slider3.Value G80(4:7) handle.G80_right_slider3.Value G80(9:10)]);
            set([handle.all_gain_leftp33 handle.all_gain_leftp13],'enable','on');
        else
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1:7) handle.G50_right_slider3.Value G50(9:10)]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1:7) handle.G80_right_slider3.Value G80(9:10)]);
        end
    case '2 kHz_right'
        g50_str = get(handle.G50_right_edit4,'string');
        g50_val = strsplit(g50_str,' ');
        g80_str = get(handle.G80_right_edit4,'string');
        g80_val = strsplit(g80_str,' ');
        if str2double(g50_val(1))-1 <= handle.G50_right_slider4.Min && str2double(g80_val(1))-1 <= handle.G80_right_slider4.Min
            set(handle.G50_right_edit4,'string',[num2str(handle.G50_right_slider4.Min) ' dB']);
            set(handle.G50_right_slider4,'value',handle.G50_right_slider4.Min);
            set(handle.G80_right_edit4,'string',[num2str(handle.G80_right_slider4.Min) ' dB']);
            set(handle.G80_right_slider4,'value',handle.G80_right_slider4.Min);
            set([handle.all_gain_rightm34 handle.all_gain_rightm14],'enable','off');

        elseif str2double(g50_val(1))-1 <= handle.G50_right_slider4.Min
            set(handle.G50_right_edit4,'string',[num2str(handle.G50_right_slider4.Min) ' dB']);
            set(handle.G50_right_slider4,'value',handle.G50_right_slider4.Min);
            set(handle.G80_right_edit4,'string',[num2str(str2double(g80_val(1))-1) ' dB']);
            set(handle.G80_right_slider4,'value',str2double(g80_val(1))-1);
        elseif str2double(g80_val(1))-1 <= handle.G80_right_slider4.Min
            set(handle.G80_right_edit4,'string',[num2str(handle.G80_right_slider4.Min) ' dB']);
            set(handle.G80_right_slider4,'value',handle.G80_right_slider4.Min);
            set(handle.G50_right_edit4,'string',[num2str(str2double(g50_val(1))-1) ' dB']);
            set(handle.G50_right_slider4,'value',str2double(g50_val(1))-1);
        else
            set(handle.G50_right_edit4,'string',[num2str(str2double(g50_val(1))-1) ' dB']);
            set(handle.G50_right_slider4,'value',str2double(g50_val(1))-1);
            set(handle.G80_right_edit4,'string',[num2str(str2double(g80_val(1))-1) ' dB']);
            set(handle.G80_right_slider4,'value',str2double(g80_val(1))-1);
        end
        set([handle.all_gain_rightp34 handle.all_gain_rightp14],'enable','on');
            
        if isequal(get(handle.link,'tag'),'link')
            g50_str2 = get(handle.G50_left_edit4,'string');
            g50_val2 = strsplit(g50_str2,' ');
            g80_str2 = get(handle.G80_left_edit4,'string');
            g80_val2 = strsplit(g80_str2,' ');
            if str2double(g50_val2(1))-1 <= handle.G50_left_slider4.Min && str2double(g80_val2(1))-1 <= handle.G80_left_slider4.Min
                set(handle.G50_left_edit4,'string',[num2str(handle.G50_left_slider4.Min) ' dB']);
                set(handle.G50_left_slider4,'value',handle.G50_left_slider4.Min);
                set(handle.G80_left_edit4,'string',[num2str(handle.G80_left_slider4.Min) ' dB']);
                set(handle.G80_left_slider4,'value',handle.G80_left_slider4.Min);
                set([handle.all_gain_leftm34 handle.all_gain_leftm14],'enable','off');

            elseif str2double(g50_val2(1))-1 <= handle.G50_left_slider4.Min
                set(handle.G50_left_edit4,'string',[num2str(handle.G50_left_slider4.Min) ' dB']);
                set(handle.G50_left_slider4,'value',handle.G50_left_slider4.Min);
                set(handle.G80_left_edit4,'string',[num2str(str2double(g80_val2(1))-1) ' dB']);
                set(handle.G80_left_slider4,'value',str2double(g80_val2(1))-1);
            elseif str2double(g80_val2(1))-1 <= handle.G80_left_slider4.Min
                set(handle.G80_left_edit4,'string',[num2str(handle.G80_left_slider4.Min) ' dB']);
                set(handle.G80_left_slider4,'value',handle.G80_left_slider4.Min);
                set(handle.G50_left_edit4,'string',[num2str(str2double(g50_val2(1))-1) ' dB']);
                set(handle.G50_left_slider4,'value',str2double(g50_val2(1))-1);
            else
                set(handle.G50_left_edit4,'string',[num2str(str2double(g50_val2(1))-1) ' dB']);
                set(handle.G50_left_slider4,'value',str2double(g50_val2(1))-1);
                set(handle.G80_left_edit4,'string',[num2str(str2double(g80_val2(1))-1) ' dB']);
                set(handle.G80_left_slider4,'value',str2double(g80_val2(1))-1);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1:3) handle.G50_left_slider4.Value G50(5:8) handle.G50_right_slider4.Value G50(10)]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1:3) handle.G80_left_slider4.Value G80(5:8) handle.G80_right_slider4.Value G80(10)]);
            set([handle.all_gain_leftp34 handle.all_gain_leftp14],'enable','on');
        else
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1:8) handle.G50_right_slider4.Value G50(10)]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1:8) handle.G80_right_slider4.Value G80(10)]);
        end
    case '4 kHz_right' 
        g50_str = get(handle.G50_right_edit5,'string');
        g50_val = strsplit(g50_str,' ');
        g80_str = get(handle.G80_right_edit5,'string');
        g80_val = strsplit(g80_str,' ');
        if str2double(g50_val(1))-1 <= handle.G50_right_slider5.Min && str2double(g80_val(1))-1 <= handle.G80_right_slider5.Min
            set(handle.G50_right_edit5,'string',[num2str(handle.G50_right_slider5.Min) ' dB']);
            set(handle.G50_right_slider5,'value',handle.G50_right_slider5.Min);
            set(handle.G80_right_edit5,'string',[num2str(handle.G80_right_slider5.Min) ' dB']);
            set(handle.G80_right_slider5,'value',handle.G80_right_slider5.Min);
            set([handle.all_gain_rightm35 handle.all_gain_rightm15],'enable','off');

        elseif str2double(g50_val(1))-1 <= handle.G50_right_slider5.Min
            set(handle.G50_right_edit5,'string',[num2str(handle.G50_right_slider5.Min) ' dB']);
            set(handle.G50_right_slider5,'value',handle.G50_right_slider5.Min);
            set(handle.G80_right_edit5,'string',[num2str(str2double(g80_val(1))-1) ' dB']);
            set(handle.G80_right_slider5,'value',str2double(g80_val(1))-1);
        elseif str2double(g80_val(1))-1 <= handle.G80_right_slider5.Min
            set(handle.G80_right_edit5,'string',[num2str(handle.G80_right_slider5.Min) ' dB']);
            set(handle.G80_right_slider5,'value',handle.G80_right_slider5.Min);
            set(handle.G50_right_edit5,'string',[num2str(str2double(g50_val(1))-1) ' dB']);
            set(handle.G50_right_slider5,'value',str2double(g50_val(1))-1);
        else
            set(handle.G50_right_edit5,'string',[num2str(str2double(g50_val(1))-1) ' dB']);
            set(handle.G50_right_slider5,'value',str2double(g50_val(1))-1);
            set(handle.G80_right_edit5,'string',[num2str(str2double(g80_val(1))-1) ' dB']);
            set(handle.G80_right_slider5,'value',str2double(g80_val(1))-1);
        end
        set([handle.all_gain_rightp35 handle.all_gain_rightp15],'enable','on');
            
        if isequal(get(handle.link,'tag'),'link')
            g50_str2 = get(handle.G50_left_edit5,'string');
            g50_val2 = strsplit(g50_str2,' ');
            g80_str2 = get(handle.G80_left_edit5,'string');
            g80_val2 = strsplit(g80_str2,' ');
            if str2double(g50_val2(1))-1 <= handle.G50_left_slider5.Min && str2double(g80_val2(1))-1 <= handle.G80_left_slider5.Min
                set(handle.G50_left_edit5,'string',[num2str(handle.G50_left_slider5.Min) ' dB']);
                set(handle.G50_left_slider5,'value',handle.G50_left_slider5.Min);
                set(handle.G80_left_edit5,'string',[num2str(handle.G80_left_slider5.Min) ' dB']);
                set(handle.G80_left_slider5,'value',handle.G80_left_slider5.Min);
                set([handle.all_gain_leftm35 handle.all_gain_leftm15],'enable','off');

            elseif str2double(g50_val2(1))-1 <= handle.G50_left_slider5.Min
                set(handle.G50_left_edit5,'string',[num2str(handle.G50_left_slider5.Min) ' dB']);
                set(handle.G50_left_slider5,'value',handle.G50_left_slider5.Min);
                set(handle.G80_left_edit5,'string',[num2str(str2double(g80_val2(1))-1) ' dB']);
                set(handle.G80_left_slider5,'value',str2double(g80_val2(1))-1);
            elseif str2double(g80_val2(1))-1 <= handle.G80_left_slider5.Min
                set(handle.G80_left_edit5,'string',[num2str(handle.G80_left_slider5.Min) ' dB']);
                set(handle.G80_left_slider5,'value',handle.G80_left_slider5.Min);
                set(handle.G50_left_edit5,'string',[num2str(str2double(g50_val2(1))-1) ' dB']);
                set(handle.G50_left_slider5,'value',str2double(g50_val2(1))-1);
            else
                set(handle.G50_left_edit5,'string',[num2str(str2double(g50_val2(1))-1) ' dB']);
                set(handle.G50_left_slider5,'value',str2double(g50_val2(1))-1);
                set(handle.G80_left_edit5,'string',[num2str(str2double(g80_val2(1))-1) ' dB']);
                set(handle.G80_left_slider5,'value',str2double(g80_val2(1))-1);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1:4) handle.G50_left_slider5.Value G50(6:9) handle.G50_right_slider5.Value]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1:4) handle.G80_left_slider5.Value G80(6:9) handle.G80_right_slider5.Value]);
            set([handle.all_gain_leftp35 handle.all_gain_leftp15],'enable','on');
        else
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1:9) handle.G50_right_slider5.Value]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1:9) handle.G80_right_slider5.Value]);
        end
    case '250 Hz_left'
        g50_str = get(handle.G50_left_edit1,'string');
        g50_val = strsplit(g50_str,' ');
        g80_str = get(handle.G80_left_edit1,'string');
        g80_val = strsplit(g80_str,' ');
        if str2double(g50_val(1))-1 <= handle.G50_left_slider1.Min && str2double(g80_val(1))-1 <= handle.G80_left_slider1.Min
            set(handle.G50_left_edit1,'string',[num2str(handle.G50_left_slider1.Min) ' dB']);
            set(handle.G50_left_slider1,'value',handle.G50_left_slider1.Min);
            set(handle.G80_left_edit1,'string',[num2str(handle.G80_left_slider1.Min) ' dB']);
            set(handle.G80_left_slider1,'value',handle.G80_left_slider1.Min);
            set([handle.all_gain_leftm31 handle.all_gain_leftm11],'enable','off');

        elseif str2double(g50_val(1))-1 <= handle.G50_left_slider1.Min
            set(handle.G50_left_edit1,'string',[num2str(handle.G50_left_slider1.Min) ' dB']);
            set(handle.G50_left_slider1,'value',handle.G50_left_slider1.Min);
            set(handle.G80_left_edit1,'string',[num2str(str2double(g80_val(1))-1) ' dB']);
            set(handle.G80_left_slider1,'value',str2double(g80_val(1))-1);
        elseif str2double(g80_val(1))-1 <= handle.G80_left_slider1.Min
            set(handle.G80_left_edit1,'string',[num2str(handle.G80_left_slider1.Min) ' dB']);
            set(handle.G80_left_slider1,'value',handle.G80_left_slider1.Min);
            set(handle.G50_left_edit1,'string',[num2str(str2double(g50_val(1))-1) ' dB']);
            set(handle.G50_left_slider1,'value',str2double(g50_val(1))-1);
        else
            set(handle.G50_left_edit1,'string',[num2str(str2double(g50_val(1))-1) ' dB']);
            set(handle.G50_left_slider1,'value',str2double(g50_val(1))-1);
            set(handle.G80_left_edit1,'string',[num2str(str2double(g80_val(1))-1) ' dB']);
            set(handle.G80_left_slider1,'value',str2double(g80_val(1))-1);
        end
        set([handle.all_gain_leftp31 handle.all_gain_leftp11],'enable','on');
            
        if isequal(get(handle.link,'tag'),'link')
            g50_str2 = get(handle.G50_right_edit1,'string');
            g50_val2 = strsplit(g50_str2,' ');
            g80_str2 = get(handle.G80_right_edit1,'string');
            g80_val2 = strsplit(g80_str2,' ');
            if str2double(g50_val2(1))-1 <= handle.G50_right_slider1.Min && str2double(g80_val2(1))-1 <= handle.G80_right_slider1.Min
                set(handle.G50_right_edit1,'string',[num2str(handle.G50_right_slider1.Min) ' dB']);
                set(handle.G50_right_slider1,'value',handle.G50_right_slider1.Min);
                set(handle.G80_right_edit1,'string',[num2str(handle.G80_right_slider1.Min) ' dB']);
                set(handle.G80_right_slider1,'value',handle.G80_right_slider1.Min);
                set([handle.all_gain_rightm31 handle.all_gain_rightm11],'enable','off');

            elseif str2double(g50_val2(1))-1 <= handle.G50_right_slider1.Min
                set(handle.G50_right_edit1,'string',[num2str(handle.G50_right_slider1.Min) ' dB']);
                set(handle.G50_right_slider1,'value',handle.G50_right_slider1.Min);
                set(handle.G80_right_edit1,'string',[num2str(str2double(g80_val2(1))-1) ' dB']);
                set(handle.G80_right_slider1,'value',str2double(g80_val2(1))-1);
            elseif str2double(g80_val2(1))-1 <= handle.G80_right_slider1.Min
                set(handle.G80_right_edit1,'string',[num2str(handle.G80_right_slider1.Min) ' dB']);
                set(handle.G80_right_slider1,'value',handle.G80_right_slider1.Min);
                set(handle.G50_right_edit1,'string',[num2str(str2double(g50_val2(1))-1) ' dB']);
                set(handle.G50_right_slider1,'value',str2double(g50_val2(1))-1);
            else
                set(handle.G50_right_edit1,'string',[num2str(str2double(g50_val2(1))-1) ' dB']);
                set(handle.G50_right_slider1,'value',str2double(g50_val2(1))-1);
                set(handle.G80_right_edit1,'string',[num2str(str2double(g80_val2(1))-1) ' dB']);
                set(handle.G80_right_slider1,'value',str2double(g80_val2(1))-1);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [handle.G50_left_slider1.Value G50(2:5) handle.G50_right_slider1.Value G50(7:10) ]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [handle.G80_left_slider1.Value G80(2:5) handle.G80_right_slider1.Value G80(7:10) ]);
            set([handle.all_gain_leftp31 handle.all_gain_leftp11],'enable','on');
        else
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [handle.G50_left_slider1.Value G50(2:10)]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [handle.G80_left_slider1.Value G80(2:10)]);
        end
    case '500 Hz_left'
        g50_str = get(handle.G50_left_edit2,'string');
        g50_val = strsplit(g50_str,' ');
        g80_str = get(handle.G80_left_edit2,'string');
        g80_val = strsplit(g80_str,' ');
        if str2double(g50_val(1))-1 <= handle.G50_left_slider2.Min && str2double(g80_val(1))-1 <= handle.G80_left_slider2.Min
            set(handle.G50_left_edit2,'string',[num2str(handle.G50_left_slider2.Min) ' dB']);
            set(handle.G50_left_slider2,'value',handle.G50_left_slider2.Min);
            set(handle.G80_left_edit2,'string',[num2str(handle.G80_left_slider2.Min) ' dB']);
            set(handle.G80_left_slider2,'value',handle.G80_left_slider2.Min);
            set([handle.all_gain_leftm32 handle.all_gain_leftm12],'enable','off');

        elseif str2double(g50_val(1))-1 <= handle.G50_left_slider2.Min
            set(handle.G50_left_edit2,'string',[num2str(handle.G50_left_slider2.Min) ' dB']);
            set(handle.G50_left_slider2,'value',handle.G50_left_slider2.Min);
            set(handle.G80_left_edit2,'string',[num2str(str2double(g80_val(1))-1) ' dB']);
            set(handle.G80_left_slider2,'value',str2double(g80_val(1))-1);
        elseif str2double(g80_val(1))-1 <= handle.G80_left_slider2.Min
            set(handle.G80_left_edit2,'string',[num2str(handle.G80_left_slider2.Min) ' dB']);
            set(handle.G80_left_slider2,'value',handle.G80_left_slider2.Min);
            set(handle.G50_left_edit2,'string',[num2str(str2double(g50_val(1))-1) ' dB']);
            set(handle.G50_left_slider2,'value',str2double(g50_val(1))-1);
        else
            set(handle.G50_left_edit2,'string',[num2str(str2double(g50_val(1))-1) ' dB']);
            set(handle.G50_left_slider2,'value',str2double(g50_val(1))-1);
            set(handle.G80_left_edit2,'string',[num2str(str2double(g80_val(1))-1) ' dB']);
            set(handle.G80_left_slider2,'value',str2double(g80_val(1))-1);
        end
        set([handle.all_gain_leftp32 handle.all_gain_leftp12],'enable','on');
            
        if isequal(get(handle.link,'tag'),'link')
            g50_str2 = get(handle.G50_right_edit2,'string');
            g50_val2 = strsplit(g50_str2,' ');
            g80_str2 = get(handle.G80_right_edit2,'string');
            g80_val2 = strsplit(g80_str2,' ');
            if str2double(g50_val2(1))-1 <= handle.G50_right_slider2.Min && str2double(g80_val2(1))-1 <= handle.G80_right_slider2.Min
                set(handle.G50_right_edit2,'string',[num2str(handle.G50_right_slider2.Min) ' dB']);
                set(handle.G50_right_slider2,'value',handle.G50_right_slider2.Min);
                set(handle.G80_right_edit2,'string',[num2str(handle.G80_right_slider2.Min) ' dB']);
                set(handle.G80_right_slider2,'value',handle.G80_right_slider2.Min);
                set([handle.all_gain_rightm32 handle.all_gain_rightm12],'enable','off');

            elseif str2double(g50_val2(1))-1 <= handle.G50_right_slider2.Min
                set(handle.G50_right_edit2,'string',[num2str(handle.G50_right_slider2.Min) ' dB']);
                set(handle.G50_right_slider2,'value',handle.G50_right_slider2.Min);
                set(handle.G80_right_edit2,'string',[num2str(str2double(g80_val2(1))-1) ' dB']);
                set(handle.G80_right_slider2,'value',str2double(g80_val2(1))-1);
            elseif str2double(g80_val2(1))-1 <= handle.G80_right_slider2.Min
                set(handle.G80_right_edit2,'string',[num2str(handle.G80_right_slider2.Min) ' dB']);
                set(handle.G80_right_slider2,'value',handle.G80_right_slider2.Min);
                set(handle.G50_right_edit2,'string',[num2str(str2double(g50_val2(1))-1) ' dB']);
                set(handle.G50_right_slider2,'value',str2double(g50_val2(1))-1);
            else
                set(handle.G50_right_edit2,'string',[num2str(str2double(g50_val2(1))-1) ' dB']);
                set(handle.G50_right_slider2,'value',str2double(g50_val2(1))-1);
                set(handle.G80_right_edit2,'string',[num2str(str2double(g80_val2(1))-1) ' dB']);
                set(handle.G80_right_slider2,'value',str2double(g80_val2(1))-1);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) handle.G50_left_slider2.Value G50(3:6) handle.G50_right_slider2.Value G50(8:10) ]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) handle.G80_left_slider2.Value G80(3:6) handle.G80_right_slider2.Value G80(8:10) ]);
            set([handle.all_gain_leftp32 handle.all_gain_leftp12],'enable','on');
        else
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1) handle.G50_left_slider2.Value G50(3:10)]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1) handle.G80_left_slider2.Value G80(3:10)]);
        end
    case '1 kHz_left'
        g50_str = get(handle.G50_left_edit3,'string');
        g50_val = strsplit(g50_str,' ');
        g80_str = get(handle.G80_left_edit3,'string');
        g80_val = strsplit(g80_str,' ');
        if str2double(g50_val(1))-1 <= handle.G50_left_slider3.Min && str2double(g80_val(1))-1 <= handle.G80_left_slider3.Min
            set(handle.G50_left_edit3,'string',[num2str(handle.G50_left_slider3.Min) ' dB']);
            set(handle.G50_left_slider3,'value',handle.G50_left_slider3.Min);
            set(handle.G80_left_edit3,'string',[num2str(handle.G80_left_slider3.Min) ' dB']);
            set(handle.G80_left_slider3,'value',handle.G80_left_slider3.Min);
            set([handle.all_gain_leftm33 handle.all_gain_leftm13],'enable','off');

        elseif str2double(g50_val(1))-1 <= handle.G50_left_slider3.Min
            set(handle.G50_left_edit3,'string',[num2str(handle.G50_left_slider3.Min) ' dB']);
            set(handle.G50_left_slider3,'value',handle.G50_left_slider3.Min);
            set(handle.G80_left_edit3,'string',[num2str(str2double(g80_val(1))-1) ' dB']);
            set(handle.G80_left_slider3,'value',str2double(g80_val(1))-1);
        elseif str2double(g80_val(1))-1 <= handle.G80_left_slider3.Min
            set(handle.G80_left_edit3,'string',[num2str(handle.G80_left_slider3.Min) ' dB']);
            set(handle.G80_left_slider3,'value',handle.G80_left_slider3.Min);
            set(handle.G50_left_edit3,'string',[num2str(str2double(g50_val(1))-1) ' dB']);
            set(handle.G50_left_slider3,'value',str2double(g50_val(1))-1);
        else
            set(handle.G50_left_edit3,'string',[num2str(str2double(g50_val(1))-1) ' dB']);
            set(handle.G50_left_slider3,'value',str2double(g50_val(1))-1);
            set(handle.G80_left_edit3,'string',[num2str(str2double(g80_val(1))-1) ' dB']);
            set(handle.G80_left_slider3,'value',str2double(g80_val(1))-1);
        end
        set([handle.all_gain_leftp33 handle.all_gain_leftp13],'enable','on');
            
        if isequal(get(handle.link,'tag'),'link')
            g50_str2 = get(handle.G50_right_edit3,'string');
            g50_val2 = strsplit(g50_str2,' ');
            g80_str2 = get(handle.G80_right_edit3,'string');
            g80_val2 = strsplit(g80_str2,' ');
            if str2double(g50_val2(1))-1 <= handle.G50_right_slider3.Min && str2double(g80_val2(1))-1 <= handle.G80_right_slider3.Min
                set(handle.G50_right_edit3,'string',[num2str(handle.G50_right_slider3.Min) ' dB']);
                set(handle.G50_right_slider3,'value',handle.G50_right_slider3.Min);
                set(handle.G80_right_edit3,'string',[num2str(handle.G80_right_slider3.Min) ' dB']);
                set(handle.G80_right_slider3,'value',handle.G80_right_slider3.Min);
                set([handle.all_gain_rightm33 handle.all_gain_rightm13],'enable','off');

            elseif str2double(g50_val2(1))-1 <= handle.G50_right_slider3.Min
                set(handle.G50_right_edit3,'string',[num2str(handle.G50_right_slider3.Min) ' dB']);
                set(handle.G50_right_slider3,'value',handle.G50_right_slider3.Min);
                set(handle.G80_right_edit3,'string',[num2str(str2double(g80_val2(1))-1) ' dB']);
                set(handle.G80_right_slider3,'value',str2double(g80_val2(1))-1);
            elseif str2double(g80_val2(1))-1 <= handle.G80_right_slider3.Min
                set(handle.G80_right_edit3,'string',[num2str(handle.G80_right_slider3.Min) ' dB']);
                set(handle.G80_right_slider3,'value',handle.G80_right_slider3.Min);
                set(handle.G50_right_edit3,'string',[num2str(str2double(g50_val2(1))-1) ' dB']);
                set(handle.G50_right_slider3,'value',str2double(g50_val2(1))-1);
            else
                set(handle.G50_right_edit3,'string',[num2str(str2double(g50_val2(1))-1) ' dB']);
                set(handle.G50_right_slider3,'value',str2double(g50_val2(1))-1);
                set(handle.G80_right_edit3,'string',[num2str(str2double(g80_val2(1))-1) ' dB']);
                set(handle.G80_right_slider3,'value',str2double(g80_val2(1))-1);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1:2) handle.G50_left_slider3.Value G50(4:7) handle.G50_right_slider3.Value G50(9:10) ]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1:2) handle.G80_left_slider3.Value G80(4:7) handle.G80_right_slider3.Value G80(9:10) ]);
            set([handle.all_gain_leftp33 handle.all_gain_leftp13],'enable','on');
        else
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1:2) handle.G50_left_slider3.Value G50(4:10)]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1:2) handle.G80_left_slider3.Value G80(4:10)]);
        end
    case '2 kHz_left'
        g50_str = get(handle.G50_left_edit4,'string');
        g50_val = strsplit(g50_str,' ');
        g80_str = get(handle.G80_left_edit4,'string');
        g80_val = strsplit(g80_str,' ');
        if str2double(g50_val(1))-1 <= handle.G50_left_slider4.Min && str2double(g80_val(1))-1 <= handle.G80_left_slider4.Min
            set(handle.G50_left_edit4,'string',[num2str(handle.G50_left_slider4.Min) ' dB']);
            set(handle.G50_left_slider4,'value',handle.G50_left_slider4.Min);
            set(handle.G80_left_edit4,'string',[num2str(handle.G80_left_slider4.Min) ' dB']);
            set(handle.G80_left_slider4,'value',handle.G80_left_slider4.Min);
            set([handle.all_gain_leftm34 handle.all_gain_leftm14],'enable','off');

        elseif str2double(g50_val(1))-1 <= handle.G50_left_slider4.Min
            set(handle.G50_left_edit4,'string',[num2str(handle.G50_left_slider4.Min) ' dB']);
            set(handle.G50_left_slider4,'value',handle.G50_left_slider4.Min);
            set(handle.G80_left_edit4,'string',[num2str(str2double(g80_val(1))-1) ' dB']);
            set(handle.G80_left_slider4,'value',str2double(g80_val(1))-1);
        elseif str2double(g80_val(1))-1 <= handle.G80_left_slider4.Min
            set(handle.G80_left_edit4,'string',[num2str(handle.G80_left_slider4.Min) ' dB']);
            set(handle.G80_left_slider4,'value',handle.G80_left_slider4.Min);
            set(handle.G50_left_edit4,'string',[num2str(str2double(g50_val(1))-1) ' dB']);
            set(handle.G50_left_slider4,'value',str2double(g50_val(1))-1);
        else
            set(handle.G50_left_edit4,'string',[num2str(str2double(g50_val(1))-1) ' dB']);
            set(handle.G50_left_slider4,'value',str2double(g50_val(1))-1);
            set(handle.G80_left_edit4,'string',[num2str(str2double(g80_val(1))-1) ' dB']);
            set(handle.G80_left_slider4,'value',str2double(g80_val(1))-1);
        end
        set([handle.all_gain_leftp34 handle.all_gain_leftp14],'enable','on');
            
        if isequal(get(handle.link,'tag'),'link')
            g50_str2 = get(handle.G50_right_edit4,'string');
            g50_val2 = strsplit(g50_str2,' ');
            g80_str2 = get(handle.G80_right_edit4,'string');
            g80_val2 = strsplit(g80_str2,' ');
            if str2double(g50_val2(1))-1 <= handle.G50_right_slider4.Min && str2double(g80_val2(1))-1 <= handle.G80_right_slider4.Min
                set(handle.G50_right_edit4,'string',[num2str(handle.G50_right_slider4.Min) ' dB']);
                set(handle.G50_right_slider4,'value',handle.G50_right_slider4.Min);
                set(handle.G80_right_edit4,'string',[num2str(handle.G80_right_slider4.Min) ' dB']);
                set(handle.G80_right_slider4,'value',handle.G80_right_slider4.Min);
                set([handle.all_gain_rightm34 handle.all_gain_rightm14],'enable','off');

            elseif str2double(g50_val2(1))-1 <= handle.G50_right_slider4.Min
                set(handle.G50_right_edit4,'string',[num2str(handle.G50_right_slider4.Min) ' dB']);
                set(handle.G50_right_slider4,'value',handle.G50_right_slider4.Min);
                set(handle.G80_right_edit4,'string',[num2str(str2double(g80_val2(1))-1) ' dB']);
                set(handle.G80_right_slider4,'value',str2double(g80_val2(1))-1);
            elseif str2double(g80_val2(1))-1 <= handle.G80_right_slider4.Min
                set(handle.G80_right_edit4,'string',[num2str(handle.G80_right_slider4.Min) ' dB']);
                set(handle.G80_right_slider4,'value',handle.G80_right_slider4.Min);
                set(handle.G50_right_edit4,'string',[num2str(str2double(g50_val2(1))-1) ' dB']);
                set(handle.G50_right_slider4,'value',str2double(g50_val2(1))-1);
            else
                set(handle.G50_right_edit4,'string',[num2str(str2double(g50_val2(1))-1) ' dB']);
                set(handle.G50_right_slider4,'value',str2double(g50_val2(1))-1);
                set(handle.G80_right_edit4,'string',[num2str(str2double(g80_val2(1))-1) ' dB']);
                set(handle.G80_right_slider4,'value',str2double(g80_val2(1))-1);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1:3) handle.G50_left_slider4.Value G50(5:8) handle.G50_right_slider4.Value G50(10) ]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1:3) handle.G80_left_slider4.Value G80(5:8) handle.G80_right_slider4.Value G80(10) ]);
            set([handle.all_gain_leftp34 handle.all_gain_leftp14],'enable','on');
        else
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1:3) handle.G50_left_slider4.Value G50(5:10)]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1:3) handle.G80_left_slider4.Value G80(5:10)]);
        end
    case '4 kHz_left'
        g50_str = get(handle.G50_left_edit5,'string');
        g50_val = strsplit(g50_str,' ');
        g80_str = get(handle.G80_left_edit5,'string');
        g80_val = strsplit(g80_str,' ');
        if str2double(g50_val(1))-1 <= handle.G50_left_slider5.Min && str2double(g80_val(1))-1 <= handle.G80_left_slider5.Min
            set(handle.G50_left_edit5,'string',[num2str(handle.G50_left_slider5.Min) ' dB']);
            set(handle.G50_left_slider5,'value',handle.G50_left_slider5.Min);
            set(handle.G80_left_edit5,'string',[num2str(handle.G80_left_slider5.Min) ' dB']);
            set(handle.G80_left_slider5,'value',handle.G80_left_slider5.Min);
            set([handle.all_gain_leftm35 handle.all_gain_leftm15],'enable','off');

        elseif str2double(g50_val(1))-1 <= handle.G50_left_slider5.Min
            set(handle.G50_left_edit5,'string',[num2str(handle.G50_left_slider5.Min) ' dB']);
            set(handle.G50_left_slider5,'value',handle.G50_left_slider5.Min);
            set(handle.G80_left_edit5,'string',[num2str(str2double(g80_val(1))-1) ' dB']);
            set(handle.G80_left_slider5,'value',str2double(g80_val(1))-1);
        elseif str2double(g80_val(1))-1 <= handle.G80_left_slider5.Min
            set(handle.G80_left_edit5,'string',[num2str(handle.G80_left_slider5.Min) ' dB']);
            set(handle.G80_left_slider5,'value',handle.G80_left_slider5.Min);
            set(handle.G50_left_edit5,'string',[num2str(str2double(g50_val(1))-1) ' dB']);
            set(handle.G50_left_slider5,'value',str2double(g50_val(1))-1);
        else
            set(handle.G50_left_edit5,'string',[num2str(str2double(g50_val(1))-1) ' dB']);
            set(handle.G50_left_slider5,'value',str2double(g50_val(1))-1);
            set(handle.G80_left_edit5,'string',[num2str(str2double(g80_val(1))-1) ' dB']);
            set(handle.G80_left_slider5,'value',str2double(g80_val(1))-1);
        end
        set([handle.all_gain_leftp35 handle.all_gain_leftp15],'enable','on');
            
        if isequal(get(handle.link,'tag'),'link')
            g50_str2 = get(handle.G50_right_edit5,'string');
            g50_val2 = strsplit(g50_str2,' ');
            g80_str2 = get(handle.G80_right_edit5,'string');
            g80_val2 = strsplit(g80_str2,' ');
            if str2double(g50_val2(1))-1 <= handle.G50_right_slider5.Min && str2double(g80_val2(1))-1 <= handle.G80_right_slider5.Min
                set(handle.G50_right_edit5,'string',[num2str(handle.G50_right_slider5.Min) ' dB']);
                set(handle.G50_right_slider5,'value',handle.G50_right_slider5.Min);
                set(handle.G80_right_edit5,'string',[num2str(handle.G80_right_slider5.Min) ' dB']);
                set(handle.G80_right_slider5,'value',handle.G80_right_slider5.Min);
                set([handle.all_gain_rightm35 handle.all_gain_rightm15],'enable','off');

            elseif str2double(g50_val2(1))-1 <= handle.G50_right_slider5.Min
                set(handle.G50_right_edit5,'string',[num2str(handle.G50_right_slider5.Min) ' dB']);
                set(handle.G50_right_slider5,'value',handle.G50_right_slider5.Min);
                set(handle.G80_right_edit5,'string',[num2str(str2double(g80_val2(1))-1) ' dB']);
                set(handle.G80_right_slider5,'value',str2double(g80_val2(1))-1);
            elseif str2double(g80_val2(1))-1 <= handle.G80_right_slider5.Min
                set(handle.G80_right_edit5,'string',[num2str(handle.G80_right_slider5.Min) ' dB']);
                set(handle.G80_right_slider5,'value',handle.G80_right_slider5.Min);
                set(handle.G50_right_edit5,'string',[num2str(str2double(g50_val2(1))-1) ' dB']);
                set(handle.G50_right_slider5,'value',str2double(g50_val2(1))-1);
            else
                set(handle.G50_right_edit5,'string',[num2str(str2double(g50_val2(1))-1) ' dB']);
                set(handle.G50_right_slider5,'value',str2double(g50_val2(1))-1);
                set(handle.G80_right_edit5,'string',[num2str(str2double(g80_val2(1))-1) ' dB']);
                set(handle.G80_right_slider5,'value',str2double(g80_val2(1))-1);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1:4) handle.G50_left_slider5.Value G50(6:9) handle.G50_right_slider5.Value]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1:4) handle.G80_left_slider5.Value G80(6:9) handle.G80_right_slider5.Value]);
            set([handle.all_gain_leftp35 handle.all_gain_leftp15],'enable','on');
        else
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1:4) handle.G50_left_slider5.Value G50(6:10)]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1:4) handle.G80_left_slider5.Value G80(6:10)]);
        end
    case 'bb_right'
        slids_50 = [handle.G50_left_slider1 handle.G50_left_slider2 handle.G50_left_slider3 handle.G50_left_slider4 handle.G50_left_slider5...
            handle.G50_right_slider1 handle.G50_right_slider2 handle.G50_right_slider3 handle.G50_right_slider4 handle.G50_right_slider5];
        edits_50 = [handle.G50_left_edit1 handle.G50_left_edit2 handle.G50_left_edit3 handle.G50_left_edit4 handle.G50_left_edit5...
            handle.G50_right_edit1 handle.G50_right_edit2 handle.G50_right_edit3 handle.G50_right_edit4 handle.G50_right_edit5];
        slids_80 = [handle.G80_left_slider1 handle.G80_left_slider2 handle.G80_left_slider3 handle.G80_left_slider4 handle.G80_left_slider5...
            handle.G80_right_slider1 handle.G80_right_slider2 handle.G80_right_slider3 handle.G80_right_slider4 handle.G80_right_slider5];
        edits_80 = [handle.G80_left_edit1 handle.G80_left_edit2 handle.G80_left_edit3 handle.G80_left_edit4 handle.G80_left_edit5...
            handle.G80_right_edit1 handle.G80_right_edit2 handle.G80_right_edit3 handle.G80_right_edit4 handle.G80_right_edit5];
        if isequal(get(handle.link,'tag'),'link')
            for cc = 1:10
                if slids_50(cc).Value -1 > slids_50(cc).Max
                    set(edits_50(cc),'string',[num2str(slids_50(cc).Max) ' dB']);
                    set(slids_50(cc),'value',slids_50(cc).Max);
                elseif slids_50(cc).Value -1 < slids_50(cc).Min
                    set(edits_50(cc),'string',[num2str(slids_50(cc).Min) ' dB']);
                    set(slids_50(cc),'value',slids_50(cc).Min);
                else
                    set(edits_50(cc),'string',[num2str(slids_50(cc).Value -1) ' dB']);
                    set(slids_50(cc),'value',slids_50(cc).Value -1);
                end
                if slids_80(cc).Value -1 > slids_80(cc).Max
                    set(edits_80(cc),'string',[num2str(slids_80(cc).Max) ' dB']);
                    set(slids_80(cc),'value',slids_80(cc).Max);
                elseif slids_80(cc).Value -1 < slids_80(cc).Min
                    set(edits_80(cc),'string',[num2str(slids_80(cc).Min) ' dB']);
                    set(slids_80(cc),'value',slids_80(cc).Min);
                else
                    set(edits_80(cc),'string',[num2str(slids_80(cc).Value -1) ' dB']);
                    set(slids_80(cc),'value',slids_80(cc).Value -1);
                end
            end
        else  
             for cc = 6:10
                if slids_50(cc).Value -1 > slids_50(cc).Max
                    set(edits_50(cc),'string',[num2str(slids_50(cc).Max) ' dB']);
                    set(slids_50(cc),'value',slids_50(cc).Max);
                elseif slids_50(cc).Value -1 < slids_50(cc).Min
                    set(edits_50(cc),'string',[num2str(slids_50(cc).Min) ' dB']);
                    set(slids_50(cc),'value',slids_50(cc).Min);
                else
                    set(edits_50(cc),'string',[num2str(slids_50(cc).Value -1) ' dB']);
                    set(slids_50(cc),'value',slids_50(cc).Value -1);
                end
                if slids_80(cc).Value -1 > slids_80(cc).Max
                    set(edits_80(cc),'string',[num2str(slids_80(cc).Max) ' dB']);
                    set(slids_80(cc),'value',slids_80(cc).Max);
                elseif slids_80(cc).Value -1 < slids_80(cc).Min
                    set(edits_80(cc),'string',[num2str(slids_80(cc).Min) ' dB']);
                    set(slids_80(cc),'value',slids_80(cc).Min);
                else
                    set(edits_80(cc),'string',[num2str(slids_80(cc).Value -1) ' dB']);
                    set(slids_80(cc),'value',slids_80(cc).Value -1);
                end
            end
        end
        G50_1 = handle.G50_left_slider1.Value;
        G50_2 = handle.G50_left_slider2.Value;
        G50_3 = handle.G50_left_slider3.Value;
        G50_4 = handle.G50_left_slider4.Value;
        G50_5 = handle.G50_left_slider5.Value;
        G50_6 = handle.G50_right_slider1.Value;
        G50_7 = handle.G50_right_slider2.Value;
        G50_8 = handle.G50_right_slider3.Value;
        G50_9 = handle.G50_right_slider4.Value;
        G50_10 = handle.G50_right_slider5.Value;

        G80_1 = handle.G80_left_slider1.Value;
        G80_2 = handle.G80_left_slider2.Value;
        G80_3 = handle.G80_left_slider3.Value;
        G80_4 = handle.G80_left_slider4.Value;
        G80_5 = handle.G80_left_slider5.Value;
        G80_6 = handle.G80_right_slider1.Value;
        G80_7 = handle.G80_right_slider2.Value;
        G80_8 = handle.G80_right_slider3.Value;
        G80_9 = handle.G80_right_slider4.Value;
        G80_10 = handle.G80_right_slider5.Value;

        mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
            [G50_1 G50_2 G50_3 G50_4 G50_5 G50_6 G50_7 G50_8 G50_9 G50_10]);
        mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
            [G80_1 G80_2 G80_3 G80_4 G80_5 G80_6 G80_7 G80_8 G80_9 G80_10]);
        
   
        if any([G50_1 G50_2 G50_3 G50_4 G50_5 G80_1 G80_2 G80_3 G80_4 G80_5] >= 1)
            set([handle.all_bb_leftm1 handle.all_gain_leftm11 handle.all_gain_leftm12 handle.all_gain_leftm13...
                handle.all_gain_leftm14 handle.all_gain_leftm15],'enable','on');
            if any([G50_1 G50_2 G50_3 G50_4 G50_5 G80_1 G80_2 G80_3 G80_4 G80_5] >= 3)
                set([handle.all_bb_leftm3 handle.all_gain_leftm31 handle.all_gain_leftm32 handle.all_gain_leftm33...
                handle.all_gain_leftm34 handle.all_gain_leftm35],'enable','on');
            end
        else
            set([handle.all_bb_leftm1 handle.all_gain_leftm11 handle.all_gain_leftm12 handle.all_gain_leftm13...
                handle.all_gain_leftm14 handle.all_gain_leftm15 handle.all_bb_leftm3 handle.all_gain_leftm31...
                handle.all_gain_leftm32 handle.all_gain_leftm33 handle.all_gain_leftm34 handle.all_gain_leftm35 ],'enable','off');
        end
        if any([handle.G50_left_slider1.Max - handle.G50_left_slider1.Value handle.G50_left_slider2.Max - handle.G50_left_slider2.Value...
                handle.G50_left_slider3.Max - handle.G50_left_slider3.Value handle.G50_left_slider4.Max - handle.G50_left_slider4.Value...
                handle.G50_left_slider5.Max - handle.G50_left_slider5.Value handle.G80_left_slider1.Max - handle.G80_left_slider1.Value...
                handle.G80_left_slider2.Max - handle.G80_left_slider2.Value handle.G80_left_slider3.Max - handle.G80_left_slider3.Value...
                handle.G80_left_slider4.Max - handle.G80_left_slider4.Value handle.G80_left_slider5.Max - handle.G80_left_slider5.Value] >= 1)
            set([handle.all_bb_leftp1 handle.all_gain_leftp11 handle.all_gain_leftp12 handle.all_gain_leftp13...
                handle.all_gain_leftp14 handle.all_gain_leftp15],'enable','on');
            if any([handle.G50_left_slider1.Max - handle.G50_left_slider1.Value handle.G50_left_slider2.Max - handle.G50_left_slider2.Value...
                handle.G50_left_slider3.Max - handle.G50_left_slider3.Value handle.G50_left_slider4.Max - handle.G50_left_slider4.Value...
                handle.G50_left_slider5.Max - handle.G50_left_slider5.Value handle.G80_left_slider1.Max - handle.G80_left_slider1.Value...
                handle.G80_left_slider2.Max - handle.G80_left_slider2.Value handle.G80_left_slider3.Max - handle.G80_left_slider3.Value...
                handle.G80_left_slider4.Max - handle.G80_left_slider4.Value handle.G80_left_slider5.Max - handle.G80_left_slider5.Value] >= 3)
                set([handle.all_bb_leftp3 handle.all_gain_leftp31 handle.all_gain_leftp32 handle.all_gain_leftp33...
                handle.all_gain_leftp34 handle.all_gain_leftp35],'enable','on');
            end
        else
            set([handle.all_bb_leftp1 handle.all_gain_leftp11 handle.all_gain_leftp12 handle.all_gain_leftp13...
                handle.all_gain_leftp14 handle.all_gain_leftp15 handle.all_bb_leftp3 handle.all_gain_leftp31...
                handle.all_gain_leftp32 handle.all_gain_leftp33 handle.all_gain_leftp34 handle.all_gain_leftp35],'enable','off');
        end
        

        if any([G50_6 G50_7 G50_8 G50_9 G50_10 G80_6 G80_7 G80_8 G80_9 G80_10] >= 1)
            set([handle.all_bb_rightm1 handle.all_gain_rightm11 handle.all_gain_rightm12 handle.all_gain_rightm13...
                handle.all_gain_rightm14 handle.all_gain_rightm15],'enable','on');
            if any([G50_6 G50_7 G50_8 G50_9 G50_10 G80_6 G80_7 G80_8 G80_9 G80_10] >= 3)
                set([handle.all_bb_rightm3 handle.all_gain_rightm31 handle.all_gain_rightm32 handle.all_gain_rightm33...
                handle.all_gain_rightm34 handle.all_gain_rightm35],'enable','on');
            end
        else
            set([handle.all_bb_rightm1 handle.all_gain_rightm11 handle.all_gain_rightm12 handle.all_gain_rightm13...
                handle.all_gain_rightm14 handle.all_gain_rightm15 handle.all_bb_rightm3 handle.all_gain_rightm31...
                handle.all_gain_rightm32 handle.all_gain_rightm33 handle.all_gain_rightm34 handle.all_gain_rightm35 ],'enable','off');
        end
        if any([handle.G50_right_slider1.Max - handle.G50_right_slider1.Value handle.G50_right_slider2.Max - handle.G50_right_slider2.Value...
                handle.G50_right_slider3.Max - handle.G50_right_slider3.Value handle.G50_right_slider4.Max - handle.G50_right_slider4.Value...
                handle.G50_right_slider5.Max - handle.G50_right_slider5.Value handle.G80_right_slider1.Max - handle.G80_right_slider1.Value...
                handle.G80_right_slider2.Max - handle.G80_right_slider2.Value handle.G80_right_slider3.Max - handle.G80_right_slider3.Value...
                handle.G80_right_slider4.Max - handle.G80_right_slider4.Value handle.G80_right_slider5.Max - handle.G80_right_slider5.Value] >= 1)
            set([handle.all_bb_rightp1 handle.all_gain_rightp11 handle.all_gain_rightp12 handle.all_gain_rightp13...
                handle.all_gain_rightp14 handle.all_gain_rightp15],'enable','on');
            if any([handle.G50_right_slider1.Max - handle.G50_right_slider1.Value handle.G50_right_slider2.Max - handle.G50_right_slider2.Value...
                handle.G50_right_slider3.Max - handle.G50_right_slider3.Value handle.G50_right_slider4.Max - handle.G50_right_slider4.Value...
                handle.G50_right_slider5.Max - handle.G50_right_slider5.Value handle.G80_right_slider1.Max - handle.G80_right_slider1.Value...
                handle.G80_right_slider2.Max - handle.G80_right_slider2.Value handle.G80_right_slider3.Max - handle.G80_right_slider3.Value...
                handle.G80_right_slider4.Max - handle.G80_right_slider4.Value handle.G80_right_slider5.Max - handle.G80_right_slider5.Value] >= 3)
                set([handle.all_bb_rightp3 handle.all_gain_rightp31 handle.all_gain_rightp32 handle.all_gain_rightp33...
                handle.all_gain_rightp34 handle.all_gain_rightp35],'enable','on');
            end
        else
            set([handle.all_bb_rightp1 handle.all_gain_rightp11 handle.all_gain_rightp12 handle.all_gain_rightp13...
                handle.all_gain_rightp14 handle.all_gain_rightp15 handle.all_bb_rightp3 handle.all_gain_rightp31...
                handle.all_gain_rightp32 handle.all_gain_rightp33 handle.all_gain_rightp34 handle.all_gain_rightp35],'enable','off');
        end
    case 'bb_left'
        slids_50 = [handle.G50_left_slider1 handle.G50_left_slider2 handle.G50_left_slider3 handle.G50_left_slider4 handle.G50_left_slider5...
            handle.G50_right_slider1 handle.G50_right_slider2 handle.G50_right_slider3 handle.G50_right_slider4 handle.G50_right_slider5];
        edits_50 = [handle.G50_left_edit1 handle.G50_left_edit2 handle.G50_left_edit3 handle.G50_left_edit4 handle.G50_left_edit5...
            handle.G50_right_edit1 handle.G50_right_edit2 handle.G50_right_edit3 handle.G50_right_edit4 handle.G50_right_edit5];
        slids_80 = [handle.G80_left_slider1 handle.G80_left_slider2 handle.G80_left_slider3 handle.G80_left_slider4 handle.G80_left_slider5...
            handle.G80_right_slider1 handle.G80_right_slider2 handle.G80_right_slider3 handle.G80_right_slider4 handle.G80_right_slider5];
        edits_80 = [handle.G80_left_edit1 handle.G80_left_edit2 handle.G80_left_edit3 handle.G80_left_edit4 handle.G80_left_edit5...
            handle.G80_right_edit1 handle.G80_right_edit2 handle.G80_right_edit3 handle.G80_right_edit4 handle.G80_right_edit5];
        if isequal(get(handle.link,'tag'),'link')
            for cc = 1:10
                if slids_50(cc).Value -1 > slids_50(cc).Max
                    set(edits_50(cc),'string',[num2str(slids_50(cc).Max) ' dB']);
                    set(slids_50(cc),'value',slids_50(cc).Max);
                elseif slids_50(cc).Value -1 < slids_50(cc).Min
                    set(edits_50(cc),'string',[num2str(slids_50(cc).Min) ' dB']);
                    set(slids_50(cc),'value',slids_50(cc).Min);
                else
                    set(edits_50(cc),'string',[num2str(slids_50(cc).Value -1) ' dB']);
                    set(slids_50(cc),'value',slids_50(cc).Value -1);
                end
                if slids_80(cc).Value -1 > slids_80(cc).Max
                    set(edits_80(cc),'string',[num2str(slids_80(cc).Max) ' dB']);
                    set(slids_80(cc),'value',slids_80(cc).Max);
                elseif slids_80(cc).Value -1 < slids_80(cc).Min
                    set(edits_80(cc),'string',[num2str(slids_80(cc).Min) ' dB']);
                    set(slids_80(cc),'value',slids_80(cc).Min);
                else
                    set(edits_80(cc),'string',[num2str(slids_80(cc).Value -1) ' dB']);
                    set(slids_80(cc),'value',slids_80(cc).Value -1);
                end
            end
        else  
             for cc = 1:5
                if slids_50(cc).Value -1 > slids_50(cc).Max
                    set(edits_50(cc),'string',[num2str(slids_50(cc).Max) ' dB']);
                    set(slids_50(cc),'value',slids_50(cc).Max);
                elseif slids_50(cc).Value -1 < slids_50(cc).Min
                    set(edits_50(cc),'string',[num2str(slids_50(cc).Min) ' dB']);
                    set(slids_50(cc),'value',slids_50(cc).Min);
                else
                    set(edits_50(cc),'string',[num2str(slids_50(cc).Value -1) ' dB']);
                    set(slids_50(cc),'value',slids_50(cc).Value -1);
                end
                if slids_80(cc).Value -1 > slids_80(cc).Max
                    set(edits_80(cc),'string',[num2str(slids_80(cc).Max) ' dB']);
                    set(slids_80(cc),'value',slids_80(cc).Max);
                elseif slids_80(cc).Value -1 < slids_80(cc).Min
                    set(edits_80(cc),'string',[num2str(slids_80(cc).Min) ' dB']);
                    set(slids_80(cc),'value',slids_80(cc).Min);
                else
                    set(edits_80(cc),'string',[num2str(slids_80(cc).Value -1) ' dB']);
                    set(slids_80(cc),'value',slids_80(cc).Value -1);
                end
            end
        end
        G50_1 = handle.G50_left_slider1.Value;
        G50_2 = handle.G50_left_slider2.Value;
        G50_3 = handle.G50_left_slider3.Value;
        G50_4 = handle.G50_left_slider4.Value;
        G50_5 = handle.G50_left_slider5.Value;
        G50_6 = handle.G50_right_slider1.Value;
        G50_7 = handle.G50_right_slider2.Value;
        G50_8 = handle.G50_right_slider3.Value;
        G50_9 = handle.G50_right_slider4.Value;
        G50_10 = handle.G50_right_slider5.Value;

        G80_1 = handle.G80_left_slider1.Value;
        G80_2 = handle.G80_left_slider2.Value;
        G80_3 = handle.G80_left_slider3.Value;
        G80_4 = handle.G80_left_slider4.Value;
        G80_5 = handle.G80_left_slider5.Value;
        G80_6 = handle.G80_right_slider1.Value;
        G80_7 = handle.G80_right_slider2.Value;
        G80_8 = handle.G80_right_slider3.Value;
        G80_9 = handle.G80_right_slider4.Value;
        G80_10 = handle.G80_right_slider5.Value;

        mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
            [G50_1 G50_2 G50_3 G50_4 G50_5 G50_6 G50_7 G50_8 G50_9 G50_10]);
        mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
            [G80_1 G80_2 G80_3 G80_4 G80_5 G80_6 G80_7 G80_8 G80_9 G80_10]);
        
   
        if any([G50_1 G50_2 G50_3 G50_4 G50_5 G80_1 G80_2 G80_3 G80_4 G80_5] >= 1)
            set([handle.all_bb_leftm1 handle.all_gain_leftm11 handle.all_gain_leftm12 handle.all_gain_leftm13...
                handle.all_gain_leftm14 handle.all_gain_leftm15],'enable','on');
            if any([G50_1 G50_2 G50_3 G50_4 G50_5 G80_1 G80_2 G80_3 G80_4 G80_5] >= 3)
                set([handle.all_bb_leftm3 handle.all_gain_leftm31 handle.all_gain_leftm32 handle.all_gain_leftm33...
                handle.all_gain_leftm34 handle.all_gain_leftm35],'enable','on');
            end
        else
            set([handle.all_bb_leftm1 handle.all_gain_leftm11 handle.all_gain_leftm12 handle.all_gain_leftm13...
                handle.all_gain_leftm14 handle.all_gain_leftm15 handle.all_bb_leftm3 handle.all_gain_leftm31...
                handle.all_gain_leftm32 handle.all_gain_leftm33 handle.all_gain_leftm34 handle.all_gain_leftm35 ],'enable','off');
        end
        if any([handle.G50_left_slider1.Max - handle.G50_left_slider1.Value handle.G50_left_slider2.Max - handle.G50_left_slider2.Value...
                handle.G50_left_slider3.Max - handle.G50_left_slider3.Value handle.G50_left_slider4.Max - handle.G50_left_slider4.Value...
                handle.G50_left_slider5.Max - handle.G50_left_slider5.Value handle.G80_left_slider1.Max - handle.G80_left_slider1.Value...
                handle.G80_left_slider2.Max - handle.G80_left_slider2.Value handle.G80_left_slider3.Max - handle.G80_left_slider3.Value...
                handle.G80_left_slider4.Max - handle.G80_left_slider4.Value handle.G80_left_slider5.Max - handle.G80_left_slider5.Value] >= 1)
            set([handle.all_bb_leftp1 handle.all_gain_leftp11 handle.all_gain_leftp12 handle.all_gain_leftp13...
                handle.all_gain_leftp14 handle.all_gain_leftp15],'enable','on');
            if any([handle.G50_left_slider1.Max - handle.G50_left_slider1.Value handle.G50_left_slider2.Max - handle.G50_left_slider2.Value...
                handle.G50_left_slider3.Max - handle.G50_left_slider3.Value handle.G50_left_slider4.Max - handle.G50_left_slider4.Value...
                handle.G50_left_slider5.Max - handle.G50_left_slider5.Value handle.G80_left_slider1.Max - handle.G80_left_slider1.Value...
                handle.G80_left_slider2.Max - handle.G80_left_slider2.Value handle.G80_left_slider3.Max - handle.G80_left_slider3.Value...
                handle.G80_left_slider4.Max - handle.G80_left_slider4.Value handle.G80_left_slider5.Max - handle.G80_left_slider5.Value] >= 3)
                set([handle.all_bb_leftp3 handle.all_gain_leftp31 handle.all_gain_leftp32 handle.all_gain_leftp33...
                handle.all_gain_leftp34 handle.all_gain_leftp35],'enable','on');
            end
        else
            set([handle.all_bb_leftp1 handle.all_gain_leftp11 handle.all_gain_leftp12 handle.all_gain_leftp13...
                handle.all_gain_leftp14 handle.all_gain_leftp15 handle.all_bb_leftp3 handle.all_gain_leftp31...
                handle.all_gain_leftp32 handle.all_gain_leftp33 handle.all_gain_leftp34 handle.all_gain_leftp35],'enable','off');
        end
        

        if any([G50_6 G50_7 G50_8 G50_9 G50_10 G80_6 G80_7 G80_8 G80_9 G80_10] >= 1)
            set([handle.all_bb_rightm1 handle.all_gain_rightm11 handle.all_gain_rightm12 handle.all_gain_rightm13...
                handle.all_gain_rightm14 handle.all_gain_rightm15],'enable','on');
            if any([G50_6 G50_7 G50_8 G50_9 G50_10 G80_6 G80_7 G80_8 G80_9 G80_10] >= 3)
                set([handle.all_bb_rightm3 handle.all_gain_rightm31 handle.all_gain_rightm32 handle.all_gain_rightm33...
                handle.all_gain_rightm34 handle.all_gain_rightm35],'enable','on');
            end
        else
            set([handle.all_bb_rightm1 handle.all_gain_rightm11 handle.all_gain_rightm12 handle.all_gain_rightm13...
                handle.all_gain_rightm14 handle.all_gain_rightm15 handle.all_bb_rightm3 handle.all_gain_rightm31...
                handle.all_gain_rightm32 handle.all_gain_rightm33 handle.all_gain_rightm34 handle.all_gain_rightm35 ],'enable','off');
        end
        if any([handle.G50_right_slider1.Max - handle.G50_right_slider1.Value handle.G50_right_slider2.Max - handle.G50_right_slider2.Value...
                handle.G50_right_slider3.Max - handle.G50_right_slider3.Value handle.G50_right_slider4.Max - handle.G50_right_slider4.Value...
                handle.G50_right_slider5.Max - handle.G50_right_slider5.Value handle.G80_right_slider1.Max - handle.G80_right_slider1.Value...
                handle.G80_right_slider2.Max - handle.G80_right_slider2.Value handle.G80_right_slider3.Max - handle.G80_right_slider3.Value...
                handle.G80_right_slider4.Max - handle.G80_right_slider4.Value handle.G80_right_slider5.Max - handle.G80_right_slider5.Value] >= 1)
            set([handle.all_bb_rightp1 handle.all_gain_rightp11 handle.all_gain_rightp12 handle.all_gain_rightp13...
                handle.all_gain_rightp14 handle.all_gain_rightp15],'enable','on');
            if any([handle.G50_right_slider1.Max - handle.G50_right_slider1.Value handle.G50_right_slider2.Max - handle.G50_right_slider2.Value...
                handle.G50_right_slider3.Max - handle.G50_right_slider3.Value handle.G50_right_slider4.Max - handle.G50_right_slider4.Value...
                handle.G50_right_slider5.Max - handle.G50_right_slider5.Value handle.G80_right_slider1.Max - handle.G80_right_slider1.Value...
                handle.G80_right_slider2.Max - handle.G80_right_slider2.Value handle.G80_right_slider3.Max - handle.G80_right_slider3.Value...
                handle.G80_right_slider4.Max - handle.G80_right_slider4.Value handle.G80_right_slider5.Max - handle.G80_right_slider5.Value] >= 3)
                set([handle.all_bb_rightp3 handle.all_gain_rightp31 handle.all_gain_rightp32 handle.all_gain_rightp33...
                handle.all_gain_rightp34 handle.all_gain_rightp35],'enable','on');
            end
        else
            set([handle.all_bb_rightp1 handle.all_gain_rightp11 handle.all_gain_rightp12 handle.all_gain_rightp13...
                handle.all_gain_rightp14 handle.all_gain_rightp15 handle.all_bb_rightp3 handle.all_gain_rightp31...
                handle.all_gain_rightp32 handle.all_gain_rightp33 handle.all_gain_rightp34 handle.all_gain_rightp35],'enable','off');
        end
end
handle = plot_data_right(handle);
handle = plot_data_left(handle);
end
