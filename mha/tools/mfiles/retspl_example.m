function [f,c] = retspl_example
% reference equivalent threshold sound pressure level
%
% Example retSPL table
%
% Do not use for audiometry!
%
% The correction determines the coupler level needed to achieve 0 dB
% HL, for a specific coupler and transducer type. In the freefield
% condition it is equivalent to the ISO hearing threshold. However,
% this is not the same as the supra threshold coupler-to-freefield
% correction, due to effects of reflections at the transducer on the
% hearing threshold.
  ;
  % frequency vector:
  f = [100,200];
  % correction vector:
  c = [0,1];

  % Important note:
  %
  % The data is linearly interpolated on a logarithmic frequency
  % scale. The lowest and highest value are used for frequencies outside
  % the specified frequency range.
  %
  % For a correct listing in the calibration dialog this function (and
  % file name) has to start with 'retspl'. The search is case
  % sensitive!
end
