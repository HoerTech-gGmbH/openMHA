function [output_signal, filterbank] = ...
  gtfb_simple_bridge_complex_bandpass_filter(filterbank, input_signal)

% [output_signal, filterbank] = ...
%    gtfb_simple_bridge_complex_bandpass_filter(filterbank, input_signal)
% Apply 1st order IIR complex bandpass filter

  [num_bands, num_frames] = size(input_signal);
  output_signal = zeros(size(input_signal));
  for frame = 1:num_frames
    filterbank.state = filterbank.state .* filterbank.A + ...
        input_signal(:,frame) .* filterbank.B;
    output_signal(:,frame) = filterbank.state;
  end

%%-----------------------------------------------------------------------------
%%
%%   Copyright (C) 2010 HoerTech gGmbH
%%                        http://www.hoertech.de/
%%
%%   Permission to use, copy, and distribute this software/file and its
%%   documentation for any purpose without permission by HoerTech
%%   is not granted.
%%   
%%   This software is provided "as is" without expressed or implied warranty.
%%
%%   Author: Tobias Herzke
%%
%%-----------------------------------------------------------------------------
