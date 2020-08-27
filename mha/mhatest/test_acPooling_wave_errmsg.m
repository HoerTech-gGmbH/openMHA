% This file is part of the HörTech Open Master Hearing Aid (openMHA);
% Copyright © 2020 HörTech gGmbH;

% openMHA is free software: you can redistribute it and/or modify;
% it under the terms of the GNU Affero General Public License as published by;
% the Free Software Foundation, version 3 of the License.;
%;
% openMHA is distributed in the hope that it will be useful,;
% but WITHOUT ANY WARRANTY; without even the implied warranty of;
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the;
% GNU Affero General Public License, version 3 for more details.;
%;
% You should have received a copy of the GNU Affero General Public License, ;
% version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.;

function test_acPooling_wave_errmsg
  mha = mha_start;
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  mha_set(mha,'instance','test_acPooling_wave_errmsg');
  mha_set(mha,'mhalib','acPooling_wave');
  mha_set(mha,'iolib', 'MHAIOParser');
  mha_set(mha,'mha.numsamples', 2);
  mha_set(mha,'mha.prob_bias',[]);
  expected = ['(acPooling_wave) prob_bias should have' ...
                ' "numsamples" (2) elements but has 0 elements'];
  actual = '';
  try
    mha_set(mha,'cmd','prepare');
  catch err
    actual = err.message;
  end
  % expected MHA error message is contained exactly once in matlab error message
  assert_equal(1, numel(findstr(actual, expected)));
end

% Local Variables:;
% mode: octave;
% coding: utf-8-unix;
% indent-tabs-mode: nil;
% End:;
