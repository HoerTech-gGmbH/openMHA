function test_fftfb_cf_to_fftbin
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2009 2018 2019 HörTech gGmbH
%
% openMHA is free software: you can redistribute it and/or modify
% it under the terms of the GNU Affero General Public License as published by
% the Free Software Foundation, version 3 of the License.
%
% openMHA is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU Affero General Public License, version 3 for more details.
%
% You should have received a copy of the GNU Affero General Public License, 
% version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.
  
  sCfg = struct;
  sCfg.srate = 44100;
  sCfg.nchannels_in = 1;
  sCfg.fragsize = 64;
  sCfg.iolib = 'MHAIOFile';
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
  
  % Prepare input and output sound file
  input_signal = 0.01*(repeatable_rand(23000,1,1)-0.5);
  inwav = 'test_fftfb_cf_to_fftbin_in.wav';
  audiowrite(inwav, input_signal, sCfg.srate, 'BitsPerSample', 32);
  unittest_teardown(@delete, inwav);
  sCfg.io.in = inwav;

  outwav = 'test_fftfb_cf_to_fftbin_out.wav';
  fclose(fopen(outwav, 'w'));
  unittest_teardown(@delete, outwav);
  sCfg.io.out = outwav;
  
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  mha_set(mha,'',sCfg);
  mha_set(mha,'cmd','start');

  output_signal = audioread(outwav);
  [c,l] = xcorr(sum(output_signal,2),input_signal);
  [tmp,idx] = max(c);
  delay = l(idx);
  assert_equal(128, delay);
  input_signal = input_signal(1:22050,:);
  output_signal = output_signal([1:22050]+delay,:);
  assert_difference_below(input_signal,output_signal,1e-7);
