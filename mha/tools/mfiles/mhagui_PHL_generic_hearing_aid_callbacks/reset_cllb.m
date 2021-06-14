function [handle] = reset_cllb(src,event)
handle = guidata(src);
mha = handle.mha;

mha_set(mha,'mha.transducers.mhachain.split.bte.adm.bypass',1)
mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.decomb.select','identity')
mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.select','identity')
mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.coh.select','coherence')
mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.fshift.select','fshift')
mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.limiter_threshold',[95 95 95 95 95 95 95 95 95 95]);
mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.maxgain', [0 0 40 40 21 0 0 40 40 21]);
mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50',[0 0 15 15 15 0 0 15 15 15]);
mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80',[0 0 0 0 0 0 0 0 0 0]);
mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_threshold',[40 40 40 40 40 40 40 40 40 40]);
mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.expansion_slope',1.5);
mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.tau_decay',0.015);
mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.tau_attack',0.005);
mha_set(mha,'mha.transducers.mhachain.headphone_volume.gains',0);

handle = readoutPlugin(handle,'adm');
handle = readoutPlugin(handle,'coh');
handle = readoutPlugin(handle,'fshift');
handle = readoutPlugin(handle,'mpo');
handle = readoutPlugin(handle,'gain');
handle = readoutPlugin(handle,'exp');
handle = readoutPlugin(handle,'mbc');
%handle = readoutPlugin(handle,'hpvol');

mha_set(mha,'mha.transducers.mhachain.signal_processing.ola.c.fresponse.equalize.gains',ones(4,82));

[handle] = maxGainCheck(handle);
set(handle.G50_all_slider_right,'value',0,'max',80,'sliderstep',[1/80 3/80]);
set(handle.G80_all_slider_right,'value',0,'max',80,'sliderstep',[1/80 3/80]);
% set(handle.G50_all_edit_right,'string','0 dB');
% set(handle.G80_all_edit_right,'string','0 dB');
set(handle.maxgain_all_edit_right,'string','80 dB');
set(handle.exp_all_edit_right,'string','40 dB');
set(handle.mpo_all_slider_right,'value',95);
set(handle.mpo_all_edit_right,'string','95 dB');

set(handle.G50_all_slider_left,'value',0,'max',80,'sliderstep',[1/80 3/80]);
set(handle.G80_all_slider_left,'value',0,'max',80,'sliderstep',[1/80 3/80]);
% set(handle.G50_all_edit_left,'string','0 dB');
% set(handle.G80_all_edit_left,'string','0 dB');
set(handle.maxgain_all_edit_left,'string','80 dB');
set(handle.exp_all_edit_left,'string','40 dB');
set(handle.mpo_all_slider_left,'value',95);
set(handle.mpo_all_edit_left,'string','95 dB');

set(handle.link,'tag','nolink','string','link');

[handle] = maxGainCheck(handle);
handle.dat.g50.old = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g50');
handle.dat.g80.old = mha_get(mha,'mha.transducers.mhachain.signal_processing.ola.c.mbc.multibandcompressor.dc_simple.g80');

handle.dat.g1.max_right = max([handle.G80_right_slider1.Max-handle.G80_right_slider1.Value; handle.G50_right_slider1.Max-handle.G50_right_slider1.Value ]);
handle.dat.g1.min_right = -max([handle.G80_right_slider1.Value-handle.G80_right_slider1.Min; handle.G50_right_slider1.Value-handle.G50_right_slider1.Min]);

handle.dat.g2.max_right = max([handle.G80_right_slider2.Max-handle.G80_right_slider2.Value; handle.G50_right_slider2.Max-handle.G50_right_slider2.Value ]);
handle.dat.g2.min_right = -max([handle.G80_right_slider2.Value-handle.G80_right_slider2.Min; handle.G50_right_slider2.Value-handle.G50_right_slider2.Min]);

handle.dat.g3.max_right = max([handle.G80_right_slider3.Max-handle.G80_right_slider3.Value; handle.G50_right_slider3.Max-handle.G50_right_slider3.Value ]);
handle.dat.g3.min_right = -max([handle.G80_right_slider3.Value-handle.G80_right_slider3.Min; handle.G50_right_slider3.Value-handle.G50_right_slider3.Min]);

handle.dat.g4.max_right = max([handle.G80_right_slider4.Max-handle.G80_right_slider4.Value; handle.G50_right_slider4.Max-handle.G50_right_slider4.Value ]);
handle.dat.g4.min_right = -max([handle.G80_right_slider4.Value-handle.G80_right_slider4.Min; handle.G50_right_slider4.Value-handle.G50_right_slider4.Min]);

handle.dat.g5.max_right = max([handle.G80_right_slider5.Max-handle.G80_right_slider5.Value; handle.G50_right_slider5.Max-handle.G50_right_slider5.Value ]);
handle.dat.g5.min_right = -max([handle.G80_right_slider5.Value-handle.G80_right_slider5.Min; handle.G50_right_slider5.Value-handle.G50_right_slider5.Min]);

handle.dat.bb.max_right = max([handle.G80_right_slider5.Max-handle.G80_right_slider5.Value;handle.G80_right_slider4.Max-handle.G80_right_slider4.Value;...
    handle.G80_right_slider3.Max-handle.G80_right_slider3.Value;handle.G80_right_slider2.Max-handle.G80_right_slider2.Value;...
    handle.G80_right_slider1.Max-handle.G80_right_slider1.Value;handle.G50_right_slider5.Max-handle.G50_right_slider5.Value;...
    handle.G50_right_slider4.Max-handle.G50_right_slider4.Value;handle.G50_right_slider3.Max-handle.G50_right_slider3.Value;...
    handle.G50_right_slider2.Max-handle.G50_right_slider2.Value;handle.G50_right_slider1.Max-handle.G50_right_slider1.Value]);
handle.dat.bb.min_right = -max([handle.G80_right_slider5.Value-handle.G80_right_slider5.Min;handle.G80_right_slider4.Value-handle.G80_right_slider4.Min;...
    handle.G80_right_slider3.Value-handle.G80_right_slider3.Min;handle.G80_right_slider2.Value-handle.G80_right_slider2.Min;...
    handle.G80_right_slider1.Value-handle.G80_right_slider1.Min;handle.G50_right_slider5.Value-handle.G50_right_slider5.Min;...
    handle.G50_right_slider4.Value-handle.G50_right_slider4.Min;handle.G50_right_slider3.Value-handle.G50_right_slider3.Min;...
    handle.G50_right_slider2.Value-handle.G50_right_slider2.Min;handle.G50_right_slider1.Value-handle.G50_right_slider1.Min]);

handle = plot_data_right(handle);
handle = plot_data_left(handle);

guidata(src,handle);
end
