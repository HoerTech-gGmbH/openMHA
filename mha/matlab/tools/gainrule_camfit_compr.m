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

  % International long-term average speech spectrum for speech with an overall 
  % level of 70dB in third-octave frequency bands, taken from Byrne et al.
  % (1994) J. Acoust. Soc. Am. 96(4) 2108-2120
  LTASS_freq = [63 80 100 125 160 200 250 315 400 500 630 800 1000 1250 ...
                1600 2000 2500 3150 4000 5000 6300 8000 10000 12500 16000];
  LTASS_edge_freq = [0, sqrt(LTASS_freq(1:end-1).*LTASS_freq(2:end)), 16000*2^(1/6)];
  LTASS_lev = [38.6 43.5 54.4 57.7 56.8 60.2 60.3 59.0 62.1 62.1 60.5 56.8 ...
             53.7 53.0 52.0 48.7 48.1 46.8 45.6 44.5 44.3 43.7 43.4 41.3 40.7];
  LTASS_intensity = 10.^(LTASS_lev/10);

  % Compute level for 65dB speech in dyncomp bands. For the lowest 3 and 
  % the last (8th) band, this computation yields the same levels as detailed
  % in Moore 1999 for his border frequencies, if a lower cutoff frequency below
  % 63Hz and an upper cutoff frequency below 6300Hz is assumed.
  % Moore seems to perform some creative intensity splitting for his bands 4 to
  % 6, which will not be recreated here.
  % Band 7 in Moore 1999 is probably erroneous (subtracted 0.5dB instead of 
  % 5dB from LTASS_lev).
  speech_level_65_in_dc_bands = zeros(size(sFitmodel.frequencies));
  for band = 1:length(sFitmodel.frequencies)
    f_range = sFitmodel.edge_frequencies(band:(band+1));
    intensity_sum = 0;
    for ltass_band = 1:length(LTASS_freq)
        ltass_range = LTASS_edge_freq(ltass_band:(ltass_band+1));
        intersection = range_intersection(f_range, ltass_range);
        portion = diff(intersection) / diff(ltass_range);
        intensity_sum = intensity_sum + LTASS_intensity(ltass_band) * portion;
    end
    speech_level_70_in_dc_band = 10*log10(intensity_sum);
    speech_level_65_in_dc_bands(band) = speech_level_70_in_dc_band - 5;
  end

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
    %htl.r = interp1(log(sAud.frequencies), sAud.r.htl, log(sFitmodel.frequencies));
  
    Gmin.(side) = htl.(side) + Conv - Lmin;
    %Gmin.r = htl.r + Conv - Lmin;
  end

  Lmid = speech_level_65_in_dc_bands;
  Gmid = gainrule_camfit_linear(sAud, sFitmodel);

  for side=sFitmodel.side
    compression_ratio.(side) = minima_distance ./ max(Lmid+Gmid.(side)(end,:) - Lmin-Gmin.(side), 13);
    compression_ratio.(side) = max(compression_ratio.(side), 1);
    %compression_ratio.r = minima_distance ./ max(Lmid+Gmid.r(end,:) - Lmin-Gmin.r, 13);
    %compression_ratio.r = max(compression_ratio.r, 1);

    sGt.(side) = gains(Lmin,Gmin.(side),compression_ratio.(side),sFitmodel.levels);
    %sGt.r =
    %gains(Lmin,Gmin.r,compression_ratio.r,sFitmodel.levels);
    sGt.noisegate.(side).level = 45*ones(size(sFitmodel.frequencies));
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
