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

% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2007 2011 2013 2015 2016 2017 HörTech gGmbH
%
% openMHA is free software: you can redistribute it and/or modify
% it under the terms of the GNU Affero General Public License as published by
% the Free Software Foundation, version 3 of the License.
%
% openMHA is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU Affero General Public License, version 3 for more details.
%
% You should have received a copy of the GNU Affero General Public License, 
% version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

  libaudprof();

  intercept_frequencies = ...
      [125 250 500 750 1000 1500 2000 3000 4000 5000 5005];
  intercepts = [-11 -10 -8 -6 0 -1 1 -1 0 1 1];
  
  intercepts = freq_interp_sh(intercept_frequencies, ...
                              intercepts, ...
                              sFitmodel.frequencies);
  noisegate = 45;
  global CAMFIT_NOISEGATE;
  if ~isempty(CAMFIT_NOISEGATE)
      noisegate = CAMFIT_NOISEGATE;
  end

  % Interpolate audiogram
  for side=sFitmodel.side
     sT = audprof.threshold_get(sAud, side, 'htl_ac');
     htl.(side) = freq_interp_sh([sT.data.f],[sT.data.hl],...
                                 sFitmodel.frequencies);
    insertion_gains.(side) = htl.(side) * 0.48 + intercepts;

    % according to B. Moore (1998), "Use of a loudness model for hearing-aid
    % fitting. II. Hearing aids with multi-channel compression" Brit. J.
    % Audiol. (33) 157-170, p. 159, do not permit negative insertion gains in
    % practice.
    insertion_gains.(side)(insertion_gains.(side) < 0) = 0;

    % set all gains to 0 for 0dB HL flat audiogram
    insertion_gains.(side) = insertion_gains.(side) * any(htl.(side));
    
    sGt.(side) = repmat(insertion_gains.(side), length(sFitmodel.levels),1);
    sGt.noisegate.(side).level = noisegate*ones(size(sFitmodel.frequencies));
    sGt.noisegate.(side).slope = ones(size(sFitmodel.frequencies));
  end
