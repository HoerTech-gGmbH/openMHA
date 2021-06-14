function [handle] = G50_slider_cllb(src,event)
handle = guidata(src);
dat = handle.dat;
mha = handle.mha;
G80 = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80');
G50 = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50');
slider_tag = get(gcbo,'tag');
switch slider_tag
    case 'slider_l_250Hz'
        val = get(handle.G50_left_slider1,'value');
        val = round(val);
        delta = val - dat.g50.old(1);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.G50_left_slider1,'value',val);
            set(handle.G50_left_edit1,'string',[num2str(val) ' dB']);
            if (dat.g50.old(6) + delta) > handle.G50_right_slider1.Max
                val2 = handle.G50_right_slider1.Max;
            elseif (dat.g50.old(6) + delta) < 0
                val2 = 0;
            else
                val2 = dat.g50.old(6) + delta;
            end
            set(handle.G50_right_slider1,'value',val2);
            set(handle.G50_right_edit1,'string',[num2str(val2) ' dB']);
            dat.g50.old(6) = val2;
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [val G50(2) G50(3) G50(4) G50(5) val2 G50(7) G50(8) G50(9) G50(10)]);
            set(handle.left_CR1,'string',num2str(round((80-50)/(G80(1)+80-(val+50)),2)));
            set(handle.right_CR1,'string',num2str(round((80-50)/(G80(6)+80-(val2+50)),2)));
        else
            set(handle.G50_left_slider1,'value',val);
            set(handle.G50_left_edit1,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [val G50(2) G50(3) G50(4) G50(5) G50(6) G50(7) G50(8) G50(9) G50(10)]);
            set(handle.left_CR1,'string',num2str(round((80-50)/(G80(1)+80-(val+50)),2)));
        end
        if handle.G50_left_slider1.Value == handle.G50_left_slider1.Min && handle.G80_left_slider1.Value == handle.G80_left_slider1.Min
            set([handle.all_gain_leftm11 handle.all_gain_leftm31],'enable','off');
        elseif handle.G50_left_slider1.Value == handle.G50_left_slider1.Max && handle.G80_left_slider1.Value == handle.G80_left_slider1.Max
            set([handle.all_gain_leftp31 handle.all_gain_leftp11],'enable','off');
            set([handle.all_gain_leftm11 handle.all_gain_leftm31],'enable','on');
        else
            set([handle.all_gain_leftm11 handle.all_gain_leftm31 handle.all_gain_leftp31 handle.all_gain_leftp11,... 
                handle.all_bb_leftm1 handle.all_bb_leftm3 handle.all_bb_leftp1 handle.all_bb_leftp3...
                handle.all_gain_leftp11 handle.all_gain_leftp31 handle.all_gain_leftm31 handle.all_gain_leftm11],'enable','on');
        end
        dat.g50.old(1) = val;
        dat.g1.max_left = max([handle.G80_left_slider1.Max-handle.G80_left_slider1.Value; handle.G50_left_slider1.Max-handle.G50_left_slider1.Value ]);
        dat.g1.min_left = -max([handle.G80_left_slider1.Value-handle.G80_left_slider1.Min; handle.G50_left_slider1.Value-handle.G50_left_slider1.Min]);
    case 'slider_l_500Hz'
        val = get(handle.G50_left_slider2,'value');
        val = round(val);
        delta = val - dat.g50.old(2);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.G50_left_slider2,'value',val);
            set(handle.G50_left_edit2,'string',[num2str(val) ' dB']);
            if (dat.g50.old(7) + delta) > handle.G50_right_slider2.Max
                val2 = handle.G50_right_slider2.Max;
            elseif (dat.g50.old(7) + delta) < 0
                val2 = 0;
            else
                val2 = dat.g50.old(7) + delta;
            end
            set(handle.G50_right_slider2,'value',val2);
            set(handle.G50_right_edit2,'string',[num2str(val2) ' dB']);
            dat.g50.old(7) = val2;
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1) val G50(3) G50(4) G50(5) G50(6) val2 G50(8) G50(9) G50(10)]);
            set(handle.left_CR2,'string',num2str(round((80-50)/(G80(2)+80-(val+50)),2)));
            set(handle.right_CR2,'string',num2str(round((80-50)/(G80(7)+80-(val2+50)),2)));
        else
            set(handle.G50_left_slider2,'value',val);
            set(handle.G50_left_edit2,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1) val G50(3) G50(4) G50(5) G50(6) G50(7) G50(8) G50(9) G50(10)]);
            set(handle.left_CR2,'string',num2str(round((80-50)/(G80(2)+80-(val+50)),2)));
        end
        if handle.G50_left_slider2.Value == handle.G50_left_slider2.Min && handle.G80_left_slider2.Value == handle.G80_left_slider2.Min
            set([handle.all_gain_leftm12 handle.all_gain_leftm32],'enable','off');
        elseif handle.G50_left_slider2.Value == handle.G50_left_slider2.Max && handle.G80_left_slider2.Value == handle.G80_left_slider2.Max
            set([handle.all_gain_leftp32 handle.all_gain_leftp12],'enable','off');
            set([handle.all_gain_leftm12 handle.all_gain_leftm32],'enable','on');
        else
            set([handle.all_gain_leftm12 handle.all_gain_leftm32 handle.all_gain_leftp32 handle.all_gain_leftp12,... 
                handle.all_bb_leftm1 handle.all_bb_leftm3 handle.all_bb_leftp1 handle.all_bb_leftp3...
                handle.all_gain_leftp12 handle.all_gain_leftp32 handle.all_gain_leftm32 handle.all_gain_leftm12],'enable','on');
        end
        dat.g50.old(2) = val;
        dat.g2.max_left = max([handle.G80_left_slider2.Max-handle.G80_left_slider2.Value; handle.G50_left_slider2.Max-handle.G50_left_slider2.Value ]);
        dat.g2.min_left = -max([handle.G80_left_slider2.Value-handle.G80_left_slider2.Min; handle.G50_left_slider2.Value-handle.G50_left_slider2.Min]);
    case 'slider_l_1kHz'
        val = get(handle.G50_left_slider3,'value');
        val = round(val);
        delta = val - dat.g50.old(3);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.G50_left_slider3,'value',val);
            set(handle.G50_left_edit3,'string',[num2str(val) ' dB']);
            if (dat.g50.old(8) + delta) > handle.G50_right_slider3.Max
                val2 = handle.G50_right_slider3.Max;
            elseif (dat.g50.old(8) + delta) < 0
                val2 = 0;
            else
                val2 = dat.g50.old(8) + delta;
            end
            set(handle.G50_right_slider3,'value',val2);
            set(handle.G50_right_edit3,'string',[num2str(val2) ' dB']);
            dat.g50.old(8) = val2;
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1) G50(2) val G50(4) G50(5) G50(6) G50(7) val2 G50(9) G50(10)]);
            set(handle.left_CR3,'string',num2str(round((80-50)/(G80(3)+80-(val+50)),2)));
            set(handle.right_CR3,'string',num2str(round((80-50)/(G80(8)+80-(val2+50)),2)));
        else
            set(handle.G50_left_slider3,'value',val);
            set(handle.G50_left_edit3,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1) G50(2) val G50(4) G50(5) G50(6) G50(7) G50(8) G50(9) G50(10)]);
            set(handle.left_CR3,'string',num2str(round((80-50)/(G80(3)+80-(val+50)),2)));
        end
        if handle.G50_left_slider3.Value == handle.G50_left_slider3.Min && handle.G80_left_slider3.Value == handle.G80_left_slider3.Min
            set([handle.all_gain_leftm13 handle.all_gain_leftm33],'enable','off');
        elseif handle.G50_left_slider3.Value == handle.G50_left_slider3.Max && handle.G80_left_slider3.Value == handle.G80_left_slider3.Max
            set([handle.all_gain_leftp33 handle.all_gain_leftp13],'enable','off');
            set([handle.all_gain_leftm13 handle.all_gain_leftm33],'enable','on');
        else
            set([handle.all_gain_leftm13 handle.all_gain_leftm33 handle.all_gain_leftp33 handle.all_gain_leftp13,... 
                handle.all_bb_leftm1 handle.all_bb_leftm3 handle.all_bb_leftp1 handle.all_bb_leftp3...
                handle.all_gain_leftp13 handle.all_gain_leftp33 handle.all_gain_leftm33 handle.all_gain_leftm13],'enable','on');
        end
        dat.g50.old(3) = val;
        dat.g3.max_left = max([handle.G80_left_slider3.Max-handle.G80_left_slider3.Value; handle.G50_left_slider3.Max-handle.G50_left_slider3.Value ]);
        dat.g3.min_left = -max([handle.G80_left_slider3.Value-handle.G80_left_slider3.Min; handle.G50_left_slider3.Value-handle.G50_left_slider3.Min]);
    case 'slider_l_2kHz'
        val = get(handle.G50_left_slider4,'value');
        val = round(val);
        delta = val - dat.g50.old(4);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.G50_left_slider4,'value',val);
            set(handle.G50_left_edit4,'string',[num2str(val) ' dB']);
            if (dat.g50.old(9) + delta) > handle.G50_right_slider4.Max
                val2 = handle.G50_right_slider4.Max;
            elseif (dat.g50.old(9) + delta) < 0
                val2 = 0;
            else
                val2 = dat.g50.old(9) + delta;
            end
            set(handle.G50_right_slider4,'value',val2);
            set(handle.G50_right_edit4,'string',[num2str(val2) ' dB']);
            dat.g50.old(9) = val2;
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1) G50(2) G50(3) val G50(5) G50(6) G50(7) G50(8) val2 G50(10)]);
            set(handle.left_CR4,'string',num2str(round((80-50)/(G80(4)+80-(val+50)),2)));
            set(handle.right_CR4,'string',num2str(round((80-50)/(G80(9)+80-(val2+50)),2)));
        else
            set(handle.G50_left_slider4,'value',val);
            set(handle.G50_left_edit4,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1) G50(2) G50(3) val G50(5) G50(6) G50(7) G50(8) G50(9) G50(10)]);
            set(handle.left_CR4,'string',num2str(round((80-50)/(G80(4)+80-(val+50)),2)));
        end
        if handle.G50_left_slider4.Value == handle.G50_left_slider4.Min && handle.G80_left_slider4.Value == handle.G80_left_slider4.Min
            set([handle.all_gain_leftm14 handle.all_gain_leftm34],'enable','off');
        elseif handle.G50_left_slider4.Value == handle.G50_left_slider4.Max && handle.G80_left_slider4.Value == handle.G80_left_slider4.Max
            set([handle.all_gain_leftp34 handle.all_gain_leftp14],'enable','off');
            set([handle.all_gain_leftm14 handle.all_gain_leftm34],'enable','on');
        else
            set([handle.all_gain_leftm14 handle.all_gain_leftm34 handle.all_gain_leftp34 handle.all_gain_leftp14,... 
                handle.all_bb_leftm1 handle.all_bb_leftm3 handle.all_bb_leftp1 handle.all_bb_leftp3...
                handle.all_gain_leftp14 handle.all_gain_leftp34 handle.all_gain_leftm34 handle.all_gain_leftm14],'enable','on');
        end
        dat.g50.old(4) = val;
        dat.g4.max_left = max([handle.G80_left_slider4.Max-handle.G80_left_slider4.Value; handle.G50_left_slider4.Max-handle.G50_left_slider4.Value ]);
        dat.g4.min_left = -max([handle.G80_left_slider4.Value-handle.G80_left_slider4.Min; handle.G50_left_slider4.Value-handle.G50_left_slider4.Min]);
    case 'slider_l_4kHz'
        val = get(handle.G50_left_slider5,'value');
        val = round(val);
        delta = val - dat.g50.old(5);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.G50_left_slider5,'value',val);
            set(handle.G50_left_edit5,'string',[num2str(val) ' dB']);
            if (dat.g50.old(10) + delta) > handle.G50_right_slider5.Max
                val2 = handle.G50_right_slider5.Max;
            elseif (dat.g50.old(10) + delta) < 0
                val2 = 0;
            else
                val2 = dat.g50.old(10) + delta;
            end
            set(handle.G50_right_slider5,'value',val2);
            set(handle.G50_right_edit5,'string',[num2str(val2) ' dB']);
            dat.g50.old(10) = val2;
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1) G50(2) G50(3) G50(4) val G50(6) G50(7) G50(8) G50(9) val2]);
            set(handle.left_CR5,'string',num2str(round((80-50)/(G80(5)+80-(val+50)),2)));
            set(handle.right_CR5,'string',num2str(round((80-50)/(G80(10)+80-(val2+50)),2)));
        else
            set(handle.G50_left_slider5,'value',val);
            set(handle.G50_left_edit5,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1) G50(2) G50(3) G50(4) val G50(6) G50(7) G50(8) G50(9) G50(10)]);
            set(handle.left_CR5,'string',num2str(round((80-50)/(G80(5)+80-(val+50)),2)));
        end
        if handle.G50_left_slider5.Value == handle.G50_left_slider5.Min && handle.G80_left_slider5.Value == handle.G80_left_slider5.Min
            set([handle.all_gain_leftm15 handle.all_gain_leftm35],'enable','off');
        elseif handle.G50_left_slider5.Value == handle.G50_left_slider5.Max && handle.G80_left_slider5.Value == handle.G80_left_slider5.Max
            set([handle.all_gain_leftp35 handle.all_gain_leftp15],'enable','off');
            set([handle.all_gain_leftm15 handle.all_gain_leftm35],'enable','on');
        else
            set([handle.all_gain_leftm15 handle.all_gain_leftm35 handle.all_gain_leftp35 handle.all_gain_leftp15,... 
                handle.all_bb_leftm1 handle.all_bb_leftm3 handle.all_bb_leftp1 handle.all_bb_leftp3...
                handle.all_gain_leftp15 handle.all_gain_leftp35 handle.all_gain_leftm35 handle.all_gain_leftm15],'enable','on');
        end
        dat.g50.old(5) = val;
        dat.g5.max_left = max([handle.G80_left_slider5.Max-handle.G80_left_slider5.Value; handle.G50_left_slider5.Max-handle.G50_left_slider5.Value ]);
        dat.g5.min_left = -max([handle.G80_left_slider5.Value-handle.G80_left_slider5.Min; handle.G50_left_slider5.Value-handle.G50_left_slider5.Min]);
    case 'slider_r_250Hz'
        val = get(handle.G50_right_slider1,'value');
        val = round(val);
        delta = val - dat.g50.old(6);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.G50_right_slider1,'value',val);
            set(handle.G50_right_edit1,'string',[num2str(val) ' dB']);
            if (dat.g50.old(1) + delta) > handle.G50_left_slider1.Max
                val2 = handle.G50_left_slider1.Max;
            elseif (dat.g50.old(1) + delta) < 0
                val2 = 0;
            else
                val2 = dat.g50.old(1) + delta;
            end
            set(handle.G50_left_slider1,'value',val2);
            set(handle.G50_left_edit1,'string',[num2str(val2) ' dB']);
            dat.g50.old(1) = val2;
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [val2 G50(2) G50(3) G50(4) G50(5) val G50(7) G50(8) G50(9) G50(10)]);
            set(handle.right_CR1,'string',num2str(round((80-50)/(G80(6)+80-(val+50)),2)));
            set(handle.left_CR1,'string',num2str(round((80-50)/(G80(1)+80-(val2+50)),2)));
        else
            set(handle.G50_right_slider1,'value',val);
            set(handle.G50_right_edit1,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1) G50(2) G50(3) G50(4) G50(5) val G50(7) G50(8) G50(9) G50(10)]);
            set(handle.right_CR1,'string',num2str(round((80-50)/(G80(6)+80-(val+50)),2)));
        end
        if handle.G50_right_slider1.Value == handle.G50_right_slider1.Min && handle.G80_right_slider1.Value == handle.G80_right_slider1.Min
            set([handle.all_gain_rightm11 handle.all_gain_rightm31],'enable','off');
        elseif handle.G50_right_slider1.Value == handle.G50_right_slider1.Max && handle.G80_right_slider1.Value == handle.G80_right_slider1.Max
            set([handle.all_gain_rightp31 handle.all_gain_rightp11],'enable','off');
            set([handle.all_gain_rightm11 handle.all_gain_rightm31],'enable','on');
        else
            set([handle.all_gain_rightm11 handle.all_gain_rightm31 handle.all_gain_rightp31 handle.all_gain_rightp11,... 
                handle.all_bb_rightm1 handle.all_bb_rightm3 handle.all_bb_rightp1 handle.all_bb_rightp3...
                handle.all_gain_rightp11 handle.all_gain_rightp31 handle.all_gain_rightm31 handle.all_gain_rightm11],'enable','on');
        end
        dat.g50.old(6) = val;
        dat.g1.max_right = max([handle.G80_right_slider1.Max-handle.G80_right_slider1.Value; handle.G50_right_slider1.Max-handle.G50_right_slider1.Value ]);
        dat.g1.min_right = -max([handle.G80_right_slider1.Value-handle.G80_right_slider1.Min; handle.G50_right_slider1.Value-handle.G50_right_slider1.Min]);
    case 'slider_r_500Hz'
        val = get(handle.G50_right_slider2,'value');
        val = round(val);
        delta = val - dat.g50.old(7);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.G50_right_slider2,'value',val);
            set(handle.G50_right_edit2,'string',[num2str(val) ' dB']);
            if (dat.g50.old(2) + delta) > handle.G50_left_slider2.Max
                val2 = handle.G50_left_slider2.Max;
            elseif (dat.g50.old(2) + delta) < 0
                val2 = 0;
            else
                val2 = dat.g50.old(2) + delta;
            end
            set(handle.G50_left_slider2,'value',val2);
            set(handle.G50_left_edit2,'string',[num2str(val2) ' dB']);
            dat.g50.old(2) = val2;
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1) val2 G50(3) G50(4) G50(5) G50(6) val G50(8) G50(9) G50(10)]);
            set(handle.right_CR2,'string',num2str(round((80-50)/(G80(7)+80-(val+50)),2)));
            set(handle.left_CR2,'string',num2str(round((80-50)/(G80(2)+80-(val2+50)),2)));
        else
            set(handle.G50_right_slider2,'value',val);
            set(handle.G50_right_edit2,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1) G50(2) G50(3) G50(4) G50(5) G50(6) val G50(8) G50(9) G50(10)]);
            set(handle.right_CR2,'string',num2str(round((80-50)/(G80(7)+80-(val+50)),2)));
        end
        if handle.G50_right_slider2.Value == handle.G50_right_slider2.Min && handle.G80_right_slider2.Value == handle.G80_right_slider2.Min
            set([handle.all_gain_rightm12 handle.all_gain_rightm32],'enable','off');
        elseif handle.G50_right_slider2.Value == handle.G50_right_slider2.Max && handle.G80_right_slider2.Value == handle.G80_right_slider2.Max
            set([handle.all_gain_rightp32 handle.all_gain_rightp12],'enable','off');
            set([handle.all_gain_rightm12 handle.all_gain_rightm32],'enable','on');
        else
            set([handle.all_gain_rightm12 handle.all_gain_rightm32 handle.all_gain_rightp32 handle.all_gain_rightp12,... 
                handle.all_bb_rightm1 handle.all_bb_rightm3 handle.all_bb_rightp1 handle.all_bb_rightp3...
                handle.all_gain_rightp12 handle.all_gain_rightp32 handle.all_gain_rightm32 handle.all_gain_rightm12],'enable','on');
        end
        dat.g50.old(7) = val;
        dat.g2.max_right = max([handle.G80_right_slider2.Max-handle.G80_right_slider2.Value; handle.G50_right_slider2.Max-handle.G50_right_slider2.Value ]);
        dat.g2.min_right = -max([handle.G80_right_slider2.Value-handle.G80_right_slider2.Min; handle.G50_right_slider2.Value-handle.G50_right_slider2.Min]);
    case 'slider_r_1kHz'
        val = get(handle.G50_right_slider3,'value');
        val = round(val);
        delta = val - dat.g50.old(8);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.G50_right_slider3,'value',val);
            set(handle.G50_right_edit3,'string',[num2str(val) ' dB']);
            if (dat.g50.old(3) + delta) > handle.G50_left_slider3.Max
                val2 = handle.G50_left_slider3.Max;
            elseif (dat.g50.old(3) + delta) < 0
                val2 = 0;
            else
                val2 = dat.g50.old(3) + delta;
            end
            set(handle.G50_left_slider3,'value',val2);
            set(handle.G50_left_edit3,'string',[num2str(val2) ' dB']);
            dat.g50.old(3) = val2;
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1) G50(2) val2 G50(4) G50(5) G50(6) G50(7) val G50(9) G50(10)]);
            set(handle.right_CR3,'string',num2str(round((80-50)/(G80(8)+80-(val+50)),2)));
            set(handle.left_CR3,'string',num2str(round((80-50)/(G80(3)+80-(val2+50)),2)));
        else
            set(handle.G50_right_slider3,'value',val);
            set(handle.G50_right_edit3,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1) G50(2) G50(3) G50(4) G50(5) G50(6) G50(7) val G50(9) G50(10)]);
            set(handle.right_CR3,'string',num2str(round((80-50)/(G80(8)+80-(val+50)),2)));
        end
        if handle.G50_right_slider3.Value == handle.G50_right_slider3.Min && handle.G80_right_slider3.Value == handle.G80_right_slider3.Min
            set([handle.all_gain_rightm13 handle.all_gain_rightm33],'enable','off');
        elseif handle.G50_right_slider3.Value == handle.G50_right_slider3.Max && handle.G80_right_slider3.Value == handle.G80_right_slider3.Max
            set([handle.all_gain_rightp33 handle.all_gain_rightp13],'enable','off');
            set([handle.all_gain_rightm13 handle.all_gain_rightm33],'enable','on');
        else
            set([handle.all_gain_rightm13 handle.all_gain_rightm33 handle.all_gain_rightp33 handle.all_gain_rightp13,... 
                handle.all_bb_rightm1 handle.all_bb_rightm3 handle.all_bb_rightp1 handle.all_bb_rightp3...
                handle.all_gain_rightp13 handle.all_gain_rightp33 handle.all_gain_rightm33 handle.all_gain_rightm13],'enable','on');
        end
        dat.g50.old(8) = val;
        dat.g3.max_right = max([handle.G80_right_slider3.Max-handle.G80_right_slider3.Value; handle.G50_right_slider3.Max-handle.G50_right_slider3.Value ]);
        dat.g3.min_right = -max([handle.G80_right_slider3.Value-handle.G80_right_slider3.Min; handle.G50_right_slider3.Value-handle.G50_right_slider3.Min]);
    case 'slider_r_2kHz'
        val = get(handle.G50_right_slider4,'value');
        val = round(val);
        delta = val - dat.g50.old(9);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.G50_right_slider4,'value',val);
            set(handle.G50_right_edit4,'string',[num2str(val) ' dB']);
            if (dat.g50.old(4) + delta) > handle.G50_left_slider4.Max
                val2 = handle.G50_left_slider4.Max;
            elseif (dat.g50.old(4) + delta) < 0
                val2 = 0;
            else
                val2 = dat.g50.old(4) + delta;
            end
            set(handle.G50_left_slider4,'value',val2);
            set(handle.G50_left_edit4,'string',[num2str(val2) ' dB']);
            dat.g50.old(4) = val2;
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1) G50(2) G50(3) val2 G50(5) G50(6) G50(7) G50(8) val G50(10)]);
            set(handle.right_CR4,'string',num2str(round((80-50)/(G80(9)+80-(val+50)),2)));
            set(handle.left_CR4,'string',num2str(round((80-50)/(G80(4)+80-(val2+50)),2)));
        else
            set(handle.G50_right_slider4,'value',val);
            set(handle.G50_right_edit4,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1) G50(2) G50(3) G50(4) G50(5) G50(6) G50(7) G50(8) val G50(10)]);
            set(handle.right_CR4,'string',num2str(round((80-50)/(G80(9)+80-(val+50)),2)));
        end
        if handle.G50_right_slider4.Value == handle.G50_right_slider4.Min && handle.G80_right_slider4.Value == handle.G80_right_slider4.Min
            set([handle.all_gain_rightm14 handle.all_gain_rightm34],'enable','off');
        elseif handle.G50_right_slider4.Value == handle.G50_right_slider4.Max && handle.G80_right_slider4.Value == handle.G80_right_slider4.Max
            set([handle.all_gain_rightp34 handle.all_gain_rightp14],'enable','off');
            set([handle.all_gain_rightm14 handle.all_gain_rightm34],'enable','on');
        else
            set([handle.all_gain_rightm14 handle.all_gain_rightm34 handle.all_gain_rightp34 handle.all_gain_rightp14,... 
                handle.all_bb_rightm1 handle.all_bb_rightm3 handle.all_bb_rightp1 handle.all_bb_rightp3...
                handle.all_gain_rightp14 handle.all_gain_rightp34 handle.all_gain_rightm34 handle.all_gain_rightm14],'enable','on');
        end
        dat.g50.old(9) = val;
        dat.g4.max_right = max([handle.G80_right_slider4.Max-handle.G80_right_slider4.Value; handle.G50_right_slider4.Max-handle.G50_right_slider4.Value ]);
        dat.g4.min_right = -max([handle.G80_right_slider4.Value-handle.G80_right_slider4.Min; handle.G50_right_slider4.Value-handle.G50_right_slider4.Min]);
    case 'slider_r_4kHz'
        val = get(handle.G50_right_slider5,'value');
        val = round(val);
        delta = val - dat.g50.old(10);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.G50_right_slider5,'value',val);
            set(handle.G50_right_edit5,'string',[num2str(val) ' dB']);
            if (dat.g50.old(5) + delta) > handle.G50_left_slider5.Max
                val2 = handle.G50_left_slider5.Max;
            elseif (dat.g50.old(5) + delta) < 0
                val2 = 0;
            else
                val2 = dat.g50.old(5) + delta;
            end
            set(handle.G50_left_slider5,'value',val2);
            set(handle.G50_left_edit5,'string',[num2str(val2) ' dB']);
            dat.g50.old(5) = val2;
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1) G50(2) G50(3) G50(4) val2 G50(6) G50(7) G50(8) G50(9) val]);
            set(handle.right_CR5,'string',num2str(round((80-50)/(G80(10)+80-(val+50)),2)));
            set(handle.left_CR5,'string',num2str(round((80-50)/(G80(5)+80-(val2+50)),2)));
        else
            set(handle.G50_right_slider5,'value',val);
            set(handle.G50_right_edit5,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1) G50(2) G50(3) G50(4) G50(5) G50(6) G50(7) G50(8) G50(9) val]);
            set(handle.right_CR5,'string',num2str(round((80-50)/(G80(10)+80-(val+50)),2)));  
        end
        if handle.G50_right_slider5.Value == handle.G50_right_slider5.Min && handle.G80_right_slider5.Value == handle.G80_right_slider5.Min
            set([handle.all_gain_rightm15 handle.all_gain_rightm35],'enable','off');
        elseif handle.G50_right_slider5.Value == handle.G50_right_slider5.Max && handle.G80_right_slider5.Value == handle.G80_right_slider5.Max
            set([handle.all_gain_rightp35 handle.all_gain_rightp15],'enable','off');
            set([handle.all_gain_rightm15 handle.all_gain_rightm35],'enable','on');
        else
            set([handle.all_gain_rightm15 handle.all_gain_rightm35 handle.all_gain_rightp35 handle.all_gain_rightp15,... 
                handle.all_bb_rightm1 handle.all_bb_rightm3 handle.all_bb_rightp1 handle.all_bb_rightp3...
                handle.all_gain_rightp15 handle.all_gain_rightp35 handle.all_gain_rightm35 handle.all_gain_rightm15],'enable','on');
        end
        dat.g50.old(10) = val;
        dat.g5.max_right = max([handle.G80_right_slider5.Max-handle.G80_right_slider5.Value; handle.G50_right_slider5.Max-handle.G50_right_slider5.Value ]);
        dat.g5.min_right = -max([handle.G80_right_slider5.Value-handle.G80_right_slider5.Min; handle.G50_right_slider5.Value-handle.G50_right_slider5.Min]);
    case 'all_right'
        val = get(handle.G50_all_slider_right,'value');
        val = round(val);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.G50_all_slider_right,'value',val);
%             set(handle.G50_all_edit_right,'string',[num2str(val) ' dB']);
            set(handle.G50_all_slider_left,'value',val);
%             set(handle.G50_all_edit_left,'string',[num2str(val) ' dB']);

            if dat.g50.ref_right(1) + val > get(handle.G50_right_slider1,'max')
                set(handle.G50_right_edit1,'string',[num2str(get(handle.G50_right_slider1,'max')) ' dB']);
                set(handle.G50_right_slider1,'value',get(handle.G50_right_slider1,'max'));
                G50_6 = get(handle.G50_right_slider1,'max');
            elseif dat.g50.ref_right(1) + val < get(handle.G50_right_slider1,'min')
                set(handle.G50_right_edit1,'string',[num2str(get(handle.G50_right_slider1,'min')) ' dB']);
                set(handle.G50_right_slider1,'value',get(handle.G50_right_slider1,'min'));
                G50_6 = get(handle.G50_right_slider1,'min');
            else 
                set(handle.G50_right_slider1,'value',dat.g50.ref_right(1) + val);
                set(handle.G50_right_edit1,'string',[num2str(dat.g50.ref_right(1) + val) ' dB']);
                G50_6 = dat.g50.ref_right(1) + val;
            end
            if dat.g50.ref_right(2) + val > get(handle.G50_right_slider2,'max')
                set(handle.G50_right_edit2,'string',[num2str(get(handle.G50_right_slider2,'max')) ' dB']);
                set(handle.G50_right_slider2,'value',get(handle.G50_right_slider2,'max'));
                G50_7 = get(handle.G50_right_slider2,'max');
            elseif dat.g50.ref_right(2) + val < get(handle.G50_right_slider2,'min')
                set(handle.G50_right_edit2,'string',[num2str(get(handle.G50_right_slider2,'min')) ' dB']);
                set(handle.G50_right_slider1,'value',get(handle.G50_right_slider2,'min'));
                G50_7 = get(handle.G50_right_slider2,'min');
            else 
                set(handle.G50_right_slider2,'value',dat.g50.ref_right(2) + val);
                set(handle.G50_right_edit2,'string',[num2str(dat.g50.ref_right(2) + val) ' dB']);
                G50_7 = dat.g50.ref_right(2) + val;
            end
            if dat.g50.ref_right(3) + val > get(handle.G50_right_slider3,'max')
                set(handle.G50_right_edit3,'string',[num2str(get(handle.G50_right_slider3,'max')) ' dB']);
                set(handle.G50_right_slider3,'value',get(handle.G50_right_slider3,'max'));
                G50_8 = get(handle.G50_right_slider3,'max');
            elseif dat.g50.ref_right(3) + val < get(handle.G50_right_slider3,'min')
                set(handle.G50_right_edit3,'string',[num2str(get(handle.G50_right_slider3,'min')) ' dB']);
                set(handle.G50_right_slider3,'value',get(handle.G50_right_slider3,'min'));
                G50_8 = get(handle.G50_right_slider3,'min');
            else 
                set(handle.G50_right_slider3,'value',dat.g50.ref_right(3) + val);
                set(handle.G50_right_edit3,'string',[num2str(dat.g50.ref_right(3) + val) ' dB']);
                G50_8 = dat.g50.ref_right(3) + val;
            end
            if dat.g50.ref_right(4) + val > get(handle.G50_right_slider4,'max')
                set(handle.G50_right_edit4,'string',[num2str(get(handle.G50_right_slider4,'max')) ' dB']);
                set(handle.G50_right_slider4,'value',get(handle.G50_right_slider4,'max'));
                G50_9 = get(handle.G50_right_slider4,'max');
            elseif dat.g50.ref_right(4) + val < get(handle.G50_right_slider4,'min')
                set(handle.G50_right_edit4,'string',[num2str(get(handle.G50_right_slider4,'min')) ' dB']);
                set(handle.G50_right_slider4,'value',get(handle.G50_right_slider4,'min'));
                G50_9 = get(handle.G50_right_slider4,'min');
            else 
                set(handle.G50_right_slider4,'value',dat.g50.ref_right(4) + val);
                set(handle.G50_right_edit4,'string',[num2str(dat.g50.ref_right(4) + val) ' dB']);
                G50_9 = dat.g50.ref_right(4) + val;
            end
            if dat.g50.ref_right(5) + val > get(handle.G50_right_slider5,'max')
                set(handle.G50_right_edit5,'string',[num2str(get(handle.G50_right_slider5,'max')) ' dB']);
                set(handle.G50_right_slider5,'value',get(handle.G50_right_slider5,'max'));
                G50_10 = get(handle.G50_right_slider5,'max');
            elseif dat.g50.ref_right(5) + val < get(handle.G50_right_slider5,'min')
                set(handle.G50_right_edit5,'string',[num2str(get(handle.G50_right_slider5,'min')) ' dB']);
                set(handle.G50_right_slider5,'value',get(handle.G50_right_slider5,'min'));
                G50_10 = get(handle.G50_right_slider5,'min');
            else 
                set(handle.G50_right_slider5,'value',dat.g50.ref_right(5) + val);
                set(handle.G50_right_edit5,'string',[num2str(dat.g50.ref_right(5) + val) ' dB']);
                G50_10 = dat.g50.ref_right(5) + val;
            end
            
            
            if dat.g50.ref_left(1) + val > get(handle.G50_left_slider1,'max')
                set(handle.G50_left_edit1,'string',[num2str(get(handle.G50_left_slider1,'max')) ' dB']);
                set(handle.G50_left_slider1,'value',get(handle.G50_left_slider1,'max'));
                G50_1 = get(handle.G50_left_slider1,'max');
            elseif dat.g50.ref_left(1) + val < get(handle.G50_left_slider1,'min')
                set(handle.G50_left_edit1,'string',[num2str(get(handle.G50_left_slider1,'min')) ' dB']);
                set(handle.G50_left_slider1,'value',get(handle.G50_left_slider1,'min'));
                G50_1 = get(handle.G50_left_slider1,'min');
            else 
                set(handle.G50_left_slider1,'value',dat.g50.ref_left(1) + val);
                set(handle.G50_left_edit1,'string',[num2str(dat.g50.ref_left(1) + val) ' dB']);
                G50_1 = dat.g50.ref_left(1) + val;
            end
            if dat.g50.ref_left(2) + val > get(handle.G50_left_slider2,'max')
                set(handle.G50_left_edit2,'string',[num2str(get(handle.G50_left_slider2,'max')) ' dB']);
                set(handle.G50_left_slider2,'value',get(handle.G50_left_slider2,'max'));
                G50_2 = get(handle.G50_left_slider2,'max');
            elseif dat.g50.ref_left(2) + val < get(handle.G50_left_slider2,'min')
                set(handle.G50_left_edit2,'string',[num2str(get(handle.G50_left_slider2,'min')) ' dB']);
                set(handle.G50_left_slider2,'value',get(handle.G50_left_slider2,'min'));
                G50_2 = get(handle.G50_left_slider2,'min');
            else 
                set(handle.G50_left_slider2,'value',dat.g50.ref_left(2) + val);
                set(handle.G50_left_edit2,'string',[num2str(dat.g50.ref_left(2) + val) ' dB']);
                G50_2 = dat.g50.ref_left(2) + val;
            end
            if dat.g50.ref_left(3) + val > get(handle.G50_left_slider3,'max')
                set(handle.G50_left_edit3,'string',[num2str(get(handle.G50_left_slider3,'max')) ' dB']);
                set(handle.G50_left_slider3,'value',get(handle.G50_left_slider3,'max'));
                G50_3 = get(handle.G50_left_slider3,'max');
            elseif dat.g50.ref_left(3) + val < get(handle.G50_left_slider3,'min')
                set(handle.G50_left_edit3,'string',[num2str(get(handle.G50_left_slider3,'min')) ' dB']);
                set(handle.G50_left_slider3,'value',get(handle.G50_left_slider3,'min'));
                G50_3 = get(handle.G50_left_slider3,'min');
            else 
                set(handle.G50_left_slider3,'value',dat.g50.ref_left(3) + val);
                set(handle.G50_left_edit3,'string',[num2str(dat.g50.ref_left(3) + val) ' dB']);
                G50_3 = dat.g50.ref_left(3) + val;
            end
            if dat.g50.ref_left(4) + val > get(handle.G50_left_slider4,'max')
                set(handle.G50_left_edit4,'string',[num2str(get(handle.G50_left_slider4,'max')) ' dB']);
                set(handle.G50_left_slider4,'value',get(handle.G50_left_slider4,'max'));
                G50_4 = get(handle.G50_left_slider4,'max');
            elseif dat.g50.ref_left(4) + val < get(handle.G50_left_slider4,'min')
                set(handle.G50_left_edit4,'string',[num2str(get(handle.G50_left_slider4,'min')) ' dB']);
                set(handle.G50_left_slider4,'value',get(handle.G50_left_slider4,'min'));
                G50_4 = get(handle.G50_left_slider4,'min');
            else 
                set(handle.G50_left_slider4,'value',dat.g50.ref_left(4) + val);
                set(handle.G50_left_edit4,'string',[num2str(dat.g50.ref_left(4) + val) ' dB']);
                G50_4 = dat.g50.ref_left(4) + val;
            end
            if dat.g50.ref_left(5) + val > get(handle.G50_left_slider5,'max')
                set(handle.G50_left_edit5,'string',[num2str(get(handle.G50_left_slider5,'max')) ' dB']);
                set(handle.G50_left_slider5,'value',get(handle.G50_left_slider5,'max'));
                G50_5 = get(handle.G50_left_slider5,'max');
            elseif dat.g50.ref_left(5) + val < get(handle.G50_left_slider5,'min')
                set(handle.G50_left_edit5,'string',[num2str(get(handle.G50_left_slider5,'min')) ' dB']);
                set(handle.G50_left_slider5,'value',get(handle.G50_left_slider5,'min'));
                G50_5 = get(handle.G50_left_slider5,'min');
            else 
                set(handle.G50_left_slider5,'value',dat.g50.ref_left(5) + val);
                set(handle.G50_left_edit5,'string',[num2str(dat.g50.ref_left(5) + val) ' dB']);
                G50_5 = dat.g50.ref_left(5) + val;
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50_1 G50_2 G50_3 G50_4 G50_5 G50_6 G50_7 G50_8 G50_9 G50_10]);
        
            set(handle.right_CR1,'string',num2str(round((80-50)/(G80(6)+80-(handle.G50_right_slider1.Value+50)),2)));
            set(handle.right_CR2,'string',num2str(round((80-50)/(G80(7)+80-(handle.G50_right_slider2.Value+50)),2)));
            set(handle.right_CR3,'string',num2str(round((80-50)/(G80(8)+80-(handle.G50_right_slider3.Value+50)),2)));
            set(handle.right_CR4,'string',num2str(round((80-50)/(G80(9)+80-(handle.G50_right_slider4.Value+50)),2)));
            set(handle.right_CR5,'string',num2str(round((80-50)/(G80(10)+80-(handle.G50_right_slider5.Value+50)),2)));
            set(handle.left_CR1,'string',num2str(round((80-50)/(G80(1)+80-(handle.G50_left_slider1.Value+50)),2)));
            set(handle.left_CR2,'string',num2str(round((80-50)/(G80(2)+80-(handle.G50_left_slider2.Value+50)),2)));
            set(handle.left_CR3,'string',num2str(round((80-50)/(G80(3)+80-(handle.G50_left_slider3.Value+50)),2)));
            set(handle.left_CR4,'string',num2str(round((80-50)/(G80(4)+80-(handle.G50_left_slider4.Value+50)),2)));
            set(handle.left_CR5,'string',num2str(round((80-50)/(G80(5)+80-(handle.G50_left_slider5.Value+50)),2)));
        
        else
            set(handle.G50_all_slider_right,'value',val);
%             set(handle.G50_all_edit_right,'string',[num2str(val) ' dB']);

            if dat.g50.ref_right(1) + val > get(handle.G50_right_slider1,'max')
                set(handle.G50_right_edit1,'string',[num2str(get(handle.G50_right_slider1,'max')) ' dB']);
                set(handle.G50_right_slider1,'value',get(handle.G50_right_slider1,'max'));
                G50_6 = get(handle.G50_right_slider1,'max');
            elseif dat.g50.ref_right(1) + val < get(handle.G50_right_slider1,'min')
                set(handle.G50_right_edit1,'string',[num2str(get(handle.G50_right_slider1,'min')) ' dB']);
                set(handle.G50_right_slider1,'value',get(handle.G50_right_slider1,'min'));
                G50_6 = get(handle.G50_right_slider1,'min');
            else 
                set(handle.G50_right_slider1,'value',dat.g50.ref_right(1) + val);
                set(handle.G50_right_edit1,'string',[num2str(dat.g50.ref_right(1) + val) ' dB']);
                G50_6 = dat.g50.ref_right(1) + val;
            end
            if dat.g50.ref_right(2) + val > get(handle.G50_right_slider2,'max')
                set(handle.G50_right_edit2,'string',[num2str(get(handle.G50_right_slider2,'max')) ' dB']);
                set(handle.G50_right_slider2,'value',get(handle.G50_right_slider2,'max'));
                G50_7 = get(handle.G50_right_slider2,'max');
            elseif dat.g50.ref_right(2) + val < get(handle.G50_right_slider2,'min')
                set(handle.G50_right_edit2,'string',[num2str(get(handle.G50_right_slider2,'min')) ' dB']);
                set(handle.G50_right_slider1,'value',get(handle.G50_right_slider2,'min'));
                G50_7 = get(handle.G50_right_slider2,'min');
            else 
                set(handle.G50_right_slider2,'value',dat.g50.ref_right(2) + val);
                set(handle.G50_right_edit2,'string',[num2str(dat.g50.ref_right(2) + val) ' dB']);
                G50_7 = dat.g50.ref_right(2) + val;
            end
            if dat.g50.ref_right(3) + val > get(handle.G50_right_slider3,'max')
                set(handle.G50_right_edit3,'string',[num2str(get(handle.G50_right_slider3,'max')) ' dB']);
                set(handle.G50_right_slider3,'value',get(handle.G50_right_slider3,'max'));
                G50_8 = get(handle.G50_right_slider3,'max');
            elseif dat.g50.ref_right(3) + val < get(handle.G50_right_slider3,'min')
                set(handle.G50_right_edit3,'string',[num2str(get(handle.G50_right_slider3,'min')) ' dB']);
                set(handle.G50_right_slider3,'value',get(handle.G50_right_slider3,'min'));
                G50_8 = get(handle.G50_right_slider3,'min');
            else 
                set(handle.G50_right_slider3,'value',dat.g50.ref_right(3) + val);
                set(handle.G50_right_edit3,'string',[num2str(dat.g50.ref_right(3) + val) ' dB']);
                G50_8 = dat.g50.ref_right(3) + val;
            end
            if dat.g50.ref_right(4) + val > get(handle.G50_right_slider4,'max')
                set(handle.G50_right_edit4,'string',[num2str(get(handle.G50_right_slider4,'max')) ' dB']);
                set(handle.G50_right_slider4,'value',get(handle.G50_right_slider4,'max'));
                G50_9 = get(handle.G50_right_slider4,'max');
            elseif dat.g50.ref_right(4) + val < get(handle.G50_right_slider4,'min')
                set(handle.G50_right_edit4,'string',[num2str(get(handle.G50_right_slider4,'min')) ' dB']);
                set(handle.G50_right_slider4,'value',get(handle.G50_right_slider4,'min'));
                G50_9 = get(handle.G50_right_slider4,'min');
            else 
                set(handle.G50_right_slider4,'value',dat.g50.ref_right(4) + val);
                set(handle.G50_right_edit4,'string',[num2str(dat.g50.ref_right(4) + val) ' dB']);
                G50_9 = dat.g50.ref_right(4) + val;
            end
            if dat.g50.ref_right(5) + val > get(handle.G50_right_slider5,'max')
                set(handle.G50_right_edit5,'string',[num2str(get(handle.G50_right_slider5,'max')) ' dB']);
                set(handle.G50_right_slider5,'value',get(handle.G50_right_slider5,'max'));
                G50_10 = get(handle.G50_right_slider5,'max');
            elseif dat.g50.ref_right(5) + val < get(handle.G50_right_slider5,'min')
                set(handle.G50_right_edit5,'string',[num2str(get(handle.G50_right_slider5,'min')) ' dB']);
                set(handle.G50_right_slider5,'value',get(handle.G50_right_slider5,'min'));
                G50_10 = get(handle.G50_right_slider5,'min');
            else 
                set(handle.G50_right_slider5,'value',dat.g50.ref_right(5) + val);
                set(handle.G50_right_edit5,'string',[num2str(dat.g50.ref_right(5) + val) ' dB']);
                G50_10 = dat.g50.ref_right(5) + val;
            end
            if handle.G50_all_slider_right.Value == handle.G50_all_slider_right.Min && handle.G80_all_slider_right.Value == handle.G80_all_slider_right.Min
                set([handle.all_gain_rightm11 handle.all_gain_rightm12 handle.all_gain_rightm13 handle.all_gain_rightm14 handle.all_gain_rightm15...
                    handle.all_gain_rightm31 handle.all_gain_rightm32 handle.all_gain_rightm33 handle.all_gain_rightm34 handle.all_gain_rightm35...
                    handle.all_bb_rightm1 handle.all_bb_rightm3],'enable','off');
            elseif handle.G50_all_slider_right.Value == handle.G50_all_slider_right.Max && handle.G80_all_slider_right.Value == handle.G80_all_slider_right.Max
                set([handle.all_gain_rightp31 handle.all_gain_rightp32 handle.all_gain_rightp33 handle.all_gain_rightp34 handle.all_gain_rightp35...
                    handle.all_gain_rightp11 handle.all_gain_rightp12 handle.all_gain_rightp13 handle.all_gain_rightp14 handle.all_gain_rightp15...
                    handle.all_bb_rightp1 handle.all_bb_rightp3],'enable','off');
                set([handle.all_gain_rightm11 handle.all_gain_rightm12 handle.all_gain_rightm13 handle.all_gain_rightm14 handle.all_gain_rightm15...
                    handle.all_gain_rightm31 handle.all_gain_rightm32 handle.all_gain_rightm33 handle.all_gain_rightm34 handle.all_gain_rightm35...
                    handle.all_bb_rightm1 handle.all_bb_rightm3],'enable','on');
            else
                set([handle.all_gain_rightm11 handle.all_gain_rightm12 handle.all_gain_rightm13 handle.all_gain_rightm14 handle.all_gain_rightm15...
                    handle.all_gain_rightm31 handle.all_gain_rightm32 handle.all_gain_rightm33 handle.all_gain_rightm34 handle.all_gain_rightm35...
                    handle.all_gain_rightp31 handle.all_gain_rightp32 handle.all_gain_rightp33 handle.all_gain_rightp34 handle.all_gain_rightp35...
                    handle.all_gain_rightp11 handle.all_gain_rightp12 handle.all_gain_rightp13 handle.all_gain_rightp14 handle.all_gain_rightp15...
                    handle.all_bb_rightm1 handle.all_bb_rightm3 handle.all_bb_rightp1 handle.all_bb_rightp3],'enable','on');
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50(1:5) G50_6 G50_7 G50_8 G50_9 G50_10]);
            set(handle.right_CR1,'string',num2str(round((80-50)/(G80(6)+80-(handle.G50_right_slider1.Value+50)),2)));
            set(handle.right_CR2,'string',num2str(round((80-50)/(G80(7)+80-(handle.G50_right_slider2.Value+50)),2)));
            set(handle.right_CR3,'string',num2str(round((80-50)/(G80(8)+80-(handle.G50_right_slider3.Value+50)),2)));
            set(handle.right_CR4,'string',num2str(round((80-50)/(G80(9)+80-(handle.G50_right_slider4.Value+50)),2)));
            set(handle.right_CR5,'string',num2str(round((80-50)/(G80(10)+80-(handle.G50_right_slider5.Value+50)),2)));
            set(handle.all_bb_right,'string','0 dB');
        end
    case 'all_left'
        val = get(handle.G50_all_slider_left,'value');
        val = round(val);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.G50_all_slider_right,'value',val);
%             set(handle.G50_all_edit_right,'string',[num2str(val) ' dB']);
            set(handle.G50_all_slider_left,'value',val);
%             set(handle.G50_all_edit_left,'string',[num2str(val) ' dB']);

            if dat.g50.ref_right(1) + val > get(handle.G50_right_slider1,'max')
                set(handle.G50_right_edit1,'string',[num2str(get(handle.G50_right_slider1,'max')) ' dB']);
                set(handle.G50_right_slider1,'value',get(handle.G50_right_slider1,'max'));
                G50_6 = get(handle.G50_right_slider1,'max');
            elseif dat.g50.ref_right(1) + val < get(handle.G50_right_slider1,'min')
                set(handle.G50_right_edit1,'string',[num2str(get(handle.G50_right_slider1,'min')) ' dB']);
                set(handle.G50_right_slider1,'value',get(handle.G50_right_slider1,'min'));
                G50_6 = get(handle.G50_right_slider1,'min');
            else 
                set(handle.G50_right_slider1,'value',dat.g50.ref_right(1) + val);
                set(handle.G50_right_edit1,'string',[num2str(dat.g50.ref_right(1) + val) ' dB']);
                G50_6 = dat.g50.ref_right(1) + val;
            end
            if dat.g50.ref_right(2) + val > get(handle.G50_right_slider2,'max')
                set(handle.G50_right_edit2,'string',[num2str(get(handle.G50_right_slider2,'max')) ' dB']);
                set(handle.G50_right_slider2,'value',get(handle.G50_right_slider2,'max'));
                G50_7 = get(handle.G50_right_slider2,'max');
            elseif dat.g50.ref_right(2) + val < get(handle.G50_right_slider2,'min')
                set(handle.G50_right_edit2,'string',[num2str(get(handle.G50_right_slider2,'min')) ' dB']);
                set(handle.G50_right_slider1,'value',get(handle.G50_right_slider2,'min'));
                G50_7 = get(handle.G50_right_slider2,'min');
            else 
                set(handle.G50_right_slider2,'value',dat.g50.ref_right(2) + val);
                set(handle.G50_right_edit2,'string',[num2str(dat.g50.ref_right(2) + val) ' dB']);
                G50_7 = dat.g50.ref_right(2) + val;
            end
            if dat.g50.ref_right(3) + val > get(handle.G50_right_slider3,'max')
                set(handle.G50_right_edit3,'string',[num2str(get(handle.G50_right_slider3,'max')) ' dB']);
                set(handle.G50_right_slider3,'value',get(handle.G50_right_slider3,'max'));
                G50_8 = get(handle.G50_right_slider3,'max');
            elseif dat.g50.ref_right(3) + val < get(handle.G50_right_slider3,'min')
                set(handle.G50_right_edit3,'string',[num2str(get(handle.G50_right_slider3,'min')) ' dB']);
                set(handle.G50_right_slider3,'value',get(handle.G50_right_slider3,'min'));
                G50_8 = get(handle.G50_right_slider3,'min');
            else 
                set(handle.G50_right_slider3,'value',dat.g50.ref_right(3) + val);
                set(handle.G50_right_edit3,'string',[num2str(dat.g50.ref_right(3) + val) ' dB']);
                G50_8 = dat.g50.ref_right(3) + val;
            end
            if dat.g50.ref_right(4) + val > get(handle.G50_right_slider4,'max')
                set(handle.G50_right_edit4,'string',[num2str(get(handle.G50_right_slider4,'max')) ' dB']);
                set(handle.G50_right_slider4,'value',get(handle.G50_right_slider4,'max'));
                G50_9 = get(handle.G50_right_slider4,'max');
            elseif dat.g50.ref_right(4) + val < get(handle.G50_right_slider4,'min')
                set(handle.G50_right_edit4,'string',[num2str(get(handle.G50_right_slider4,'min')) ' dB']);
                set(handle.G50_right_slider4,'value',get(handle.G50_right_slider4,'min'));
                G50_9 = get(handle.G50_right_slider4,'min');
            else 
                set(handle.G50_right_slider4,'value',dat.g50.ref_right(4) + val);
                set(handle.G50_right_edit4,'string',[num2str(dat.g50.ref_right(4) + val) ' dB']);
                G50_9 = dat.g50.ref_right(4) + val;
            end
            if dat.g50.ref_right(5) + val > get(handle.G50_right_slider5,'max')
                set(handle.G50_right_edit5,'string',[num2str(get(handle.G50_right_slider5,'max')) ' dB']);
                set(handle.G50_right_slider5,'value',get(handle.G50_right_slider5,'max'));
                G50_10 = get(handle.G50_right_slider5,'max');
            elseif dat.g50.ref_right(5) + val < get(handle.G50_right_slider5,'min')
                set(handle.G50_right_edit5,'string',[num2str(get(handle.G50_right_slider5,'min')) ' dB']);
                set(handle.G50_right_slider5,'value',get(handle.G50_right_slider5,'min'));
                G50_10 = get(handle.G50_right_slider5,'min');
            else 
                set(handle.G50_right_slider5,'value',dat.g50.ref_right(5) + val);
                set(handle.G50_right_edit5,'string',[num2str(dat.g50.ref_right(5) + val) ' dB']);
                G50_10 = dat.g50.ref_right(5) + val;
            end
            
            
            if dat.g50.ref_left(1) + val > get(handle.G50_left_slider1,'max')
                set(handle.G50_left_edit1,'string',[num2str(get(handle.G50_left_slider1,'max')) ' dB']);
                set(handle.G50_left_slider1,'value',get(handle.G50_left_slider1,'max'));
                G50_1 = get(handle.G50_left_slider1,'max');
            elseif dat.g50.ref_left(1) + val < get(handle.G50_left_slider1,'min')
                set(handle.G50_left_edit1,'string',[num2str(get(handle.G50_left_slider1,'min')) ' dB']);
                set(handle.G50_left_slider1,'value',get(handle.G50_left_slider1,'min'));
                G50_1 = get(handle.G50_left_slider1,'min');
            else 
                set(handle.G50_left_slider1,'value',dat.g50.ref_left(1) + val);
                set(handle.G50_left_edit1,'string',[num2str(dat.g50.ref_left(1) + val) ' dB']);
                G50_1 = dat.g50.ref_left(1) + val;
            end
            if dat.g50.ref_left(2) + val > get(handle.G50_left_slider2,'max')
                set(handle.G50_left_edit2,'string',[num2str(get(handle.G50_left_slider2,'max')) ' dB']);
                set(handle.G50_left_slider2,'value',get(handle.G50_left_slider2,'max'));
                G50_2 = get(handle.G50_left_slider2,'max');
            elseif dat.g50.ref_left(2) + val < get(handle.G50_left_slider2,'min')
                set(handle.G50_left_edit2,'string',[num2str(get(handle.G50_left_slider2,'min')) ' dB']);
                set(handle.G50_left_slider2,'value',get(handle.G50_left_slider2,'min'));
                G50_2 = get(handle.G50_left_slider2,'min');
            else 
                set(handle.G50_left_slider2,'value',dat.g50.ref_left(2) + val);
                set(handle.G50_left_edit2,'string',[num2str(dat.g50.ref_left(2) + val) ' dB']);
                G50_2 = dat.g50.ref_left(2) + val;
            end
            if dat.g50.ref_left(3) + val > get(handle.G50_left_slider3,'max')
                set(handle.G50_left_edit3,'string',[num2str(get(handle.G50_left_slider3,'max')) ' dB']);
                set(handle.G50_left_slider3,'value',get(handle.G50_left_slider3,'max'));
                G50_3 = get(handle.G50_left_slider3,'max');
            elseif dat.g50.ref_left(3) + val < get(handle.G50_left_slider3,'min')
                set(handle.G50_left_edit3,'string',[num2str(get(handle.G50_left_slider3,'min')) ' dB']);
                set(handle.G50_left_slider3,'value',get(handle.G50_left_slider3,'min'));
                G50_3 = get(handle.G50_left_slider3,'min');
            else 
                set(handle.G50_left_slider3,'value',dat.g50.ref_left(3) + val);
                set(handle.G50_left_edit3,'string',[num2str(dat.g50.ref_left(3) + val) ' dB']);
                G50_3 = dat.g50.ref_left(3) + val;
            end
            if dat.g50.ref_left(4) + val > get(handle.G50_left_slider4,'max')
                set(handle.G50_left_edit4,'string',[num2str(get(handle.G50_left_slider4,'max')) ' dB']);
                set(handle.G50_left_slider4,'value',get(handle.G50_left_slider4,'max'));
                G50_4 = get(handle.G50_left_slider4,'max');
            elseif dat.g50.ref_left(4) + val < get(handle.G50_left_slider4,'min')
                set(handle.G50_left_edit4,'string',[num2str(get(handle.G50_left_slider4,'min')) ' dB']);
                set(handle.G50_left_slider4,'value',get(handle.G50_left_slider4,'min'));
                G50_4 = get(handle.G50_left_slider4,'min');
            else 
                set(handle.G50_left_slider4,'value',dat.g50.ref_left(4) + val);
                set(handle.G50_left_edit4,'string',[num2str(dat.g50.ref_left(4) + val) ' dB']);
                G50_4 = dat.g50.ref_left(4) + val;
            end
            if dat.g50.ref_left(5) + val > get(handle.G50_left_slider5,'max')
                set(handle.G50_left_edit5,'string',[num2str(get(handle.G50_left_slider5,'max')) ' dB']);
                set(handle.G50_left_slider5,'value',get(handle.G50_left_slider5,'max'));
                G50_5 = get(handle.G50_left_slider5,'max');
            elseif dat.g50.ref_left(5) + val < get(handle.G50_left_slider5,'min')
                set(handle.G50_left_edit5,'string',[num2str(get(handle.G50_left_slider5,'min')) ' dB']);
                set(handle.G50_left_slider5,'value',get(handle.G50_left_slider5,'min'));
                G50_5 = get(handle.G50_left_slider5,'min');
            else 
                set(handle.G50_left_slider5,'value',dat.g50.ref_left(5) + val);
                set(handle.G50_left_edit5,'string',[num2str(dat.g50.ref_left(5) + val) ' dB']);
                G50_5 = dat.g50.ref_left(5) + val;
            end

            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50_1 G50_2 G50_3 G50_4 G50_5 G50_6 G50_7 G50_8 G50_9 G50_10]);
        
            set(handle.right_CR1,'string',num2str(round((80-50)/(G80(6)+80-(handle.G50_right_slider1.Value+50)),2)));
            set(handle.right_CR2,'string',num2str(round((80-50)/(G80(7)+80-(handle.G50_right_slider2.Value+50)),2)));
            set(handle.right_CR3,'string',num2str(round((80-50)/(G80(8)+80-(handle.G50_right_slider3.Value+50)),2)));
            set(handle.right_CR4,'string',num2str(round((80-50)/(G80(9)+80-(handle.G50_right_slider4.Value+50)),2)));
            set(handle.right_CR5,'string',num2str(round((80-50)/(G80(10)+80-(handle.G50_right_slider5.Value+50)),2)));
            set(handle.left_CR1,'string',num2str(round((80-50)/(G80(1)+80-(handle.G50_left_slider1.Value+50)),2)));
            set(handle.left_CR2,'string',num2str(round((80-50)/(G80(2)+80-(handle.G50_left_slider2.Value+50)),2)));
            set(handle.left_CR3,'string',num2str(round((80-50)/(G80(3)+80-(handle.G50_left_slider3.Value+50)),2)));
            set(handle.left_CR4,'string',num2str(round((80-50)/(G80(4)+80-(handle.G50_left_slider4.Value+50)),2)));
            set(handle.left_CR5,'string',num2str(round((80-50)/(G80(5)+80-(handle.G50_left_slider5.Value+50)),2)));
        else
            set(handle.G50_all_slider_left,'value',val);
%             set(handle.G50_all_edit_left,'string',[num2str(val) ' dB']);

            if dat.g50.ref_left(1) + val > get(handle.G50_left_slider1,'max')
                set(handle.G50_left_edit1,'string',[num2str(get(handle.G50_left_slider1,'max')) ' dB']);
                set(handle.G50_left_slider1,'value',get(handle.G50_left_slider1,'max'));
                G50_1 = get(handle.G50_left_slider1,'max');
            elseif dat.g50.ref_left(1) + val < get(handle.G50_left_slider1,'min')
                set(handle.G50_left_edit1,'string',[num2str(get(handle.G50_left_slider1,'min')) ' dB']);
                set(handle.G50_left_slider1,'value',get(handle.G50_left_slider1,'min'));
                G50_1 = get(handle.G50_left_slider1,'min');
            else 
                set(handle.G50_left_slider1,'value',dat.g50.ref_left(1) + val);
                set(handle.G50_left_edit1,'string',[num2str(dat.g50.ref_left(1) + val) ' dB']);
                G50_1 = dat.g50.ref_left(1) + val;
            end
            if dat.g50.ref_left(2) + val > get(handle.G50_left_slider2,'max')
                set(handle.G50_left_edit2,'string',[num2str(get(handle.G50_left_slider2,'max')) ' dB']);
                set(handle.G50_left_slider2,'value',get(handle.G50_left_slider2,'max'));
                G50_2 = get(handle.G50_left_slider2,'max');
            elseif dat.g50.ref_left(2) + val < get(handle.G50_left_slider2,'min')
                set(handle.G50_left_edit2,'string',[num2str(get(handle.G50_left_slider2,'min')) ' dB']);
                set(handle.G50_left_slider2,'value',get(handle.G50_left_slider2,'min'));
                G50_2 = get(handle.G50_left_slider2,'min');
            else 
                set(handle.G50_left_slider2,'value',dat.g50.ref_left(2) + val);
                set(handle.G50_left_edit2,'string',[num2str(dat.g50.ref_left(2) + val) ' dB']);
                G50_2 = dat.g50.ref_left(2) + val;
            end
            if dat.g50.ref_left(3) + val > get(handle.G50_left_slider3,'max')
                set(handle.G50_left_edit3,'string',[num2str(get(handle.G50_left_slider3,'max')) ' dB']);
                set(handle.G50_left_slider3,'value',get(handle.G50_left_slider3,'max'));
                G50_3 = get(handle.G50_left_slider3,'max');
            elseif dat.g50.ref_left(3) + val < get(handle.G50_left_slider3,'min')
                set(handle.G50_left_edit3,'string',[num2str(get(handle.G50_left_slider3,'min')) ' dB']);
                set(handle.G50_left_slider3,'value',get(handle.G50_left_slider3,'min'));
                G50_3 = get(handle.G50_left_slider3,'min');
            else 
                set(handle.G50_left_slider3,'value',dat.g50.ref_left(3) + val);
                set(handle.G50_left_edit3,'string',[num2str(dat.g50.ref_left(3) + val) ' dB']);
                G50_3 = dat.g50.ref_left(3) + val;
            end
            if dat.g50.ref_left(4) + val > get(handle.G50_left_slider4,'max')
                set(handle.G50_left_edit4,'string',[num2str(get(handle.G50_left_slider4,'max')) ' dB']);
                set(handle.G50_left_slider4,'value',get(handle.G50_left_slider4,'max'));
                G50_4 = get(handle.G50_left_slider4,'max');
            elseif dat.g50.ref_left(4) + val < get(handle.G50_left_slider4,'min')
                set(handle.G50_left_edit4,'string',[num2str(get(handle.G50_left_slider4,'min')) ' dB']);
                set(handle.G50_left_slider4,'value',get(handle.G50_left_slider4,'min'));
                G50_4 = get(handle.G50_left_slider4,'min');
            else 
                set(handle.G50_left_slider4,'value',dat.g50.ref_left(4) + val);
                set(handle.G50_left_edit4,'string',[num2str(dat.g50.ref_left(4) + val) ' dB']);
                G50_4 = dat.g50.ref_left(4) + val;
            end
            if dat.g50.ref_left(5) + val > get(handle.G50_left_slider5,'max')
                set(handle.G50_left_edit5,'string',[num2str(get(handle.G50_left_slider5,'max')) ' dB']);
                set(handle.G50_left_slider5,'value',get(handle.G50_left_slider5,'max'));
                G50_5 = get(handle.G50_left_slider5,'max');
            elseif dat.g50.ref_left(5) + val < get(handle.G50_left_slider5,'min')
                set(handle.G50_left_edit5,'string',[num2str(get(handle.G50_left_slider5,'min')) ' dB']);
                set(handle.G50_left_slider5,'value',get(handle.G50_left_slider5,'min'));
                G50_5 = get(handle.G50_left_slider5,'min');
            else 
                set(handle.G50_left_slider5,'value',dat.g50.ref_left(5) + val);
                set(handle.G50_left_edit5,'string',[num2str(dat.g50.ref_left(5) + val) ' dB']);
                G50_5 = dat.g50.ref_left(5) + val;
            end
            if handle.G50_all_slider_left.Value == handle.G50_all_slider_left.Min && handle.G80_all_slider_left.Value == handle.G80_all_slider_left.Min
                set([handle.all_gain_leftm11 handle.all_gain_leftm12 handle.all_gain_leftm13 handle.all_gain_leftm14 handle.all_gain_leftm15...
                    handle.all_gain_leftm31 handle.all_gain_leftm32 handle.all_gain_leftm33 handle.all_gain_leftm34 handle.all_gain_leftm35...
                    handle.all_bb_leftm1 handle.all_bb_leftm3],'enable','off');
            elseif handle.G50_all_slider_left.Value == handle.G50_all_slider_left.Max && handle.G80_all_slider_left.Value == handle.G80_all_slider_left.Max
                set([handle.all_gain_leftp31 handle.all_gain_leftp32 handle.all_gain_leftp33 handle.all_gain_leftp34 handle.all_gain_leftp35...
                    handle.all_gain_leftp11 handle.all_gain_leftp12 handle.all_gain_leftp13 handle.all_gain_leftp14 handle.all_gain_leftp15...
                    handle.all_bb_leftp1 handle.all_bb_leftp3],'enable','off');
                set([handle.all_gain_leftm11 handle.all_gain_leftm12 handle.all_gain_leftm13 handle.all_gain_leftm14 handle.all_gain_leftm15...
                    handle.all_gain_leftm31 handle.all_gain_leftm32 handle.all_gain_leftm33 handle.all_gain_leftm34 handle.all_gain_leftm35...
                    handle.all_bb_leftm1 handle.all_bb_leftm3],'enable','on');
            else
                set([handle.all_gain_leftm11 handle.all_gain_leftm12 handle.all_gain_leftm13 handle.all_gain_leftm14 handle.all_gain_leftm15...
                    handle.all_gain_leftm31 handle.all_gain_leftm32 handle.all_gain_leftm33 handle.all_gain_leftm34 handle.all_gain_leftm35...
                    handle.all_gain_leftp31 handle.all_gain_leftp32 handle.all_gain_leftp33 handle.all_gain_leftp34 handle.all_gain_leftp35...
                    handle.all_gain_leftp11 handle.all_gain_leftp12 handle.all_gain_leftp13 handle.all_gain_leftp14 handle.all_gain_leftp15...
                    handle.all_bb_leftm1 handle.all_bb_leftm3 handle.all_bb_leftp1 handle.all_bb_leftp3],'enable','on');
            end
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',...
                [G50_1 G50_2 G50_3 G50_4 G50_5 G50(6:10)]);
            set(handle.left_CR1,'string',num2str(round((80-50)/(G80(1)+80-(handle.G50_left_slider1.Value+50)),2)));
            set(handle.left_CR2,'string',num2str(round((80-50)/(G80(2)+80-(handle.G50_left_slider2.Value+50)),2)));
            set(handle.left_CR3,'string',num2str(round((80-50)/(G80(3)+80-(handle.G50_left_slider3.Value+50)),2)));
            set(handle.left_CR4,'string',num2str(round((80-50)/(G80(4)+80-(handle.G50_left_slider4.Value+50)),2)));
            set(handle.left_CR5,'string',num2str(round((80-50)/(G80(5)+80-(handle.G50_left_slider5.Value+50)),2)));
        end
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
