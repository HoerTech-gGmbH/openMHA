function sGt = gainrule_camfit_linear(sAud, sFitmodel)
% sGt = gainrule_camfit_linear(sAud, sFitmodel)
% sAud.frequencies contains the audiogram frequencies
% sAud.l.htl       contains the subject-specific hearing threshold levels in
%                  dB HL for the left ear
% sAud.r.htl       the same for the right ear
% sFitmodel.frequencies contains the center frequencies for the amplification bands
% sFitmodel.levels      contains input levels in SPL for which to compute the gains
% sGt              contains 2 matrices, l and r that contain gains in dB
%                  for every input level (rows) and band (columns)
% sGt              may also contain an expansion_slope field.
% Linear cambridge rule for hearing aid fittings.  
% Implemented as described in B. Moore (1998), "Use of a loudness model for 
% hearing-aid fitting. I. Linear hearing aids" Brit. J. Audiol. (32) 317-335
  
  intercept_frequencies = ...
      [125 250 500 750 1000 1500 2000 3000 4000 5000 5005];
  intercepts = [-11 -10 -8 -6 0 -1 1 -1 0 1 1];
  
  intercepts = freq_interp_sh(intercept_frequencies, ...
			      intercepts, ...
			      sFitmodel.frequencies);

  % Interpolate audiogram
  for side=sFitmodel.side
%    htl.(side) = freq_interp_sh([sAud.(side).htl_ac.data.f],...
%				[sAud.(side).htl_ac.data.hl],...
%				sFitmodel.frequencies);
     htl.(side) = freq_interp_sh([sAud.frequencies],...
				[sAud.(side).htl],...
				sFitmodel.frequencies);
    insertion_gains.(side) = htl.(side) * 0.48 + intercepts;

    % according to B. Moore (1998), "Use of a loudness model for hearing-aid
    % fitting. II. Hearing aids with multi-channel compression" Brit. J.
    % Audiol. (33) 157-170, p. 159, do not permit negative insertion gains in
    % practice.
    insertion_gains.(side)(insertion_gains.(side) < 0) = 0;

    sGt.(side) = repmat(insertion_gains.(side), length(sFitmodel.levels),1);
    sGt.noisegate.(side).level = 45*ones(size(sFitmodel.frequencies));
    sGt.noisegate.(side).slope = ones(size(sFitmodel.frequencies));
  end
