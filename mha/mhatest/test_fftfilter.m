%% This file is part of the HörTech Open Master Hearing Aid (openMHA)
%% Copyright © 2005 2006 2007 2013 2014 2015 2016 2019 HörTech gGmbH
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

function test_fftfilter

  % Clean up after we are finished
  inwav = 'IN.wav';
  outwav = 'OUT.wav';
  unittest_teardown(@delete, [inwav]);
  unittest_teardown(@delete, [outwav]);
  fclose(fopen(inwav, 'w'));
  fclose(fopen(outwav, 'w'));

  dsc.instance = 'test_fftfilter';
  dsc.mhalib =  'fftfilter';
  dsc.iolib = 'MHAIOFile';
  dsc.fragsize = 400;
  dsc.nchannels_in = 2;
  dsc.mha.fftlen = dsc.fragsize / 200 * 512;
  dsc.srate = 44100;
  f = dsc.srate / dsc.fragsize * 4;
  dsc.io.in = inwav;
  dsc.io.out = outwav;

  % Create random noise, write to temp file
  snd_in = 0.01*(repeatable_rand(dsc.srate,2,1) - 0.5);
  snd_in(:,2) = snd_in(:,1) * 0.2;
  audiowrite(inwav,snd_in,dsc.srate,'BitsPerSample',32);

  % Default impulse response is a single 1, i.e. the filter is identity
  mha = mha_start();
  unittest_teardown(@mha_set, mha, 'cmd', 'quit');
  mha_set(mha,'',dsc);
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  snd_out=audioread(outwav);
  assert_difference_below(snd_in, snd_out, 10^(-150/20) );

  % This is a test with a long impulse response and a long fragsize
  % The impulse response is still identity, because it starts with a 1 and
  % all other impulse response samples are zero.
  mha_set(mha,'mha.irs',[1 zeros(1,399)]);
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  snd_out=audioread(outwav);
  % In addition to testing for identical values, the following comparison
  % proves that fftfilter does not introduce additional delay on top of
  % the delay introduced by the impulse response itself (which here is none).
  assert_difference_below(snd_in, snd_out, 10^(-150/20) );

  irs = repeatable_rand( 1, 200, 1 );
  mha_set(mha,'mha.irs',irs);
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');
  snd_out=audioread(outwav);
  matlab_out = fftfilt(irs,snd_in);
  assert_difference_below(matlab_out, snd_out, 10^(-150/20) );

end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
