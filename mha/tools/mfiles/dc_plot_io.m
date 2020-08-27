function output_levels = dc_plot_io(gtmin, gtstep, gtdata, input_levels, log_interp)
% function output_levels = dc_plot_io(gtmin, gtstep, gtdata, input_levels, log_interp)
%
% gtmin, gtstep, gtdata - configuration variable values for the dc plugin
% input_levels          - mxn input matrix with test input levels in dB SPL
% log_interp            - boolean if logarithmic interpolation should be used
% output_levels         - mxn matrix of produced output levels for each of the
%                         n input levels and each of the m bands/channels
%
% This function temporarily starts an openMHA instance and has it process test
% data. Your Octave/Matlab session needs to be set up to call mha_start, mha_set
% etc. Please refer to the getting started guide for examples how to do this:
% http://www.openmha.org/docs/openMHA_starting_guide.pdf
% The function plots the input-output characteristic that it measures in a new
% figure.

% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2019 2020 HörTech gGmbH
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

  channels = [size(gtmin,2), size(gtstep,2), size(gtdata,1)];
  if any(diff(channels))
    error 'inconsistent number of channels';
  end
  channels = channels(1);

  dsc.nchannels_in = channels;
  dsc.fragsize = size(input_levels,2);
  dsc.mhalib = 'transducers';
  dsc.mha.plugin_name = 'dc';
  dsc.iolib = 'MHAIOParser';
  dsc.mha.dc.gtmin = gtmin;
  dsc.mha.dc.gtstep = gtstep;
  dsc.mha.dc.gtdata = gtdata;
  dsc.mha.dc.log_interp = log_interp;

  % no level filtering for direct level control
  dsc.mha.dc.tau_attack = zeros(1,channels);
  dsc.mha.dc.tau_rmslev = zeros(1,channels);
  dsc.mha.dc.tau_decay = zeros(1,channels);

  % make sure we can get input and output levels in and out of mha
  max_level = max(max(input_levels)) + max(max(gtdata)) + 6;
  dsc.mha.calib_in.peaklevel = ones(1,channels) * max_level;
  dsc.mha.calib_out.peaklevel = ones(1,channels) * max_level;

  mha = mha_start();
  mha_set(mha,'',dsc);
  mha_set(mha,'cmd','start');
  
  output_levels = zeros(channels, 0);
    
  amplitudes = 10.^((input_levels-max_level)/20);
  % Check if input is vector or matrix
  if size(amplitudes,1) == 1
  mha_set(mha, 'io.input', repmat(amplitudes,channels,1));
  else
  mha_set(mha, 'io.input', amplitudes);
  end
  outamps = mha_get(mha, 'io.output');
  output_levels = log10(outamps) * 20 + max_level;

  mha_set(mha,'cmd','quit');

  plot(input_levels', output_levels');
  xlabel('input level / dB');
  ylabel('output level / dB');
end
