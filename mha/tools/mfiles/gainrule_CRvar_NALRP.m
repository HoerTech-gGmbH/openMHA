function sGt = gainrule_CRvar_NALRP(sAud,sFitmodel,compression_ratio)
% Wrapper function for the gainrule NAL-RP with a variable compression rate

% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2013 2019 2020 HörTech gGmbH
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

  if nargin < 3
    get_ratio_input = true;
    while get_ratio_input
      answer_ratio = inputdlg('Please enter a compression ratio.', 'Compression Ratio');
      answer_ratio = str2double(answer_ratio);
      if length(answer_ratio) == 1 && ~isnan(answer_ratio) && isreal(answer_ratio)
        if answer_ratio >= 1 && answer_ratio < 100
          compression_ratio = answer_ratio;
          get_ratio_input = false;
        else
          errordlg('The compression ratio must be between 1 and 100!');
        end
      end
    end
  end
  if compression_ratio < 1 || compression_ratio >= 100
    error('The compression ratio of gainrule_CRvar_NALRP must be between 1 and 100!');
  end

  sGt = gainruleimpl_CRx_NALRP(sAud,sFitmodel,compression_ratio);


function sGt = gainruleimpl_CRx_NALRP( sAud, sFitmodel, compression_ratio )
% This fitting rule helper applies NAL-RP gains at 65 dB input level and applies
% compression_ratio compression_ratio.  Wrappers for compression ratios 2 and 3
% exist, more may be provided by yourself.
%
% Author: Giso Grimm 2013 2019

  %% Configuration parameters:
  c_slope = 1/compression_ratio;
  l_target_ltass = 65;
  l_kneepoint_ltass = 40;
  %% gain rule:
  sAud = audprof2aud( sAud );

  sGt.compression = struct;
  % knee point and target level in compressor bands:
  l_kneepoint = LTASS_speech_level_in_frequency_bands( sFitmodel.edge_frequencies, l_kneepoint_ltass );
  l_target = LTASS_speech_level_in_frequency_bands( sFitmodel.edge_frequencies, l_target_ltass );
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
