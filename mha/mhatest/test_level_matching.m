% This function tests plugin level_matching by checking the output levels
% of a mismatched input signal
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2019 HörTech gGmbH

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
%

function test_level_matching
  if ~has_plugin('level_matching')
    warning('Plugin level_matching not found. Skipping tests');
    return
  end
  %% Set up clean-up after we are finished
  inwav = 'IN.wav';
  outwav = 'OUT.wav';
  unittest_teardown(@delete, [inwav]);
  unittest_teardown(@delete, [outwav]);
  fclose(fopen(inwav, 'w'));
  fclose(fopen(outwav, 'w'));

  test_level_matching_wave;
  test_level_matching_spec;
  test_level_matching_adm;
end

function test_level_matching_spec
  inwav = 'IN.wav';
  outwav = 'OUT.wav';

  %% White noise input
  signal_length=1; % in s
  srate=44100; % in Hz
  t=(0:signal_length*srate-1)/srate;
  x=zeros(1,numel(t));
  y=zeros(1,numel(t));
  %% randomise mismatch between -4dB and +4dB
  mismatch_range=10;
  mismatch=10.^((-mismatch_range+2*mismatch_range*repeatable_rand(3,1,4711))/20);
  frequencies=[1000, 5000, 15000];
  for i=1:numel(frequencies)
    x=x+0.2*sin(2*pi*frequencies(i)*t);
    y=y+0.2*mismatch(i)*sin(2*pi*frequencies(i)*t);
  end
  snd_in=[x' y'];
  audiowrite(inwav,snd_in,srate);

  %% basic mha config for 2 channel matching
  desc.nchannels_in=2;
  desc.srate=srate;
  desc.fragsize=128;

  desc.mhalib='overlapadd';
  desc.mha.wnd.len=512;
  desc.mha.fftlen=1024;
  desc.mha.plugin_name='mhachain';

  desc.mha.mhachain.algos={'fftfilterbank' 'level_matching' 'combinechannels'};

  desc.mha.mhachain.fftfilterbank.ftype='edge';
  desc.mha.mhachain.fftfilterbank.f=[0 2500 10000 22050];
  desc.mha.mhachain.combinechannels.outchannels=2;
  desc.mha.mhachain.level_matching.lp_level_tau=.01;
  % Always try to match
  desc.mha.mhachain.level_matching.range=999;
  desc.mha.mhachain.level_matching.channels=[[0 3];[1 4];[2 5];];
  desc.mha.mhachain.level_matching.lp_level_tau=0.01;

  desc.iolib='MHAIOFile';
  desc.io.in=inwav;
  desc.io.out=outwav;


  %% execute mha with the given config file in the example directory,
  %% start processing, quit
  mha = mha_start;
  mha_set(mha,'',desc);
  mha_set(mha,'cmd','prepare');
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','quit');

  snd_out = audioread(outwav);

  %% Drop beginning where adaption takes place
  snd_out=snd_out(end-10000:end,:);
  snd_in=  snd_in(end-10000:end,:);
  in_levels  = 10*log10(mean(snd_in.^2));
  out_levels = 10*log10(mean(snd_out.^2));
  %% Reference level should not be changed
  assert_difference_below(in_levels(1), out_levels(1), 1e-2);
  %% End levels should be equal
  assert_difference_below(out_levels(1), out_levels(2), 1e-2);

end


function test_level_matching_wave
  inwav = 'IN.wav';
  outwav = 'OUT.wav';

  %% White noise input @ -20 dB(FS) to avoid clipping on positive mismatch
  x=10^(-20/40)*repeatable_rand(32000,1,0);
  mismatch_range=4;
  mismatch=10.^((-mismatch_range+2*mismatch_range*repeatable_rand(1,1,0))/20);
  snd_in=[x mismatch*x];
  audiowrite(inwav,snd_in,16000);

  %% basic mha config for 2 channel matching
  desc.instance = 'test_level_matching';
  desc.nchannels_in = 2;
  desc.fragsize = 64;
  desc.srate = 16000;
  desc.mhalib = 'level_matching';
  desc.mha.channels=[[0 1]];
  desc.mha.lp_level_tau=.01;
  % Always try to match
  desc.mha.range=999;
  % Effectively disable signal lp for this test
  desc.mha.lp_signal_fpass=8000;
  desc.mha.lp_signal_fstop=8000;
  desc.iolib = 'MHAIOFile';
  desc.io.in=inwav;
  desc.io.out=outwav;

  %% execute mha with the given config file in the example directory,
  %% start processing, quit
  mha = mha_start;
  mha_set(mha,'',desc);
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','quit');

  snd_out = audioread(outwav);
  %% Drop beginning where adaption takes place
  snd_out=snd_out(end-10000:end,:);
  snd_in=  snd_in(end-10000:end,:);
  in_levels  = 10*log10(mean(snd_in.^2));
  out_levels = 10*log10(mean(snd_out.^2));
  %% Reference level should not be changed
  assert_difference_below(in_levels(1), out_levels(1), 1e-2);
  %% End levels should be equal
  assert_difference_below(out_levels(1), out_levels(2), 1e-2);
end

function test_level_matching_adm
  inwav = 'IN.wav';
  outwav = 'OUT.wav';

  %% White noise input @ -20 dB(FS) to avoid clipping on positive mismatch
  x=10^(-20/40)*repeatable_rand(32000,1,0);
  mismatch_range=4;
  mismatch=10.^((-mismatch_range+2*mismatch_range*repeatable_rand(1,1,0))/20);
  delayed_x=[0 x(1:end-1)']';
  snd_in=[mismatch*delayed_x x];
  audiowrite(inwav,snd_in,16000);

  %% basic mha config for 2 channel matching
  desc.instance = 'test_level_matching';
  desc.nchannels_in = 2;
  desc.fragsize = 64;
  desc.srate = 16000;
  desc.mhalib = 'mhachain';
  desc.mha.algos={'altplugs:level_matching', 'adm'};
  desc.mha.level_matching.plugs={'level_matching', 'identity'};
  desc.mha.level_matching.select= 'level_matching';
  desc.mha.level_matching.level_matching.channels=[[0 1]];
  desc.mha.level_matching.level_matching.lp_level_tau=.01;
  % Always try to match
  desc.mha.level_matching.level_matching.range=999;
  % Effectively disable signal lp for this test
  desc.mha.level_matching.level_matching.lp_signal_fpass=8000;
  desc.mha.level_matching.level_matching.lp_signal_fstop=8000;

  desc.mha.adm.front_channels= [0];
  desc.mha.adm.rear_channels = [1];
  desc.mha.adm.distances = [1 * 340/desc.srate];
  desc.mha.adm.tau_beta = [50e-3];
  desc.mha.adm.mu_beta= [1e-4];
  desc.io.in = inwav;
  desc.io.out = outwav;

  desc.iolib = 'MHAIOFile';
  desc.io.in=inwav;
  desc.io.out=outwav;

  %% execute mha with the given config file in the example directory,
  %% start processing, quit
  mha = mha_start;
  mha_set(mha,'',desc);
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');

  snd_out = audioread(outwav);

  %% Drop beginning where adaption takes place
  snd_out=snd_out(end-10000:end,1);
  snd_in=  snd_in(end-10000:end,1);
  in_levels  = 10*log10(mean(snd_in.^2));
  out_levels = 10*log10(mean(snd_out.^2));

  att_w=in_levels - out_levels;

  %Re-run w/o level matching
  mha_set(mha,'mha.level_matching.select','identity');
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','quit');

  snd_out = audioread(outwav);
  %% Drop beginning where adaption takes place
  snd_out=snd_out(end-10000:end,1);
  snd_in=  snd_in(end-10000:end,1);
  in_levels  = 10*log10(mean(snd_in.^2));
  out_levels = 10*log10(mean(snd_out.^2));
  att_wo=in_levels - out_levels;
  assert_all(att_w>0, att_wo<att_w)
end


% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
