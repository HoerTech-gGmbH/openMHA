function sGt = gainrule_CR2_NALRP( sAud, sFitmodel )
% This fitting rule applies NAL-RP gains at 65 dB input level and applies
% compression ratio 2. It shares the implementation with gainrule_CR3_NALRP.
%
% Author: Giso Grimm 2013 2019
%
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

  compression_ratio = 2;
  sGt = gainruleimpl_CRx_NALRP(sAud, sFitmodel, compression_ratio);
