% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2022 Hörzentrum Oldenburg gGmbH
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

function cal_lev = get_calibration_level(output_level)

  if nargin < 1
    output_level = 80;
  end

addpath(mha_install_dirs('mfiles'));

mha = mha_start();
mha_query(mha, [], 'read:calibrate_setup.cfg');
mha_set(mha, 'mha.noise.lev', output_level);
mha_set(mha, 'cmd', 'prepare');
mha_set(mha, 'cmd', 'start');
pause(10)
mha_set(mha, 'cmd', 'release');
mha_set(mha, 'cmd', 'quit');
fid = fopen('calibration_values.dat');
rmslev = fread(fid, Inf, 'double');
fclose(fid);

cal_lev = output_level - mean(rmslev(10:end-10));

end
