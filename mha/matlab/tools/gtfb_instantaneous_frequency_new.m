function [ifest] = gtfb_instantaneous_frequency_new(srate, center_freqs_hz, band_widths_hz, filter_taus)
% ifest = gtfb_instantaneous_frequency_new(srate, center_freqs_hz, ...
%                                          band_widths_hz, filter_taus)
%
% Constructor for instantaneous frequency estimator object
%
% all parameters except srate are vectors of same size, one element per
% gammatone filter band
% srate           - scalar, sampling frequency / Hz
% center_freqs_hz - Center frequencies of filters in gammatone filterbank in Hz
% band_widths_hz  - Bandwidths of gammatone filters in Hz. Used to convert
%                   instantaneous frequency difference from center frequency to
%                   approximate ERB difference.
% filter_taus     - time constants / seconds for filtering instantaneous 
%                   frequency    

  ifest.center_freqs_hz = center_freqs_hz(:);
  ifest.band_widths_hz = band_widths_hz(:);
  ifest.filter_A = exp( -1.0./(filter_taus(:) * srate) );
  ifest.filter_A((filter_taus(:) <= 0) | (srate <= 0)) = 0;
  ifest.filter_B = 1 - ifest.filter_A;
  ifest.filter_state = zeros(size(ifest.filter_A));
  ifest.if_state = ifest.filter_state;
  ifest.srate_div_2pi = srate / (2*pi);

  if any(diff([length(ifest.center_freqs_hz), length(ifest.band_widths_hz), ...
	       length(ifest.filter_A)]))
    error('vector parameters need to have same length');
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
