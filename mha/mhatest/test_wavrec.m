% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2020 2021 HörTech gGmbH
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

function test_wavrec
  %test_wavrec_formats();
  test_wavrec_channel_permutations();
end

function test_wavrec_channel_permutations

  %% Clean up after we are finished
  inwav = 'IN.wav';
  unittest_teardown(@delete, [inwav]);
  fclose(fopen(inwav , 'w'));
  outwav = 'OUT.wav';
  unittest_teardown(@delete, [outwav]);
  fclose(fopen(outwav , 'w'));
  rec = 'rec';
  recwav = [rec '.wav'];
  unittest_teardown(@delete, [recwav]);
  fclose(fopen(recwav , 'w'));

  nchannels=4;
  srate=16000;
  fragsize=31;
  len=1024;
  % Use channel values as markers 0.1 always belongs to channel 1, etc...
  expected=[0.1*ones(1,len); 0.2*ones(1,len); 0.3*ones(1,len); 0.4*ones(1,len)]';
  audiowrite(inwav,expected,srate);
  mha = mha_start;
  unittest_teardown(@mha_set, mha, 'cmd','quit');
  mha_set(mha,'fragsize',fragsize);
  mha_set(mha,'nchannels_in',nchannels);
  mha_set(mha,'srate',srate);
  mha_set(mha,'iolib','MHAIOFile');
  mha_set(mha,'io.in',inwav);
  mha_set(mha,'io.out',outwav);
  mha_set(mha,'io.length',512);
  mha_set(mha,'mhalib','wavrec');
  mha_set(mha,'mha.prefix', rec);
  mha_set(mha,'mha.record', true);
  mha_set(mha,'mha.use_date', false);
  mha_set(mha,'mha.minwrite', 120);
  mha_set(mha,'mha.fifolen', 121);
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  [actual,fs]=audioread(recwav);
  % Resize expected to actual: We might have missed some samples, but channel markes must be intact
  expected=expected(1:size(actual,1),1:size(actual,2));
  assert_almost(expected,actual,1e-3);
end


function test_wavrec_formats
  nchannels=10; srate=32000;
  samples = [1.1 0 0.5 1 2 inf nan -0.25 -1 -3 -inf];
  
  fclose(fopen('test_wavrec.wav','w'));
  unittest_teardown(@delete, 'test_wavrec.wav');
  
  [info, fs, audiodata] = use_wavrec('32_bit_float',samples, nchannels,srate);
  assert_equal(srate, fs);
  assert_almost(repmat(samples(~isnan(samples))',10,1), audiodata(~isnan(audiodata)), 5e-8);
 
 
  [info, fs, audiodata] = use_wavrec('Signed_16_bit_PCM',samples, nchannels,srate);
  assert_equal(16,info.BitsPerSample);
  assert_equal(srate, fs);
  % mimick the clipping
  nan_index = find(isnan(samples));
  % different version of libsndfile replace NaN either with -1 or 0
  if (audiodata(nan_index,1) == 0)
    nan_replacement = 0;
  else
    nan_replacement = -1;
  end
  samples(isnan(samples)) = nan_replacement;
  samples = min(samples, ones(1,11) * 32767/32768);
  samples = max(samples, ones(1,11) * -1);
  assert_almost(repmat(samples',1,10), audiodata, 2/65536);
end

function [info, fs, audiodata] = use_wavrec(format, samples, nchannels, srate)
  mha = mha_start;
  unittest_teardown(@mha_set, mha, 'cmd','quit');
  mha_set(mha,'fragsize',1);
  mha_set(mha,'nchannels_in',nchannels);
  mha_set(mha,'srate',srate);
  mha_set(mha,'iolib','MHAIOParser');
  mha_set(mha,'mhalib','wavrec');
  mha_set(mha,'mha.output_sample_format',format);
  mha_set(mha,'mha.prefix', 'test_wavrec');
  mha_set(mha,'mha.record', true);
  mha_set(mha,'mha.use_date', false);
  mha_set(mha,'cmd','start');
  for sample = samples
    mha_set(mha,'io.input',sample*ones(nchannels,1));
  end
  mha_set(mha,'cmd','release');
  [audiodata,fs] = audioread('test_wavrec.wav');
  info = audioinfo('test_wavrec.wav');
end
