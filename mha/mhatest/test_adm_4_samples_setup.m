%% This file is part of the HörTech Open Master Hearing Aid (openMHA)
%% Copyright © 2019 HörTech gGmbH
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

function test_adm_4_samples_setup
  inwav = 'IN.wav';
  outwav = 'OUT.wav';
  fclose(fopen(inwav, 'w'));
  unittest_teardown(@delete, [inwav]);
  fclose(fopen(outwav, 'w'));
  unittest_teardown(@delete, [outwav]);

  %Create pseudo random noise
  snd = repeatable_rand(44104,1,26129) - 0.5;

  % Make arrive 4 samples earlier at second channel than at first channel
  snd = [snd(1:44100),snd(5:end)];

  % Check that snd(:,2) is 4 samples earlier than snd(:,1)
  assert_equal(snd(    100,2), snd(  104,1));
  assert_equal(snd(   1000,2), snd( 1004,1));
  assert_equal(snd(      1,2), snd(    5,1));
  assert_equal(snd(1:end-4,2), snd(5:end,1));

                                % write input signal to file
  audiowrite(inwav,snd,44100,'BitsPerSample',32);

  % Create mha to process the signal with an ADM with mic distance 4 samples
  dsc.instance = 'test_adm_4_samples_setup';
  dsc.mhalib =  'adm';
  dsc.iolib = 'MHAIOFile';
  dsc.fragsize = 64;
  dsc.nchannels_in = 2;
  dsc.mha.front_channels= [0];
  dsc.mha.rear_channels = [1];
  dsc.mha.distances = [4 * 340/44100];
  dsc.io.in = inwav;
  dsc.io.out = outwav;

  mha=mha_start;
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  mha_set(mha,'',dsc);
  mha_set(mha,'mha.tau_beta', mean(mha_get(mha,'mha.tau_beta')));
  mha_set(mha,'mha.mu_beta',  mean(mha_get(mha,'mha.mu_beta')));
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');

  snd_out=audioread(outwav);

  input_level = rms(snd(43100:end,1));
  output_level = rms(snd_out(43100:end,1));

  assert_all(output_level / input_level < 0.004); % 48 dB attenuation
  

function r = rms(signal)
  r=sqrt(mean(signal.^2));
