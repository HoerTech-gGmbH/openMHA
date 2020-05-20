function sGt = gainrule_CR3_NALRP( sAud, sFitmodel )
  %% Configuration parameters:
  c_slope = 1/3;
  l_target_ltass = 65;
  l_kneepoint_ltass = 40;
  %% gain rule:
  sAud = audprof2aud( sAud );
  sGt = struct;
  sGt.compression = struct;
  % knee point and target level in compressor bands:
  l_kneepoint = speech_level_in_dc_bands( sFitmodel, l_kneepoint_ltass );
  l_target = speech_level_in_dc_bands( sFitmodel, l_target_ltass );
  for channel='lr'
      g_NALRP = NalRP(sAud.(channel).htl,...
                      sAud.frequencies,...
                      sFitmodel.frequencies);
      % maximum gain is applied at kneepoint:
      gmax = g_NALRP' + (c_slope-1).*(l_kneepoint-l_target);
      % configure compressor parameter for model-based compressors:
      l_par = struct('gain',gmax,...
                     'l_kneepoint',l_kneepoint,...
                     'c_slope',c_slope);
      sGt.compression.(channel) = l_par;
      % calculate gain table for gaintable-based compressors:
      for band=1:numel(sFitmodel.frequencies)
          sGt.(channel)(:,band) = ...
              (sFitmodel.levels<l_kneepoint(band)).*gmax(band) ...
              + (sFitmodel.levels>=l_kneepoint(band)).* ...
              (gmax(band)+ ...
               (sFitmodel.levels-l_kneepoint(band))*(c_slope-1));
      end
  end
  
function g = NalRP(htl,fhtl,f)
  f=f(:);%must be column vector
  htl = htl(:);
  fhtl = fhtl(:);

  fPrescr=[250 500 1000 1500 2000 3000 4000 6000]';
  ifP500=2;ifP1000=3;ifP2000=5;%indices into fPrescr
  gainZero=[-17 -8 1 1 -1 -2 -2 -2]';%NAL speech-equalizing REIG for HTL=0
  gainFactor=0.31;%times hearing loss, at each freq
  gainFactorM=0.15;%times 3-freq average hearing loss
  gainFactorP=0.20;%times 3-freq average part exceeding 60 dB HL
  ;
  %gain increases as 46% of HTL below 60, and 66% above 60 dB HL
  hSevere2000=[95 100 105 110 115 120]';%severe loss at 2000 Hz
  gSevere=[4 3 0 -1 -2 -2 -2 -2 ;...%Byrne et al(1991), Fig 9.
	   6 4 0 -2 -3 -3 -3 -3 ;...
	   8 5 0 -3 -5 -5 -5 -5;...
	   11 7 0 -3 -6 -6 -6 -6;...
	   13 8 0 -4 -8 -8 -8 -8;...
	   15 9 0 -5 -9 -9 -9 -9];
  %corrections for high loss at 2000 Hz

  %extrapolate
  fhtl=[1;fhtl;20000];
  %extrapolate
  htl=[htl(1);htl;htl(end)];

  hLoss=interp1(log(fhtl),htl,log(fPrescr));
  hLoss3FA=mean(hLoss([ifP500,ifP1000,ifP2000]));%500, 1000, 2000 Hz
  gPrescr=gainFactor*hLoss+gainFactorM*hLoss3FA+gainZero;%NAL-R original

  if hLoss3FA>60%NAL-RP correction for severe loss
    gPrescr=gPrescr+gainFactorP*(hLoss3FA-60);
  end;
  if hLoss(ifP2000)>=95%add extra NAL-RP correction
    gPrescr=gPrescr+(interp1(hSevere2000,gSevere,hLoss(ifP2000)))';
  end;
  gPrescr=max(0,gPrescr);%no negative REIG

  fPrescr=[realmin;50;fPrescr;10000;100000];%extend for extrapolation
  gPrescr=[0;0;gPrescr;0;0];%extend for extrapolation
  g=interp1(log(fPrescr),gPrescr,log(f));%single column vector

function l = speech_level_in_dc_bands( sFitmodel, targetlevel )
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
  l = zeros(size(sFitmodel.frequencies));
  for band = 1:length(sFitmodel.frequencies)
    f_range = sFitmodel.edge_frequencies(band:(band+1));
    intensity_sum = 0;
    for ltass_band = 1:length(LTASS_freq)
        ltass_range = LTASS_edge_freq(ltass_band:(ltass_band+1));
        intersection = range_intersection(f_range, ltass_range);
        portion = diff(intersection) / diff(ltass_range);
        intensity_sum = intensity_sum + LTASS_intensity(ltass_band) * portion;
    end
    l(band) = 10*log10(intensity_sum) + (targetlevel-70);
  end
    