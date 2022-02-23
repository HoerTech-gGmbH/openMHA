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

function stPlot = preparePlot()
  stPlot.hFig = figure();
  set(stPlot.hFig,'units','normalized','Position',[.1 .1 .8 .8]);
  stPlot.hAx1 = subplot(2,3,1);
  stPlot.hAx2 = subplot(2,3,2);
  stPlot.hAx3 = subplot(2,3,3);
  stPlot.hAx4 = subplot(2,3,4);
  stPlot.hAx5 = subplot(2,3,5);
  stPlot.hAx6 = subplot(2,3,6);
end
