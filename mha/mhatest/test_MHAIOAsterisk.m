% This function tests that when two TCP clients connect to the MHA,
% it is not possible for one connection to block the other by flooding
% the MHA with commands.
%
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

function test_MHAIOAsterisk

  % create an MHA for this test
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');

  % Configure MHA for MHAIOAsterisk processing
  fragsize=160; srate=8000;
  mha_set(mha,'instance', 'test_MHAIOAsterisk');
  mha_set(mha,'srate',srate);
  mha_set(mha,'nchannels_in',1);
  mha_set(mha,'fragsize',fragsize);
  mha_set(mha,'iolib','MHAIOAsterisk');
  mha_set(mha,'mhalib','transducers');
  mha_set(mha,'mha.calib_in.peaklevel',85);
  mha_set(mha,'mha.calib_out.peaklevel',85);
  mha_set(mha,'mha.plugin_name','mhachain');
  mha_set(mha,'mha.mhachain.algos',{'rmslevel','gain'});
  mha_set(mha,'mha.mhachain.gain.gains', 20*log10(0.5));
  mha_set(mha,'cmd', 'prepare');

  assert_equal('stopped', mha_get(mha,'state'));
  
  % Create test signal
  f = 440; % Hz
  signal = round(32760 * sin(2*pi*[1:2*fragsize]'/srate*f));
  label = 'Fraunhofer:';
  asterisk = fopen('asterisk.in~','wb');
  
  fwrite(asterisk, label, 'unsigned char');
  fwrite(asterisk, signal, 'int16');
  fclose(asterisk);

  % Use netcat as an Asterisk simulator
  [code,text]=system(sprintf('nc -w 1 %s %d <asterisk.in~ >asterisk.out~', ...
                             mha.host, mha_get(mha,'io.port')));
  assert_equal(0, code, ... % Expect success exit code
               'Netcat (nc) invocation failed: unexpected exit code');

  % Read output from MHA
  asterisk = fopen('asterisk.out~', 'rb');
  label = '';
  next_character = '';
  while (isempty(next_character) || next_character ~= ':')
    next_character = fread(asterisk,1,'unsigned char');
    label = [label char(next_character)];
  end
   
  signal_out = fread(asterisk, Inf, 'int16');
  fclose(asterisk);
  delete('asterisk.in~');
  delete('asterisk.out~');
  assert_difference_below(signal_out,round(signal/2),1);
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
