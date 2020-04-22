%% This file is part of the HörTech Open Master Hearing Aid (openMHA)
%% Copyright © 2020 HörTech gGmbH
%%
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

function test_amvdr_bf

  %% Clean up after we are finished
  inwav = '../Audiofiles/subject1_ref_Baumgaertel2015_AMVDR_in.wav';
  outwav = 'OUT.wav';
  unittest_teardown(@delete, [outwav]);
  fclose(fopen(outwav, 'w'));

  %% Test configuration
  dir='../../reference_algorithms/Baumgaertel2015_AMVDR/';
  cfg='Baumgaertel2015_AMVDR.cfg';
  setenv('MHA_CFG_DIR',dir);
  %% Load input signal and reference output from disk
  [xAlgo,fs]=audioread(inwav);
  refyAlgo=audioread('../Audiofiles/subject1_ref_Baumgaertel2015_AMVDR_out.wav');

  %% Basic mha setup
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  mha_query(mha,'',['read:' dir cfg]);
  mha_set(mha,'mha.spAF.filtL.mu',0.004);
  mha_set(mha,'mha.spAF.filtR.mu',0.004);
  mha_set(mha,'io.in',inwav);
  mha_set(mha,'io.out',outwav);

  %% Sampling rate of this algorithm
  fsAlgo=mha_get(mha,'srate');

  %% Run test
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');

  %% Get output signal
  yAlgo = audioread(outwav);

  %% Test fidelity - 1e-7 is an empiric value
  assert_difference_below(refyAlgo,yAlgo,1e-7);
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
