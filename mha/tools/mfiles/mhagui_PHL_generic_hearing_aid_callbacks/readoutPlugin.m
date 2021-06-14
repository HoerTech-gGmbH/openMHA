function [handle] = readoutPlugin(handle, plugin)
mha = handle.mha;

switch plugin
    case 'bbc'
        sel = mha_get(mha,'mha.transducers.calib_out.do_clipping');
        if sel == 1
            set(handle.rb.bbc_off,'value',0);
            set(handle.rb.bbc_on,'value',1);
        else
            set(handle.rb.bbc_off,'value',1);
            set(handle.rb.bbc_on,'value',0);
        end
    case 'adm'
        sel = mha_get(mha,'mha.transducers.mhachain.split.bte.adm.bypass');
        if sel == 1
            set(handle.rb.adm_off,'value',1);
            set(handle.rb.adm_on,'value',0);
        else
            set(handle.rb.adm_off,'value',0);
            set(handle.rb.adm_on,'value',1);
        end
    case 'coh'
        sel = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.coh.select');
        if isequal(sel,'identity')
            set(handle.rb.coh_off,'value',1)
            set(handle.rb.coh_on,'value',0)
        else
            set(handle.rb.coh_on,'value',1)
            set(handle.rb.coh_off,'value',0)
        end
    case 'fshift'
        sel = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.fshift.select');
        if isequal(sel,'identity')
            set(handle.rb.fshift_off,'value',1)
            set(handle.rb.fshift_on,'value',0)
        else
            set(handle.rb.fshift_on,'value',1)
            set(handle.rb.fshift_off,'value',0)
        end
    case 'mbc'
        sel = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.select');
        if isequal(sel,'identity')
            set(handle.rb.mbc_off,'value',1)
            set(handle.rb.mbc_on,'value',0)
            set(handle.gain_right.Children(6:end-2),'enable','off');
            set(handle.mpo_right.Children,'enable','off');
            set(handle.exp_right.Children,'enable','off');
            set(handle.gain_left.Children(6:end-2),'enable','off');
            set(handle.mpo_left.Children,'enable','off');
            set(handle.exp_left.Children,'enable','off');
        else
            set(handle.rb.mbc_on,'value',1)
            set(handle.rb.mbc_off,'value',0)
            set(handle.gain_right.Children(6:end-2),'enable','on');
            set(handle.mpo_right.Children,'enable','on');
            set(handle.exp_right.Children,'enable','on');
            set(handle.gain_left.Children(6:end-2),'enable','on');
            set(handle.mpo_left.Children,'enable','on');
            set(handle.exp_left.Children,'enable','on');
        end

    case 'mpo'
        mpo = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold');
        % left HA
        set(handle.mpo_left_edit1,'string',[num2str(mpo(1)) ' dB']);
        set(handle.mpo_left_edit2,'string',[num2str(mpo(2)) ' dB']);
        set(handle.mpo_left_edit3,'string',[num2str(mpo(3)) ' dB']);
        set(handle.mpo_left_edit4,'string',[num2str(mpo(4)) ' dB']);
        set(handle.mpo_left_edit5,'string',[num2str(mpo(5)) ' dB']);

        set(handle.mpo_left_slider1,'value',mpo(1));
        set(handle.mpo_left_slider2,'value',mpo(2));
        set(handle.mpo_left_slider3,'value',mpo(3));
        set(handle.mpo_left_slider4,'value',mpo(4));
        set(handle.mpo_left_slider5,'value',mpo(5));
        % right HA
        set(handle.mpo_right_edit1,'string',[num2str(mpo(6)) ' dB']);
        set(handle.mpo_right_edit2,'string',[num2str(mpo(7)) ' dB']);
        set(handle.mpo_right_edit3,'string',[num2str(mpo(8)) ' dB']);
        set(handle.mpo_right_edit4,'string',[num2str(mpo(9)) ' dB']);
        set(handle.mpo_right_edit5,'string',[num2str(mpo(10)) ' dB']);
        
        set(handle.mpo_right_slider1,'value',mpo(6));
        set(handle.mpo_right_slider2,'value',mpo(7));
        set(handle.mpo_right_slider3,'value',mpo(8));
        set(handle.mpo_right_slider4,'value',mpo(9));
        set(handle.mpo_right_slider5,'value',mpo(10));
    case 'gain'
        G80 = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80');
        % left HA
        set(handle.G80_left_edit1,'string',[num2str(G80(1)) ' dB']);
        set(handle.G80_left_edit2,'string',[num2str(G80(2)) ' dB']);
        set(handle.G80_left_edit3,'string',[num2str(G80(3)) ' dB']);
        set(handle.G80_left_edit4,'string',[num2str(G80(4)) ' dB']);
        set(handle.G80_left_edit5,'string',[num2str(G80(5)) ' dB']);

        set(handle.G80_left_slider1,'value',G80(1));
        set(handle.G80_left_slider2,'value',G80(2));
        set(handle.G80_left_slider3,'value',G80(3));
        set(handle.G80_left_slider4,'value',G80(4));
        set(handle.G80_left_slider5,'value',G80(5));
        
        % right HA
        set(handle.G80_right_edit1,'string',[num2str(G80(6)) ' dB']);
        set(handle.G80_right_edit2,'string',[num2str(G80(7)) ' dB']);
        set(handle.G80_right_edit3,'string',[num2str(G80(8)) ' dB']);
        set(handle.G80_right_edit4,'string',[num2str(G80(9)) ' dB']);
        set(handle.G80_right_edit5,'string',[num2str(G80(10)) ' dB']);
        
        set(handle.G80_right_slider1,'value',G80(6));
        set(handle.G80_right_slider2,'value',G80(7));
        set(handle.G80_right_slider3,'value',G80(8));
        set(handle.G80_right_slider4,'value',G80(9));
        set(handle.G80_right_slider5,'value',G80(10));
  
        G50 = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50');
        % left HA
        set(handle.G50_left_edit1,'string',[num2str(G50(1)) ' dB']);
        set(handle.G50_left_edit2,'string',[num2str(G50(2)) ' dB']);
        set(handle.G50_left_edit3,'string',[num2str(G50(3)) ' dB']);
        set(handle.G50_left_edit4,'string',[num2str(G50(4)) ' dB']);
        set(handle.G50_left_edit5,'string',[num2str(G50(5)) ' dB']);

        set(handle.G50_left_slider1,'value',G50(1));
        set(handle.G50_left_slider2,'value',G50(2));
        set(handle.G50_left_slider3,'value',G50(3));
        set(handle.G50_left_slider4,'value',G50(4));
        set(handle.G50_left_slider5,'value',G50(5));
        
        % right HA
        set(handle.G50_right_edit1,'string',[num2str(G50(6)) ' dB']);
        set(handle.G50_right_edit2,'string',[num2str(G50(7)) ' dB']);
        set(handle.G50_right_edit3,'string',[num2str(G50(8)) ' dB']);
        set(handle.G50_right_edit4,'string',[num2str(G50(9)) ' dB']);
        set(handle.G50_right_edit5,'string',[num2str(G50(10)) ' dB']);
        
        set(handle.G50_right_slider1,'value',G50(6));
        set(handle.G50_right_slider2,'value',G50(7));
        set(handle.G50_right_slider3,'value',G50(8));
        set(handle.G50_right_slider4,'value',G50(9));
        set(handle.G50_right_slider5,'value',G50(10));
        
        CR = round((80-50)./(G80+80-(G50+50)),2);
        set(handle.left_CR1,'string',num2str(CR(1)));
        set(handle.left_CR2,'string',num2str(CR(2)));
        set(handle.left_CR3,'string',num2str(CR(3)));
        set(handle.left_CR4,'string',num2str(CR(4)));
        set(handle.left_CR5,'string',num2str(CR(5)));
        
        set(handle.right_CR1,'string',num2str(CR(6)));
        set(handle.right_CR2,'string',num2str(CR(7)));
        set(handle.right_CR3,'string',num2str(CR(8)));
        set(handle.right_CR4,'string',num2str(CR(9)));
        set(handle.right_CR5,'string',num2str(CR(10)));

        maxgain = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain');
        %left HA
        set(handle.maxgain_left_edit1,'string',[num2str(maxgain(1)) ' dB']);
        set(handle.maxgain_left_edit2,'string',[num2str(maxgain(2)) ' dB']);
        set(handle.maxgain_left_edit3,'string',[num2str(maxgain(3)) ' dB']);
        set(handle.maxgain_left_edit4,'string',[num2str(maxgain(4)) ' dB']);
        set(handle.maxgain_left_edit5,'string',[num2str(maxgain(5)) ' dB']);
        %right HA
        set(handle.maxgain_right_edit1,'string',[num2str(maxgain(6)) ' dB']);
        set(handle.maxgain_right_edit2,'string',[num2str(maxgain(7)) ' dB']);
        set(handle.maxgain_right_edit3,'string',[num2str(maxgain(8)) ' dB']);
        set(handle.maxgain_right_edit4,'string',[num2str(maxgain(9)) ' dB']);
        set(handle.maxgain_right_edit5,'string',[num2str(maxgain(10)) ' dB']);
        
        % tau
        AT = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.tau_attack');
        RT = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.tau_decay');
        set(handle.RT_right_edit,'string',[num2str(RT) ' s']);
        set(handle.RT_left_edit,'string',[num2str(RT) ' s']);
        set(handle.AT_right_edit,'string',[num2str(AT) ' s']);
        set(handle.AT_left_edit,'string',[num2str(AT) ' s']);        
    case 'exp'
        exp = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold');
        slope = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_slope');
        
        set(handle.exp_left_edit1,'string', [num2str(exp(1)) ' dB']);
        set(handle.exp_left_edit2,'string', [num2str(exp(2)) ' dB']);
        set(handle.exp_left_edit3,'string', [num2str(exp(3)) ' dB']);
        set(handle.exp_left_edit4,'string', [num2str(exp(4)) ' dB']);
        set(handle.exp_left_edit5,'string', [num2str(exp(5)) ' dB']);
        
        set(handle.exp_right_edit1,'string', [num2str(exp(6)) ' dB']);
        set(handle.exp_right_edit2,'string', [num2str(exp(7)) ' dB']);
        set(handle.exp_right_edit3,'string', [num2str(exp(8)) ' dB']);
        set(handle.exp_right_edit4,'string', [num2str(exp(9)) ' dB']);
        set(handle.exp_right_edit5,'string', [num2str(exp(10)) ' dB']);
        
        set(handle.slope_right_edit,'string',num2str(slope));
        set(handle.slope_left_edit,'string',num2str(slope));
end
% handles = plot_data_right(handles,mha);
% handles = plot_data_left(handles,mha);
end
