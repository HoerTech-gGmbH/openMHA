function sGt = gainrule_linear40( sAud, sCfg )
% GAINRULE_LINEAR40 - Prescribe 40% of hearing loss as insertion
% gain (no compression)
  
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2007 2008 2009 2010 2011 2013 2015 2017 HörTech gGmbH
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

  nLev = length(sCfg.levels);
  sGt = struct;
  libaudprof();
  for side=sCfg.side
    sT = audprof.threshold_get( sAud, side, 'htl_ac' );
    htl = freq_interp_sh([sT.data.f],[sT.data.hl],...
                         sCfg.frequencies);
    sGt.(side) = repmat(0.4*htl,[nLev 1]);
    sGt.noisegate.(side).level = 35*ones(size(sCfg.frequencies));
    sGt.noisegate.(side).slope = ones(size(sCfg.frequencies));
  end
