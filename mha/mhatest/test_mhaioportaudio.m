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
    mha_set(mha,'nchannels_in',1);
    mha_set(mha,'srate',16000);
    mha_set(mha,'fragsize',1024);
    if ispc % On our windows buildserver, the portaudio lib can fail with
      try   % 'Internal PortAudio error' when the build job is started by the
            % Task Scheduler Library without any user actually logged in.
            % Do not fail because of this, instead, skip this test.
        mha_set(mha,'iolib','MHAIOPortAudio');
      catch e
        if ~isempty(strfind(e.message,'Internal PortAudio error'))
          return; % Skip this test on Windows if we have this error.
        end
        rethrow(e);
      end
    else % On other OS, the above error is not ignored.
      mha_set(mha,'iolib','MHAIOPortAudio');
    end
    % Get monitor variables added in T1588. We can only test for presence
    % and correct default values as there's no guarantee the test machine
    % has a PortAudio device installed
    paInputLatency=mha_get(mha,'io.stream_info.paInputLatency');
    paOutputLatency=mha_get(mha,'io.stream_info.paOutputLatency');
    paSampleRate=mha_get(mha,'io.stream_info.paSampleRate');
    assert_equal(0, paInputLatency);
    assert_equal(0, paOutputLatency);
    assert_equal(0, paSampleRate);

    % Test existence of config vars added for T1586
    mha_set(mha,'io.suggested_input_latency',1);
    mha_set(mha,'io.suggested_output_latency',1);

    % Test range
    n_errors=0;
    try
      mha_set(mha,'io.suggested_input_latency', -1 );
    catch e
      n_errors++;
      assert(~isempty(strfind(e.message,'range')));
    end
    assert_equal(n_errors,1);

    try
      mha_set(mha,'io.suggested_output_latency', -1 );
    catch e
      n_errors++;
      assert(~isempty(strfind(e.message,'range')));
    end
    assert_equal(n_errors,2);
end
% Local Variables:
% mode: octave
% coding: utf-8-unix
% indent-tabs-mode: nil
% End:
