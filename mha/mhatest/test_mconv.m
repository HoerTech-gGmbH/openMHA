%% This file is part of the HörTech Open Master Hearing Aid (openMHA)
%% Copyright © 2007 2009 2010 2013 2014 2015 2018 2019 HörTech gGmbH
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

function test_mconv

%Clean up after we are finished
  inwav = 'IN.wav';
  outwav = 'OUT.wav';
  unittest_teardown(@delete, [inwav]);
  unittest_teardown(@delete, [outwav]);
  fclose(fopen(inwav, 'w'));
  fclose(fopen(outwav, 'w'));

  %Create random noise, write to temp file
  snd_in = repeatable_rand(512,2,4743);
  dsc.srate=44100;
  audiowrite(inwav,snd_in,dsc.srate,'BitsPerSample',32);

% input channel, output channel, impulse response
chirs = {
  [0 0 1 0]
  [1 1 1 0]
  [0 2 0.5 0]
  [1 2 0.5 0]
  [0 3 zeros(1,64) 1]
  [1 4 zeros(1,128) 1]
  [0 5 zeros(1,73) 0.5]
  [1 5 zeros(1,74) 0.5]
  [0 6 1e-3*[0.047434 0.142788 0.126835 -0.235968 -1.005656 -1.850098 -2.088463 -1.060075 1.355883 4.402704 6.580945 6.303247 2.799511 -3.205102 -9.478097 -13.077594 -11.686978 -4.885299 5.301771 14.950250 19.768410 17.002173 6.864827 -7.218686 -19.777395 -25.469039 -21.378535 -8.440016 8.692206 23.358253 29.543266 24.384646 9.476377 -9.616433 -25.485619 -31.815312 -25.938152 -9.963344 9.999735 26.225968 32.416450 26.180323 9.966577 -9.917872 -25.800141 -31.642898 -25.366190 -9.588158 9.476484 24.491358 29.849990 23.785293 8.938727 -8.785590 -22.584496 -27.384269 -21.712302 -8.120637 7.944655 20.331671 24.546401 19.381113 7.219518 -7.035495 -17.936920 -21.575898 -16.975195 -6.301527 6.120380 15.553214 18.649639 14.627986 5.414036 -5.243182 -13.286553 -15.888069 -12.428690 -4.588095 4.432063 11.203432 13.364867 10.430368 3.841603 -3.702678 -9.339300 -11.117408 -8.658364 -3.182489 3.061320 7.706637 9.156527 7.118000 2.611570 -2.507685 -6.301954 -7.474861 -5.801067 -2.124927 2.037147 5.111468 6.053525 4.690953 1.715766 -1.642516 -4.115462 -4.867192 -3.766516 -1.375804 1.315343 3.291461 3.887765 3.004849 1.096252 -1.046822 -2.616449 -3.086895 -2.383151 -0.868470 0.828402 2.068295 2.437596 1.879918 0.684379 -0.652147 -1.626619 -1.915187 -1.475608 -0.536684 0.510932 1.273226 1.497750 1.152958 0.418968 -0.398521 -0.992261 -1.166263 -0.897043 -0.325708 0.309565 0.770164 0.904517 0.695186 0.252225 -0.239546 -0.595527 -0.698909 -0.536778 -0.194615 0.184703 0.458871 0.538166 0.413048 0.149657 -0.141943 -0.352412 -0.413049 -0.316821 -0.114721 0.108741 0.269818 0.316056 0.242283 0.087680 -0.083062 -0.205984 -0.241149 -0.184758 -0.066826 0.063272 0.156825 0.183500 0.140518 0.050798 -0.048073 -0.119091 -0.139280 -0.106603 -0.038519 0.036435 0.090218]]
};

% computing expected signal
expected_signal = zeros(512,7);
input_channels =  []; output_channels = [];
for i = 1:length(chirs)
  input_channel = chirs{i}(1) + 1;
  input_channels = [input_channels, input_channel-1];
  output_channel = chirs{i}(2) + 1;
  output_channels = [output_channels, output_channel-1];
  irs = chirs{i}(3:length(chirs{i}));
  chirs{i} = [irs zeros(1, 182-length(irs))];
  expected_signal(:,output_channel) = expected_signal(:,output_channel) + ...
      filter(irs, 1, snd_in(:,input_channel));
end

% Set up mha
dsc.instance = 'test_mconv';
dsc.mhalib =  'mconv';
dsc.iolib = 'MHAIOFile';
dsc.fragsize = 64;
dsc.nchannels_in = 2;
dsc.mha.nchannels_out=7;
dsc.mha.irs=chirs;
dsc.mha.inch=input_channels;
dsc.mha.outch=output_channels;
dsc.io.in = inwav;
dsc.io.out = outwav;

mha=mha_start;

mha_set(mha,'',dsc);
mha_set(mha,'cmd','start');
mha_set(mha,'cmd','release');
snd_out=audioread(outwav);

assert_difference_below(expected_signal, snd_out, 10^(-120/20) );
