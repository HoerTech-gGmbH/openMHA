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

function test_rohbeam

  %% Clean up after we are finished
  inwav = 'IN.wav';
  outwav = 'OUT.wav';
  unittest_teardown(@delete, [inwav]);
  unittest_teardown(@delete, [outwav]);
  fclose(fopen(inwav, 'w'));
  fclose(fopen(outwav, 'w'));

  %% Test configuration
  dir='../../reference_algorithms/Rohdenburg2007_beam/';
  cfg='Rohdenburg2007_beam.cfg';

  %% Load input signal and reference output from disk

  [xAlgo,fs]=audioread('../AudioFiles/subject1_ref_input.wav');
  xAlgo=xAlgo(:,3:8);
  refyAlgo=audioread('../AudioFiles/Rohdenburg2007_beam.wav');

  %% Basic mha setup
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  mha_query(mha,'',['read:' dir cfg]);
  mha_set(mha,'mha.overlapadd.rohBeam.sampled_hrir_path',[dir 'hrir2_0_16k.wav']);
  mha_set(mha,'io.in',inwav);
  mha_set(mha,'io.out',outwav);

  %% Sampling rate of this algorithm
  fsAlgo=mha_get(mha,'srate');

  %% Prepare signal for first run
  xAlgo=resample(xAlgo, fsAlgo, fs);
  audiowrite(inwav,xAlgo,fsAlgo,'BitsPerSample',32);

  %% Run test
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');

  %% Get output signal and do post-processing
  yAlgo = audioread(outwav);
  yAlgo=resample(yAlgo,fs,fsAlgo);

  %% Test fidelity - 2e-5 is an empiric value
  assert_difference_below(refyAlgo,yAlgo,1e-4);
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
