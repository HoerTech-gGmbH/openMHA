function test_LTASS_speech_level_in_frequency_bands
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2019 2020 HörTech gGmbH
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

% For only one broadband "filterbank" band, the band level should be equal to
% the broadband level.
  assert_difference_below(70, LTASS_speech_level_in_frequency_bands([0,20e3],70), 0.04);
  assert_difference_below(65, LTASS_speech_level_in_frequency_bands([0,20e3],65), 0.04);

% For a filterbank that duplicates the LTASS third-octave bands, the resulting
% levels are the LTASS levels from Byrne et al. (1994)
  LTASS_freq = [63 80 100 125 160 200 250 315 400 500 630 800 1000 1250 ...
                   1600 2000 2500 3150 4000 5000 6300 8000 10000 12500 16000];
  LTASS_edge = [0, sqrt(LTASS_freq(1:end-1).*LTASS_freq(2:end)), 20e3];
  assert_difference_below([38.6 43.5 54.4 57.7 56.8 60.2 60.3 59.0 62.1 62.1 60.5 56.8 ...
                             53.7 53.0 52.0 48.7 48.1 46.8 45.6 44.5 44.3 43.7 43.4 41.3 40.7], ...
                          LTASS_speech_level_in_frequency_bands(LTASS_edge, 70), ...
                          1e-7);

% Intensity summation: first two third-octave bands together in filterbank band
  intensity_sum_first_two_3rdoct = sum(10.^[3.86 4.35]);
  assert_difference_below(10*log10(intensity_sum_first_two_3rdoct), ...
                          LTASS_speech_level_in_frequency_bands(LTASS_edge([1,3]), 70), ...
                          1e-7);
% Intensity splitting: Only half of third third-octave band in filterbank band
  assert_difference_below(54.4-10*log10(2), ...
                          LTASS_speech_level_in_frequency_bands([LTASS_edge(3),mean(LTASS_edge(3:4))], 70), ...
                          1e-7);
  error_raised = [];
  try % function insists on at least 2 edge frequencies for one band minimum
    LTASS_speech_level_in_frequency_bands([1],1); % should throw
    error_raised = false; % should not be reached
  catch
    error_raised = true;
  end
  assert_all(error_raised);
