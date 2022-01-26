% Checks output of analysemhaplugin
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2022 Hörzentrum Oldenburg gGmbH

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

function test_analysemhaplugin

  % Assume analysemhaplugin is not in PATH if MHA_INSTALL_DIR is set.
  global MHA_INSTALL_DIR;
  if ~isempty(MHA_INSTALL_DIR)
    binary = [MHA_INSTALL_DIR,'/analysemhaplugin'];
  else
    MHA_INSTALL_DIR = getenv('MHA_INSTALL_DIR');
    if ~isempty(MHA_INSTALL_DIR)
      binary = [MHA_INSTALL_DIR,'/analysemhaplugin'];
    else
      binary = 'analysemhaplugin'; % Use PATH when MHA_INSTALL_DIR is not set.
    end
  end

  % Run analysemhaplugin on plugin rmslevel.
  [status,output] = system([binary ' rmslevel']);

  % Check information contained in output:
  
  % There will be 2 AC sections (spec & wave) with AC variables after prepare:
  assert_equal(2, numel(findstr('AC variables after prepare:', output)));

  % There should be documentation on variables
  assert_equal(1, numel(findstr('# RMS level in dB', output)));
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
