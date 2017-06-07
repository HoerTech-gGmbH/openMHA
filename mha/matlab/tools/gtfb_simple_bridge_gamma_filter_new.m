function gammatone_filters = ...
 gtfb_simple_bridge_gamma_filter_new(coeffs, norm, srate, order, delays, phases, resynthesis_gains, prefilter_order)
%gammatone_filters = ...
%  gtfb_simple_bridge_gamma_filter_new(coeffs, norm, srate, order, ...
%                       delays, phases, resynthesis_gains, prefilter_order)
%
% creates a gammatone filterbank from
%
% coeffs - complex recursive filter coefficients
% norm - normalization factor per filter stage
% srate - sampling rate
% order - gammatone filter order
% delays - delays in samples to apply to the bands
% phases - phase corrections to apply to the bands in radiant
% resynthesis_gains - linear gain to apply before resynthesis
% prefilter_order - number of filter order to apply in
%                   gtfb_simple_bridge_gamma_prefilter

  gammatone_filters.order = order;
  gammatone_filters.A = coeffs(:);
  gammatone_filters.B = norm(:);
  gammatone_filters.stages = ...
      repmat(gtfb_simple_bridge_complex_bandpass_new_from_A_B(coeffs,norm),...
             order,1);
  gammatone_filters.delay = gtfb_simple_bridge_delay_new(delays);
  gammatone_filters.stages(1).B = ...
      gammatone_filters.stages(1).B .* exp(1i * phases(:));
  gammatone_filters.resynthesis_gains = resynthesis_gains(:);
  gammatone_filters.prefilter_order = prefilter_order;
  
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
