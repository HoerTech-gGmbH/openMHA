% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2019 HörTech gGmbH
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

function test_addsndfile
  % The first test checks that addsndfile finds soundfiles in ../Audiofiles but
  % not in /. We do not need to create sound files for the first test.
  test_addsndfile_path();

  % The remaining tests use sound files. Produce some example sound files.
  rampwav = 'test_addsndfile_ramp.wav';
  zerowav = 'test_addsndfile_zero.wav';
  outwav = 'test_addsndfile_OUT.wav';

  % ramps with different levels in 2 channels
  create_testfile(rampwav,1/256);
  % silence in 2 channels
  create_testfile(zerowav,0);
  % sound file where MHA stores output
  create_testfile(outwav,1/1024);

  % tests empty and non-empty filename variable
  test_addsndfile_filename(zerowav,rampwav,outwav);

  % tests for the different levelmodes
  test_addsndfile_levelmode_relative(zerowav,rampwav,outwav);
  test_addsndfile_levelmode_rms(zerowav,rampwav,outwav);
  test_addsndfile_levelmode_peak(zerowav,rampwav,outwav);
end

function create_testfile(inwav, factor)
  % Create a sound file with ramps with different levels in 2 channels
  srate = 44100;
  % right channel has lower level than left channel (unless factor is 0)
  signal = [[1:200]',[1:200]'/2] * factor;
  audiowrite(inwav, signal, srate, 'BitsPerSample', 32);
  % delete produced sound files after the test
  unittest_teardown(@delete, inwav);
end

function test_addsndfile_path
  % test if the configuration variable path does what it claims to do.
  dsc.instance = 'test_addsndfile_path';
  dsc.mhalib = 'addsndfile';
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');

  mha_set(mha,'',dsc);

  % Check we find sound files in dir Audiofiles but not in root /

  % The fileseparator really depends on the operating system where
  % the MHA runs, which can differ from the OS where octave runs.
  % We use filesep here only to test backslashes actually work on windows.
  mha_set(mha,'mha.path',['../Audiofiles',filesep()]);
  sound_files_found = mha_get(mha,'mha.files');
  % length(sound_files_found) should be the number of sound files found in path
  unittest_assert(length(sound_files_found), 'No sound files found in path');

  % A sane computer will not have any sound files in the root directory.
  mha_set(mha,'mha.path','/');
  sound_files_found = mha_get(mha,'mha.files');
  assert_equal(0, length(sound_files_found), 'sound files found in /');

  % repeat first test, but with trailing forward slash also on windows.
  mha_set(mha,'mha.path','../Audiofiles/'); % / works on all systems
  sound_files_found = mha_get(mha,'mha.files');
  unittest_assert(length(sound_files_found), 'No sound files found in path');
end

function test_addsndfile_filename(zerowav,rampwav,outwav)
  zerosignal = audioread(zerowav);

  % test if the configuration variable filename does what it claims to do.
  % At first, filename is left empty.
  dsc.instance = 'test_addsndfile_filename';
  dsc.nchannels_in = 2;
  dsc.mha.channels = 0:1;
  dsc.mhalib = 'addsndfile';
  dsc.iolib = 'MHAIOFile';
  dsc.io.in = zerowav;
  dsc.io.out = outwav;
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');

  mha_set(mha,'',dsc);

  % Check addsndfile does not alter sound when filename empty
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  assert_equal(zerosignal, audioread(outwav));
  % This has also checked that mha has produced output and outwav was
  % overwritten with zeros: This is the first test that executes.
  % outwav was initially created with non-zero content.
  
  % Check addsndfile does alter sound when filename non-empty
  mha_set(mha,'mha.filename',rampwav);
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  assert_not_equal(zerosignal, audioread(outwav));
end

function test_addsndfile_levelmode_relative(zerowav,rampwav,outwav)
  rampsignal = audioread(rampwav);

  % test if levelmode relative behaves as described.
  dsc.instance = 'test_addsndfile_levelmode_relative';
  dsc.nchannels_in = 2;
  dsc.mhalib = 'addsndfile';
  dsc.mha.channels = 0:1;
  dsc.iolib = 'MHAIOFile';
  dsc.io.in = zerowav;
  dsc.io.out = outwav;
  dsc.mha.filename = rampwav;
  dsc.mha.levelmode = 'relative';
  % This setting adds ~94 dB to the file RMS db re FS
  % Because of the 1Pa convention in MHA, this results in the samples being
  % transferred with scaling factor 1.0 from sound file to MHA, i.e. unmodified.
  dsc.mha.level = log10(5e4) * 20;
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');

  mha_set(mha,'',dsc);

  % test that the addsndfile sound data is written to outwav
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  assert_almost(rampsignal, audioread(outwav), 1e-6, 'incorrect at level=94');

  % add sound from sound file with amplitude factor 0.1
  mha_set(mha, 'mha.level', log10(5e4) * 20 - 20);
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  assert_almost(rampsignal/10, audioread(outwav), 1e-6, ...
                'incorrect at level=74');

  % Level differences between channels are preserved.
  % Preservation has been verified above, now verify channel differences exist.
  channel_levels = 10*log10(sum(audioread(outwav).^2));
  assert_all(abs(diff(channel_levels)) > 6); % factor 2 is 6.02 dB
end

function test_addsndfile_levelmode_rms(zerowav,rampwav,outwav)
  % test behaviour of levelmode rms.
  dsc.instance = 'test_addsndfile_levelmode_rms';
  dsc.nchannels_in = 2;
  dsc.mhalib = 'addsndfile';
  dsc.mha.channels = 0:1;
  dsc.iolib = 'MHAIOFile';
  dsc.io.in = zerowav;
  dsc.io.out = outwav;
  dsc.mha.filename = rampwav;
  dsc.mha.levelmode = 'rms';
  % This setting causes both channels to be amplified so that their combined RMS
  % is ~74 dB inside mha, but the original inter-channel level difference is
  % preserved.  ~74 dB which corresponds to a RMS 0f 0.1 inside mha under the
  % 1Pa convention.
  dsc.mha.level = log10(5e4) * 20 - 20;
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');

  mha_set(mha,'',dsc);

  % test that the addsndfile sound data is written to outwav
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  rms =  sqrt(mean(audioread(outwav).^2));

  assert_almost(2, rms(1)/rms(2), 1e-6, ...
                'rms: the inter-channel level difference is not preserved');
  assert_almost(0.1, sqrt(mean(rms.^2)), 1e-6, ...
                'rms: the cross-channel combined RMS has unexpected value');
end

function test_addsndfile_levelmode_peak(zerowav,rampwav,outwav)
  % test if levelmode peak behaves as documented.
  dsc.instance = 'test_addsndfile_levelmode_peak';
  dsc.nchannels_in = 2;
  dsc.mhalib = 'addsndfile';
  dsc.mha.channels = 0:1;
  dsc.iolib = 'MHAIOFile';
  dsc.io.in = zerowav;
  dsc.io.out = outwav;
  dsc.mha.filename = rampwav;
  dsc.mha.levelmode = 'peak';
  % This setting causes both channels to be amplified so that the maximum
  % magnitude sample from all channels combined is at ~94 dB inside mha
  % which corresponds to an amplitude of 1Pa inside mha.
  dsc.mha.level = log10(5e4) * 20;
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');

  mha_set(mha,'',dsc);

  % test that the addsndfile sound data is written to outwav
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  peak =  max(abs(audioread(outwav)));

  assert_almost([1 0.5], peak, 1e-6, ...
                'peak: unexpected maximum sample values');

  % test with different level
  level=37;
  mha_set(mha,'mha.level', level);
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  peak =  max(abs(audioread(outwav)));
  expected_peak = 10^(level/20)*2*10^(-5);
  assert_almost([1 0.5] * expected_peak, peak, 1e-6, ...
                'peak: unexpected maximum sample values for level');
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
