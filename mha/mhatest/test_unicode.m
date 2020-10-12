%% This file is part of the HörTech Open Master Hearing Aid (openMHA)
%% Copyright © 2020 HörTech gGmbH
%%
%% openMHA is free software: you can redistribute it and/or modify
%% it under the terms of the GNU Affero General Public License as published by
%% the Free Software Foundation, version 3 of the License.
%%
%% openMHA is distributed in the hope that it will be useful,
%% but WITHOUT ANY WARRANTY; without even the implied warranty of
%% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%% GNU Affero General Public License, version 3 for more details.
%%
%% You should have received a copy of the GNU Affero General Public License,
%% version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

function test_unicode
  signal_in = zeros(64,2);
  fname_in = 'test_unicöde.wav';
  audiowrite(fname_in,signal_in,44100);
  fname_out = 'test_unicöde2.wav';
  unittest_teardown(@delete, [fname_in]);
  fclose(fopen(fname_out,'w'));
  unittest_teardown(@delete, [fname_out]);

  dsc.srate = 44100;
  dsc.fragsize = 64;
  dsc.nchannels_in = 2;
  dsc.iolib = 'MHAIOFile';
  dsc.io.in = fname_in;
  dsc.io.out = fname_out;
  dsc.mhalib = 'identity';
  mha = mha_start;
  unittest_teardown(@mha_set, mha, 'cmd','quit');
  mha_set(mha,'',dsc);
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  signal_out = audioread(fname_out);
  assert_equal(signal_in,signal_out);
