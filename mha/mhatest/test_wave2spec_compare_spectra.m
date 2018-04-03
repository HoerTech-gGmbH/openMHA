function test_wave2spec_compare_spectra
% Testing the analysis part of overlap-add when hanning window is used.
% STFT spectra computed with MHA plugin wave2spec are compared to spectra
% computed with matlab/octave.  Test performed for 3 different combinations
% of FFT length, window length, and hopsize.
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2005 2006 2007 2015 2018 HörTech gGmbH

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
  
% 4 sets of 3 parameters: window shift, window length, fft length
params = [200 400 512
          100 200 256
          64  128 182
          64  128 324];

for setup = 1:size(params, 1)
  % mha description
  dsc.instance = ['test_wave2spec' , num2str(setup)];
  dsc.fragsize = params(setup,1);
  dsc.nchannels_in = 1;
  dsc.mhalib = 'mhachain';
  dsc.mha.algos = {'wave2spec','save_spec','acmon','spec2wave'};
  dsc.mha.wave2spec.wndlen = params(setup,2);
  dsc.mha.wave2spec.fftlen = params(setup,3);
  dsc.iolib = 'MHAIOParser';
  
  input_signal = repeatable_rand(dsc.fragsize, dsc.nchannels_in, 0) - 0.5;
  input_signal = repmat(input_signal, 2, 1);
  
  [hanning, zero_padding_1, zero_padding_2] = ...
      mha_hanning(dsc.mha.wave2spec.wndlen, dsc.mha.wave2spec.fftlen);

  matlab_spec = fft([zero_padding_1
                     input_signal .* hanning
                     zero_padding_2]).';
  matlab_spec = matlab_spec(1:(floor(dsc.mha.wave2spec.fftlen / 2)+1));
  mha = mha_start;
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  mha_set(mha,'',dsc);
  mha_set(mha,'cmd','start');
  
  mha_process_by_parser(mha, input_signal');

  mha_spec = mha_get(mha, 'mha.acmon.save_spec');

  assert_almost(matlab_spec, mha_spec, 1e-4);
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
