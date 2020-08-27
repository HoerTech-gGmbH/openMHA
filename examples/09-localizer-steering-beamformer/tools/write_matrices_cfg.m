function write_matrices_cfg(varargin)
% Write a configuration file for the doasvm_classification MHA plug-in.
%
% WRITE_MATRICES_CFG() will load the MAT file localisation_matlab/modelData.mat
% and write MHA configuration data into the file matrices.cfg.
%
% WRITE_MATRICES_CFG(IN_FILE) will load the MAT file IN_FILE and write MHA
% configuration data into the file matrices.cfg.
%
% WRITE_MATRICES_CFG(IN_FILE, OUT_FILE) will load the MAT file IN_FILE and
% write MHA configuration data into the file OUT_FILE.
%
% The *.cfg files produced by this function are used to parameterise the MHA
% plug-in doasvm_classification.  See the file README.md for more information.
%
% Input parameters
% ----------------
%
%      in_file:     The name of the MAT file to load (optional).
%      out_file:    The name of the MHA configuration file to write (optional).
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2018 HörTech gGmbH

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

p = inputParser();
p.addOptional('in_file', 'localisation_matlab/modelData', @ischar);
p.addOptional('out_file', 'matrices.cfg', @ischar);
p.parse(varargin{:});

data = load(p.Results.in_file);

ncol_w = size(data.w, 2);
ncol_b = size(data.b, 2);
ncol_x = size(data.x, 2);
ncol_y = size(data.y, 2);
ncol_angles = size(data.angles, 2);

fmt_str_w = ['[' repmat('%.27f ', 1, ncol_w-1) '%f];'];
fmt_str_b = ['[' repmat('%.27f ', 1, ncol_b-1) '%f]'];
fmt_str_x = ['[' repmat('%.27f ', 1, ncol_x-1) '%f]'];
fmt_str_y = ['[' repmat('%.27f ', 1, ncol_y-1) '%f]'];
fmt_str_angles = ['[' repmat('%d ', 1, ncol_angles-1) '%d]'];

print_str = [sprintf('w = [%s]\n', sprintf(fmt_str_w, data.w.')) ...
             sprintf('b = %s\n', sprintf(fmt_str_b, data.b)) ...
             sprintf('x = %s\n', sprintf(fmt_str_x, data.x)) ...
             sprintf('y = %s\n', sprintf(fmt_str_y, data.y)) ...
             sprintf('angles = %s', sprintf(fmt_str_angles, data.angles))];

fid = fopen(p.Results.out_file, 'w');
fprintf(fid, print_str);
fclose(fid);
