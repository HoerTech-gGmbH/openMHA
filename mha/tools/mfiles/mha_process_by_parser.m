function [output_signal] = mha_process_by_parser(mha, input_signal)
%
% Process a long signal by dividing it into chunks and sending them to MHAIOParser
%
% output_signal:  wave output of mha (ch,samples)
% input_signal:   wave input for mha (ch,samples)
% handle:     an MHA-handle, derived by mha = mha_start();
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2014 2018 HörTech gGmbH

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

  nchannels_in = mha_get(mha,'nchannels_in');
  fragsize = mha_get(mha,'fragsize');
  output_signal = {};
  for startindex = [1:fragsize:size(input_signal,2)]
    endindex = min([startindex + fragsize - 1, size(input_signal,2)]);
    chunk = zeros(nchannels_in, fragsize);
    chunk(:,1:(endindex-startindex+1)) = input_signal(:,startindex:endindex);
    mha_set (mha,'io.input',chunk);
    answer = mha_get(mha,'io.output');
    output_signal = [output_signal, {answer}];
  end
  output_signal = cell2mat(output_signal);
  output_signal = output_signal(:,1:size(input_signal,2));

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
