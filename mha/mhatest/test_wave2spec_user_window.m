function test_wave2spec_user_window
% Testing the possibility of having a user-defined analysis window in wave2spec.
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

  test_wave2spec_user_window_with_workaround();
  test_wave2spec_user_window_direct_1();
  test_wave2spec_user_window_direct_2();
end

function test_wave2spec_user_window_with_workaround()
% This workaround was necessary to use user-defined analysis windows
% in wave2spec before fixing a bug in wave2spec.
% This test checks that configurations using the workaround still work with
% the bug fix in place.
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  mha_set(mha,'instance','test_wave2spec_user_window_with_workaround');
  mha_set(mha,'fragsize',4);
  mha_set(mha,'mhalib','mhachain');
  mha_set(mha,'mha.algos',{'wave2spec','spec2wave'});
  mha_set(mha,'mha.wave2spec.fftlen',16);
  mha_set(mha,'mha.wave2spec.wndlen',8);
  mha_set(mha,'mha.wave2spec.userwnd',[0.1 0.2 0.8 1 1 0.8 0.2 0.1]);
  mha_set(mha,'iolib','MHAIOParser');
  % The workaround is to not set the window type to user in the first prepare
  mha_set(mha,'cmd','prepare');
  mha_set(mha,'cmd','release');
  % ... but to have this dummy prepare/release cycle before using user-window.
  mha_set(mha,'mha.wave2spec.wndtype','user');
  mha_set(mha,'cmd','prepare');
  assert_equal('stopped', mha_get(mha,'state'));
end

function test_wave2spec_user_window_direct_1()
% Test that no workaround is needed to use user-defined windows
% Variant 1. First set window type to user, then set the user window.
  
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  mha_set(mha,'instance','test_wave2spec_user_window_with_workaround');
  mha_set(mha,'fragsize',4);
  mha_set(mha,'mhalib','mhachain');
  mha_set(mha,'mha.algos',{'wave2spec','spec2wave'});
  mha_set(mha,'mha.wave2spec.fftlen',16);
  mha_set(mha,'mha.wave2spec.wndlen',8);
  mha_set(mha,'iolib','MHAIOParser');
                                % does not work yet:
  mha_set(mha,'mha.wave2spec.wndtype','user');
  mha_set(mha,'mha.wave2spec.userwnd',[0.1 0.2 0.8 1 1 0.8 0.2 0.1]);
  mha_set(mha,'cmd','prepare');
  assert_equal('stopped', mha_get(mha,'state'));
end  

function test_wave2spec_user_window_direct_2()
% Test that no workaround is needed to use user-defined windows
% Variant 2. First set the user window, then set window type.
  
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  mha_set(mha,'instance','test_wave2spec_user_window_with_workaround');
  mha_set(mha,'fragsize',4);
  mha_set(mha,'mhalib','mhachain');
  mha_set(mha,'mha.algos',{'wave2spec','spec2wave'});
  mha_set(mha,'mha.wave2spec.fftlen',16);
  mha_set(mha,'mha.wave2spec.wndlen',8);
  mha_set(mha,'iolib','MHAIOParser');
  mha_set(mha,'mha.wave2spec.userwnd',[0.1 0.2 0.8 1 1 0.8 0.2 0.1]);

  % Regression test: This used to expose a bug in user window handling. 
  mha_set(mha,'mha.wave2spec.wndtype','user');
  mha_set(mha,'cmd','prepare');
  assert_equal('stopped', mha_get(mha,'state'));
end  

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
