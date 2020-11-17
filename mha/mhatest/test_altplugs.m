%% This function tests plugin altplugs for regressions
%%
%% This file is part of the HörTech Open Master Hearing Aid (openMHA)
%% Copyright © 2020 HörTech gGmbH

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

function test_altplugs
  %Clean up after we are finished
  outwav = 'OUT.wav';
  unittest_teardown(@delete, [outwav]);
  fclose(fopen(outwav, 'w'));
  inwav = 'IN.wav';
  % Basic MHA setup
  dsc.srate = 16000;
  dsc.instance = 'test_altplugs';
  dsc.mhalib =  'altplugs';
  % Apply gain to ensure that there are audio samples > 1
  dsc.iolib = 'MHAIOFile';
  dsc.fragsize = 4000;
  dsc.nchannels_in = 1;
  dsc.io.out = outwav;
  mha=mha_start;
  mha_set(mha,'',dsc);
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');

  % Actual tests
test_ramp(mha,dsc,inwav,outwav)
end

% See T92
function test_ramp(mha, dsc, inwav, outwav)
  unittest_teardown(@delete, [inwav]);
  fclose(fopen(inwav, 'w'));
  snd_in=ones(dsc.srate,1)*0.5;
  audiowrite(inwav,snd_in,dsc.srate,'BitsPerSample', 32);
  mha_set(mha,'io.in',inwav);
  mha_set(mha,'io.length',dsc.srate/4);
  mha_set(mha,'mha.ramplen',0.5);
  mha_set(mha,'mha.plugs',{'identity:i1', 'identity:i2'});
  mha_set(mha,'mha.select','i1');
  % First quarter of signal is equal to 0.5
  mha_set(mha,'cmd','start');
  % Set to other identity - ramps from t=0.25s to t=0.75s
  mha_set(mha,'mha.select','i2');
  % Process reset of signal
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  snd_out=audioread(outwav);
  % First 0.25s are equal to snd_in
  assert_difference_below(snd_in(1:dsc.srate/4),snd_out(1:dsc.srate/4),1e-6);
  % 0.25s..0.75s is occupied by ramp, but 0.75s..1s is equal to snd_in again
  assert_difference_below(snd_in(3*dsc.srate/4:end),snd_out(3*dsc.srate/4:end),1e-6);
end
% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
