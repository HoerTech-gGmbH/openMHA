function sGt = gainrule_NAL_NL1(sAud, sCfg)
% sGt = gainrule_NAL_NL1_bilateral(sAud, sCfg)
% NAL NL1 gainrule. Prescribes gains to each ear as if
%  (a) this ear is the only aided ear
%  (b) the other ear is deaf (HTL=125 dB)
% sAud.frequencies contains the audiogram frequencies
% sAud.l.htl       contains the subject-specific hearing threshold levels in
%                  dB HL for the left ear
% sAud.r.htl       the same for the right ear
% sAud.l.htl_bc    contains the subject-specific bone codutction hearing
%                  threshold levels in dB HL for the left ear
% sAud.r.htl_bc    the same for the right ear
% sCfg.frequencies contains the center frequencies for the amplification bands
% sCfg.levels      contains input levels in SPL for which to compute the gains
% side             either 'l' or 'r', denotes the ear to aid.
% sGt              contains 2 matrices, l and r that contain gains in dB
%                  for every input level (rows) and band (columns)
% sGt              may also contain an expansion_slope field.
% Compute gains for compression according to NAL NL1
  
  sAud = audprof2aud( sAud );
aud_frequencies = sAud.frequencies;

if min(aud_frequencies) > 500
    error('Values at 500Hz, 1000Hz, 2000Hz, 4000Hz need to be entered!')
end

target_frequencies = sCfg.frequencies;
crossOver = sCfg.edge_frequencies(2:(end-1));


% Values at 500Hz, 1000Hz, 2000Hz, 4000Hz need to be entered, all the
% others are optional. If the optional values are not entered the values
% should be made equal to 999.
aud_frequencies_NAL = [250 500 1000 1500 2000 3000 4000 6000 8000];
htl_ac.r = ones(size(aud_frequencies_NAL))*999;
htl_ac.l = htl_ac.r;

for side = 'lr'
    htl_ac.(side)(ismember(aud_frequencies_NAL, aud_frequencies)) = ...
        sAud.(side).htl(ismember(aud_frequencies, aud_frequencies_NAL));
end

% max 18 channels
channels = size(target_frequencies,2);
nlevels = length(sCfg.levels);
if channels > 18, error('18 channels maximum allowable!'), end
sGt.r = zeros(nlevels,channels); %allocation
sGt.l = sGt.r;
discrimn.l = sGt.l;
discrimn.r = sGt.l;
ltass.l = sGt.l;
ltass.r = sGt.l;

% need to be called before 'RealEarInsertionGain'
nl1mex('setBWC', channels, crossOver);

for side = 'lr'
    if side == 'l'
        other_side = 'r';
    else
        other_side = 'l';
    end

    AC = htl_ac.(side);
    BC = AC;
    ACother = htl_ac.(other_side);

    for channel = 1:channels

        l_broadband = 70;
        [l_narrowband, covered_band_indices] = ...
            gain_bb(sCfg.edge_frequencies(channel:(channel+1)));
        l_delta = l_narrowband - l_broadband;

        for level_index = 1:nlevels

            % [REIR] = nl1mex('RealEarInsertionGain', AC, BC, L,...
            %             limiting, channels, direction, mic, ACother,
            %             noOfAids)
            % Isertion gain for one channel and one level
            % AC[9] 	    -	Air Conduction Loss
            % BC[9]         -  	Bone Conduction Loss
            % L             -	Level of broadband signal
            % limiting      - 	0 – limiting off
            %                   1 – wideband limiting
            %                   2 – multichannel limiting
            % channels      -   number of channels, 1 to 18.
            % direction     -   direction of sound
            %                   0 - 0°
            %                   1 - 45°
            % mic		    -   Reference Position
            % 					0 - Undisturbed Field
            % 					1 - Head Surface
            % ACother[9]	-	AC Loss for the other ear
            % noOfAids      -   0 - Unilateral
            %                   1 - Bilateral
            bblev = sCfg.levels(level_index) - l_delta;
            if (bblev < 51)
                bblev = 51;
            end
            insertion_gain = nl1mex('RealEarInsertionGain',...
                AC, BC, bblev, 2, channels, 0, 0, ACother, 1);

            d = nl1mex('Discrim', bblev, AC,BC);
            usable_gain_indices = d .* covered_band_indices;
            ltass.(side)(level_index, channel) = l_narrowband;
            if sum(usable_gain_indices) == 0 % avoid NaN intermediate result
                sGt.(side)(level_index, channel) = 0;
                discrimn.(side)(level_index, channel) = 0;
            else
                best_gain_index = floor(mean(find(usable_gain_indices)));
                d = sum(d(covered_band_indices)) > sum(covered_band_indices) * 0.7;
                discrimn.(side)(level_index, channel) = d;
                sGt.(side)(level_index,channel) = insertion_gain(best_gain_index) * d;
            end
        end
    end
end

sGt.discrimn = discrimn;
sGt.ltass = ltass;
sGt.orig_r = sGt.r;
sGt.orig_l = sGt.l;
sGt.r = nal_nl1_fill_void(sCfg.levels,sCfg.frequencies,sGt.r,discrimn.r,20);
sGt.l = nal_nl1_fill_void(sCfg.levels,sCfg.frequencies,sGt.l,discrimn.l,20);


function [LTASS_level, covered_band_indices] = gain_bb(edge_frequencies)
% LTASS_level = gain_bb(edge_frequencies)
% edge_frequencies      edge frequencies; first row: lower edge second row
%                       upper edge
% Calculation of level of a narrow-band part of the broadband international
% long-term average speech spectrum with an overall
% level of 70dB from third-octave frequency bands (taken from Byrne et al.
% (1994) J. Acoust. Soc. Am. 96(4) 2108-2120)
LTASS_freq = [63 80 100 125 160 200 250 315 400 500 630 800 1000 1250 ...
    1600 2000 2500 3150 4000 5000 6300 8000 10000 12500 16000];
LTASS_lev = [38.6 43.5 54.4 57.7 56.8 60.2 60.3 59.0 62.1 62.1 60.5 56.8 ...
    53.7 53.0 52.0 48.7 48.1 46.8 45.6 44.5 44.3 43.7 43.4 41.3 40.7];
LTASS_intensity = 10.^(LTASS_lev/10);
covered_band_indices = ...
    (LTASS_freq > edge_frequencies(1)) & (LTASS_freq <= edge_frequencies(2));
intensity_sum = sum(LTASS_intensity(covered_band_indices));
covered_band_indices = covered_band_indices(4:22);
LTASS_level = 10*log10(intensity_sum);












