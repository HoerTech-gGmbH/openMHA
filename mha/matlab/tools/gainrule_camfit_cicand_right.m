function sGt = gainrule_camfit_cicand_right(sAud, sCfg)
% sGt = gainrule_camfit_linear(sAud, sCfg)
% sAud.frequencies contains the audiogram frequencies
% sAud.l.htl       contains the subject-specific hearing threshold levels in
%                  dB HL for the left ear
% sAud.r.htl       the same for the right ear
% sCfg.frequencies contains the center frequencies for the amplification bands
% sCfg.levels      contains input levels in SPL for which to compute the gains
% sGt              contains 2 matrices, l and r that contain gains in dB
%                  for every input level (rows) and band (columns)
% sGt              may also contain an expansion_slope field.
% Compute gains for compression according to Moore et al. (1999) "Use of a
% loudness model for hearing aid fitting: II. Hearing aids with multi-channel
% compression." Brit. J. Audiol. (33) 157-170
% This rule is modified to fit severe hearing losses, on right ear only.
  sAud = audprof2aud( sAud );
sGt = camfit_ci_candidate_unilateral(sAud, sCfg, 'r');
