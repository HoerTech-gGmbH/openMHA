function [handle,dat] = maxgain_cllb(src,event)
handle = guidata(src);
dat = handle.dat;
mha = handle.mha;

maxgain = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain');
G50 = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50');
G80 = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80');
edit_tag = get(gcbo,'tag');
switch edit_tag
    case 'edit_l_250Hz'
        newmax_str = get(handle.maxgain_left_edit1,'String');
        newmax = strsplit(newmax_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(newmax{1}) < 0
                newmax{1}= '0';
                set(handle.maxgain_left_edit1,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit1,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_left_edit1,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_right1,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_left_edit1,'string',[newmax_str ' dB']); 
                set(handle.maxgain_right_edit1,'string',[newmax_str ' dB']);
            end   
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                [str2double(newmax(1)) maxgain(2:5) str2double(newmax(1)) maxgain(7:10)]);
            
            if handle.G80_left_slider1.Value > str2double(newmax{1})
                set(handle.G80_left_slider1,'value',str2double(newmax{1}));
                set(handle.G80_left_edit1,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [str2double(newmax(1)) G80(2:10)]);
            end
            if handle.G80_right_slider1.Value > str2double(newmax{1})
                set(handle.G80_right_slider1,'value',str2double(newmax{1}));
                set(handle.G80_right_edit1,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1:5) str2double(newmax(1)) G80(7:10)]);
            end
            if handle.G50_left_slider1.Value > str2double(newmax{1})
                set(handle.G50_left_slider1,'value',str2double(newmax{1}));
                set(handle.G50_left_edit1,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [str2double(newmax(1)) G50(2:10)]);
            end
            if handle.G50_right_slider1.Value > str2double(newmax{1})
                set(handle.G50_right_slider1,'value',str2double(newmax{1}));
                set(handle.G50_right_edit1,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [g50(1:5) str2double(newmax(1)) G50(7:10)]);
            end
            
        else
            if str2double(newmax{1}) < 0
                newmax{1}= '0';
                set(handle.maxgain_left_edit1,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_left_edit1,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_left_edit1,'string',[newmax_str ' dB']); 
            end   
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                [str2double(newmax(1)) maxgain(2:10)]);
            if handle.G80_left_slider1.Value > str2double(newmax{1})
                set(handle.G80_left_slider1,'value',str2double(newmax{1}));
                set(handle.G80_left_edit1,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [str2double(newmax(1)) G80(2:10)]);
            end
            if handle.G50_left_slider1.Value > str2double(newmax{1})
                set(handle.G50_left_slider1,'value',str2double(newmax{1}));
                set(handle.G50_left_edit1,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [str2double(newmax(1)) G50(2:10)]);
            end
            
        end
    case 'edit_l_500Hz'
        newmax_str = get(handle.maxgain_left_edit2,'String');
        newmax = strsplit(newmax_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(newmax{1}) < 0
                newmax{1}= '0';
                set(handle.maxgain_left_edit2,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit2,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_left_edit2,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit2,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_left_edit2,'string',[newmax_str ' dB']);
                set(handle.maxgain_right_edit2,'string',[newmax_str ' dB']);
            end   
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                [maxgain(1) str2double(newmax(1)) maxgain(3:6) str2double(newmax(1)) maxgain(8:10)]);
            if handle.G80_left_slider2.Value > str2double(newmax{1})
                set(handle.G80_left_slider2,'value',str2double(newmax{1}));
                set(handle.G80_left_edit2,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) str2double(newmax(1)) G80(3:10)]);
            end
            if handle.G80_right_slider2.Value > str2double(newmax{1})
                set(handle.G80_right_slider2,'value',str2double(newmax{1}));
                set(handle.G80_right_edit2,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1:6) str2double(newmax(1)) G80(8:10)]);
            end
            if handle.G50_left_slider2.Value > str2double(newmax{1})
                set(handle.G50_left_slider2,'value',str2double(newmax{1}));
                set(handle.G50_left_edit2,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) str2double(newmax(1)) G50(3:10)]);
            end
            if handle.G50_right_slider2.Value > str2double(newmax{1})
                set(handle.G50_right_slider2,'value',str2double(newmax{1}));
                set(handle.G50_right_edit2,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1:6) str2double(newmax(1)) G50(8:10)]);
            end
        else
            if str2double(newmax{1}) < 0
                newmax{1}= '0';
                set(handle.maxgain_left_edit2,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_left_edit2,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_left_edit2,'string',[newmax_str ' dB']);
            end   
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                [maxgain(1) str2double(newmax(1)) maxgain(3:10)]);
            if handle.G80_left_slider2.Value > str2double(newmax{1})
                set(handle.G80_left_slider2,'value',str2double(newmax{1}));
                set(handle.G80_left_edit2,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) str2double(newmax(1)) G80(3:10)]);
            end
            if handle.G50_left_slider2.Value > str2double(newmax{1})
                set(handle.G50_left_slider2,'value',str2double(newmax{1}));
                set(handle.G50_left_edit2,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) str2double(newmax(1)) G50(3:10)]);
            end
        end
    case 'edit_l_1kHz'
        newmax_str = get(handle.maxgain_left_edit3,'String');
        newmax = strsplit(newmax_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(newmax{1}) < 0
                newmax{1}= '0';
                set(handle.maxgain_left_edit3,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit3,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_left_edit3,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit3,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_left_edit3,'string',[newmax_str ' dB']); 
                set(handle.maxgain_right_edit3,'string',[newmax_str ' dB']); 
            end   
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                [maxgain(1:2) str2double(newmax(1)) maxgain(4:7) str2double(newmax(1)) maxgain(9:10)]);
            if handle.G80_left_slider3.Value > str2double(newmax{1})
                set(handle.G80_left_slider3,'value',str2double(newmax{1}));
                set(handle.G80_left_edit3,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) str2double(newmax(1)) G80(4) G80(5) G80(6) G80(7) G80(8) G80(9) G80(10)]);
            end
            if handle.G80_right_slider3.Value > str2double(newmax{1})
                set(handle.G80_right_slider3,'value',str2double(newmax{1}));
                set(handle.G80_right_edit3,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) G80(3) G80(4) G80(5) G80(6) G80(7) str2double(newmax(1)) G80(9) G80(10)]);
            end
            if handle.G50_left_slider3.Value > str2double(newmax{1})
                set(handle.G50_left_slider3,'value',str2double(newmax{1}));
                set(handle.G50_left_edit3,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) str2double(newmax(1)) G50(4) G50(5) G50(6) G50(7) G50(8) G50(9) G50(10)]);
            end
            if handle.G50_right_slider3.Value > str2double(newmax{1})
                set(handle.G50_right_slider3,'value',str2double(newmax{1}));
                set(handle.G50_right_edit3,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) G50(3) G50(4) G50(5) G50(6) G50(7) str2double(newmax(1)) G50(9) G50(10)]);
            end
        else
            if str2double(newmax{1}) < 0
                newmax{1}= '0';
                set(handle.maxgain_left_edit3,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_left_edit3,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_left_edit3,'string',[newmax_str ' dB']); 
            end   
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                [maxgain(1) maxgain(2) str2double(newmax(1)) maxgain(4:10)]);
            if handle.G80_left_slider3.Value > str2double(newmax{1})
                set(handle.G80_left_slider3,'value',str2double(newmax{1}));
                set(handle.G80_left_edit3,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) str2double(newmax(1)) G80(4) G80(5) G80(6) G80(7) G80(8) G80(9) G80(10)]);
            end
            if handle.G50_left_slider3.Value > str2double(newmax{1})
                set(handle.G50_left_slider3,'value',str2double(newmax{1}));
                set(handle.G50_left_edit3,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) str2double(newmax(1)) G50(4) G50(5) G50(6) G50(7) G50(8) G50(9) G50(10)]);
            end
        end
    case 'edit_l_2kHz'
        newmax_str = get(handle.maxgain_left_edit4,'String');
        newmax = strsplit(newmax_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(newmax{1}) < 0
                newmax{1}= '0';
                set(handle.maxgain_left_edit4,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit4,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_left_edit4,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit4,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_left_edit4,'string',[newmax_str ' dB']); 
                set(handle.maxgain_right_edit4,'string',[newmax_str ' dB']); 
            end   
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                [maxgain(1:3) str2double(newmax(1)) maxgain(5:8) str2double(newmax(1)) maxgain(10)]);
            if handle.G80_left_slider4.Value > str2double(newmax{1})
                set(handle.G80_left_slider4,'value',str2double(newmax{1}));
                set(handle.G80_left_edit4,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) G80(3) str2double(newmax(1)) G80(5) G80(6) G80(7) G80(8) G80(9) G80(10)]);
            end
            if handle.G80_right_slider4.Value > str2double(newmax{1})
                set(handle.G80_right_slider4,'value',str2double(newmax{1}));
                set(handle.G80_right_edit4,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) G80(3) G80(4) G80(5) G80(6) G80(7) G80(8) str2double(newmax(1)) G80(10)]);
            end
            if handle.G50_left_slider4.Value > str2double(newmax{1})
                set(handle.G50_left_slider4,'value',str2double(newmax{1}));
                set(handle.G50_left_edit4,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) G50(3) str2double(newmax(1)) G50(5) G50(6) G50(7) G50(8) G50(9) G50(10)]);
            end
            if handle.G50_right_slider4.Value > str2double(newmax{1})
                set(handle.G50_right_slider4,'value',str2double(newmax{1}));
                set(handle.G50_right_edit4,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) G50(3) G50(4) G50(5) G50(6) G50(7) G50(8) str2double(newmax(1)) G50(10)]);
            end
        else
            if str2double(newmax{1}) < 0
                newmax{1}= '0';
                set(handle.maxgain_left_edit4,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_left_edit4,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_left_edit4,'string',[newmax_str ' dB']); 
            end   
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                [maxgain(1:3) str2double(newmax(1)) maxgain(5:10)]);
            if handle.G80_left_slider4.Value > str2double(newmax{1})
                set(handle.G80_left_slider4,'value',str2double(newmax{1}));
                set(handle.G80_left_edit4,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) G80(3) str2double(newmax(1)) G80(5) G80(6) G80(7) G80(8) G80(9) G80(10)]);
            end
            if handle.G50_left_slider4.Value > str2double(newmax{1})
                set(handle.G50_left_slider4,'value',str2double(newmax{1}));
                set(handle.G50_left_edit4,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) G50(3) str2double(newmax(1)) G50(5) G50(6) G50(7) G50(8) G50(9) G50(10)]);
            end
        end
    case 'edit_l_4kHz'
        newmax_str = get(handle.maxgain_left_edit5,'String');
        newmax = strsplit(newmax_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(newmax{1}) < 0
                newmax{1}= '0';
                set(handle.maxgain_left_edit5,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit5,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_left_edit5,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit5,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_left_edit5,'string',[newmax_str ' dB']); 
                set(handle.maxgain_right_edit5,'string',[newmax_str ' dB']); 
            end   
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                [maxgain(1:4) str2double(newmax(1)) maxgain(6:9) str2double(newmax(1))]);
            if handle.G80_left_slider5.Value > str2double(newmax{1})
                set(handle.G80_left_slider5,'value',str2double(newmax{1}));
                set(handle.G80_left_edit5,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) G80(3) G80(4) str2double(newmax(1)) G80(6) G80(7) G80(8) G80(9) G80(10)]);
            end
            if handle.G80_right_slider5.Value > str2double(newmax{1})
                set(handle.G80_right_slider5,'value',str2double(newmax{1}));
                set(handle.G80_right_edit5,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) G80(3) G80(4) G80(5) G80(6) G80(7) G80(8) G80(9) str2double(newmax(1))]);
            end
            if handle.G50_left_slider5.Value > str2double(newmax{1})
                set(handle.G50_left_slider5,'value',str2double(newmax{1}));
                set(handle.G50_left_edit5,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) G50(3) G50(4) str2double(newmax(1)) G50(6) G50(7) G50(8) G50(9) G50(10)]);
            end
            if handle.G50_right_slider5.Value > str2double(newmax{1})
                set(handle.G50_right_slider5,'value',str2double(newmax{1}));
                set(handle.G50_right_edit5,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) G50(3) G50(4) G50(5) G50(6) G50(7) G50(8) G50(9) str2double(newmax(1))]);
            end
        else
            if str2double(newmax{1}) < 0
                newmax{1}= '0';
                set(handle.maxgain_left_edit5,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_left_edit5,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_left_edit5,'string',[newmax_str ' dB']); 
            end   
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                [maxgain(1:4) str2double(newmax(1)) maxgain(6:10)]);
            if handle.G80_left_slider5.Value > str2double(newmax{1})
                set(handle.G80_left_slider5,'value',str2double(newmax{1}));
                set(handle.G80_left_edit5,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) G80(3) G80(4) str2double(newmax(1)) G80(6) G80(7) G80(8) G80(9) G80(10)]);
            end
            if handle.G50_left_slider5.Value > str2double(newmax{1})
                set(handle.G50_left_slider5,'value',str2double(newmax{1}));
                set(handle.G50_left_edit5,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) G50(3) G50(4) str2double(newmax(1)) G50(6) G50(7) G50(8) G50(9) G50(10)]);
            end
        end
    case 'edit_r_250Hz'
        newmax_str = get(handle.maxgain_right_edit1,'String');
        newmax = strsplit(newmax_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(newmax{1}) < 0
                newmax{1}= '0';
                set(handle.maxgain_right_edit1,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit1,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_right_edit1,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit1,'string',[num2str(newmax{1}) ' dB']);
            else 
                set(handle.maxgain_right_edit1,'string',[newmax_str ' dB']);
                set(handle.maxgain_left_edit1,'string',[newmax_str ' dB']);
            end   
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                [str2double(newmax(1)) maxgain(2:5) str2double(newmax(1)) maxgain(7:10)]);
            if handle.G80_right_slider1.Value > str2double(newmax{1})
                set(handle.G80_right_slider1,'value',str2double(newmax{1}));
                set(handle.G80_right_edit1,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) G80(3) G80(4) G80(5) str2double(newmax(1)) G80(7) G80(8) G80(9) G80(10)]);
            end
            if handle.G80_left_slider1.Value > str2double(newmax{1})
                set(handle.G80_left_slider1,'value',str2double(newmax{1}));
                set(handle.G80_left_edit1,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [str2double(newmax(1)) G80(2) G80(3) G80(4) G80(5) G80(6) G80(7) G80(8) G80(9) G80(10)]);
            end
            if handle.G50_right_slider1.Value > str2double(newmax{1})
                set(handle.G50_right_slider1,'value',str2double(newmax{1}));
                set(handle.G50_right_edit1,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) G50(3) G50(4) G50(5) str2double(newmax{1}) G50(7) G50(8) G50(9) G50(10)]);
            end
            if handle.G50_left_slider1.Value > str2double(newmax{1})
                set(handle.G50_left_slider1,'value',str2double(newmax{1}));
                set(handle.G50_left_edit1,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [str2double(newmax{1}) G50(2) G50(3) G50(4) G50(5) G50(6) G50(7) G50(8) G50(9) G50(10)]);
            end
        else
            if str2double(newmax{1}) < 0
                newmax{1}= '0';
                set(handle.maxgain_right_edit1,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_right_edit1,'string',[num2str(newmax{1}) ' dB']);
            else 
                set(handle.maxgain_right_edit1,'string',[newmax_str ' dB']);
            end   
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                [maxgain(1:5) str2double(newmax(1)) maxgain(7:10)]);
            if handle.G80_right_slider1.Value > str2double(newmax{1})
                set(handle.G80_right_slider1,'value',str2double(newmax{1}));
                set(handle.G80_right_edit1,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) G80(3) G80(4) G80(5) str2double(newmax(1)) G80(7) G80(8) G80(9) G80(10)]);
            end
            if handle.G50_right_slider1.Value > str2double(newmax{1})
                set(handle.G50_right_slider1,'value',str2double(newmax{1}));
                set(handle.G50_right_edit1,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) G50(3) G50(4) G50(5) str2double(newmax{1}) G50(7) G50(8) G50(9) G50(10)]);
            end
            
        end
    case 'edit_r_500Hz'
        newmax_str = get(handle.maxgain_right_edit2,'String');
        newmax = strsplit(newmax_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(newmax{1}) < 0
                newmax{1}= '0';
                set(handle.maxgain_right_edit2,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit2,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_right_edit2,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit2,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_right_edit2,'string',[newmax_str ' dB']);
                set(handle.maxgain_left_edit2,'string',[newmax_str ' dB']);
            end   
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                [maxgain(1) str2double(newmax(1)) maxgain(3:6) str2double(newmax(1)) maxgain(8:10)]);
            if handle.G80_right_slider2.Value > str2double(newmax{1})
                set(handle.G80_right_slider2,'value',str2double(newmax{1}));
                set(handle.G80_right_edit2,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) G80(3) G80(4) G80(5) G80(6) str2double(newmax(1)) G80(8) G80(9) G80(10)]);
            end
            if handle.G80_left_slider2.Value > str2double(newmax{1})
                set(handle.G80_left_slider2,'value',str2double(newmax{1}));
                set(handle.G80_left_edit2,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) str2double(newmax(1)) G80(3) G80(4) G80(5) G80(6) G80(7) G80(8) G80(9) G80(10)]);
            end
            if handle.G50_right_slider2.Value > str2double(newmax{1})
                set(handle.G50_right_slider2,'value',str2double(newmax{1}));
                set(handle.G50_right_edit2,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) G50(3) G50(4) G50(5) G50(6) str2double(newmax(1)) G50(8) G50(9) G50(10)]);
            end
            if handle.G50_left_slider2.Value > str2double(newmax{1})
                set(handle.G50_left_slider2,'value',str2double(newmax{1}));
                set(handle.G50_left_edit2,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) str2double(newmax(1)) G50(3) G50(4) G50(5) G50(6) G50(7) G50(8) G50(9) G50(10)]);
            end
        else
            if str2double(newmax{1}) < 0
                newmax{1}= '0';
                set(handle.maxgain_right_edit2,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_right_edit2,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_right_edit2,'string',[newmax_str ' dB']);
            end   
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                [maxgain(1:6) str2double(newmax(1)) maxgain(8:10)]);
            if handle.G80_right_slider2.Value > str2double(newmax{1})
                set(handle.G80_right_slider2,'value',str2double(newmax{1}));
                set(handle.G80_right_edit2,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) G80(3) G80(4) G80(5) G80(6) str2double(newmax(1)) G80(8) G80(9) G80(10)]);
            end
            if handle.G50_right_slider2.Value > str2double(newmax{1})
                set(handle.G50_right_slider2,'value',str2double(newmax{1}));
                set(handle.G50_right_edit2,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) G50(3) G50(4) G50(5) G50(6) str2double(newmax(1)) G50(8) G50(9) G50(10)]);
            end
        end
    case 'edit_r_1kHz'
        newmax_str = get(handle.maxgain_right_edit3,'String');
        newmax = strsplit(newmax_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(newmax{1}) < 0
                newmax{1}= '0';
                set(handle.maxgain_right_edit3,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit3,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_right_edit3,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit3,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_right_edit3,'string',[newmax_str ' dB']);
                set(handle.maxgain_left_edit3,'string',[newmax_str ' dB']);
            end   
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                [maxgain(1:2) str2double(newmax(1)) maxgain(4:7) str2double(newmax(1)) maxgain(9:10)]);
            if handle.G80_right_slider3.Value > str2double(newmax{1})
                set(handle.G80_right_slider3,'value',str2double(newmax{1}));
                set(handle.G80_right_edit3,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) G80(3) G80(4) G80(5) G80(6) G80(7) str2double(newmax(1)) G80(9) G80(10)]);
            end
            if handle.G80_left_slider3.Value > str2double(newmax{1})
                set(handle.G80_left_slider3,'value',str2double(newmax{1}));
                set(handle.G80_left_edit3,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) str2double(newmax(1)) G80(4) G80(5) G80(6) G80(7) G80(8) G80(9) G80(10)]);
            end
            if handle.G50_right_slider3.Value > str2double(newmax{1})
                set(handle.G50_right_slider3,'value',str2double(newmax{1}));
                set(handle.G50_right_edit3,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) G50(3) G50(4) G50(5) G50(6) G50(7) str2double(newmax(1)) G50(9) G50(10)]);
            end
            if handle.G50_left_slider3.Value > str2double(newmax{1})
                set(handle.G50_left_slider3,'value',str2double(newmax{1}));
                set(handle.G50_left_edit3,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) str2double(newmax(1)) G50(4) G50(5) G50(6) G50(7) G50(8) G50(9) G50(10)]);
            end
        else
            if str2double(newmax{1}) < 0
                newmax{1}= '0';
                set(handle.maxgain_right_edit3,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_right_edit3,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_right_edit3,'string',[newmax_str ' dB']);
            end   
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                [maxgain(1:7) str2double(newmax(1)) maxgain(9:10)]);
            if handle.G80_right_slider3.Value > str2double(newmax{1})
                set(handle.G80_right_slider3,'value',str2double(newmax{1}));
                set(handle.G80_right_edit3,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) G80(3) G80(4) G80(5) G80(6) G80(7) str2double(newmax(1)) G80(9) G80(10)]);
            end
           
            if handle.G50_right_slider3.Value > str2double(newmax{1})
                set(handle.G50_right_slider3,'value',str2double(newmax{1}));
                set(handle.G50_right_edit3,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) G50(3) G50(4) G50(5) G50(6) G50(7) str2double(newmax(1)) G50(9) G50(10)]);
            end
        end
    case 'edit_r_2kHz'
        newmax_str = get(handle.maxgain_right_edit4,'String');
        newmax = strsplit(newmax_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(newmax{1}) < 0
                newmax{1}= '0';
                set(handle.maxgain_right_edit4,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit4,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_right_edit4,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit4,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_right_edit4,'string',[newmax_str ' dB']); 
                set(handle.maxgain_left_edit4,'string',[newmax_str ' dB']); 
            end   
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                [maxgain(1:2) str2double(newmax(1)) maxgain(4:8) str2double(newmax(1)) maxgain(10)]);
            if handle.G80_right_slider4.Value > str2double(newmax{1})
                set(handle.G80_right_slider4,'value',str2double(newmax{1}));
                set(handle.G80_right_edit4,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) G80(3) G80(4) G80(5) G80(6) G80(7) G80(8) str2double(newmax(1)) G80(10)]);
            end
            if handle.G80_left_slider4.Value > str2double(newmax{1})
                set(handle.G80_left_slider4,'value',str2double(newmax{1}));
                set(handle.G80_left_edit4,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) G80(3) str2double(newmax(1)) G80(5) G80(6) G80(7) G80(8) G80(9) G80(10)]);
            end
            if handle.G50_right_slider4.Value > str2double(newmax{1})
                set(handle.G50_right_slider4,'value',str2double(newmax{1}));
                set(handle.G50_right_edit4,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) G50(3) G50(4) G50(5) G50(6) G50(7) G50(8) str2double(newmax(1)) G50(10)]);
            end
            if handle.G50_left_slider4.Value > str2double(newmax{1})
                set(handle.G50_left_slider4,'value',str2double(newmax{1}));
                set(handle.G50_left_edit4,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) G50(3) str2double(newmax(1)) G50(5) G50(6) G50(7) G50(8) G50(9) G50(10)]);
            end
        else
            if str2double(newmax{1}) < 0
                newmax{1}= '0';
                set(handle.maxgain_right_edit4,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_right_edit4,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_right_edit4,'string',[newmax_str ' dB']); 
            end   
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                [maxgain(1:8) str2double(newmax(1)) maxgain(10)]);
            if handle.G80_right_slider4.Value > str2double(newmax{1})
                set(handle.G80_right_slider4,'value',str2double(newmax{1}));
                set(handle.G80_right_edit4,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) G80(3) G80(4) G80(5) G80(6) G80(7) G80(8) str2double(newmax(1)) G80(10)]);
            end
            if handle.G50_right_slider4.Value > str2double(newmax{1})
                set(handle.G50_right_slider4,'value',str2double(newmax{1}));
                set(handle.G50_right_edit4,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) G50(3) G50(4) G50(5) G50(6) G50(7) G50(8) str2double(newmax(1)) G50(10)]);
            end
        end
    case 'edit_r_4kHz'
        newmax_str = get(handle.maxgain_right_edit5,'String');
        newmax = strsplit(newmax_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(newmax{1}) < 0
                newmax{1}= '0';
                set(handle.maxgain_right_edit5,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit5,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_right_edit5,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit5,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_right_edit5,'string',[newmax_str ' dB']);
                set(handle.maxgain_left_edit5,'string',[newmax_str ' dB']);
            end   
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                [maxgain(1:4) str2double(newmax(1)) maxgain(6:9) str2double(newmax(1))]);
            if handle.G80_right_slider5.Value > str2double(newmax{1})
                set(handle.G80_right_slider5,'value',str2double(newmax{1}));
                set(handle.G80_right_edit5,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) G80(3) G80(4) G80(5) G80(6) G80(7) G80(8) G80(9) str2double(newmax(1))]);
            end
            if handle.G80_left_slider5.Value > str2double(newmax{1})
                set(handle.G80_left_slider5,'value',str2double(newmax{1}));
                set(handle.G80_left_edit5,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1) G80(2) G80(3) G80(4) str2double(newmax(1)) G80(6) G80(7) G80(8) G80(9) G80(10)]);
            end
            if handle.G50_right_slider5.Value > str2double(newmax{1})
                set(handle.G50_right_slider5,'value',str2double(newmax{1}));
                set(handle.G50_right_edit5,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) G50(3) G50(4) G50(5) G50(6) G50(7) G50(8) G50(9) str2double(newmax(1))]);
            end
            if handle.G50_left_slider5.Value > str2double(newmax{1})
                set(handle.G50_left_slider5,'value',str2double(newmax{1}));
                set(handle.G50_left_edit5,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1) G50(2) G50(3) G50(4) str2double(newmax(1)) G50(6) G50(7) G50(8) G50(9) G50(10)]);
            end
        else
            if str2double(newmax{1}) < 0
                newmax{1}= '0';
                set(handle.maxgain_right_edit5,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_right_edit5,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_right_edit5,'string',[newmax_str ' dB']);
            end   
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                [maxgain(1:9) str2double(newmax(1))]);
            if handle.G80_right_slider5.Value > str2double(newmax{1})
                set(handle.G80_right_slider5,'value',str2double(newmax{1}));
                set(handle.G80_right_edit5,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [G80(1:9) str2double(newmax(1))]);
            end
            if handle.G50_right_slider5.Value > str2double(newmax{1})
                set(handle.G50_right_slider5,'value',str2double(newmax{1}));
                set(handle.G50_right_edit5,'string',[newmax{1} ' dB']);
                mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [G50(1:9) str2double(newmax(1))]);
            end
        end
    case 'all_right'
        newmax_str = get(handle.maxgain_all_edit_right,'String');
        newmax = strsplit(newmax_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(newmax{1}) <= 0
                newmax{1}= '0';
                set(handle.maxgain_all_edit_right,'string',[num2str(newmax{1}) ' dB']);
                set(handle.G50_all_edit_right,'string',[num2str(newmax{1}) ' dB']);
                set(handle.G80_all_edit_right,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit1,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit2,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit3,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit4,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit5,'string',[num2str(newmax{1}) ' dB']);
                
                set(handle.maxgain_all_edit_left,'string',[num2str(newmax{1}) ' dB']);
                set(handle.G50_all_edit_left,'string',[num2str(newmax{1}) ' dB']);
                set(handle.G80_all_edit_left,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit1,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit2,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit3,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit4,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit5,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_all_edit_right,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit1,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit2,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit3,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit4,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit5,'string',[num2str(newmax{1}) ' dB']);
                
                set(handle.maxgain_all_edit_left,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit1,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit2,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit3,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit4,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit5,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_all_edit_right,'string',[newmax_str ' dB']);
                set(handle.maxgain_right_edit1,'string',[newmax_str ' dB']);
                set(handle.maxgain_right_edit2,'string',[newmax_str ' dB']);
                set(handle.maxgain_right_edit3,'string',[newmax_str ' dB']);
                set(handle.maxgain_right_edit4,'string',[newmax_str ' dB']);
                set(handle.maxgain_right_edit5,'string',[newmax_str ' dB']);
                
                set(handle.maxgain_all_edit_left,'string',[newmax_str ' dB']);
                set(handle.maxgain_left_edit1,'string',[newmax_str ' dB']);
                set(handle.maxgain_left_edit2,'string',[newmax_str ' dB']);
                set(handle.maxgain_left_edit3,'string',[newmax_str ' dB']);
                set(handle.maxgain_left_edit4,'string',[newmax_str ' dB']);
                set(handle.maxgain_left_edit5,'string',[newmax_str ' dB']);
            end   
        mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
        repmat(str2double(newmax(1)),1,10));
    
        if handle.G80_all_slider_right.Value > str2double(newmax{1})
            set(handle.G80_all_slider_right.Value,'value',str2double(newmax{1}));
            set(handle.G80_all_edit_right.Value,'string',[newmax{1} ' dB']);
        end
        if handle.G50_all_slider_right.Value > str2double(newmax{1})
            set(handle.G50_all_slider_right.Value,'value',str2double(newmax{1}));
            set(handle.G50_all_edit_right.Value,'string',[newmax{1} ' dB']);
        end
        if handle.G80_right_slider1.Value > str2double(newmax{1})
            set(handle.G80_right_slider1,'value',str2double(newmax{1}));
            set(handle.G80_right_edit1,'string',[newmax{1} ' dB']);
        end
        if handle.G50_right_slider1.Value > str2double(newmax{1})
            set(handle.G50_right_slider1,'value',str2double(newmax{1}));
            set(handle.G50_right_edit1,'string',[newmax{1} ' dB']);
        end
        if handle.G80_right_slider2.Value > str2double(newmax{1})
            set(handle.G80_right_slider2,'value',str2double(newmax{1}));
            set(handle.G80_right_edit2,'string',[newmax{1} ' dB']);
        end
        if handle.G50_right_slider2.Value > str2double(newmax{1})
            set(handle.G50_right_slider2,'value',str2double(newmax{1}));
            set(handle.G50_right_edit2,'string',[newmax{1} ' dB']);
        end
        if handle.G80_right_slider3.Value > str2double(newmax{1})
            set(handle.G80_right_slider3,'value',str2double(newmax{1}));
            set(handle.G80_right_edit3,'string',[newmax{1} ' dB']);
        end
        if handle.G50_right_slider3.Value > str2double(newmax{1})
            set(handle.G50_right_slider3,'value',str2double(newmax{1}));
            set(handle.G50_right_edit3,'string',[newmax{1} ' dB']);
        end
        if handle.G80_right_slider4.Value > str2double(newmax{1})
            set(handle.G80_right_slider4,'value',str2double(newmax{1}));
            set(handle.G80_right_edit4,'string',[newmax{1} ' dB']);
        end
        if handle.G50_right_slider4.Value > str2double(newmax{1})
            set(handle.G50_right_slider4,'value',str2double(newmax{1}));
            set(handle.G50_right_edit4,'string',[newmax{1} ' dB']);
        end
        if handle.G80_right_slider5.Value > str2double(newmax{1})
            set(handle.G80_right_slider5,'value',str2double(newmax{1}));
            set(handle.G80_right_edit5,'string',[newmax{1} ' dB']);
        end
        if handle.G50_right_slider5.Value > str2double(newmax{1})
            set(handle.G50_right_slider5,'value',str2double(newmax{1}));
            set(handle.G50_right_edit5,'string',[newmax{1} ' dB']);
        end
        
        if handle.G80_all_slider_left.Value > str2double(newmax{1})
            set(handle.G80_all_slider_left.Value,'value',str2double(newmax{1}));
            set(handle.G80_all_edit_left.Value,'string',[newmax{1} ' dB']);
        end
        if handle.G50_all_slider_left.Value > str2double(newmax{1})
            set(handle.G50_all_slider_left.Value,'value',str2double(newmax{1}));
            set(handle.G50_all_edit_left.Value,'string',[newmax{1} ' dB']);
        end
        if handle.G80_left_slider1.Value > str2double(newmax{1})
            set(handle.G80_left_slider1,'value',str2double(newmax{1}));
            set(handle.G80_left_edit1,'string',[newmax{1} ' dB']);
        end
        if handle.G50_left_slider1.Value > str2double(newmax{1})
            set(handle.G50_left_slider1,'value',str2double(newmax{1}));
            set(handle.G50_left_edit1,'string',[newmax{1} ' dB']);
        end
        if handle.G80_left_slider2.Value > str2double(newmax{1})
            set(handle.G80_left_slider2,'value',str2double(newmax{1}));
            set(handle.G80_left_edit2,'string',[newmax{1} ' dB']);
        end
        if handle.G50_left_slider2.Value > str2double(newmax{1})
            set(handle.G50_left_slider2,'value',str2double(newmax{1}));
            set(handle.G50_left_edit2,'string',[newmax{1} ' dB']);
        end
        if handle.G80_left_slider3.Value > str2double(newmax{1})
            set(handle.G80_left_slider3,'value',str2double(newmax{1}));
            set(handle.G80_left_edit3,'string',[newmax{1} ' dB']);
        end
        if handle.G50_left_slider3.Value > str2double(newmax{1})
            set(handle.G50_left_slider3,'value',str2double(newmax{1}));
            set(handle.G50_left_edit3,'string',[newmax{1} ' dB']);
        end
        if handle.G80_left_slider4.Value > str2double(newmax{1})
            set(handle.G80_left_slider4,'value',str2double(newmax{1}));
            set(handle.G80_left_edit4,'string',[newmax{1} ' dB']);
        end
        if handle.G50_left_slider4.Value > str2double(newmax{1})
            set(handle.G50_left_slider4,'value',str2double(newmax{1}));
            set(handle.G50_left_edit4,'string',[newmax{1} ' dB']);
        end
        if handle.G80_left_slider5.Value > str2double(newmax{1})
            set(handle.G80_left_slider5,'value',str2double(newmax{1}));
            set(handle.G80_left_edit5,'string',[newmax{1} ' dB']);
        end
        if handle.G50_left_slider5.Value > str2double(newmax{1})
            set(handle.G50_left_slider5,'value',str2double(newmax{1}));
            set(handle.G50_left_edit5,'string',[newmax{1} ' dB']);
        end
        mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [handle.G50_left_slider1.Value handle.G50_left_slider2.Value handle.G50_left_slider3.Value handle.G50_left_slider4.Value handle.G50_left_slider5.Value handle.G50_right_slider1.Value handle.G50_right_slider2.Value handle.G50_right_slider3.Value handle.G50_right_slider4.Value handle.G50_right_slider5.Value ]);
        mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [handle.G80_left_slider1.Value handle.G80_left_slider2.Value handle.G80_left_slider3.Value handle.G80_left_slider4.Value handle.G80_left_slider5.Value handle.G80_right_slider1.Value handle.G80_right_slider2.Value handle.G80_right_slider3.Value handle.G80_right_slider4.Value handle.G80_right_slider5.Value ]);  
        else
            if str2double(newmax{1}) <= 0
                newmax{1}= '0';
                set(handle.maxgain_all_edit_right,'string',[num2str(newmax{1}) ' dB']);
                set(handle.G50_all_edit_right,'string',[num2str(newmax{1}) ' dB']);
                set(handle.G80_all_edit_right,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit1,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit2,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit3,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit4,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit5,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_all_edit_right,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit1,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit2,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit3,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit4,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit5,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_all_edit_right,'string',[newmax_str ' dB']);
                set(handle.maxgain_right_edit1,'string',[newmax_str ' dB']);
                set(handle.maxgain_right_edit2,'string',[newmax_str ' dB']);
                set(handle.maxgain_right_edit3,'string',[newmax_str ' dB']);
                set(handle.maxgain_right_edit4,'string',[newmax_str ' dB']);
                set(handle.maxgain_right_edit5,'string',[newmax_str ' dB']);
            end 
        mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
        [maxgain(1:5) repmat(str2double(newmax(1)),1,5)]);
        
        if handle.G80_all_slider_right.Value > str2double(newmax{1})
            set(handle.G80_all_slider_right.Value,'value',str2double(newmax{1}));
            set(handle.G80_all_edit_right.Value,'string',[newmax{1} ' dB']);
        end
        if handle.G50_all_slider_right.Value > str2double(newmax{1})
            set(handle.G50_all_slider_right.Value,'value',str2double(newmax{1}));
            set(handle.G50_all_edit_right.Value,'string',[newmax{1} ' dB']);
        end
        if handle.G80_right_slider1.Value > str2double(newmax{1})
            set(handle.G80_right_slider1,'value',str2double(newmax{1}));
            set(handle.G80_right_edit1,'string',[newmax{1} ' dB']);
        end
        if handle.G50_right_slider1.Value > str2double(newmax{1})
            set(handle.G50_right_slider1,'value',str2double(newmax{1}));
            set(handle.G50_right_edit1,'string',[newmax{1} ' dB']);
        end
        if handle.G80_right_slider2.Value > str2double(newmax{1})
            set(handle.G80_right_slider2,'value',str2double(newmax{1}));
            set(handle.G80_right_edit2,'string',[newmax{1} ' dB']);
        end
        if handle.G50_right_slider2.Value > str2double(newmax{1})
            set(handle.G50_right_slider2,'value',str2double(newmax{1}));
            set(handle.G50_right_edit2,'string',[newmax{1} ' dB']);
        end
        if handle.G80_right_slider3.Value > str2double(newmax{1})
            set(handle.G80_right_slider3,'value',str2double(newmax{1}));
            set(handle.G80_right_edit3,'string',[newmax{1} ' dB']);
        end
        if handle.G50_right_slider3.Value > str2double(newmax{1})
            set(handle.G50_right_slider3,'value',str2double(newmax{1}));
            set(handle.G50_right_edit3,'string',[newmax{1} ' dB']);
        end
        if handle.G80_right_slider4.Value > str2double(newmax{1})
            set(handle.G80_right_slider4,'value',str2double(newmax{1}));
            set(handle.G80_right_edit4,'string',[newmax{1} ' dB']);
        end
        if handle.G50_right_slider4.Value > str2double(newmax{1})
            set(handle.G50_right_slider4,'value',str2double(newmax{1}));
            set(handle.G50_right_edit4,'string',[newmax{1} ' dB']);
        end
        if handle.G80_right_slider5.Value > str2double(newmax{1})
            set(handle.G80_right_slider5,'value',str2double(newmax{1}));
            set(handle.G80_right_edit5,'string',[newmax{1} ' dB']);
        end
        if handle.G50_right_slider5.Value > str2double(newmax{1})
            set(handle.G50_right_slider5,'value',str2double(newmax{1}));
            set(handle.G50_right_edit5,'string',[newmax{1} ' dB']);
        end
        mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1:5) handle.G50_right_slider1.Value handle.G50_right_slider2.Value handle.G50_right_slider3.Value handle.G50_right_slider4.Value handle.G50_right_slider5.Value ]);
        mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1:5) handle.G80_right_slider1.Value handle.G80_right_slider2.Value handle.G80_right_slider3.Value handle.G80_right_slider4.Value handle.G80_right_slider5.Value ]);  
        end
 
    case 'all_left'
        newmax_str = get(handle.maxgain_all_edit_left,'String');
        newmax = strsplit(newmax_str,' ');
        if isequal(get(handle.link,'tag'),'link')
            if str2double(newmax{1}) <= 0
                newmax{1}= '0';
                set(handle.maxgain_all_edit_left,'string',[num2str(newmax{1}) ' dB']);
                set(handle.G50_all_edit_left,'string',[num2str(newmax{1}) ' dB']);
                set(handle.G80_all_edit_left,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit1,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit2,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit3,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit4,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit5,'string',[num2str(newmax{1}) ' dB']);

                set(handle.maxgain_all_edit_right,'string',[num2str(newmax{1}) ' dB']);
                set(handle.G50_all_edit_right,'string',[num2str(newmax{1}) ' dB']);
                set(handle.G80_all_edit_right,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit1,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit2,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit3,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit4,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit5,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_all_edit_left,'string',[num2str(newmax{1}) ' dB']);
                set(handle.G50_all_edit_left,'string',[num2str(newmax{1}) ' dB']);
                set(handle.G80_all_edit_left,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit1,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit2,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit3,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit4,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit5,'string',[num2str(newmax{1}) ' dB']);

                set(handle.maxgain_all_edit_right,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit1,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit2,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit3,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit4,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_right_edit5,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_all_edit_left,'string',[newmax_str ' dB']);
                set(handle.maxgain_left_edit1,'string',[newmax_str ' dB']);
                set(handle.maxgain_left_edit2,'string',[newmax_str ' dB']);
                set(handle.maxgain_left_edit3,'string',[newmax_str ' dB']);
                set(handle.maxgain_left_edit4,'string',[newmax_str ' dB']);
                set(handle.maxgain_left_edit5,'string',[newmax_str ' dB']);

                set(handle.maxgain_all_edit_right,'string',[newmax_str ' dB']);
                set(handle.maxgain_right_edit1,'string',[newmax_str ' dB']);
                set(handle.maxgain_right_edit2,'string',[newmax_str ' dB']);
                set(handle.maxgain_right_edit3,'string',[newmax_str ' dB']);
                set(handle.maxgain_right_edit4,'string',[newmax_str ' dB']);
                set(handle.maxgain_right_edit5,'string',[newmax_str ' dB']);
            end   
        
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
               repmat(str2double(newmax(1)),1,10));

            if handle.G80_all_slider_right.Value > str2double(newmax{1})
                set(handle.G80_all_slider_right.Value,'value',str2double(newmax{1}));
                set(handle.G80_all_edit_right.Value,'string',[newmax{1} ' dB']);
            end
            if handle.G50_all_slider_right.Value > str2double(newmax{1})
                set(handle.G50_all_slider_right.Value,'value',str2double(newmax{1}));
                set(handle.G50_all_edit_right.Value,'string',[newmax{1} ' dB']);
            end
            if handle.G80_right_slider1.Value > str2double(newmax{1})
                set(handle.G80_right_slider1,'value',str2double(newmax{1}));
                set(handle.G80_right_edit1,'string',[newmax{1} ' dB']);
            end
            if handle.G50_right_slider1.Value > str2double(newmax{1})
                set(handle.G50_right_slider1,'value',str2double(newmax{1}));
                set(handle.G50_right_edit1,'string',[newmax{1} ' dB']);
            end
            if handle.G80_right_slider2.Value > str2double(newmax{1})
                set(handle.G80_right_slider2,'value',str2double(newmax{1}));
                set(handle.G80_right_edit2,'string',[newmax{1} ' dB']);
            end
            if handle.G50_right_slider2.Value > str2double(newmax{1})
                set(handle.G50_right_slider2,'value',str2double(newmax{1}));
                set(handle.G50_right_edit2,'string',[newmax{1} ' dB']);
            end
            if handle.G80_right_slider3.Value > str2double(newmax{1})
                set(handle.G80_right_slider3,'value',str2double(newmax{1}));
                set(handle.G80_right_edit3,'string',[newmax{1} ' dB']);
            end
            if handle.G50_right_slider3.Value > str2double(newmax{1})
                set(handle.G50_right_slider3,'value',str2double(newmax{1}));
                set(handle.G50_right_edit3,'string',[newmax{1} ' dB']);
            end
            if handle.G80_right_slider4.Value > str2double(newmax{1})
                set(handle.G80_right_slider4,'value',str2double(newmax{1}));
                set(handle.G80_right_edit4,'string',[newmax{1} ' dB']);
            end
            if handle.G50_right_slider4.Value > str2double(newmax{1})
                set(handle.G50_right_slider4,'value',str2double(newmax{1}));
                set(handle.G50_right_edit4,'string',[newmax{1} ' dB']);
            end
            if handle.G80_right_slider5.Value > str2double(newmax{1})
                set(handle.G80_right_slider5,'value',str2double(newmax{1}));
                set(handle.G80_right_edit5,'string',[newmax{1} ' dB']);
            end
            if handle.G50_right_slider5.Value > str2double(newmax{1})
                set(handle.G50_right_slider5,'value',str2double(newmax{1}));
                set(handle.G50_right_edit5,'string',[newmax{1} ' dB']);
            end

            if handle.G80_all_slider_left.Value > str2double(newmax{1})
                set(handle.G80_all_slider_left.Value,'value',str2double(newmax{1}));
                set(handle.G80_all_edit_left.Value,'string',[newmax{1} ' dB']);
            end
            if handle.G50_all_slider_left.Value > str2double(newmax{1})
                set(handle.G50_all_slider_left.Value,'value',str2double(newmax{1}));
                set(handle.G50_all_edit_left.Value,'string',[newmax{1} ' dB']);
            end
            if handle.G80_left_slider1.Value > str2double(newmax{1})
                set(handle.G80_left_slider1,'value',str2double(newmax{1}));
                set(handle.G80_left_edit1,'string',[newmax{1} ' dB']);
            end
            if handle.G50_left_slider1.Value > str2double(newmax{1})
                set(handle.G50_left_slider1,'value',str2double(newmax{1}));
                set(handle.G50_left_edit1,'string',[newmax{1} ' dB']);
            end
            if handle.G80_left_slider2.Value > str2double(newmax{1})
                set(handle.G80_left_slider2,'value',str2double(newmax{1}));
                set(handle.G80_left_edit2,'string',[newmax{1} ' dB']);
            end
            if handle.G50_left_slider2.Value > str2double(newmax{1})
                set(handle.G50_left_slider2,'value',str2double(newmax{1}));
                set(handle.G50_left_edit2,'string',[newmax{1} ' dB']);
            end
            if handle.G80_left_slider3.Value > str2double(newmax{1})
                set(handle.G80_left_slider3,'value',str2double(newmax{1}));
                set(handle.G80_left_edit3,'string',[newmax{1} ' dB']);
            end
            if handle.G50_left_slider3.Value > str2double(newmax{1})
                set(handle.G50_left_slider3,'value',str2double(newmax{1}));
                set(handle.G50_left_edit3,'string',[newmax{1} ' dB']);
            end
            if handle.G80_left_slider4.Value > str2double(newmax{1})
                set(handle.G80_left_slider4,'value',str2double(newmax{1}));
                set(handle.G80_left_edit4,'string',[newmax{1} ' dB']);
            end
            if handle.G50_left_slider4.Value > str2double(newmax{1})
                set(handle.G50_left_slider4,'value',str2double(newmax{1}));
                set(handle.G50_left_edit4,'string',[newmax{1} ' dB']);
            end
            if handle.G80_left_slider5.Value > str2double(newmax{1})
                set(handle.G80_left_slider5,'value',str2double(newmax{1}));
                set(handle.G80_left_edit5,'string',[newmax{1} ' dB']);
            end
            if handle.G50_left_slider5.Value > str2double(newmax{1})
                set(handle.G50_left_slider5,'value',str2double(newmax{1}));
                set(handle.G50_left_edit5,'string',[newmax{1} ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [handle.G50_left_slider1.Value handle.G50_left_slider2.Value handle.G50_left_slider3.Value handle.G50_left_slider4.Value handle.G50_left_slider5.Value handle.G50_right_slider1.Value handle.G50_right_slider2.Value handle.G50_right_slider3.Value handle.G50_right_slider4.Value handle.G50_right_slider5.Value ]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [handle.G80_left_slider1.Value handle.G80_left_slider2.Value handle.G80_left_slider3.Value handle.G80_left_slider4.Value handle.G80_left_slider5.Value handle.G80_right_slider1.Value handle.G80_right_slider2.Value handle.G80_right_slider3.Value handle.G80_right_slider4.Value handle.G80_right_slider5.Value ]); 
        else
            if str2double(newmax{1}) <= 0
                newmax{1}= '0';
                set(handle.maxgain_all_edit_left,'string',[num2str(newmax{1}) ' dB']);
                set(handle.G50_all_edit_left,'string',[num2str(newmax{1}) ' dB']);
                set(handle.G80_all_edit_left,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit1,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit2,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit3,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit4,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit5,'string',[num2str(newmax{1}) ' dB']);
            elseif str2double(newmax{1}) > 80
                newmax{1}= '80';
                set(handle.maxgain_all_edit_left,'string',[num2str(newmax{1}) ' dB']);
                set(handle.G50_all_edit_left,'string',[num2str(newmax{1}) ' dB']);
                set(handle.G80_all_edit_left,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit1,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit2,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit3,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit4,'string',[num2str(newmax{1}) ' dB']);
                set(handle.maxgain_left_edit5,'string',[num2str(newmax{1}) ' dB']);
            else
                set(handle.maxgain_all_edit_left,'string',[newmax_str ' dB']);
                set(handle.maxgain_left_edit1,'string',[newmax_str ' dB']);
                set(handle.maxgain_left_edit2,'string',[newmax_str ' dB']);
                set(handle.maxgain_left_edit3,'string',[newmax_str ' dB']);
                set(handle.maxgain_left_edit4,'string',[newmax_str ' dB']);
                set(handle.maxgain_left_edit5,'string',[newmax_str ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain',...
                     [repmat(str2double(newmax(1)),1,5) maxgain(6:10)]);
            if handle.G80_all_slider_left.Value > str2double(newmax{1})
                set(handle.G80_all_slider_left.Value,'value',str2double(newmax{1}));
                set(handle.G80_all_edit_left.Value,'string',[newmax{1} ' dB']);
            end
            if handle.G50_all_slider_left.Value > str2double(newmax{1})
                set(handle.G50_all_slider_left.Value,'value',str2double(newmax{1}));
                set(handle.G50_all_edit_left.Value,'string',[newmax{1} ' dB']);
            end
            if handle.G80_all_slider_left.Value > str2double(newmax{1})
                set(handle.G80_all_slider_left.Value,'value',str2double(newmax{1}));
                set(handle.G80_all_edit_left.Value,'string',[newmax{1} ' dB']);
            end
            if handle.G50_all_slider_left.Value > str2double(newmax{1})
                set(handle.G50_all_slider_left.Value,'value',str2double(newmax{1}));
                set(handle.G50_all_edit_left.Value,'string',[newmax{1} ' dB']);
            end
            if handle.G80_left_slider1.Value > str2double(newmax{1})
                set(handle.G80_left_slider1,'value',str2double(newmax{1}));
                set(handle.G80_left_edit1,'string',[newmax{1} ' dB']);
            end
            if handle.G50_left_slider1.Value > str2double(newmax{1})
                set(handle.G50_left_slider1,'value',str2double(newmax{1}));
                set(handle.G50_left_edit1,'string',[newmax{1} ' dB']);
            end
            if handle.G80_left_slider2.Value > str2double(newmax{1})
                set(handle.G80_left_slider2,'value',str2double(newmax{1}));
                set(handle.G80_left_edit2,'string',[newmax{1} ' dB']);
            end
            if handle.G50_left_slider2.Value > str2double(newmax{1})
                set(handle.G50_left_slider2,'value',str2double(newmax{1}));
                set(handle.G50_left_edit2,'string',[newmax{1} ' dB']);
            end
            if handle.G80_left_slider3.Value > str2double(newmax{1})
                set(handle.G80_left_slider3,'value',str2double(newmax{1}));
                set(handle.G80_left_edit3,'string',[newmax{1} ' dB']);
            end
            if handle.G50_left_slider3.Value > str2double(newmax{1})
                set(handle.G50_left_slider3,'value',str2double(newmax{1}));
                set(handle.G50_left_edit3,'string',[newmax{1} ' dB']);
            end
            if handle.G80_left_slider4.Value > str2double(newmax{1})
                set(handle.G80_left_slider4,'value',str2double(newmax{1}));
                set(handle.G80_left_edit4,'string',[newmax{1} ' dB']);
            end
            if handle.G50_left_slider4.Value > str2double(newmax{1})
                set(handle.G50_left_slider4,'value',str2double(newmax{1}));
                set(handle.G50_left_edit4,'string',[newmax{1} ' dB']);
            end
            if handle.G80_left_slider5.Value > str2double(newmax{1})
                set(handle.G80_left_slider5,'value',str2double(newmax{1}));
                set(handle.G80_left_edit5,'string',[newmax{1} ' dB']);
            end
            if handle.G50_left_slider5.Value > str2double(newmax{1})
                set(handle.G50_left_slider5,'value',str2double(newmax{1}));
                set(handle.G50_left_edit5,'string',[newmax{1} ' dB']);
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                    [handle.G50_left_slider1.Value handle.G50_left_slider2.Value handle.G50_left_slider3.Value handle.G50_left_slider4.Value handle.G50_left_slider5.Value G50(6:10)]);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                    [handle.G80_left_slider1.Value handle.G80_left_slider2.Value handle.G80_left_slider3.Value handle.G80_left_slider4.Value handle.G80_left_slider5.Value G80(6:10)]);
        end
end
[handle] = maxGainCheck(handle);
handle = plot_data_right(handle);
handle = plot_data_left(handle);

G80 = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80');
G50 = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50');
g_p1 = [handle.all_gain_leftp11 handle.all_gain_leftp12 handle.all_gain_leftp13 handle.all_gain_leftp14 handle.all_gain_leftp15...
    handle.all_gain_rightp11 handle.all_gain_rightp12 handle.all_gain_rightp13 handle.all_gain_rightp14 handle.all_gain_rightp15];
g_p3 = [handle.all_gain_leftp31 handle.all_gain_leftp32 handle.all_gain_leftp33 handle.all_gain_leftp34 handle.all_gain_leftp35...
    handle.all_gain_rightp31 handle.all_gain_rightp32 handle.all_gain_rightp33 handle.all_gain_rightp34 handle.all_gain_rightp35];
g_m3 = [handle.all_gain_leftm31 handle.all_gain_leftm32 handle.all_gain_leftm33 handle.all_gain_leftm34 handle.all_gain_leftm35...
    handle.all_gain_rightm31 handle.all_gain_rightm32 handle.all_gain_rightm33 handle.all_gain_rightm34 handle.all_gain_rightm35];
g_m1 = [handle.all_gain_leftm11 handle.all_gain_leftm12 handle.all_gain_leftm13 handle.all_gain_leftm14 handle.all_gain_leftm15...
    handle.all_gain_rightm11 handle.all_gain_rightm12 handle.all_gain_rightm13 handle.all_gain_rightm14 handle.all_gain_rightm15];
max_g50 = [handle.G50_left_slider1.Max handle.G50_left_slider2.Max handle.G50_left_slider3.Max handle.G50_left_slider4.Max handle.G50_left_slider5.Max...
    handle.G50_right_slider1.Max handle.G50_right_slider2.Max handle.G50_right_slider3.Max handle.G50_right_slider4.Max handle.G50_right_slider5.Max];
max_g80 = [handle.G80_left_slider1.Max handle.G80_left_slider2.Max handle.G80_left_slider3.Max handle.G80_left_slider4.Max handle.G80_left_slider5.Max...
    handle.G80_right_slider1.Max handle.G80_right_slider2.Max handle.G80_right_slider3.Max handle.G80_right_slider4.Max handle.G80_right_slider5.Max];
gall_p1 = zeros(1,10);
gall_p3 = zeros(1,10);
gall_m3 = zeros(1,10);
gall_m1 = zeros(1,10);
for cc = 1:10
    if G50(cc)+3 > max_g50(cc) && G80(cc)+3 > max_g80(cc)
        set(g_p3(cc),'enable','off');
        gall_p3(cc) = 0;
        if G50(cc)+1 > max_g50(cc) && G80(cc)+1 > max_g80(cc)
            set(g_p1(cc),'enable','off');
            gall_p1(cc) = 0;
        else
            set(g_p1(cc),'enable','on');
            gall_p1(cc) = 1;
        end      
    else 
        set([g_p1(cc) g_p3(cc)],'enable','on');
        gall_p1(cc) = 1;
        gall_p3(cc) = 1;
    end
    if G50(cc)-3 <= 0 && G80(cc)-3 <= 0
        set(g_m3(cc),'enable','off');
        gall_m3(cc) = 0;
        if G50(cc)-1 <= 0 && G80(cc)-1 <= 0
            set(g_m1(cc),'enable','off');
            gall_m1(cc) = 0;
        end            
    else 
        set([g_m3(cc) g_m1(cc)],'enable','on');
        gall_m3(cc) = 1;
        gall_m1(cc) = 1;
    end
end
if gall_p1(1:5) == zeros(1,5)
    set(handle.all_bb_leftp1,'enable','off');
else
    set(handle.all_bb_leftp1,'enable','on');
end
if gall_p3(1:5) == zeros(1,5)
    set(handle.all_bb_leftp3,'enable','off');
else
    set(handle.all_bb_leftp3,'enable','on');
end
if gall_m3(1:5) == zeros(1,5)
    set(handle.all_bb_leftm3,'enable','off');
else
    set(handle.all_bb_leftm3,'enable','on');
end
if gall_m1(1:5) == zeros(1,5)
    set(handle.all_bb_leftm1,'enable','off');
else
    set(handle.all_bb_leftm1,'enable','on');
end

if gall_p1(6:10) == zeros(1,5)
    set(handle.all_bb_rightp1,'enable','off');
else
    set(handle.all_bb_rightp1,'enable','on');
end
if gall_p3(6:10) == zeros(1,5)
    set(handle.all_bb_rightp3,'enable','off');
else
    set(handle.all_bb_rightp3,'enable','on');
end
if gall_m3(6:10) == zeros(1,5)
    set(handle.all_bb_rightm3,'enable','off');
else
    set(handle.all_bb_rightm3,'enable','on');
end
if gall_m1(6:10) == zeros(1,5)
    set(handle.all_bb_rightm1,'enable','off');
else
    set(handle.all_bb_rightm1,'enable','on');
end
dat.g50.old = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50');
dat.g80.old = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80');

handle.dat = dat;
guidata(src,handle);
end
