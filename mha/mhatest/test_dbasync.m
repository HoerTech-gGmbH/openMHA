function test_dbasync
% Tests dbasync plugin.
% The following conditions are tested:
% - Throws Exceptions for delays too small (includes negative).
% - Assert delays big enough are handled correctly (includes huge delays).
% - Channelcount is modified by inner plugin.
% - Unprepare command is properly propagated to inner processing loop and inner
%   processing loop exits cleanly.

  %% Set up clean-up after we are finished
  inwav = 'IN.wav';
  outwav = 'OUT.wav';
  unittest_teardown(@delete, [inwav]);
  unittest_teardown(@delete, [outwav]);
  fclose(fopen(inwav, 'w'));
  fclose(fopen(outwav, 'w'));

  in.srate = 44100;
  in.siglen = 4096;

  inner_fragsizes = [ 32 64 63 66 4032  0];
  outer_fragsizes = [256 64 64 64   64 64];

  %% delay is computed each time:
  %% includes negative -> throw exception
  %% includes 0 -> throw exception if < min_delay
  %% includes min_delay - 1
  %% includes min_delay
  %% includes min_delay + 1
  %% includes min_delay + 1000

  nchannels_ins = [1];
  nchannels_outs = [1];

  inner_exceptions = [false true];

  for fragsize_index = 1:length(inner_fragsizes);
    inner_fragsize = inner_fragsizes(fragsize_index);
    outer_fragsize = outer_fragsizes(fragsize_index);
    min_delay = inner_fragsize - gcd(inner_fragsize, outer_fragsize);
    delays = [-1, 0, min_delay-1, min_delay, min_delay+1];

    in.fragsize = outer_fragsize;
    in.wndlen = 2*outer_fragsize;
    in.fftlen = 3*outer_fragsize;

    for nchannels_in = nchannels_ins;
      in.channels = nchannels_in;
      in.signal = repeatable_rand(in.channels, in.siglen,0) - 0.5;
      audiowrite(inwav,in.signal',in.srate,'BitsPerSample',32);
      for delay = delays;
        for nchannels_out = nchannels_outs;
          dbasync = struct('delay', delay, ...
                           'fragsize', inner_fragsize, ...
                           'plugin_name', 'matrixmixer');
          dbasync.matrixmixer.m = channelcount_matrix(nchannels_in, nchannels_out);
          run_test(dbasync,in,nchannels_out,delay,min_delay,inner_fragsize,outer_fragsize)
        end
      end
    end
  end
end

function matrix = channelcount_matrix(nchannels_in, nchannels_out)
  %% A mixer matrix for the matrixmixer to
  %% convert channelcounts. Keeping either
  %% the first n channels, or repeating
  %% channels as necessary.
  matrix = zeros(nchannels_out, nchannels_in);
  if nchannels_in
    for ch = 0:(nchannels_out-1)
      matrix(ch+1,mod(ch,nchannels_in)+1) = 1;
    end
  end
end


function run_test(dbasync,in,nchannels_out,delay,min_delay,inner_fragsize,outer_fragsize)
  inwav = 'IN.wav';
  outwav = 'OUT.wav';

  mha = mha_start;
  unittest_teardown(@mha_set, mha, 'cmd','quit');
  mha_set(mha, 'nchannels_in', in.channels);
  mha_set(mha, 'srate', in.srate);
  mha_set(mha, 'fragsize', in.fragsize);
  mha_set(mha, 'iolib', 'MHAIOFile');
  mha_set(mha, 'io.in', inwav);
  mha_set(mha, 'io.out', outwav);
  mha_set(mha, 'mhalib', 'dbasync');
  expected_error = '';
  catched_error = '';
  if delay < min_delay
    expected_error = ...
    sprintf('(dbasync) Delay %d too small: need at least %d delay for outer fragsize %d and inner fragsize %d', ...
            delay, min_delay, outer_fragsize, inner_fragsize);
  end
  if inner_fragsize <= 0
    expected_error = sprintf('(mha_parser) The value %d is not in the range [1,].', inner_fragsize);
  end
  if delay < 0
    expected_error = ...
    sprintf('(mha_parser) The value %d is not in the range [0,].', ...
            delay);
  end
  try
    mha_set(mha, 'mha', dbasync);
    mha_set(mha, 'cmd', 'prepare');
  catch
    catched_error = lasterr;
    if isempty(expected_error)
      rethrow(lasterror);
    end
  end
  if (length(catched_error) < length(expected_error)) || ...
     ~isequal(catched_error((length(catched_error)-length(expected_error)+1):end), expected_error)
    error('','expected error ''%s'', but got error ''%s''', ...
          expected_error, catched_error);
  end

  if isempty(expected_error)
    assert_equal(nchannels_out, mha_get(mha, 'nchannels_out'));
    try
      mha_set(mha, 'cmd', 'start');
      mha_set(mha, 'cmd', 'release');
    catch
      err = lasterror;
      err.message = sprintf('%s (%d,%d,%d,%d,%d)', err.message, fragsize_index, nchannels_in, delay, nchannels_out, i);
      rethrow(err);
    end
    out.signal=audioread(outwav)';
    assert_difference_below(dbasync.matrixmixer.m * in.signal(:,1:end-delay), ...
                            out.signal(:,1+delay:end), 1e-6);
  end
end

% Local Variables:
% indent-tabs-mode: nil
% End:
