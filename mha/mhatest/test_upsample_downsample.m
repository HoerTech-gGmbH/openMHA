% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2018 HörTech gGmbH
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

function [] = test_upsample_downsample
    dsc.instance = 'test_upsample';
    dsc.nchannels_in = 1;
    dsc.srate = 16000;
    dsc.fragsize = 256;
    dsc.mhalib = 'mhachain';
    dsc.mha.algos='[upsample downsample]';
    dsc.mha.upsample.ratio = 2;
    dsc.mha.downsample.ratio =  dsc.mha.upsample.ratio;
    dsc.iolib = 'MHAIOParser';
    dsc.mha.upsample.antialias.A = [1 -1.4797 1.4037 -0.6809 0.1919 -0.0221];
    dsc.mha.upsample.antialias.B = [0.0129 0.0645 0.1290 0.1290 0.0645 0.0129];
    dsc.mha.downsample.antialias.A = [1 -1.4797 1.4037 -0.6809 0.1919 -0.0221];
    dsc.mha.downsample.antialias.B = [0.0129 0.0645 0.1290 0.1290 0.0645 0.0129];

    mha = mha_start();
    unittest_teardown(@mha_set, mha, 'cmd', 'quit');

    mha_set(mha,'',dsc);
    mha_set(mha, 'cmd', 'start');

    % range
    x = [1:dsc.fragsize];
    % input sequence
    in = 0.5*cos(pi*0.1*x);

    % I/O
    mha_set(mha, 'io.input', in);
    out = mha_get(mha, 'io.output');

    % match the phases of input and output signals, ignoring settling time of filter
    err = @(phi) mean((out(5:end) - 0.5*cos(pi*0.1*x(5:end)+phi) ).^ 2);
    phi = 1;
    [phi,e] = fminsearch(err, phi);

    % Input and output have to be of the same size
    assert_equal(size(in),size(out));

    % expect difference between input and output of the order of -45dB
    assert_difference_below(0,sqrt(e),10^(-40/20));
end

% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
