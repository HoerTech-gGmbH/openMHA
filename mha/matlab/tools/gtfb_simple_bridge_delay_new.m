function delay = gtfb_simple_bridge_delay_new(delays_samples)
% delay = gtfb_simple_bridge_delay_new(delays_samples)
%
% creates a new delay object that applies band-dependent delays.

% delays_samples The channel dependent delays in samples.
% delay         The new delay object

number_of_bands = length(delays_samples);
delay.type           = 'gtfb_simple_bridge_delay';
delay.delays_samples = delays_samples;
delay.memory         = zeros(number_of_bands, max(delay.delays_samples));

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
