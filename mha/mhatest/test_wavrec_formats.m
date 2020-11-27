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
