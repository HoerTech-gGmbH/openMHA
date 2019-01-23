function test_fftfb_hanning_log_edge
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2005 2006 2008 2009 2012 2018 HörTech gGmbH
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
  
  srate = 44100;
  channels_in = 1;
  fragsize = 64;
  wndlen = 2*fragsize;
  fftlen = 1000;
  fb = [0.5 1.5 4.5 9.5 22.5 29.5 36.5 44.5] * (srate/(fftlen/2));
  fft_bins = floor(fftlen / 2) + 1;
  
  dsc.instance = 'test_fftfb_hanning_log_edge';
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
  dsc.mha.fftfilterbank.ovltype = 'hanning';
  dsc.mha.fftfilterbank.ftype = 'edge';
  dsc.mha.fftfilterbank.plateau = 0.9;  
    
  mha = mha_start();
  mha_set(mha,'',dsc);
  mha_set(mha,'mha.prepare',true);
  %mha_query(mha,'','')
  
  channels_out = mha_get(mha,'mha.config_out.channels');
  assert_equal(channels_in * (length(fb))-1, channels_out);
  
  % process
  input_signal = ones(fft_bins, channels_in);
  mha_set(mha,'mha.signal.input_spec',input_signal);
  output_signal = mha_get(mha,'mha.signal.output_spec');
  output_signal_sum = sum(output_signal, 2);

  
  %% Remove plots once this test works
  %figure, plot(abs(output_signal)), title('signal in different bands');
  %figure, plot(sum(output_signal, 2)), title('sum of all bands');
  
  % check: combined signal should be 0 outside of the covered range,
  %        1 inside the covered range, and 0.5 at the boundaries.
  lower_bin = round(fb(1) / srate * fftlen);
  upper_bin = round(fb(end) / srate * fftlen);
  
  expected_sum = [ zeros(lower_bin, channels_in)
                   repmat(0.5, 1, channels_in)
                   ones(upper_bin - lower_bin - 1, channels_in)
                   repmat(0.5, 1, channels_in)
                   zeros(fft_bins - upper_bin - 1, channels_in) ];
  
  %figure, plot([expected_sum, output_signal_sum])
  
  assert_difference_below(expected_sum, output_signal_sum, 1.4e-5);

  
