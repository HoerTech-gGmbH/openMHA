function handle = mpo_slider_cllb(src,event)
handle = guidata(src);
mha = handle.mha;
mpo = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold');
slider_tag = get(gcbo,'tag');
switch slider_tag
    case 'slider_l_250Hz'
        val = get(handle.mpo_left_slider1,'value');
        val = round(val);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.mpo_left_slider1,'value',val);
            set(handle.mpo_left_edit1,'string',[num2str(val) ' dB']);
            set(handle.mpo_right_slider1,'value',val);
            set(handle.mpo_right_edit1,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [val mpo(2) mpo(3) mpo(4) mpo(5) val mpo(7) mpo(8) mpo(9) mpo(10)]); 
        else
            set(handle.mpo_left_slider1,'value',val);
            set(handle.mpo_left_edit1,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [val mpo(2) mpo(3) mpo(4) mpo(5) mpo(6) mpo(7) mpo(8) mpo(9) mpo(10)]);
        end
    case 'slider_l_500Hz'
        val = get(handle.mpo_left_slider2,'value');
        val = round(val);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.mpo_left_slider2,'value',val);
            set(handle.mpo_left_edit2,'string',[num2str(val) ' dB']);
            set(handle.mpo_right_slider2,'value',val);
            set(handle.mpo_right_edit2,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) val mpo(3) mpo(4) mpo(5) mpo(6) val mpo(8) mpo(9) mpo(10)]);
        else
            set(handle.mpo_left_slider2,'value',val);
            set(handle.mpo_left_edit2,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) val mpo(3) mpo(4) mpo(5) mpo(6) mpo(7) mpo(8) mpo(9) mpo(10)]);
        end
    case 'slider_l_1kHz'
        val = get(handle.mpo_left_slider3,'value');
        val = round(val);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.mpo_left_slider3,'value',val);
            set(handle.mpo_left_edit3,'string',[num2str(val) ' dB']);
            set(handle.mpo_right_slider3,'value',val);
            set(handle.mpo_right_edit3,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) val mpo(4) mpo(5) mpo(6) mpo(7) val mpo(9) mpo(10)]);
        else
            set(handle.mpo_left_slider3,'value',val);
            set(handle.mpo_left_edit3,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) val mpo(4) mpo(5) mpo(6) mpo(7) mpo(8) mpo(9) mpo(10)]);
        end
    case 'slider_l_2kHz'
        val = get(handle.mpo_left_slider4,'value');
        val = round(val);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.mpo_left_slider4,'value',val);
            set(handle.mpo_left_edit4,'string',[num2str(val) ' dB']);
            set(handle.mpo_right_slider4,'value',val);
            set(handle.mpo_right_edit4,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) mpo(3) val mpo(5) mpo(6) mpo(7) mpo(8) val mpo(10)]);
        else
            set(handle.mpo_left_slider4,'value',val);
            set(handle.mpo_left_edit4,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) mpo(3) val mpo(5) mpo(6) mpo(7) mpo(8) mpo(9) mpo(10)]);
        end
        
    case 'slider_l_4kHz'
        val = get(handle.mpo_left_slider5,'value');
        val = round(val);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.mpo_left_slider5,'value',val);
            set(handle.mpo_left_edit5,'string',[num2str(val) ' dB']);
            set(handle.mpo_right_slider5,'value',val);
            set(handle.mpo_right_edit5,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) mpo(3) mpo(4) val mpo(6) mpo(7) mpo(8) mpo(9) val]);
        else
            set(handle.mpo_left_slider5,'value',val);
            set(handle.mpo_left_edit5,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) mpo(3) mpo(4) val mpo(6) mpo(7) mpo(8) mpo(9) mpo(10)]);
        end

    case 'slider_r_250Hz'
        val = get(handle.mpo_right_slider1,'value');
        val = round(val);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.mpo_right_slider1,'value',val);
            set(handle.mpo_right_edit1,'string',[num2str(val) ' dB']);
            set(handle.mpo_left_slider1,'value',val);
            set(handle.mpo_left_edit1,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [val mpo(2) mpo(3) mpo(4) mpo(5) val mpo(7) mpo(8) mpo(9) mpo(10)]);
        else
            set(handle.mpo_right_slider1,'value',val);
            set(handle.mpo_right_edit1,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) mpo(3) mpo(4) mpo(5) val mpo(7) mpo(8) mpo(9) mpo(10)]);
        end
        
    case 'slider_r_500Hz'
        val = get(handle.mpo_right_slider2,'value');
        val = round(val);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.mpo_right_slider2,'value',val);
            set(handle.mpo_right_edit2,'string',[num2str(val) ' dB']);
            set(handle.mpo_left_slider2,'value',val);
            set(handle.mpo_left_edit2,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) val mpo(3) mpo(4) mpo(5) mpo(6) val mpo(8) mpo(9) mpo(10)]);
        else
            set(handle.mpo_right_slider2,'value',val);
            set(handle.mpo_right_edit2,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) mpo(3) mpo(4) mpo(5) mpo(6) val mpo(8) mpo(9) mpo(10)]);
        end
        
    case 'slider_r_1kHz'
        val = get(handle.mpo_right_slider3,'value');
        val = round(val);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.mpo_right_slider3,'value',val);
            set(handle.mpo_right_edit3,'string',[num2str(val) ' dB']);
            set(handle.mpo_left_slider3,'value',val);
            set(handle.mpo_left_edit3,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) val mpo(4) mpo(5) mpo(6) mpo(7) val mpo(9) mpo(10)]);
        else
            set(handle.mpo_right_slider3,'value',val);
            set(handle.mpo_right_edit3,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) mpo(3) mpo(4) mpo(5) mpo(6) mpo(7) val mpo(9) mpo(10)]);
        end
        
    case 'slider_r_2kHz'
        val = get(handle.mpo_right_slider4,'value');
        val = round(val);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.mpo_right_slider4,'value',val);
            set(handle.mpo_right_edit4,'string',[num2str(val) ' dB']);
            set(handle.mpo_left_slider4,'value',val);
            set(handle.mpo_left_edit4,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) mpo(3) val mpo(5) mpo(6) mpo(7) mpo(8) val mpo(10)]);
        else
            set(handle.mpo_right_slider4,'value',val);
            set(handle.mpo_right_edit4,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) mpo(3) mpo(4) mpo(5) mpo(6) mpo(7) mpo(8) val mpo(10)]);
        end
        
    case 'slider_r_4kHz'
        val = get(handle.mpo_right_slider5,'value');
        val = round(val);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.mpo_right_slider5,'value',val);
            set(handle.mpo_right_edit5,'string',[num2str(val) ' dB']);
            set(handle.mpo_left_slider5,'value',val);
            set(handle.mpo_left_edit5,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) mpo(3) mpo(4) val mpo(6) mpo(7) mpo(8) mpo(9) val]);
        else
            set(handle.mpo_right_slider5,'value',val);
            set(handle.mpo_right_edit5,'string',[num2str(val) ' dB']);
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) mpo(3) mpo(4) mpo(5) mpo(6) mpo(7) mpo(8) mpo(9) val]);
        end
        
    case 'all_right'
        val = get(handle.mpo_all_slider_right,'value');
        val = round(val);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.mpo_all_edit_right,'string',[num2str(val) ' dB']);
            set(handle.mpo_right_slider1,'value',val);
            set(handle.mpo_right_slider2,'value',val);
            set(handle.mpo_right_slider3,'value',val);
            set(handle.mpo_right_slider4,'value',val);
            set(handle.mpo_right_slider5,'value',val);
            set(handle.mpo_right_edit1,'string',[num2str(val) ' dB']);
            set(handle.mpo_right_edit2,'string',[num2str(val) ' dB']);
            set(handle.mpo_right_edit3,'string',[num2str(val) ' dB']);
            set(handle.mpo_right_edit4,'string',[num2str(val) ' dB']);
            set(handle.mpo_right_edit5,'string',[num2str(val) ' dB']);

            set(handle.mpo_all_edit_left,'string',[num2str(val) ' dB']);
            set(handle.mpo_all_slider_left,'value',val);
            set(handle.mpo_left_slider1,'value',val);
            set(handle.mpo_left_slider2,'value',val);
            set(handle.mpo_left_slider3,'value',val);
            set(handle.mpo_left_slider4,'value',val);
            set(handle.mpo_left_slider5,'value',val);
            set(handle.mpo_left_edit1,'string',[num2str(val) ' dB']);
            set(handle.mpo_left_edit2,'string',[num2str(val) ' dB']);
            set(handle.mpo_left_edit3,'string',[num2str(val) ' dB']);
            set(handle.mpo_left_edit4,'string',[num2str(val) ' dB']);
            set(handle.mpo_left_edit5,'string',[num2str(val) ' dB']);
            
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                repmat(val,1,10));
        else
            set(handle.mpo_all_edit_right,'string',[num2str(val) ' dB']);
            set(handle.mpo_right_slider1,'value',val);
            set(handle.mpo_right_slider2,'value',val);
            set(handle.mpo_right_slider3,'value',val);
            set(handle.mpo_right_slider4,'value',val);
            set(handle.mpo_right_slider5,'value',val);
            set(handle.mpo_right_edit1,'string',[num2str(val) ' dB']);
            set(handle.mpo_right_edit2,'string',[num2str(val) ' dB']);
            set(handle.mpo_right_edit3,'string',[num2str(val) ' dB']);
            set(handle.mpo_right_edit4,'string',[num2str(val) ' dB']);
            set(handle.mpo_right_edit5,'string',[num2str(val) ' dB']);

            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [mpo(1) mpo(2) mpo(3) mpo(4) mpo(5) repmat(val,1,5)]);
        end
    case 'all_left'
        val = get(handle.mpo_all_slider_left,'value');
        val = round(val);
        if isequal(get(handle.link,'tag'),'link')
            set(handle.mpo_all_edit_left,'string',[num2str(val) ' dB']);
            set(handle.mpo_left_slider1,'value',val);
            set(handle.mpo_left_slider2,'value',val);
            set(handle.mpo_left_slider3,'value',val);
            set(handle.mpo_left_slider4,'value',val);
            set(handle.mpo_left_slider5,'value',val);
            set(handle.mpo_left_edit1,'string',[num2str(val) ' dB']);
            set(handle.mpo_left_edit2,'string',[num2str(val) ' dB']);
            set(handle.mpo_left_edit3,'string',[num2str(val) ' dB']);
            set(handle.mpo_left_edit4,'string',[num2str(val) ' dB']);
            set(handle.mpo_left_edit5,'string',[num2str(val) ' dB']);

            set(handle.mpo_all_edit_right,'string',[num2str(val) ' dB']);
            set(handle.mpo_all_slider_right,'value',val);
            set(handle.mpo_right_slider1,'value',val);
            set(handle.mpo_right_slider2,'value',val);
            set(handle.mpo_right_slider3,'value',val);
            set(handle.mpo_right_slider4,'value',val);
            set(handle.mpo_right_slider5,'value',val);
            set(handle.mpo_right_edit1,'string',[num2str(val) ' dB']);
            set(handle.mpo_right_edit2,'string',[num2str(val) ' dB']);
            set(handle.mpo_right_edit3,'string',[num2str(val) ' dB']);
            set(handle.mpo_right_edit4,'string',[num2str(val) ' dB']);
            set(handle.mpo_right_edit5,'string',[num2str(val) ' dB']);
            
            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                repmat(val,1,10));
        else
            set(handle.mpo_all_edit_left,'string',[num2str(val) ' dB']);
            set(handle.mpo_left_slider1,'value',val);
            set(handle.mpo_left_slider2,'value',val);
            set(handle.mpo_left_slider3,'value',val);
            set(handle.mpo_left_slider4,'value',val);
            set(handle.mpo_left_slider5,'value',val);
            set(handle.mpo_left_edit1,'string',[num2str(val) ' dB']);
            set(handle.mpo_left_edit2,'string',[num2str(val) ' dB']);
            set(handle.mpo_left_edit3,'string',[num2str(val) ' dB']);
            set(handle.mpo_left_edit4,'string',[num2str(val) ' dB']);
            set(handle.mpo_left_edit5,'string',[num2str(val) ' dB']);

            mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',...
                [repmat(val,1,5) mpo(6) mpo(7) mpo(8) mpo(9) mpo(10)]);
        end
end
handle = plot_data_right(handle);
handle = plot_data_left(handle);
end
