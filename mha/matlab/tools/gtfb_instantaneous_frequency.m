function [dIF, ifest] = gtfb_instantaneous_frequency(ifest, gammatone_filtered)
% [dIF, ifest] = ifest_erb(ifest, gammatone_filtered)
%
% estimation of instantaneous frequency, difference in ERB from center 
% frequency of respective band.
%
% ifest              - instantaneous frequency estimator
% gammatone_filtered - the filterbank output signal of a complex valued
%                      gammatone filterbank. rows correspond to bands,
%                      columns to samples.
% dIF                - result matrix for all bands and samples
%
% ifest is the object containing states and parameters for IF estimation.
% the result is, for each band and sample, the instantaneous frequency in ERB.

  [num_bands, num_frames] = size(gammatone_filtered);
  
  if num_bands ~= length(ifest.filter_A)
    error('number of rows does not match number of channels');
  end

  dIF = zeros(size(gammatone_filtered));

  for band = 1:num_bands
    for frame = 1:num_frames
      Cin = gammatone_filtered(band, frame);
      C = Cin;
      C = C * ifest.if_state(band);
      C = ifest.filter_state(band) * ifest.filter_A(band) + ...
          C * ifest.filter_B(band);
      ifest.filter_state(band) = C;

      % calculate IF deviation:
      dIF(band,frame) = ...
	  (ifest.srate_div_2pi * angle(C) - ifest.center_freqs_hz(band)) / ...
          ifest.band_widths_hz(band);

      ifest.if_state(band) = conj(Cin);
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
