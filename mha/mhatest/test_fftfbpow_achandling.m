function test_fftfbpow_achandling
% Checks for regression of T274: fftfbpow stored the measured levels in an AC
% vector, but only inserted the first AC vector created during prepare into the
% AC space. Later vectors created by configuration variable updates were not
% correctly inserted.
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2018 2019 HörTech gGmbH
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
  levels = mha_get(mha, 'mha.mhachain.acmon.fftfbpow');
  assert_all(levels(1) > levels(2));

  % change frequencies of filterbank and process again
  mha_set(mha, 'mha.mhachain.fftfbpow.f', [0 86.2]);
  mha_set(mha, 'io.input', sin220Hz);

  % Now level in lower band should be smaller than in higher band:
  levels = mha_get(mha, 'mha.mhachain.acmon.fftfbpow');
  assert_all(levels(1) < levels(2));

  % The last test failed in an older version because of the AC handling bug in
  % fftfbpow.
  % The 220 Hz sinusoid should result in a higher level in the band with
  % the (not quite) "center frequency at 86 Hz, not 0 Hz.

  % Check if the measured level is correct. We expect ~91 dB

  spl_for_1Pa_approx = 94;
  crest_for_sinusoid_approx = -3;
  assert_difference_below(spl_for_1Pa_approx + crest_for_sinusoid_approx, ...
                          levels(2), 0.1);
