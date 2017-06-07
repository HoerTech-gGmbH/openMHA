function [output, delay] = gtfb_simple_bridge_delay(delay, input)
% [output, delay] = gtfb_simple_bridge_delay(delay, input)
%
% Each band (row) of the input data will be delayed by a band-dependend
% ammount of samples
%
% PARAMETERS
% delay   A delay structure created from gtfb_simple_bridge_delay_new.
%         The delay will be returned with updated delayline states as the second
%         return parameter
% input   A matrix containing the signal to delay.  Each row
%         corresponds to a filterbank band
% output  A real matrix containing the delay's output

[number_of_bands, number_of_samples] = size(input);
if number_of_bands ~= length(delay.delays_samples)
  if number_of_bands == 1
      number_of_bands = length(delay.delays_samples);
      input = repmat(input,number_of_bands,1);
  else
      error('input rows must match the number of bands');
  end
end

output = zeros(number_of_bands, number_of_samples);
for band = [1:number_of_bands]
  if (delay.delays_samples(band) == 0)
    output(band,:) = input(band,:);
  else
    tmp_out = [delay.memory(band,1:delay.delays_samples(band)), input(band,:)];
    delay.memory(band,1:delay.delays_samples(band)) = ...
        tmp_out(number_of_samples+1:length(tmp_out));
    output(band,:) = tmp_out(1:number_of_samples);
  end
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
