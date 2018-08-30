function response = test_fftfb_level_summation_effects_rect_wave_irs
% When the signal is divided into bands, an intensity summation across
% bands will result in a lower intensity than when computing a
% broadband intensity sum, because filters overlap (even rectangular
% filters overlap in 1 FFT bin), and some bins are split into two
% bands with wheighting factors that (depending on filter shape) may
% add up to 1.0 per bin in the amplitude domain, but will add to
% lesser than 1 in the intensity domain.
%
% rectangular filter shapes should be unaffected, because bins are
% never split between band.
%
% Unfortunately we see a difference between the intensity of the broadband
% signal and the intensity sums of the narrowband signals of ~4dB.
  
  srate = 44100;
  channels_in = 1;
  fragsize = 64;
  wndlen = 2*fragsize;
  fftlen = 128;
  fb = [125 500 1000 2000 4000 6000];
  fft_bins = floor(fftlen / 2) + 1;
  
  dsc.instance = 'test_fftfb_level_summation_effects_rect_wave_irs';
  dsc.fragsize = fragsize;
  dsc.srate = srate;
  dsc.nchannels_in = channels_in;
  dsc.iolib = 'MHAIOParser';
  dsc.mhalib = 'fftfilterbank';
  dsc.mha.fftlen = fftlen;
  dsc.mha.f = fb;
  dsc.mha.fscale = 'log';
  dsc.mha.ovltype = 'rect';
  dsc.mha.ftype = 'center';
  dsc.mha.plateau = 0;
  
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  mha_set(mha,'',dsc);
  mha_set(mha,'cmd','start');

  impulse = [1, zeros(1,127)];
  response = mha_process_by_parser(mha, impulse);
  response = response';
				% plot impulse responses
				% plot(response');
 
				% plot shapes of frequency bands
				% plot(abs(fft(response')));

			       % plot summation of all frequency bands
			       % plot(abs(sum(fft(response),2)));

  assert_almost(ones(128,1), abs(sum(fft(response),2)), 1e-3);
