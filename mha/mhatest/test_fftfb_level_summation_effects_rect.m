function test_fftfb_level_summation_effects_rect
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2018 HörTech gGmbH
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
  
  srate = 44100;
  channels_in = 1;
  fragsize = 64;
  wndlen = 2*fragsize;
  fftlen = 128;
  fb = [125 500 1000 2000 4000 6000];
  fft_bins = floor(fftlen / 2) + 1;
  
  dsc.instance = 'test_fftfb_bug';
  dsc.iolib = 'MHAIOParser';
  dsc.mhalib = 'testplugin';
  dsc.mha.plugin_name = 'fftfilterbank';
  dsc.mha.config_in.domain = 'MHA_SPECTRUM';
  dsc.mha.config_in.fftlen = fftlen;
  dsc.mha.config_in.wndlen = wndlen;
  dsc.mha.config_in.fragsize = fragsize;
  dsc.mha.config_in.srate = srate;
  dsc.mha.config_in.channels = channels_in;
  dsc.mha.fftfilterbank.f = fb;
  dsc.mha.fftfilterbank.fscale = 'log';
  dsc.mha.fftfilterbank.ovltype = 'rect';
  dsc.mha.fftfilterbank.ftype = 'center';
  dsc.mha.fftfilterbank.plateau = 0;  
  
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  mha_set(mha,'',dsc);
  mha_set(mha,'mha.prepare',true);
    
  channels_out = mha_get(mha,'mha.config_out.channels');
  assert_equal(channels_in * length(fb), channels_out);
  
  % process
  input_signal = ones(fft_bins, channels_in);
  mha_set(mha,'mha.signal.input_spec',input_signal);
  output_signal = mha_get(mha,'mha.signal.output_spec');
  output_signal_sum = sum(output_signal, 2);

  % broadband level comparison. For simplicity, ignoring the double impact of
  % DC and nyquist bins.
  
  input_signal_intensity_sum = sum(input_signal.^2);
  output_signal_intensity_sum = sum(sum(output_signal.^2));

  db_difference_broadband = ...
     10*log10(input_signal_intensity_sum / output_signal_intensity_sum);

  % main speech band level comparison (500 - 2000 Hz)
  lower_bin = floor(500 / srate * fftlen) + 1;
  upper_bin = ceil(2000 / srate * fftlen) + 1;
  
  input_signal_intensity_sum =sum(input_signal(lower_bin:upper_bin).^2);
  output_signal_intensity_sum=sum(sum(output_signal(lower_bin:upper_bin,:).^2));

  db_difference_speechband = ...
     10*log10(input_signal_intensity_sum / output_signal_intensity_sum);
  
  %% Remove plots once this test works
  % figure, plot(abs(output_signal)), title('weighting factors in different bands');
  % figure, plot(sum(output_signal, 2)), title('sum of all bands');
  
  assert_difference_below(0, db_difference_broadband, 0.5);
  assert_difference_below(0, db_difference_speechband, 0.5);
