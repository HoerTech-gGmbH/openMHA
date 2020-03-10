function result = has_plugin(plug_name)
% result = has_plugin(plug_name)                              %
% Checks if a plugin with the given name exists in the plugin list.
% Returns true if so, false if not. This function relies on the "plugins"
% global variable filled and so may only be called after set_environment has
% been executed.
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2019 HörTech gGmbH
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
  result=false;
  global plugins;
  if (isempty(plugins))
    error('has_plugin: Can not find plugin list.')
  end
  for idx=1:numel(plugins)
    [~,name,~]=fileparts(cell2mat(plugins(idx)));
    if strcmp(name,plug_name)==1
      result=true;
    end
  end
end
% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
