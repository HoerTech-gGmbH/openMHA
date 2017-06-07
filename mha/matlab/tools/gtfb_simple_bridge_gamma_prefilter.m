function [output_signal, gammatone_filters] = ...
      gtfb_simple_bridge_gamma_prefilter(gammatone_filters, input_signal)

% Apply first part of gammatone filterbank orders to input signal
% [output_signal, gammatone_filters] = ...
%      gtfb_simple_bridge_gamma_prefilter(gammatone_filters, input_signal)

output_signal = input_signal;
for stage = 1:gammatone_filters.prefilter_order
 [output_signal, gammatone_filters] = ...
   gtfb_simple_bridge_gamma_filter(gammatone_filters, output_signal, stage);
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
