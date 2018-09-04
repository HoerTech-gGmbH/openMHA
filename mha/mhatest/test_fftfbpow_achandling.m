function test_fftfbpow_achandling
% Checks for regression of T274: fftfbpow stored the measured levels in an AC
% vector, but only inserted the first AC vector created during prepare into the
% AC space. Later vectors created by configuration variable updates were not
% correctly inserted.

  % quick cfg using defaults: srate=44100,fragsize=200,wnd=400,fft=512
  dsc.instance = 'test_fftfbpow_achandling';
  dsc.mhalib = 'overlapadd';
  dsc.mha.plugin_name = 'mhachain';
  dsc.mha.mhachain.algos = {'fftfbpow','acmon'};
  dsc.iolib = 'MHAIOParser';
  dsc.mha.mhachain.fftfbpow.f = [500 5000];

  % start and configure mha
  mha = mha_start;
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  mha_set(mha, '', dsc);
  mha_set(mha, 'cmd', 'start');

  % fill analysis window with sinusoid
  sin220Hz = sin(2*pi*220 * [1:200]/44100); % sin(2 pi f t)
  mha_set(mha, 'io.input', sin220Hz);
  mha_set(mha, 'io.input', sin220Hz);

  % Now level in lower band should be bigger than in higher band:
  levels = mha_get(mha, 'mha.mhachain.acmon.fftfbpow')
  assert_all(levels(1) > levels(2));

  % change frequencies of filterbank and process again
  mha_set(mha, 'mha.mhachain.fftfbpow.f', [0 86.2]);
  mha_set(mha, 'io.input', sin220Hz);

  % Now level in lower band should be smaller than in higher band:
  levels = mha_get(mha, 'mha.mhachain.acmon.fftfbpow')
  assert_all(levels(1) < levels(2));

  

  % The last test failed because of the AC handling bug in fftfbpow.
  % The 220 Hz sinusoid should result in a higher level in the band with
  % the (not quite) "center frequency at 86 Hz, not 0 Hz.
