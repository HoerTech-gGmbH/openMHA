function sGt = camfit_ci_candidate_unilateral(sAud_, sCfg, side)
% sGt = camfit_ci_candidate_unilateral(sAud, sCfg, side)
% sAud.frequencies contains the audiogram frequencies
% sAud.l.htl       contains the subject-specific hearing threshold levels in
%                  dB HL for the left ear
% sAud.r.htl       the same for the right ear
% sCfg.frequencies contains the center frequencies for the amplification bands
% sCfg.levels      contains input levels in SPL for which to compute the gains
% side             either 'l' or 'r', denotes the ear to aid.
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
  LTASS_lev = [38.6 43.5 54.4 57.7 56.8 60.2 60.3 59.0 62.1 62.1 60.5 56.8 ...
             53.7 53.0 52.0 48.7 48.1 46.8 45.6 44.5 44.3 43.7 43.4 41.3 40.7];
  LTASS_intensity = 10.^(LTASS_lev/10);
  sAud = audprof2aud( sAud_ );
  
  % Compute level for 65dB speech in dyncomp bands. For the lowest 3 and 
  % the last (8th) band, this computation yields the same levels as detailed
  % in Moore 1999 for his border frequencies, if a lower cutoff frequency below
  % 63Hz and an upper cutoff frequency below 6300Hz is assumed.
  % Moore seems to perform some creative intensity splitting for his bands 4 to
  % 6, which will not be recreated here.
  % Band 7 in Moore 1999 is probably erroneous (subtracted 0.5dB instead of 
  % 5dB from LTASS_lev).
  speech_level_65_in_dc_bands = zeros(size(sCfg.frequencies));
  for band = 1:length(sCfg.frequencies)
    f_low = sCfg.edge_frequencies(band);
    f_high =  sCfg.edge_frequencies(band + 1);
    intensity_sum = sum(LTASS_intensity((LTASS_freq > f_low) & ...
                                        (LTASS_freq <= f_high)));
    speech_level_70_in_dc_band = 10*log10(intensity_sum);
    speech_level_65_in_dc_bands(band) = speech_level_70_in_dc_band - 5;
  end

  % minima in lowest level speech that needs to be understood is 38 dB below 
  % speech_level_65_in_dc_bands.
  % For the ci candidates in AIN, we have changed this to 18dB, since the
  % CI candidates need to understand 65dB SPL speech presented in speech
  % tests.
  minima_distance = 18;
  max_compression = 2.92; %see Moore, Camfit
  threshold = minima_distance/max_compression;
  % Conversion factors of HL thresholds to SPL thresholds
  Conv = isothr(sCfg.frequencies);

  % Interpolate audiogram
  sAud.l.htl = interp1(sAud.frequencies(~isnan(sAud.l.htl)),sAud.l.htl(~isnan(sAud.l.htl)),sAud.frequencies,'pchip');
  sAud.r.htl = interp1(sAud.frequencies(~isnan(sAud.r.htl)),sAud.r.htl(~isnan(sAud.r.htl)),sAud.frequencies,'pchip');
  
  htl.l = interp1(log(sAud.frequencies), sAud.l.htl, log(sCfg.frequencies));
  htl.r = interp1(log(sAud.frequencies), sAud.r.htl, log(sCfg.frequencies));
  
  % adapt the audiogram: Eliminate detrimental effect from useless
  % amplification
  htl = adapt_audiogram(sCfg.frequencies, htl, side);
  
  Lmin = speech_level_65_in_dc_bands - minima_distance;
  Gmin.l = htl.l + Conv - Lmin;
  Gmin.r = htl.r + Conv - Lmin;

  Lmid = speech_level_65_in_dc_bands;
  Gmid = gainrule_camfit_linear(sAud_, sCfg);

  compression_ratio.l = minima_distance ./ max(Lmid+Gmid.l(end,:) - Lmin-Gmin.l, threshold);
  compression_ratio.l = max(compression_ratio.l, 1);
  compression_ratio.r = minima_distance ./ max(Lmid+Gmid.r(end,:) - Lmin-Gmin.r, threshold);
  compression_ratio.r = max(compression_ratio.r, 1);

  sGt.l = gains(Lmin,Gmin.l,compression_ratio.l,sCfg.levels);
  sGt.r = gains(Lmin,Gmin.r,compression_ratio.r,sCfg.levels);

  sGt.l(sGt.l > 80) = 80;
  sGt.r(sGt.r > 80) = 80;

  % Where the threshold is 0, the gains should also be 0
  for s = 'lr'
      for i = 1:length(htl.(s))
          if htl.(s)(i) == 0
              sGt.(s)(:,i) = 0;
          end
      end
      % avoid negative gains
      sGt.(s) = (sGt.(s) + abs(sGt.(s))) / 2;
  end
  
function g=gains(compr_thr_inputs, compr_thr_gains, compression_ratios,levels) 
  levels = repmat(levels(:), 1, length(compr_thr_inputs));
  compr_thr_inputs = repmat(compr_thr_inputs, size(levels,1), 1);
  compr_thr_gains = repmat(compr_thr_gains, size(levels,1), 1);
  compression_ratios = repmat(compression_ratios, size(levels,1), 1);

  compr_thr_outputs = compr_thr_inputs + compr_thr_gains;
  outputs = (levels - compr_thr_inputs) ./ compression_ratios + compr_thr_outputs;
  g = outputs - levels;

function htl=adapt_audiogram(frequencies, htl, side)
  % adapt the audiogram: Eliminate detrimental effect from useless
  % amplification. Amplify at the frequencies where the user has
  % lowest thresholds, special consideration of the frequency range
  % 500Hz-2Khz important for speech reception.
  for s = 'lr'
      h = htl.(s);
      % what is the minimum htl?
      m_total = min(h);
      if (m_total < 60)
          useless = 85;
      elseif (m_total < 70)
          useless = 95;
      elseif (m_total < 80)
          useless = 100;
      elseif (m_total < 90)
          useless = 105;
      else
          useless = 110;
      end
      % what is the minimum in the speech relevant range from 500-2000
      indexes = find(frequencies >= 500 & frequencies <= 2000);
      m_speech = min(h(indexes));
      old_useless = useless;
      while sum((h(indexes) < useless)) >= (3 - (m_total < m_speech))
          old_useless = useless;
          useless = useless - 5;
      end
      useless = old_useless;

      h(h >= useless) = 0;
      h(frequencies > 4800) = 0;
      htl.(s) = h; 
      if ~isempty(side) && ~isequal(side(1), s)
          htl.(s)(:) = 0;
      end
  end
  