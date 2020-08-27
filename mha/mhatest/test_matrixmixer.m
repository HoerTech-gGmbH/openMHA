function test_matrixmixer
% Testing the matrixmixer plugin.
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2005 2006 2007 2015 2018 HörTech gGmbH

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

  % A test signal.
  x = [1:1024]' / 8192;
  x = [x;flipud(x)];
  inwav = 'test_matrixmixer_in.wav';
  outwav = 'test_matrixmixer_out.wav';

  % first test: apply factor to a one-channel signal.
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  dsc.srate = 44100;
  dsc.instance = 'test_matrixmixer';
  dsc.nchannels_in = 1;
  dsc.mhalib = 'matrixmixer:m';
  dsc.iolib = 'MHAIOFile';
  dsc.mha.m = 0.5;
  dsc.io.in = inwav;
  dsc.io.out = outwav;
  mha_set(mha,'',dsc);
  audiowrite(inwav, x, dsc.srate, 'BitsPerSample', 32);
  unittest_teardown(@delete, inwav);
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  unittest_teardown(@delete, outwav);
  y = audioread(outwav);
  assert_difference_below(0.5*x, y, 1e-6);

  % second test: daisy-chain two matrixmixers.
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  clear dsc;
  dsc.srate = 44100;
  dsc.instance = 'test_matrixmixer2';
  % In this test MHA processes 10 input channels.
  dsc.nchannels_in = 10;
  dsc.mhalib = 'mhachain';
  dsc.iolib = 'MHAIOFile';
  dsc.io.in = inwav;
  dsc.io.out = outwav;
  % We need to assign unique names to the 2 matrixmixers in the same chain 
  dsc.mha.algos = {'matrixmixer:m1','matrixmixer:m2'};
  % the first matrixmixer produces a single output channel
  dsc.mha.m1.m = [0.6 zeros(1,9)];
  % the second matrixmixer produces again 10 output channels
  dsc.mha.m2.m = [0.6; zeros(9,1)];
  mha_set(mha,'',dsc);
  audiowrite(inwav, repmat(x,[1 10]), dsc.srate, 'BitsPerSample', 32);
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  y = audioread(outwav);
  assert_equal( size(x,1), size(y,1) );
  assert_equal( 10*size(x,2), size(y,2) );
  assert_difference_below(0.6*0.6*x, y(:,1), 1e-6);

  % third test: produce two output channels from single input channel
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  clear dsc;
  dsc.srate = 44100;
  dsc.instance = 'test_matrixmixer3';
  dsc.nchannels_in = 1;
  dsc.mhalib = 'mhachain';
  dsc.iolib = 'MHAIOFile';
  dsc.io.in = inwav;
  dsc.io.out = outwav;
  dsc.mha.algos = {'matrixmixer:m'};
  dsc.mha.m.m = [[0.5];[-0.2]];
  mha_set(mha,'',dsc);
  audiowrite(inwav, x, dsc.srate, 'BitsPerSample', 32);
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  y = audioread(outwav);
  assert_equal( size(x,1), size(y,1) );
  assert_equal( 2*size(x,2), size(y,2) );
  assert_difference_below(0.5*x, y(:,1), 1e-6);
  assert_difference_below(-0.2*x, y(:,2), 1e-6);

  % Fourth test: Run matrixmixer in STFT domain
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  clear dsc;
  dsc.srate = 44100;
  dsc.instance = 'test_matrixmixer4';
  dsc.nchannels_in = 1;
  dsc.mhalib = 'mhachain';
  dsc.iolib = 'MHAIOFile';
  dsc.io.in = inwav;
  dsc.io.out = outwav;
  dsc.mha.algos = {'wave2spec','matrixmixer:m','spec2wave'};
  dsc.mha.m.m = [0.5];
  mha_set(mha,'',dsc);
  fftlen = mha_get(mha, 'mha.wave2spec.fftlen');
  wndlen = mha_get(mha, 'mha.wave2spec.wndlen');
  fragsize = mha_get(mha, 'fragsize');
  ola_delay = fragsize + (fftlen - wndlen) / 2;
  audiowrite(inwav, [zeros(fftlen,1);x;zeros(ola_delay+fftlen,1)], ...
             dsc.srate, 'BitsPerSample', 32);
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  y = audioread(outwav);
  y = y((fftlen + ola_delay+1):(end-fftlen));
  assert_difference_below(0.5*x, y, 1e-5);

  % Fifth test: matrixmixer changes channel count in STFT domain
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  clear dsc;
  dsc.srate = 44100;
  dsc.instance = 'test_matrixmixer5';
  dsc.nchannels_in = 1;
  dsc.mhalib = 'mhachain';
  dsc.iolib = 'MHAIOFile';
  dsc.io.in = inwav;
  dsc.io.out = outwav;
  dsc.mha.algos = {'wave2spec','matrixmixer:m','spec2wave'};
  dsc.mha.m.m = [0.2 -0.5]'; % note: transposition operator
  mha_set(mha,'',dsc);
  fftlen = mha_get(mha, 'mha.wave2spec.fftlen');
  wndlen = mha_get(mha, 'mha.wave2spec.wndlen');
  fragsize = mha_get(mha, 'fragsize');
  ola_delay = fragsize + (fftlen - wndlen) / 2;
  audiowrite(inwav, [zeros(fftlen,1);x;zeros(ola_delay+fftlen,1)], ...
             dsc.srate, 'BitsPerSample', 32);
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  y = audioread(outwav);
  y = y((fftlen + ola_delay+1):(end-fftlen),:);
  assert_difference_below(0.2*x, y(:,1), 1e-5);
  assert_difference_below(-0.5*x, y(:,2), 1e-5);

  % Sixth test: Mix channels together in STFT domain.
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  clear dsc;
  dsc.srate = 44100;
  dsc.instance = 'test_matrixmixer6';
  dsc.nchannels_in = 3;
  dsc.mhalib = 'mhachain';
  dsc.iolib = 'MHAIOFile';
  dsc.io.in = inwav;
  dsc.io.out = outwav;
  dsc.mha.algos = {'wave2spec','matrixmixer:m','spec2wave'};
  dsc.mha.m.m = [0.2, -0.5, 0.1];
  mha_set(mha,'',dsc);
  fftlen = mha_get(mha, 'mha.wave2spec.fftlen');
  wndlen = mha_get(mha, 'mha.wave2spec.wndlen');
  fragsize = mha_get(mha, 'fragsize');
  ola_delay = fragsize + (fftlen - wndlen) / 2;
  audiowrite(inwav, [zeros(fftlen,3);x,x,x;zeros(ola_delay+fftlen,3)], ...
             dsc.srate, 'BitsPerSample', 32);
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  y = audioread(outwav);
  y = y((fftlen + ola_delay+1):(end-fftlen),:);
  assert_difference_below(-0.2*x, y, 1e-5);

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
