% Execute MHA with MHAIOportaudio
% and check that the expected monitor variables are there.
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2021 HörTech gGmbH

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

function test_mhaioportaudio
    mha = mha_start;
    unittest_teardown(@mha_set, mha, 'cmd', 'quit');
    mha_set(mha,"nchannels_in",1);
    mha_set(mha,"srate",16000);
    mha_set(mha,"fragsize",1024);
    mha_set(mha,"iolib","MHAIOPortAudio");

    % Get monitor variables added in T1588. We can only test for presence
    % and correct default values as there's no guarantee the test machine
    % has a PortAudio device installed
    paInputLatency=mha_get(mha,"io.stream_info.paInputLatency");
    paOutputLatency=mha_get(mha,"io.stream_info.paOutputLatency");
    paSampleRate=mha_get(mha,'io.stream_info.paSampleRate');
    assert_equal(0, paInputLatency);
    assert_equal(0, paOutputLatency);
    assert_equal(0, paSampleRate);
end
% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
