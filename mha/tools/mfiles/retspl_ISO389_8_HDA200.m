function [f,c] = retspl_ISO389_8_HDA200
% reference equivalent threshold sound pressure level
%
% retSPL from ISO 389-8:2004(E), ISO/TR 389-5:1998(E) (current at Dec. 2006)
%
% TransducerName: HDA200
%
% -- abstract: --
% ISO 389-8:2004 specifies reference equivalent threshold sound
% pressure levels (RETSPLs) for pure tones in the frequency range
% from 125 Hz to 8 kHz, applicable to the calibration of air
% conduction audiometers equipped with a particular model of
% circumaural earphones (SENNHEISER HDA 200).
%
% Some notes and references on the derivation and the test
% conditions used to determine the recommended reference levels are
% given in Annex A and the Bibliography.
%
% The sound attenuation of the earphone is given in Annex B. For
% speech audiometers of types A-E and B-E, the correction figures
% of the earphone for a free-field equivalent output are given in
% Annex C.
%
% ISO 389-5:2006 specifies reference equivalent threshold sound
% pressure levels (RETSPLs) of pure tones in the frequency range
% from 8 kHz to 16 kHz applicable to the calibration of air
% conduction audiometers for specific earphones.

  f = [...
      125,...
      160,...
      200,...
      250,...
      315,...
      400,...
      500,...
      630,...
      750,...
      800,...
      1000,...
      1250,...
      1500,...
      1600,...
      2000,...
      2500,...
      3000,...
      3150,...
      4000,...
      5000,...
      6000,...
      6300,...
      8000,...
      9000,...
      10000,...
      11200,...
      12500,...
      14000,...
      16000 ...
      ];

  c = [...
      30.5,...
      26.0,...
      22.0,...
      18.0,...
      15.5,...
      13.5,...
      11.0,...
      8.0,...
      6.0,...
      6.0,...
      5.5,...
      6.0,...
      5.5,...
      5.5,...
      4.5,...
      3.0,...
      2.5,...
      4.0,...
      9.5,...
      14.0,...
      17.0,...
      17.5,...
      17.5,...
      18.5,...
      22,...
      23,...
      28,...
      36,...
      56 ...
      ];
end
