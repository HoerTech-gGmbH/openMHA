function sGt = gainrule_template(sAud,sFitmodel)
% function sGt = gainrule_template(sAud,sFitmodel)
%
% Template for a function implementing a gainrule (fitting rule, prescription
% rule) for openMHA dynamic compressors.
%
% Input Parameters: (all frequencies given in Hz)
% sAud - Auditory profile information: A struct with the following fields:
% sAud.client_id - 8 character ID of the hearing impaired person, containing
%                  initials and date of birth.
% sAud.id        - ID of the audiogram of this hearing impaired person. By
%                  default it contains the date of the audiogram measurement,
%                  but this can be changed by the person entering the data.
% sAud.l         - audiogram data for left ear
% sAud.r         - audiogram data for right ear
%
% The audiogram data is itself a struct and may contain the following fields,
% here exemplified only for the left ear:
% sAud.l.htl_ac  - Hearing thresholds for air conduction
% sAud.l.htl_bc  - Hearing thresholds for bone conduction
% sAud.l.ucl     - Uncomfortable levels
%
% Each of these three fields is only present if the corresponding measurments
% have been entered in the audiogram entry dialog by the user.
% The measurement data can be extracted from the following fields, here only
% exemplified for the air conduction thresholds:
%
% sAud.l.htl_ac.data - a vector of structs containing the air conduction
%                      thresholds for the left ear.
%                      Each struct element of the vector has the fields f, hl:
% sAud.l.htl_ac.data(n).f  - nth measured frequency for the left ear.
% sAud.l.htl_ac.data(n).hl - corresponding air conduction threshold in dB HL.
%
%
%
% sFitmodel - information about the dynamic compressor to fit:
% sFitmodel.frequencies - Center frequencies of the dynamic compression bands.
% sFitmodel.edge_frequencies - A vector containing in this order:
%                    - The lower cutoff frequency of the lowest frequency band
%                    - Crossover frequencies between adjacent bands
%                    - The upper cutoff frequency of the highest frequency band
% sFitmodel.levels - The input band levels in dB SPL for which to compute
%                    insertion gains.
% sFitmodel.channels - The number of ears to fit (1 or 2).
% sFitmodel.sides    - Which ears to fit: 'l', 'r', or 'lr'
%
% The vector sFitmodel.edge_frequencies always contains one more element than
% the vector sFitmodel.frequencies.
%
% Note that the audiogram frequencies may differ from the dynamic compressor
% frequencies, and may also differ across ears. Interpolation or extrapolation
% may be required as a consequence in order to compute the gains for all
% requested compression bands.
%
%
%
% Outpout parameter:
% sGt - A struct containing the dynamic compression gaintables in the following
%       fields:
% sGt.l - A (LxB) real-valued matrix containing applicable insertion gains in
%         dB for the left ear.
%         L is the number of input levels in sFitmodel.levels.
%         B is the number of center frequencies in sFitmodel.frequencies.
% sGt.r - The same for the right ear

% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2021 HörTech gGmbH
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

% This template gain rule assigns 6 dB insertion gain to all dynamic
% compression bands and input levels regardless of the audiogram.
% Specializations may take into account the distribution and widths of
% the frequency bands and the hearing threshold levels of the audiogram.
sGt.l = repmat(6, length(sFitmodel.levels), length(sFitmodel.frequencies));
sGt.r = sGt.l;
