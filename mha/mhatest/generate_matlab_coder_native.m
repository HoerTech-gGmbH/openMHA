%% This function tests if the matlab coder skeleton compiles.
%%
%% This file is part of the HörTech Open Master Hearing Aid (openMHA)
%% Copyright © 2021 HörTech gGmbH

%% openMHA is free software: you can redistribute it and/or modify
%% it under the terms of the GNU Affero General Public License as published by
%% the Free Software Foundation, version 3 of the License.
%%
%% openMHA is distributed in the hope that it will be useful,
%% but WITHOUT ANY WARRANTY; without even the implied warranty of
%% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
%% GNU Affero General Public License, version 3 for more details.
%%
%% You should have received a copy of the GNU Affero General Public License, 
%% version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.
%%
function generate_matlab_coder_native()
    % Define directories
    PLUGIN_DIR = "../../mha/plugins/matlabcoder_skeleton";
    MATLABCODER_DIR = "../../examples/23-matlab-coder/";
    addpath(MATLABCODER_DIR);
    % Use Matlab coder script to generate C++ files
    make;
    % Create plugin directory and copy the matlab generated files into it
    mkdir(PLUGIN_DIR);
    copyfile("codegen/lib/process/", PLUGIN_DIR)
    rmdir('codegen', 's'); % Remove the generated files from test folder
    % Copy the plugin skeleton into the plugin directory
    copyfile(MATLABCODER_DIR + "plugin_skeleton/", PLUGIN_DIR)
    
    % Local Variables:
    % mode: octave
    % coding: utf-8-unix
    % indent-tabs-mode: nil
    % End:
