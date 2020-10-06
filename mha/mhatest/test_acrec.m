% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2020 HörTech gGmbH

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

function test_acrec()

  out = 'test.dat';
  unittest_teardown(@delete, [out]);
  fclose(fopen(out, 'w'));

  % basic mha config for 1 channel
  desc.instance = 'test_acrec';
  desc.nchannels_in = 1;
  desc.fragsize = 64;
  desc.srate = 16000;
  desc.mhalib = 'mhachain';
  desc.iolib = 'MHAIOParser';

  desc.mha.algos = {'save_wave','acrec'};
  desc.mha.acrec.varname='save_wave';
  %acrec expects its filename w/o suffix
  desc.mha.acrec.prefix='test';
  desc.mha.acrec.use_date='no';
  desc.mha.acrec.record='yes';

  mha = mha_start();
  mha_set(mha,'',desc);
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  expected=repeatable_rand(64,1,0) - 0.5;
  mha_set(mha,'cmd','start');
  mha_set(mha,'io.input',expected');
  mha_set(mha,'cmd','release');
  in=fopen(out);
  unittest_teardown(@fclose, in);
  actual=fread(in,64,'double');
  assert_difference_below(expected, actual, 1e-4);
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
