%% This function tests plugin MHAIOFile for regressions
%%
%% This file is part of the HörTech Open Master Hearing Aid (openMHA)
%% Copyright © 2019 HörTech gGmbH

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
%%
function test_mhaiofile
  %Clean up after we are finished
  outwav = 'OUT.wav';
  unittest_teardown(@delete, [outwav]);
  fclose(fopen(outwav, 'w'));

  % Basic MHA setup
  dsc.srate = 44100;
  dsc.instance = 'test_mhaiofile';
  dsc.mhalib =  'gain';
  % Apply gain to ensure that there are audio samples > 1
  dsc.mha.gains=[16];
  dsc.iolib = 'MHAIOFile';
  dsc.fragsize = 64;
  dsc.nchannels_in = 1;
  dsc.io.out = outwav;
  mha=mha_start;
  mha_set(mha,'',dsc);
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');

  % Actual tests
  test_no_sign_flip(mha, dsc, outwav);
  test_float_has_no_clipping(mha, dsc, outwav)
  test_int_has_clipping(mha, dsc, outwav)
end


function test_float_has_no_clipping(mha, dsc, outwav)
  mha_set(mha,'io.in','../Audiofiles/float32.wav');
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  snd_out=audioread(outwav);
  assert_all(~isempty(snd_out(abs(snd_out)>1)));
end

function test_int_has_clipping(mha, dsc, outwav)
  mha_set(mha,'io.in','../Audiofiles/int16.wav');
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  snd_out=audioread(outwav);
  assert_all(isempty(snd_out(abs(snd_out)>1)));
end


% See T703
function test_no_sign_flip(mha, dsc, outwav)
  inwav = 'IN.wav';
  unittest_teardown(@delete, [inwav]);
  fclose(fopen(inwav, 'w'));
  snd_in=sin(2*pi*440*(0:dsc.srate)/dsc.srate);
  audiowrite(inwav,snd_in,dsc.srate);
  mha_set(mha,'io.in',inwav);
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  snd_out=audioread(outwav);
  num_sign_flips=numel(detect_flips(snd_out));
  assert_equal(num_sign_flips,0);
end

function idx=detect_flips(x)
  idx = find(abs(diff(x))>1);
end
% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
