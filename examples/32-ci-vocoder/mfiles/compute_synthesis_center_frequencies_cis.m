% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2024 Hörzentrum Oldenburg gGmbH
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


% Script for computing synthesis (i.e., CI auralization) filterbank center 
% frequencies (CIS strategy), plotting the results, and displaying them on 
% the screen for use with the script compute_gammatone_filter_coefficients_cis.m

clear;
close all;
clc;

% -------------------------------------------------------------------------

% The center frequencies of the filterbank for synthesis (i.e., CI 
% auralization) reflect the insertion angle of the electrodes in the 
% cochlea, which in turn depends on the insertion depth of the electrode 
% array and on the spacing of the electrodes on the array. Following 
% Landsberger et al. (2015b), stimulated frequencies are taken here to 
% correspond most closely to spiral ganglion neuron location. Insertion 
% depths are taken from Landsberger et al. (2015a) and mapped onto spiral 
% ganglion frequencies, interpolating data from Stakhovskaya et al. (2007).
% 
% Landsberger, D., Svrakic, M., Roland, J. T., Jr., & Svirsky, M. A. (2015a). 
% Average insertion angles for Cochlear Contour Advance, Advanced Bionics 
% HiFocus 1J, and MED-EL Standard and Flex28 electctrodes. ResearchGate. 
% https://doi.org/10.13140/RG.2.1.2637.4483
% 
% Landsberger, D., Svrakic, M., Roland, J. T., Jr., & Svirsky, M. A. (2015b). 
% The relationship between insertion angles, default frequency allocations, 
% and spiral ganglion place pitch in cochlear implants. Ear & Hearing, 
% 36(5), e207-e213. https://doi.org/10.1097/AUD.0000000000000163
% 
% Stakhovskaya, O., Sridhar, D., Bonham, B. H., & Leake, P. A. (2007). 
% Frequency map for the human cochlear spiral ganglion: Implications for 
% cochlear implants. Journal of the Association for Research in 
% Otolaryngology, 8, 220-233. https://doi.org/10.1007/s10162-007-0076-9

insertion_angle_stakhovskaya = [0 90 180 270 360 450 540 630 720];  % insertion angles reported by Stakhovskaya et al. (2007) / degrees
spiral_ganglion_frequency_stakhovskaya = [17225 6193 3174 1539 785 550 366 215 58];  % spiral ganglion frequencies reported by Stakhovskaya et al. (2007) / Hz
insertion_angle_cis_landsberger = [9.6155 27.5575 50.4675 84.1615 127.7437 174.0508 224.2746 274.8243 331.6564 392.8502 450.7127 544.0318];  % insertion angles reported by Landsberger et al. (2015a)

% Compute the spiral ganglion frequencies for the CIS strategy / Hz, and 
% plot those frequencies along with the interpolated mapping of angular 
% insertion depth onto spiral ganglion frequency, based on Stakhovskaya 
% et al. (2007):
spiral_ganglion_frequency_cis = round(interp1(insertion_angle_stakhovskaya, spiral_ganglion_frequency_stakhovskaya, insertion_angle_cis_landsberger, 'pchip'));
figure;
insertion_angle_stakhovskaya_interp = insertion_angle_stakhovskaya(1):insertion_angle_stakhovskaya(end);
plot(insertion_angle_cis_landsberger, spiral_ganglion_frequency_cis, 'o');
hold on;
spiral_ganglion_frequency_stakhovskaya_interp = interp1(insertion_angle_stakhovskaya, spiral_ganglion_frequency_stakhovskaya, insertion_angle_stakhovskaya_interp, 'pchip');
plot(insertion_angle_stakhovskaya_interp, spiral_ganglion_frequency_stakhovskaya_interp, 'r--');
hold off;
grid on;
xlabel('Insertion Angle / degrees');
yticks(0:2000:18000);
yticklabels((0:2000:18000)/1000);
ylabel('Spiral Ganglion Frequency / kHz');
title({'Synthesis Filterbank Center Frequencies (CIS)', ''});

% Display the results:
expression = '(?<=e[+-])0';
sFc = strtrim(sprintf('%.0f ', fliplr(spiral_ganglion_frequency_cis)));
sFc = regexprep(sFc, expression, '');
fprintf('Synthesis filterbank center frequencies for CIS / Hz:\n\n');
fprintf(['fc = [' sFc '];\n\n']);
