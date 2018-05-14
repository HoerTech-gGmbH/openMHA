function test_wave2spec_check_level
% Testing the level preservation property of wave2spec.
% Signal levels in time domain and spectral domain are compared.
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2018 HörTech gGmbH

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
  
  % Two different combination of STFT parameters
  stft_parameters = [struct('fftlen', 800, 'wndlen', 600, 'fragsize', 300)
                     struct('fftlen', 27, 'wndlen', 27, 'fragsize', 18)];

  for stft_index = 1:size(stft_parameters, 1)
    
    % All window types that wave2spec knows:
    window_types = {'hanning','rect','hamming','bartlett','blackman','user'};

    for window_index = 1:size(window_types, 2)

      % create an mha config with these stft parameters and this window type
      mha_config = generate_mha_test_config(stft_parameters(stft_index), ...
                                            window_types{window_index});

      % create an mha instance with this config
      mha = start_mha_test_instance_with_config(mha_config);
      
      % create a rectangular waveform signal as a test input signal
      input_signal = create_rectangular_wave(stft_parameters(stft_index));

      % process signal in mha
      mha_process_by_parser(mha, input_signal');

      % compare levels of last fragment in time domain and spectral domain
      level_wave = mha_get(mha, 'mha.level_wave.level_db');
      level_spec = mha_get(mha, 'mha.level_spec.level_db');
      assert_almost(level_wave, level_spec, 1e-7);
      assert_almost(93.9794, level_wave, 1e-7);
    end
  end
end

function mha_config = generate_mha_test_config(stft_parameters, window_type)
% create an mha config with these stft parameters and this window type

  mha_config.instance = ['test_wave2spec_check_level', num2str(stft_parameters.wndlen), window_type];
  mha_config.fragsize = stft_parameters.fragsize;
  mha_config.nchannels_in = 1;
  mha_config.mhalib = 'mhachain';
  mha_config.mha.algos = {'rmslevel:level_wave','wave2spec','rmslevel:level_spec','spec2wave'};
  mha_config.mha.wave2spec.wndlen = stft_parameters.wndlen;
  mha_config.mha.wave2spec.fftlen = stft_parameters.fftlen;
  mha_config.mha.wave2spec.wndtype = window_type;
  if isequal(window_type, 'user')
    mha_config.mha.wave2spec.userwnd = repeatable_rand(1, stft_parameters.wndlen, 'u'-0);
  end
  mha_config.iolib = 'MHAIOParser';
end

function  mha = start_mha_test_instance_with_config(mha_config)
% create an mha instance with this config
  mha = mha_start;
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  mha_set(mha,'',mha_config);
  mha_set(mha,'cmd','start');
end

function input_signal = create_rectangular_wave(stft_parameters);
  % Create a rectangular waveform signal as a test input signal.
  % Length of the created signal is a multiple of the stft hop size,
  % and at least the window length. 
  length = stft_parameters.fragsize * ceil(stft_parameters.wndlen / stft_parameters.fragsize);

  % create an input signal of all ones, some positive, some negative.
  % The frequency does not matter for this test:
  % Choose arbitrary period of 10 samples.
  input_signal = [ones(5,1); -ones(5,1)];
  while (size(input_signal) < length)
    input_signal = [input_signal;input_signal];
  end
  input_signal = input_signal(1:length);
end  

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
