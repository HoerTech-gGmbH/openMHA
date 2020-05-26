function sGt = gainrule_camfit_compr(sAud, sFitmodel)
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
% Compute gains for compression according to Moore et al. (1999) "Use of a
% loudness model for hearing aid fitting: II. Hearing aids with multi-channel
% compression." Brit. J. Audiol. (33) 157-170
%
% Please note that this gain rule by default applies a noise gate below 45 dB
% band level.  If you want a different noise gate level, please set the global
% variable CAMFIT_NOISEGATE before this function is called.  Please note that
% the gain rule now limits the gains so that in each band 100 dB output level
% is not exceeded. If you need higher output levels, please set the global
% variable CAMFIT_MAXOUT before this function is called.

% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2007 2009 2011 2013 2015 2017 2018 2020 HörTech gGmbH
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

  % International long-term average speech spectrum for speech with an overall 
  % level of 70dB in third-octave frequency bands, taken from Byrne et al.
  % (1994) J. Acoust. Soc. Am. 96(4) 2108-2120
  LTASS_freq = [63 80 100 125 160 200 250 315 400 500 630 800 1000 1250 ...
                1600 2000 2500 3150 4000 5000 6300 8000 10000 12500 16000];
  LTASS_edge_freq = [0, sqrt(LTASS_freq(1:end-1).*LTASS_freq(2:end)), 16000*2^(1/6)];
  LTASS_lev = [38.6 43.5 54.4 57.7 56.8 60.2 60.3 59.0 62.1 62.1 60.5 56.8 ...
             53.7 53.0 52.0 48.7 48.1 46.8 45.6 44.5 44.3 43.7 43.4 41.3 40.7];
  LTASS_intensity = 10.^(LTASS_lev/10);

  speech_level_65_in_dc_bands = ...
    LTASS_speech_level_in_frequency_bands(sFitmodel.edge_frequencies, 65);

  % minima in lowest level speech that needs to be understood is 38 dB below 
  % speech_level_65_in_dc_bands
  minima_distance = 38;

  % Conversion factors of HL thresholds to SPL thresholds
  Conv = isothr(sFitmodel.frequencies);

  Lmin = speech_level_65_in_dc_bands - minima_distance;
  
  % Interpolate audiogram
  for side=sFitmodel.side
    htl.(side) = freq_interp_sh([sAud.(side).htl_ac.data.f],...
                                [sAud.(side).htl_ac.data.hl],...
                                sFitmodel.frequencies);
    Gmin.(side) = htl.(side) + Conv - Lmin;
  end

  Lmid = speech_level_65_in_dc_bands;
  Gmid = gainrule_camfit_linear(sAud, sFitmodel);

  noisegate = 45;
  global CAMFIT_NOISEGATE;
  if ~isempty(CAMFIT_NOISEGATE)
      noisegate = CAMFIT_NOISEGATE;
  end

  max_output_level = 100;
  global CAMFIT_MAXOUT;
  if ~isempty(CAMFIT_MAXOUT)
    max_output_level = CAMFIT_MAXOUT;
  end

  for side=sFitmodel.side
    compression_ratio.(side) = minima_distance ./ max(Lmid+Gmid.(side)(end,:) - Lmin-Gmin.(side), 13);
    compression_ratio.(side) = max(compression_ratio.(side), 1);

    sGt.(side) = gains(Lmin,Gmin.(side),compression_ratio.(side),sFitmodel.levels);
    
    % set negative gains to zero
    sGt.(side) = (sGt.(side) + abs(sGt.(side))) / 2;
    
    % where output level is greater than max_output_level, reduce gain
    output_levels = sGt.(side) + repmat(sFitmodel.levels(:),1,length(Gmin.(side)));
    safe_output_levels = min(output_levels, max_output_level);
    sGt.(side) = sGt.(side) - (output_levels - safe_output_levels);

    % set all gains to zero for 0dB HL flat audiogram
    sGt.(side) = sGt.(side) * any(htl.(side));
    
    sGt.noisegate.(side).level = noisegate*ones(size(sFitmodel.frequencies));
    sGt.noisegate.(side).slope = ones(size(sFitmodel.frequencies));
  end

function g=gains(compr_thr_inputs, compr_thr_gains, compression_ratios,levels) 
  levels = repmat(levels(:), 1, length(compr_thr_inputs));
  compr_thr_inputs = repmat(compr_thr_inputs, size(levels,1), 1);
  compr_thr_gains = repmat(compr_thr_gains, size(levels,1), 1);
  compression_ratios = repmat(compression_ratios, size(levels,1), 1);

  compr_thr_outputs = compr_thr_inputs + compr_thr_gains;
  outputs = (levels - compr_thr_inputs) ./ compression_ratios + compr_thr_outputs;
  g = outputs - levels;
