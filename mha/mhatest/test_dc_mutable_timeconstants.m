% This file is part of the HörTech Open Master Hearing Aid (openMHA);
% Copyright © 2018 2019 HörTech gGmbH;

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

% Test dc plugin: Regression test for T435:
% dc time constants can be changed during processing
function test_dc_mutable_timeconstants();
  % basic mha config for 2 channel dc;
  desc.instance = 'test_dc';
  desc.nchannels_in = 1;
  desc.fragsize = 64;
  desc.srate = 16000;
  desc.mhalib = 'overlapadd';
  desc.iolib = 'MHAIOParser';
  desc.mha.fftlen = 256;
  desc.mha.wnd.type = 'hanning';
  desc.mha.wnd.len = 128;

  %load dc into the stft processing;
  desc.mha.plugin_name = 'mhachain';

  desc.mha.mhachain.algos= {'fftfilterbank', 'dc', 'combinechannels'};
  desc.mha.mhachain.fftfilterbank.f = [200 2000];
  desc.mha.mhachain.dc.gtdata = [[20 10 -10]; [20 10 -10]];
  desc.mha.mhachain.dc.gtmin=[20];
  desc.mha.mhachain.dc.gtstep=[40];
  desc.mha.mhachain.combinechannels.outchannels=2;

  desc.mha.mhachain.dc.tau_attack=[0.25];
  desc.mha.mhachain.dc.tau_decay=[0.25];

  out=[];
  % start processing, quit;
  mha = mha_start;
  mha_set(mha,'',desc);
  % ensure MHA is exited after the test;
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');

  % time constants can be changed while in prepared mode:
  mha_set(mha,'cmd','start');
  mha_set(mha,'mha.mhachain.dc.tau_attack',[0.25 0.25]);
  mha_set(mha,'mha.mhachain.dc.tau_decay',[5 5]);

  % time constants can be changed during processing:
  mha_set(mha,'io.input',zeros(1,desc.fragsize));
  mha_set(mha,'mha.mhachain.dc.tau_attack',[0.005 0.005]);
  mha_set(mha,'mha.mhachain.dc.tau_decay',[0.1 0.1]);

  % Test is successful when no error was raised when setting time constants
end

% Local Variables:;
% mode: octave;
% coding: utf-8-unix;
% indent-tabs-mode: nil;
% End:;
