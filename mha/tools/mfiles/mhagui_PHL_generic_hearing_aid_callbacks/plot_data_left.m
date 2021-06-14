function handle = plot_data_left(handle)

mha = handle.mha;
z = get(handle.p_left,'children');
if length(z) > 1
    delete(z(1:end));
end
exp = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold');
m_exp = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_slope');
g80 = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80');
g50 = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50');
mpo = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold');
col = ['y' 'r' 'g' 'b' 'c'];

switch handle.TB1_left.Value
    case 1
        plot(handle.p_left,[0 140],[0 140],'linewidth',1,'linestyle','--','color',[.5 .5 .5]);
        hold on
        for c = 1:5
            m_comp = (80+g80(c)-(50+g50(c)))/(80-50);
            LA_exp = 50+g50(c)-m_comp*(50-exp(c));
            LE_zero = exp(c)-LA_exp*(1/m_exp);
            LE_mpo = (1/m_comp)*(mpo(c)-(80+g80(c)))+80;
            x = [LE_zero exp(c) LE_mpo 140];
            y = [0 LA_exp mpo(c) mpo(c)];
            plot(handle.p_left,x,y,'linewidth',1.5,'color',(col(c)))
            hold on
            if c == 5
                handle.p_left.YLabel.String = 'LA (dB)';
                handle.p_left.YLim = [0 140];
                handle.p_left.YTick = 0:10:140;
                handle.p_left.YTickLabel = {'0','','20','','40','','60','','80','','100','','120','','140'};
                z = get(handle.p_left,'children');
                legend([z(5) z(4) z(3) z(2) z(1)],'250 Hz','500 Hz','1 kHz','2 kHz','4 kHz','location','northwest')
            end
        end
    case 0
        plot(handle.p_left,[0 140],[0 0],'linewidth',1,'linestyle','--','color',[.5 .5 .5]);
        hold on
        for c = 1:5
            m_comp = (80+g80(c)-(50+g50(c)))/(80-50);
            LA_exp = 50+g50(c)-m_comp*(50-exp(c));
            LE_zero = exp(c)-LA_exp*(1/m_exp);
            LE_mpo = (1/m_comp)*(mpo(c)-(80+g80(c)))+80;
            x = [LE_zero exp(c) LE_mpo 140];
            y = [(0-LE_zero) (LA_exp-exp(c)) (mpo(c)-LE_mpo) ((mpo(c)-LE_mpo)-(140-LE_mpo))];
            plot(handle.p_left,x,y,'linewidth',1.5,'color',(col(c)))
            hold on
            if c == 5
                handle.p_left.YLim = [-20 60];
                handle.p_left.YTick = -20:5:60;
                handle.p_left.YTickLabel = {'-20','','-10','','0','','10','','20','','30','','40','','50','','60'};
                handle.p_left.YLabel.String = 'V (dB)';
                z = get(handle.p_left,'children');
                legend([z(5) z(4) z(3) z(2) z(1)],'250 Hz','500 Hz','1 kHz','2 kHz','4 kHz','location','northeast')
            end
        end
end
end
