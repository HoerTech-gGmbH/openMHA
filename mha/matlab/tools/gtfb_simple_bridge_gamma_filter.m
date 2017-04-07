function [output_signal, gammatone_filters] = ...
      gtfb_simple_bridge_gamma_filter(gammatone_filters, input_signal, stage)

% Apply specified stage of gammatone filterbank to input signal
% [output_signal, gammatone_filters] = ...
%      gtfb_simple_bridge_gamma_filter(gammatone_filters, input_signal, stage)

if stage > gammatone_filters.order
  output_signal = input_signal;
  return;
end

if (stage == 1) &&  isfield(gammatone_filters,'delay')
    [input_signal, gammatone_filters.delay] = ...
        gtfb_simple_bridge_delay(gammatone_filters.delay, input_signal);
end

[output_signal, gammatone_filters.stages(stage)] = ...
    gtfb_simple_bridge_complex_bandpass_filter(...
        gammatone_filters.stages(stage), input_signal);

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
