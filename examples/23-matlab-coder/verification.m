function verification(name, signal)
%% function verification(name, signal)
%%
%% name     - openMHA plugin name
%% signal   - input signal to verify the outputs match
%% fragsize - fragment size to be used
%%
%% 
%% This function should be placed in the plugin directory along with 
%% the matlab process function.
%% It can be used to verify if Matlab process function and openMHA plugin have
%% identical outputs.
%%
%% This file is part of the HörTech Open Master Hearing Aid (openMHA)
%% Copyright © 2020 HörTech gGmbH
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


  %% Basic mha setup
  addpath('../../mhatest');
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  mha_set(mha,'nchannels_in', size(signal, 2)) % Channel info in second dimension with Matlab convention
  mha_set(mha,'fragsize', size(signal, 1))
  mha_set(mha,'iolib','MHAIOParser')
  mha_set(mha,'mhalib', name)
  mha_set(mha,'cmd', 'prepare');
  mha_set(mha,'io.input', signal'); % Signal dimensions flipped for MHA processing
  matlab_output = process(signal);
  mha_set(mha, 'cmd', 'start');
  mha_output = mha_get(mha,'io.output');
  mha_output = mha_output'; % Revert to original dimensions
  %% Test outputs from matlab processed and mha plugin
  assert_almost(matlab_output, mha_output, 10^-5);
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
