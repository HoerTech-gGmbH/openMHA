function [handle,dat] = G80_edit_cllb(src,event)
handle = guidata(src);
dat = handle.dat;
mha = handle.mha;

G80 = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80');
G50 = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50');
edit_tag = get(gcbo,'tag');
switch edit_tag
    case 'edit_l_250Hz'
        G50_str = get(handle.G50_left_edit1,'string');
        new_G50 = strsplit(G50_str,' ');
        G80_str = get(handle.G80_left_edit1,'string');
        set(handle.G80_left_edit1,'string',[G80_str ' dB']);
        new_G80 = strsplit(G80_str,' ');
        if str2double(new_G80(1)) < get(handle.G80_left_slider1,'min')
            new_G80{1} = num2str(get(handle.G80_left_slider1,'min'));
            set(handle.G80_left_edit1,'string',[num2str(get(handle.G80_left_slider1,'min')) ' dB']);
        
        elseif str2double(new_G80(1)) > get(handle.G80_left_slider1,'max')
            new_G80{1} = num2str(get(handle.G80_left_slider1,'max'));
            set(handle.G80_left_edit1,'string',[num2str(get(handle.G80_left_slider1,'max')) ' dB']);
        end
        val = str2double(new_G80(1));
        delta = val - dat.g80.old(1);
        if isequal(get(handle.link,'tag'),'link')
            if (dat.g80.old(6) + delta) > handle.G80_right_slider1.Max
                val2 = handle.G80_right_slider1.Max;
            elseif (dat.g80.old(6) + delta) < 0
                val2 = 0;
            else
                val2 = dat.g80.old(6) + delta;
            end
            set(handle.G80_right_slider1,'value',val2);
            set(handle.G80_right_edit1,'string',[num2str(val2) ' dB']);
            dat.g80.old(6) = val2;
            set(handle.right_CR1,'string',num2str(round((80-50)/(val2+80-(G50(6)+50)),2)));
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [val G80(2) G80(3) G80(4) G80(5) val2 G80(7) G80(8) G80(9) G80(10)]);
        else
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [val G80(2) G80(3) G80(4) G80(5) G80(6) G80(7) G80(8) G80(9) G80(10)]);
        end
    
        if str2double(new_G50(1)) == handle.G50_left_slider1.Min && str2double(new_G80(1)) == handle.G80_left_slider1.Min
            set([handle.all_gain_leftm11 handle.all_gain_leftm31],'enable','off');
        elseif str2double(new_G50(1)) == handle.G50_left_slider1.Max && str2double(new_G80(1)) == handle.G80_left_slider1.Max
            set([handle.all_gain_leftp31 handle.all_gain_leftp11],'enable','off');
            set([handle.all_gain_leftm11 handle.all_gain_leftm31],'enable','on');
        else
            set([handle.all_gain_leftm11 handle.all_gain_leftm31 handle.all_gain_leftp31 handle.all_gain_leftp11,... 
                handle.all_bb_leftm1 handle.all_bb_leftm3 handle.all_bb_leftp1 handle.all_bb_leftp3...
                handle.all_gain_leftp11 handle.all_gain_leftp31 handle.all_gain_leftm31 handle.all_gain_leftm11],'enable','on');
        end
        set(handle.G80_left_slider1,'value',str2double(new_G80(1)));
        set(handle.left_CR1,'string',num2str(round((80-50)/(val+80-(G50(1)+50)),2)));
        dat.g80.old(1) = val;
        dat.g1.max_left = max([handle.G80_left_slider1.Max-handle.G80_left_slider1.Value; handle.G50_left_slider1.Max-handle.G50_left_slider1.Value ]);
        dat.g1.min_left = -max([handle.G80_left_slider1.Value-handle.G80_left_slider1.Min; handle.G50_left_slider1.Value-handle.G50_left_slider1.Min]);
        
    case 'edit_l_500Hz'
        G80_str = get(handle.G80_left_edit2,'string');
        set(handle.G80_left_edit2,'string',[G80_str ' dB']);
        new_G80 = strsplit(G80_str,' ');
        G50_str = get(handle.G50_left_edit2,'string');
        new_G50 = strsplit(G50_str,' ');
        if str2double(new_G80(1)) < get(handle.G80_left_slider2,'min')
            new_G80{1} = num2str(get(handle.G80_left_slider2,'min'));
            set(handle.G80_left_edit2,'string',[num2str(get(handle.G80_left_slider2,'min')) ' dB']);
        
        elseif str2double(new_G80(1)) > get(handle.G80_left_slider2,'max')
            new_G80{1} = num2str(get(handle.G80_left_slider1,'max'));
            set(handle.G80_left_edit2,'string',[num2str(get(handle.G80_left_slider2,'max')) ' dB']);
        end
        val = str2double(new_G80(1));
        delta = val - dat.g80.old(2);
        if isequal(get(handle.link,'tag'),'link')
            if (dat.g80.old(7) + delta) > handle.G80_right_slider2.Max
                val2 = handle.G80_right_slider2.Max;
            elseif (dat.g80.old(7) + delta) < 0
                val2 = 0;
            else
                val2 = dat.g80.old(7) + delta;
            end
            set(handle.G80_right_slider2,'value',val2);
            set(handle.G80_right_edit2,'string',[num2str(val2) ' dB']);
            dat.g80.old(7) = val2;
            set(handle.right_CR2,'string',num2str(round((80-50)/(val2+80-(G50(7)+50)),2)));
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1) val G80(3) G80(4) G80(5) val2 G80(7) G80(8) G80(9) G80(10)]);
        else
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1) val G80(3) G80(4) G80(5) G80(6) G80(7) G80(8) G80(9) G80(10)]);
        end
        
        if str2double(new_G50(1)) == handle.G50_left_slider2.Min && str2double(new_G80(1)) == handle.G80_left_slider2.Min
            set([handle.all_gain_leftm12 handle.all_gain_leftm32],'enable','off');
        elseif str2double(new_G50(1)) == handle.G50_left_slider2.Max && str2double(new_G80(1)) == handle.G80_left_slider2.Max
            set([handle.all_gain_leftp32 handle.all_gain_leftp12],'enable','off');
            set([handle.all_gain_leftm12 handle.all_gain_leftm32],'enable','on');
        else
            set([handle.all_gain_leftm12 handle.all_gain_leftm32 handle.all_gain_leftp32 handle.all_gain_leftp12,... 
                handle.all_bb_leftm1 handle.all_bb_leftm3 handle.all_bb_leftp1 handle.all_bb_leftp3...
                handle.all_gain_leftp12 handle.all_gain_leftp32 handle.all_gain_leftm32 handle.all_gain_leftm12],'enable','on');
        end
        
        set(handle.G80_left_slider2,'value',str2double(new_G80(1)));
        set(handle.left_CR2,'string',num2str(round((80-50)/(val+80-(G50(2)+50)),2)));
        dat.g80.old(2) = val;
        dat.g2.max_left = max([handle.G80_left_slider2.Max-handle.G80_left_slider2.Value; handle.G50_left_slider2.Max-handle.G50_left_slider2.Value ]);
        dat.g2.min_left = -max([handle.G80_left_slider2.Value-handle.G80_left_slider2.Min; handle.G50_left_slider2.Value-handle.G50_left_slider2.Min]);
    case 'edit_l_1kHz'
        G80_str = get(handle.G80_left_edit3,'string');
        set(handle.G80_left_edit3,'string',[G80_str ' dB']);
        new_G80 = strsplit(G80_str,' ');
        G50_str = get(handle.G50_left_edit3,'string');
        new_G50 = strsplit(G50_str,' ');
        if str2double(new_G80(1)) < get(handle.G80_left_slider3,'min')
            new_G80{1} = num2str(get(handle.G80_left_slider3,'min'));
            set(handle.G80_left_edit3,'string',[num2str(get(handle.G80_left_slider3,'min')) ' dB']);
        
        elseif str2double(new_G80(1)) > get(handle.G80_left_slider3,'max')
            new_G80{1} = num2str(get(handle.G80_left_slider3,'max'));
            set(handle.G80_left_edit3,'string',[num2str(get(handle.G80_left_slider3,'max')) ' dB']);
        end
        val = str2double(new_G80(1));
        delta = val - dat.g80.old(3);
        if isequal(get(handle.link,'tag'),'link')
            if (dat.g80.old(8) + delta) > handle.G80_right_slider3.Max
                val2 = handle.G80_right_slider3.Max;
            elseif (dat.g80.old(8) + delta) < 0
                val2 = 0;
            else
                val2 = dat.g80.old(8) + delta;
            end
            set(handle.G80_right_slider3,'value',val2);
            set(handle.G80_right_edit3,'string',[num2str(val2) ' dB']);
            dat.g80.old(8) = val2;
            set(handle.right_CR3,'string',num2str(round((80-50)/(val2+80-(G50(8)+50)),2)));
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1) G80(2) val G80(4) G80(5) G80(6) G80(7) val2 G80(9) G80(10)]);
        else
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1) G80(2) val G80(4) G80(5) G80(6) G80(7) G80(8) G80(9) G80(10)]);
        end
        
        if str2double(new_G50(1)) == handle.G50_left_slider3.Min && str2double(new_G80(1)) == handle.G80_left_slider3.Min
            set([handle.all_gain_leftm13 handle.all_gain_leftm33],'enable','off');
        elseif str2double(new_G50(1)) == handle.G50_left_slider3.Max && str2double(new_G80(1)) == handle.G80_left_slider3.Max
            set([handle.all_gain_leftp33 handle.all_gain_leftp13],'enable','off');
            set([handle.all_gain_leftm13 handle.all_gain_leftm33],'enable','on');
        else
            set([handle.all_gain_leftm13 handle.all_gain_leftm33 handle.all_gain_leftp33 handle.all_gain_leftp13,... 
                handle.all_bb_leftm1 handle.all_bb_leftm3 handle.all_bb_leftp1 handle.all_bb_leftp3...
                handle.all_gain_leftp13 handle.all_gain_leftp33 handle.all_gain_leftm33 handle.all_gain_leftm13],'enable','on');
        end
        
        set(handle.G80_left_slider3,'value',str2double(new_G80(1)));
        set(handle.left_CR3,'string',num2str(round((80-50)/(val+80-(G50(3)+50)),2)));
        dat.g80.old(3) = val;
        dat.g3.max_left = max([handle.G80_left_slider3.Max-handle.G80_left_slider3.Value; handle.G50_left_slider3.Max-handle.G50_left_slider3.Value ]);
        dat.g3.min_left = -max([handle.G80_left_slider3.Value-handle.G80_left_slider3.Min; handle.G50_left_slider3.Value-handle.G50_left_slider3.Min]);
    case 'edit_l_2kHz'
        G80_str = get(handle.G80_left_edit4,'string');
        set(handle.G80_left_edit4,'string',[G80_str ' dB']);
        new_G80 = strsplit(G80_str,' ');
        G50_str = get(handle.G50_left_edit4,'string');
        new_G50 = strsplit(G50_str,' ');
        if str2double(new_G80(1)) < get(handle.G80_left_slider4,'min')
            new_G80{1} = num2str(get(handle.G80_left_slider4,'min'));
            set(handle.G80_left_edit4,'string',[num2str(get(handle.G80_left_slider4,'min')) ' dB']);
        
        elseif str2double(new_G80(1)) > get(handle.G80_left_slider4,'max')
            new_G80{1} = num2str(get(handle.G80_left_slider1,'max'));
            set(handle.G80_left_edit4,'string',[num2str(get(handle.G80_left_slider4,'max')) ' dB']);
        end
        val = str2double(new_G80(1));
        delta = val - dat.g80.old(4);
        if isequal(get(handle.link,'tag'),'link')
            if (dat.g80.old(9) + delta) > handle.G80_right_slider4.Max
                val2 = handle.G80_right_slider4.Max;
            elseif (dat.g80.old(9) + delta) < 0
                val2 = 0;
            else
                val2 = dat.g80.old(9) + delta;
            end
            set(handle.G80_right_slider4,'value',val2);
            set(handle.G80_right_edit4,'string',[num2str(val2) ' dB']);
            dat.g80.old(9) = val2;
            set(handle.right_CR4,'string',num2str(round((80-50)/(val2+80-(G50(9)+50)),2)));
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1) G80(2) G80(3) val G80(5) G80(6) G80(7) G80(8) val2 G80(10)]);
        else
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1) G80(2) G80(3) val G80(5) G80(6) G80(7) G80(8) G80(9) G80(10)]);
        end
        
        if str2double(new_G50(1)) == handle.G50_left_slider4.Min && str2double(new_G80(1)) == handle.G80_left_slider4.Min
            set([handle.all_gain_leftm14 handle.all_gain_leftm34],'enable','off');
        elseif str2double(new_G50(1)) == handle.G50_left_slider4.Max && str2double(new_G80(1)) == handle.G80_left_slider4.Max
            set([handle.all_gain_leftp34 handle.all_gain_leftp14],'enable','off');
            set([handle.all_gain_leftm14 handle.all_gain_leftm34],'enable','on');
        else
            set([handle.all_gain_leftm14 handle.all_gain_leftm34 handle.all_gain_leftp34 handle.all_gain_leftp14,... 
                handle.all_bb_leftm1 handle.all_bb_leftm3 handle.all_bb_leftp1 handle.all_bb_leftp3...
                handle.all_gain_leftp14 handle.all_gain_leftp34 handle.all_gain_leftm34 handle.all_gain_leftm14],'enable','on');
        end
 
        set(handle.G80_left_slider4,'value',str2double(new_G80(1)));
        set(handle.left_CR4,'string',num2str(round((80-50)/(val+80-(G50(4)+50)),2)));
        dat.g80.old(4) = val;
        dat.g4.max_left = max([handle.G80_left_slider4.Max-handle.G80_left_slider4.Value; handle.G50_left_slider4.Max-handle.G50_left_slider4.Value ]);
        dat.g4.min_left = -max([handle.G80_left_slider4.Value-handle.G80_left_slider4.Min; handle.G50_left_slider4.Value-handle.G50_left_slider4.Min]);
    case 'edit_l_4kHz'
        G80_str = get(handle.G80_left_edit5,'string');
        set(handle.G80_left_edit5,'string',[G80_str ' dB']);
        new_G80 = strsplit(G80_str,' ');
        G50_str = get(handle.G50_left_edit5,'string');
        new_G50 = strsplit(G50_str,' ');
        if str2double(new_G80(1)) < get(handle.G80_left_slider5,'min')
            new_G80{1} = num2str(get(handle.G80_left_slider5,'min'));
            set(handle.G80_left_edit5,'string',[num2str(get(handle.G80_left_slider5,'min')) ' dB']);
        
        elseif str2double(new_G80(1)) > get(handle.G80_left_slider5,'max')
            new_G80{1} = num2str(get(handle.G80_left_slider5,'max'));
            set(handle.G80_left_edit5,'string',[num2str(get(handle.G80_left_slider5,'max')) ' dB']);
        end
        val = str2double(new_G80(1));
        delta = val - dat.g80.old(5);
        if isequal(get(handle.link,'tag'),'link')
            if (dat.g80.old(10) + delta) > handle.G80_right_slider5.Max
                val2 = handle.G80_right_slider5.Max;
            elseif (dat.g80.old(10) + delta) < 0
                val2 = 0;
            else
                val2 = dat.g80.old(10) + delta;
            end
            set(handle.G80_right_slider5,'value',val2);
            set(handle.G80_right_edit5,'string',[num2str(val2) ' dB']);
            dat.g80.old(10) = val2;
            set(handle.right_CR5,'string',num2str(round((80-50)/(val2+80-(G50(10)+50)),2)));
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1) G80(2) G80(3) G80(4) val G80(6) G80(7) G80(8) val2 G80(10)]);
        else
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1) G80(2) G80(3) G80(4) val G80(6) G80(7) G80(8) G80(9) G80(10)]);
        end
        
        if str2double(new_G50(1)) == handle.G50_left_slider5.Min && str2double(new_G80(1)) == handle.G80_left_slider5.Min
            set([handle.all_gain_leftm15 handle.all_gain_leftm35],'enable','off');
        elseif str2double(new_G50(1)) == handle.G50_left_slider5.Max && str2double(new_G80(1)) == handle.G80_left_slider5.Max
            set([handle.all_gain_leftp35 handle.all_gain_leftp15],'enable','off');
            set([handle.all_gain_leftm15 handle.all_gain_leftm35],'enable','on');
        else
            set([handle.all_gain_leftm15 handle.all_gain_leftm35 handle.all_gain_leftp35 handle.all_gain_leftp15,... 
                handle.all_bb_leftm1 handle.all_bb_leftm3 handle.all_bb_leftp1 handle.all_bb_leftp3...
                handle.all_gain_leftp15 handle.all_gain_leftp35 handle.all_gain_leftm35 handle.all_gain_leftm15],'enable','on');
        end
       
        set(handle.G80_left_slider5,'value',str2double(new_G80(1)));
        set(handle.left_CR5,'string',num2str(round((80-50)/(val+80-(G50(5)+50)),2)));
        dat.g80.old(5) = val;
        dat.g5.max_left = max([handle.G80_left_slider5.Max-handle.G80_left_slider5.Value; handle.G50_left_slider5.Max-handle.G50_left_slider5.Value ]);
        dat.g5.min_left = -max([handle.G80_left_slider5.Value-handle.G80_left_slider5.Min; handle.G50_left_slider5.Value-handle.G50_left_slider5.Min]);
    case 'edit_r_250Hz'
        G80_str = get(handle.G80_right_edit1,'string');
        set(handle.G80_right_edit1,'string',[G80_str ' dB']);
        new_G80 = strsplit(G80_str,' ');
        G50_str = get(handle.G50_right_edit1,'string');
        new_G50 = strsplit(G50_str,' ');
        if str2double(new_G80(1)) < get(handle.G80_right_slider1,'min')
            new_G80{1} = num2str(get(handle.G80_right_slider1,'min'));
            set(handle.G80_right_edit1,'string',[num2str(get(handle.G80_right_slider1,'min')) ' dB']);
        
        elseif str2double(new_G80(1)) > get(handle.G80_right_slider1,'max')
            new_G80{1} = num2str(get(handle.G80_right_slider1,'max'));
            set(handle.G80_right_edit1,'string',[num2str(get(handle.G80_right_slider1,'max')) ' dB']);
        end
        val = str2double(new_G80(1));
        delta = val - dat.g80.old(6);
        if isequal(get(handle.link,'tag'),'link')
            if (dat.g80.old(1) + delta) > handle.G80_left_slider1.Max
                val2 = handle.G80_left_slider1.Max;
            elseif (dat.g80.old(1) + delta) < 0
                val2 = 0;
            else
                val2 = dat.g80.old(1) + delta;
            end
            set(handle.G80_left_slider1,'value',val2);
            set(handle.G80_left_edit1,'string',[num2str(val2) ' dB']);
            dat.g80.old(1) = val2;
            set(handle.left_CR1,'string',num2str(round((80-50)/(val2+80-(G50(1)+50)),2)));
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [val2 G80(2) G80(3) G80(4) G80(5) val G80(7) G80(8) G80(9) G80(10)]);
        else
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1) G80(2) G80(3) G80(4) G80(5) val G80(7) G80(8) G80(9) G80(10)]);
        end
        
        if str2double(new_G50(1)) == handle.G50_right_slider1.Min && str2double(new_G80(1)) == handle.G80_right_slider1.Min
            set([handle.all_gain_rightm11 handle.all_gain_rightm31],'enable','off');
        elseif str2double(new_G50(1)) == handle.G50_right_slider1.Max && str2double(new_G80(1)) == handle.G80_right_slider1.Max
            set([handle.all_gain_rightp31 handle.all_gain_rightp11],'enable','off');
            set([handle.all_gain_rightm11 handle.all_gain_rightm31],'enable','on');
        else
            set([handle.all_gain_rightm11 handle.all_gain_rightm31 handle.all_gain_rightp31 handle.all_gain_rightp11,... 
                handle.all_bb_rightm1 handle.all_bb_rightm3 handle.all_bb_rightp1 handle.all_bb_rightp3...
                handle.all_gain_rightp11 handle.all_gain_rightp31 handle.all_gain_rightm31 handle.all_gain_rightm11],'enable','on');
        end
        
        set(handle.G80_right_slider1,'value',str2double(new_G80(1)));
        set(handle.right_CR1,'string',num2str(round((80-50)/(val+80-(G50(6)+50)),2)));
        dat.g80.old(6) = val;
        dat.g1.max_right = max([handle.G80_right_slider1.Max-handle.G80_right_slider1.Value; handle.G50_right_slider1.Max-handle.G50_right_slider1.Value ]);
        dat.g1.min_right = -max([handle.G80_right_slider1.Value-handle.G80_right_slider1.Min; handle.G50_right_slider1.Value-handle.G50_right_slider1.Min]);
    case 'edit_r_500Hz'
        G80_str = get(handle.G80_right_edit2,'string');
        set(handle.G80_right_edit2,'string',[G80_str ' dB']);
        new_G80 = strsplit(G80_str,' ');
        G50_str = get(handle.G50_right_edit2,'string');
        new_G50 = strsplit(G50_str,' ');
        if str2double(new_G80(1)) < get(handle.G80_right_slider2,'min')
            new_G80{1} = num2str(get(handle.G80_right_slider2,'min'));
            set(handle.G80_right_edit2,'string',[num2str(get(handle.G80_right_slider2,'min')) ' dB']);
        
        elseif str2double(new_G80(1)) > get(handle.G80_right_slider2,'max')
            new_G80{1} = num2str(get(handle.G80_right_slider2,'max'));
            set(handle.G80_right_edit2,'string',[num2str(get(handle.G80_right_slider2,'max')) ' dB']);
        end
        val = str2double(new_G80(1));
        delta = val - dat.g80.old(7);
        if isequal(get(handle.link,'tag'),'link')
            if (dat.g80.old(2) + delta) > handle.G80_left_slider2.Max
                val2 = handle.G80_left_slider2.Max;
            elseif (dat.g80.old(2) + delta) < 0
                val2 = 0;
            else
                val2 = dat.g80.old(2) + delta;
            end
            set(handle.G80_left_slider2,'value',val2);
            set(handle.G80_left_edit2,'string',[num2str(val2) ' dB']);
            dat.g80.old(2) = val2;
            set(handle.left_CR2,'string',num2str(round((80-50)/(val2+80-(G50(2)+50)),2)));
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1) val2 G80(3) G80(4) G80(5) G80(6) val G80(8) G80(9) G80(10)]);
        else
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1) G80(2) G80(3) G80(4) G80(5) G80(6) val G80(8) G80(9) G80(10)]);
        end
        
        if str2double(new_G50(1)) == handle.G50_right_slider1.Min && str2double(new_G80(1)) == handle.G80_right_slider1.Min
            set([handle.all_gain_rightm12 handle.all_gain_rightm32],'enable','off');
        elseif str2double(new_G50(1)) == handle.G50_right_slider1.Max && str2double(new_G80(1)) == handle.G80_right_slider1.Max
            set([handle.all_gain_rightp32 handle.all_gain_rightp12],'enable','off');
            set([handle.all_gain_rightm12 handle.all_gain_rightm32],'enable','on');
        else
            set([handle.all_gain_rightm12 handle.all_gain_rightm32 handle.all_gain_rightp32 handle.all_gain_rightp12,... 
                handle.all_bb_rightm1 handle.all_bb_rightm3 handle.all_bb_rightp1 handle.all_bb_rightp3...
                handle.all_gain_rightp12 handle.all_gain_rightp32 handle.all_gain_rightm32 handle.all_gain_rightm12],'enable','on');
        end

        set(handle.G80_right_slider2,'value',str2double(new_G80(1)));
        set(handle.right_CR2,'string',num2str(round((80-50)/(val+80-(G50(7)+50)),2)));
        dat.g80.old(7) = val;        
        dat.g2.max_right = max([handle.G80_right_slider2.Max-handle.G80_right_slider2.Value; handle.G50_right_slider2.Max-handle.G50_right_slider2.Value ]);
        dat.g2.min_right = -max([handle.G80_right_slider2.Value-handle.G80_right_slider2.Min; handle.G50_right_slider2.Value-handle.G50_right_slider2.Min]);
    case 'edit_r_1kHz'
        G80_str = get(handle.G80_right_edit3,'string');
        set(handle.G80_right_edit3,'string',[G80_str ' dB']);
        new_G80 = strsplit(G80_str,' ');
        G50_str = get(handle.G50_right_edit3,'string');
        new_G50 = strsplit(G50_str,' ');
        if str2double(new_G80(1)) < get(handle.G80_right_slider3,'min')
            new_G80{1} = num2str(get(handle.G80_right_slider3,'min'));
            set(handle.G80_right_edit3,'string',[num2str(get(handle.G80_right_slider3,'min')) ' dB']);
        
        elseif str2double(new_G80(1)) > get(handle.G80_right_slider3,'max')
            new_G80{1} = num2str(get(handle.G80_right_slider3,'max'));
            set(handle.G80_right_edit3,'string',[num2str(get(handle.G80_right_slider3,'max')) ' dB']);
        end
        val = str2double(new_G80(1));
        delta = val - dat.g80.old(8);
        if isequal(get(handle.link,'tag'),'link')
            if (dat.g80.old(3) + delta) > handle.G80_left_slider3.Max
                val2 = handle.G80_left_slider3.Max;
            elseif (dat.g80.old(3) + delta) < 0
                val2 = 0;
            else
                val2 = dat.g80.old(3) + delta;
            end
            set(handle.G80_left_slider3,'value',val2);
            set(handle.G80_left_edit3,'string',[num2str(val2) ' dB']);
            dat.g80.old(3) = val2;
            set(handle.left_CR3,'string',num2str(round((80-50)/(val2+80-(G50(3)+50)),2)));
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1) G80(2) val2 G80(4) G80(5) G80(6) G80(7) val G80(9) G80(10)]);
        else
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1) G80(2) G80(3) G80(4) G80(5) G80(6) G80(7) val G80(9) G80(10)]);
        end
        
        if str2double(new_G50(1)) == handle.G50_right_slider3.Min && str2double(new_G80(1)) == handle.G80_right_slider3.Min
            set([handle.all_gain_rightm13 handle.all_gain_rightm33],'enable','off');
        elseif str2double(new_G50(1)) == handle.G50_right_slider3.Max && str2double(new_G80(1)) == handle.G80_right_slider3.Max
            set([handle.all_gain_rightp33 handle.all_gain_rightp13],'enable','off');
            set([handle.all_gain_rightm13 handle.all_gain_rightm33],'enable','on');
        else
            set([handle.all_gain_rightm13 handle.all_gain_rightm33 handle.all_gain_rightp33 handle.all_gain_rightp13,... 
                handle.all_bb_rightm1 handle.all_bb_rightm3 handle.all_bb_rightp1 handle.all_bb_rightp3...
                handle.all_gain_rightp13 handle.all_gain_rightp33 handle.all_gain_rightm33 handle.all_gain_rightm13],'enable','on');
        end

        set(handle.G80_right_slider3,'value',str2double(new_G80(1)));
        set(handle.right_CR3,'string',num2str(round((80-50)/(val+80-(G50(8)+50)),2)));
        dat.g80.old(8) = val;        
        dat.g3.max_right = max([handle.G80_right_slider3.Max-handle.G80_right_slider3.Value; handle.G50_right_slider3.Max-handle.G50_right_slider3.Value ]);
        dat.g3.min_right = -max([handle.G80_right_slider3.Value-handle.G80_right_slider3.Min; handle.G50_right_slider3.Value-handle.G50_right_slider3.Min]);
    case 'edit_r_2kHz'
        G80_str = get(handle.G80_right_edit4,'string');
        set(handle.G80_right_edit4,'string',[G80_str ' dB']);
        new_G80 = strsplit(G80_str,' ');
        G50_str = get(handle.G50_right_edit4,'string');
        new_G50 = strsplit(G50_str,' ');
        if str2double(new_G80(1)) < get(handle.G80_right_slider4,'min')
            new_G80{1} = num2str(get(handle.G80_right_slider4,'min'));
            set(handle.G80_right_edit4,'string',[num2str(get(handle.G80_right_slider4,'min')) ' dB']);
        
        elseif str2double(new_G80(1)) > get(handle.G80_right_slider4,'max')
            new_G80{1} = num2str(get(handle.G80_right_slider4,'max'));
            set(handle.G80_right_edit4,'string',[num2str(get(handle.G80_right_slider4,'max')) ' dB']);
        end
        val = str2double(new_G80(1));
        delta = val - dat.g80.old(9);
        if isequal(get(handle.link,'tag'),'link')
            if (dat.g80.old(4) + delta) > handle.G80_left_slider4.Max
                val2 = handle.G80_left_slider4.Max;
            elseif (dat.g80.old(4) + delta) < 0
                val2 = 0;
            else
                val2 = dat.g80.old(4) + delta;
            end
            set(handle.G80_left_slider4,'value',val2);
            set(handle.G80_left_edit4,'string',[num2str(val2) ' dB']);
            dat.g80.old(4) = val2;
            set(handle.left_CR4,'string',num2str(round((80-50)/(val2+80-(G50(4)+50)),2)));
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1) G80(2) G80(3) val2 G80(5) G80(6) G80(7) G80(8) val G80(10)]);
        else
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1) G80(2) G80(3) G80(4) G80(5) G80(6) G80(7) G80(8) val G80(10)]);
        end
        
        if str2double(new_G50(1)) == handle.G50_right_slider4.Min && str2double(new_G80(1)) == handle.G80_right_slider4.Min
            set([handle.all_gain_rightm14 handle.all_gain_rightm34],'enable','off');
        elseif str2double(new_G50(1)) == handle.G50_right_slider4.Max && str2double(new_G80(1)) == handle.G80_right_slider4.Max
            set([handle.all_gain_rightp34 handle.all_gain_rightp14],'enable','off');
            set([handle.all_gain_rightm14 handle.all_gain_rightm34],'enable','on');
        else
            set([handle.all_gain_rightm14 handle.all_gain_rightm34 handle.all_gain_rightp34 handle.all_gain_rightp14,... 
                handle.all_bb_rightm1 handle.all_bb_rightm3 handle.all_bb_rightp1 handle.all_bb_rightp3...
                handle.all_gain_rightp14 handle.all_gain_rightp34 handle.all_gain_rightm34 handle.all_gain_rightm14],'enable','on');
        end

        set(handle.G80_right_slider4,'value',str2double(new_G80(1)));
        set(handle.right_CR4,'string',num2str(round((80-50)/(val+80-(G50(9)+50)),2)));
        dat.g80.old(9) = val;                
        dat.g4.max_right = max([handle.G80_right_slider4.Max-handle.G80_right_slider4.Value; handle.G50_right_slider4.Max-handle.G50_right_slider4.Value ]);
        dat.g4.min_right = -max([handle.G80_right_slider4.Value-handle.G80_right_slider4.Min; handle.G50_right_slider4.Value-handle.G50_right_slider4.Min]);
    case 'edit_r_4kHz'
        G80_str = get(handle.G80_right_edit5,'string');
        set(handle.G80_right_edit5,'string',[G80_str ' dB']);
        new_G80 = strsplit(G80_str,' ');
        G50_str = get(handle.G50_right_edit5,'string');
        new_G50 = strsplit(G50_str,' ');
        if str2double(new_G80(1)) < get(handle.G80_right_slider5,'min')
            new_G80{1} = num2str(get(handle.G80_right_slider5,'min'));
            set(handle.G80_right_edit5,'string',[num2str(get(handle.G80_right_slider5,'min')) ' dB']);
        
        elseif str2double(new_G80(1)) > get(handle.G80_right_slider5,'max')
            new_G80{1} = num2str(get(handle.G80_right_slider5,'max'));
            set(handle.G80_right_edit5,'string',[num2str(get(handle.G80_right_slider5,'max')) ' dB']);
        end
        val = str2double(new_G80(1));
        delta = val - dat.g80.old(10);
        if isequal(get(handle.link,'tag'),'link')
            if (dat.g80.old(5) + delta) > handle.G80_left_slider5.Max
                val2 = handle.G80_left_slider5.Max;
            elseif (dat.g80.old(5) + delta) < 0
                val2 = 0;
            else
                val2 = dat.g80.old(5) + delta;
            end
            set(handle.G80_left_slider5,'value',val2);
            set(handle.G80_left_edit5,'string',[num2str(val2) ' dB']);
            dat.g80.old(5) = val2;
            set(handle.left_CR5,'string',num2str(round((80-50)/(val2+80-(G50(5)+50)),2)));
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1) G80(2) G80(3) G80(4) val2 G80(6) G80(7) G80(8) G80(9) val]);
        else
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',...
                [G80(1) G80(2) G80(3) G80(4) G80(5) G80(6) G80(7) G80(8) G80(9) val]);
        end
        
        if str2double(new_G50(1)) == handle.G50_right_slider5.Min && str2double(new_G80(1)) == handle.G80_right_slider5.Min
            set([handle.all_gain_rightm15 handle.all_gain_rightm35],'enable','off');
        elseif str2double(new_G50(1)) == handle.G50_right_slider5.Max && str2double(new_G80(1)) == handle.G80_right_slider5.Max
            set([handle.all_gain_rightp35 handle.all_gain_rightp15],'enable','off');
            set([handle.all_gain_rightm15 handle.all_gain_rightm35],'enable','on');
        else
            set([handle.all_gain_rightm15 handle.all_gain_rightm35 handle.all_gain_rightp35 handle.all_gain_rightp15,... 
                handle.all_bb_rightm1 handle.all_bb_rightm3 handle.all_bb_rightp1 handle.all_bb_rightp3...
                handle.all_gain_rightp15 handle.all_gain_rightp35 handle.all_gain_rightm35 handle.all_gain_rightm15],'enable','on');
        end

        set(handle.G80_right_slider5,'value',str2double(new_G80(1)));
        set(handle.right_CR5,'string',num2str(round((80-50)/(val+80-(G50(10)+50)),2)));
        dat.g80.old(10) = val;                
        dat.g5.max_right = max([handle.G80_right_slider5.Max-handle.G80_right_slider5.Value; handle.G50_right_slider5.Max-handle.G50_right_slider5.Value ]);
        dat.g5.min_right = -max([handle.G80_right_slider5.Value-handle.G80_right_slider5.Min; handle.G50_right_slider5.Value-handle.G50_right_slider5.Min]);

end
dat.bb.max_right = max([handle.G80_right_slider5.Max-handle.G80_right_slider5.Value;handle.G80_right_slider4.Max-handle.G80_right_slider4.Value;...
    handle.G80_right_slider3.Max-handle.G80_right_slider3.Value;handle.G80_right_slider2.Max-handle.G80_right_slider2.Value;...
    handle.G80_right_slider1.Max-handle.G80_right_slider1.Value;handle.G50_right_slider5.Max-handle.G50_right_slider5.Value;...
    handle.G50_right_slider4.Max-handle.G50_right_slider4.Value;handle.G50_right_slider3.Max-handle.G50_right_slider3.Value;...
    handle.G50_right_slider2.Max-handle.G50_right_slider2.Value;handle.G50_right_slider1.Max-handle.G50_right_slider1.Value]);
dat.bb.min_right = -max([handle.G80_right_slider5.Value-handle.G80_right_slider5.Min;handle.G80_right_slider4.Value-handle.G80_right_slider4.Min;...
    handle.G80_right_slider3.Value-handle.G80_right_slider3.Min;handle.G80_right_slider2.Value-handle.G80_right_slider2.Min;...
    handle.G80_right_slider1.Value-handle.G80_right_slider1.Min;handle.G50_right_slider5.Value-handle.G50_right_slider5.Min;...
    handle.G50_right_slider4.Value-handle.G50_right_slider4.Min;handle.G50_right_slider3.Value-handle.G50_right_slider3.Min;...
    handle.G50_right_slider2.Value-handle.G50_right_slider2.Min;handle.G50_right_slider1.Value-handle.G50_right_slider1.Min]);

handle = plot_data_right(handle);
handle = plot_data_left(handle);

handle.dat = dat;
guidata(src,handle);
end
