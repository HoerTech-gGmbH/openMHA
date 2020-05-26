function l = LTASS_speech_level_in_frequency_bands(edge_frequencies, targetlevel)
% Compute the physical levels of filterbank frequency bands for a broadband
% LTASS signal with broadband level targetlevel / dB
%
% INPUTS:
% - edge_frequencies [1 x num_bands+1]
%   A vector containing containing filter bank edge frequencies. The first
%   band has a lower cutoff frequency edge_frequencies(1) and an upper cutoff
%   frequency edge_frequencies(2). The second band has a lower cutoff frequency
%   edge_frequencies(2) etc.
% - targetlevel      [scalar]
%   The broadband level / dB of the LTASS signal for which to approximate the
%   narrow band levels.
%
% OUTPUTS:
% - l [1 x num_bands]
%   The computed narrow-band levels for each of the filterbank bands.
%   Band levels are computed by distributing the third-octave LTASS intensities
%   across the filterbank bands with intensity summation.
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2007 2009 2011 2013 2015 2017 2018 2019 2020 HörTech gGmbH
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

  % The LTASS spectrum was published as SPL levels in third-octave frequency
  % bands for a broadband level of 70 dB SPL.  Some hearing aid compression
  % prescription rules (e.g. the compressive Cambridge rule by Moore, NAL-NL1,
  % NAL-NL2) prescribe the gains to apply to the frequency band of the
  % compressor depending on the broadband LTASS input level.  To use these
  % prescription rules with MHA compressors, we need to compute the SPL
  % that would be measured at the input of a (narrow) compressor band when
  % an LTASS spectrum with a certain broad band level is received.
  %
  % Here, we compute the levels that would be measured at the input of the
  % compressors frequency bands (hereafter named fitmodel frequency bands)
  % for a broadband signal with level
  %     /targetlevel/ dB SPL
  % and an LTASS shaped spectrum.
  %
  % The third-octave bands defining the LTASS
  % spectrum are named LTASS third-octave bands hereafter to differentiate
  % them from the fitmodel frequency bands.  To compute the signal level of one
  % fitmodel frequency band for an LTASS-shaped broadband spectrum of
  %     /targetlevel/ dB SPL,
  % we sum up the intensities of the LTASS third-octave bands which
  % are contained completely in the target fitmodel frequency band,
  % and add to that any partial intensities of the LTASS third-octave bands
  % that overlap only partially with the target fitmodel frequency band.

  num_bands = length(edge_frequencies) - 1;
  if num_bands < 1
    error('At least two edge frequencies are required');
  end
  % result vector: per-fitmodel-frequency-band entries will be computed as
  % dB SPL and contain the partial SPL of a /targetlevel/ dB SPL LTASS signal
  % that falls into that respective fitmodel frequency band.
  l = zeros(1, num_bands);

  % Loop over all fitmodel frequency bands
  for band = 1:num_bands
    % Isolate [lower upper] edge frequencies of current fitmodel frequency band
    f_range = edge_frequencies(band:(band+1));
    % intensity_sum accumulates intensities from all LTASS third-octave bands
    % that fall into the current fitmodel frequency band, completely or
    % partially.
    intensity_sum = 0;
    % Loop over all LTASS third-octave bands to find those overlapping with
    % f_range, the edge frequencies of the target fitmodel frequency band
    for ltass_band = 1:length(LTASS_freq)
        % Isolate [lower upper] edge frequencies of the LTASS third-octave band
        ltass_range = LTASS_edge_freq(ltass_band:(ltass_band+1));
        % intersect both ranges
        intersection = range_intersection(f_range, ltass_range);
        % Compute what part of the current LTASS third-octave band overlaps
        % with the current fitmodel frequency band
        portion = diff(intersection) / diff(ltass_range);
        % Add this part to the intensity sum for the current fitmodel
        % frequency band
        intensity_sum = intensity_sum + LTASS_intensity(ltass_band) * portion;
    end
    % Convert back from intensity to dB and adjust levels
    % (published data used for summation is for 70 dB)
    l(band) = 10*log10(intensity_sum) + (targetlevel-70);
  end
