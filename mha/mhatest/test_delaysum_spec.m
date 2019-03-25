% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2017 2018 2019 HörTech gGmbH
%
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

function test_delaysum_spec

  dsc.instance = 'test_delaysum';
    dsc.nchannels_in = 2;
    dsc.srate=44100;
    dsc.fragsize = 200;
    dsc.mhalib = 'overlapadd';
    dsc.mha.fftlen=800;
    dsc.mha.wnd.len=400;
    dsc.mha.plugin_name='delaysum_spec';
    % fixed delay of 0.5 samples
    dl = 0.5/dsc.srate;
    dsc.mha.delaysum_spec.gain=[1 -1];
    dsc.mha.delaysum_spec.groupdelay = [0 dl];

    mha = mha_start();
    unittest_teardown(@mha_set, mha, 'cmd', 'quit');

    mha_set(mha,'',dsc);
    mha_set(mha, 'iolib', 'MHAIOParser');
    mha_set(mha, 'cmd', 'start');

    % Input signal
    x = [1:10*dsc.fragsize];
    ch1_input = 0.5*cos(pi*0.1*x);
    % Channel 2 is advanced by 0.5 samples relative to channel 1
    ch2_input = 0.5*cos(pi*0.1*(x+0.5));
    in_sig = [ch1_input; ch2_input];

    output_signal=[];
    for i = 1:10
      mha_set(mha, 'io.input', in_sig(:,(i-1)*dsc.fragsize+1:i*dsc.fragsize));
      output_signal = [output_signal mha_get(mha, 'io.output')];
    end

    % Discard transient
    output_signal=output_signal(4*dsc.fragsize:end);

    %If the plugin works correctly, the signals should have canceled each other out
    assert_difference_below(output_signal,zeros(size(output_signal)), 10^(-100/20));
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
