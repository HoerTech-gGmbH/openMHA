function complex_bandpass_filterbank = ...
      gtfb_simple_bridge_complex_bandpass_new_from_A_B(A,B)

% Creates a new complex bandpass filterbank
% complex_bandpass_filterbank = ...
%      gtfb_simple_bridge_complex_bandpass_new_from_A_B(A,B)
%
% A is a vector of complex filter coefficients, one coefficient for each band
% B is a vector of possibly complex weight factors

  complex_bandpass_filterbank.type = 'gtfb_simple_bridge_complex_bandpass';
  complex_bandpass_filterbank.A = A(:);
  complex_bandpass_filterbank.B = B(:);
  complex_bandpass_filterbank.state = complex_bandpass_filterbank.A * 0;

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
