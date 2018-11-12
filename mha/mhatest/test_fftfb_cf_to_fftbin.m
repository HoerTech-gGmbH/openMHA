function test_fftfb_cf_to_fftbin
  pkg load signal;
  
  sCfg = struct;
  sCfg.srate = 44100;
  sCfg.nchannels_in = 1;
  sCfg.fragsize = 64;
  sCfg.iolib = 'MHAIOParser';
  sCfg.mhalib = 'overlapadd';
  sCfg.mha.fftlen = 256;
  sCfg.mha.wnd.len = 128;
  sCfg.mha.zerownd.type = 'rect';
  sCfg.mha.plugin_name = 'smoothgains_bridge:sg';
  sCfg.mha.sg.plugin_name = 'mhachain:ch';
  sCfg.mha.sg.ch.algos = {'fftfilterbank:fb','combinechannels:cmb'};
  sCfg.mha.sg.ch.fb.fscale = 'log';
  sCfg.mha.sg.ch.fb.ovltype = 'rect';
  sCfg.mha.sg.ch.fb.plateau = 0;
  sCfg.mha.sg.ch.fb.ftype = 'center';
  sCfg.mha.sg.ch.fb.f = [176 297 500 841 1414 2378 4000 6727 11314];
  sCfg.mha.sg.ch.cmb.outchannels = 1;
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  mha_set(mha,'',sCfg);
  mha_set(mha,'cmd','start');
  
  % process a bit
  input_signal = 0.01*(repeatable_rand(23000,1,1)-0.5);
  %input_signal = [1;zeros(44099,1)];
  output_signal = mha_process_by_parser(mha, input_signal')';
  [c,l] = xcorr(sum(output_signal,2),input_signal);
  [tmp,idx] = max(c);
  delay = l(idx);
  assert_equal(128, delay);
  input_signal = input_signal(1:22050,:);
  output_signal = output_signal([1:22050]+delay,:);
  assert_difference_below(input_signal,output_signal,1e-7);
  