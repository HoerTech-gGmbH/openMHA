%% This file is part of the HörTech Open Master Hearing Aid (openMHA)
%% Copyright © 2021 HörTech gGmbH
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

function test_rmslevel_silence

    % Test output computed by RMSlevel for complete silence input in 3 conditions:
    % 1) waveform processing, 2) stft processing with SPL 3) stft with HL
    test_rmslevel_silence_wave();
    test_rmslevel_silence_stft('spl');
    test_rmslevel_silence_stft('hl');
end

function test_rmslevel_silence_wave
    mha = mha_start;
    unittest_teardown(@mha_set,mha,'cmd','quit');

    CH = 3; % Unusual channel count, to detect later
    % Minimalistic setup to test rmslevel monitor and AC variables
    mha_set(mha,'instance','test_rmslevel_silence_wave');
    mha_set(mha,'nchannels_in', CH);
    mha_set(mha,'iolib','MHAIOParser');
    mha_set(mha,'mhalib','mhachain');
    mha_set(mha,'mha.algos',{'rmslevel:RmS','acmon'});

    % Before first prepare, all monitors should be empty
    mon = get_result(mha,'mha.RmS.');
    assert_all([isempty(mon.level)
                isempty(mon.level_db)
                isempty(mon.peak)
                isempty(mon.peak_db)]);

    % Prepare. Then, before the first processing block, all monitor
    % variables and ac variable should contain NaNs.
    mha_set(mha,'cmd','start');
    check_result(mha,'mha.RmS.',NaN(4,CH));
    check_result(mha,'mha.acmon.RmS_',NaN(4,CH));
    
    % Finally, after processing 1 block of complete silence, all
    % results should be at minimum possible value.
    mha_set(mha,'io.input',zeros(CH,mha_get(mha,'fragsize')));
    expected = [2e-10,2e-10,2e-10  % level
                 -100, -100, -100  % level_dB
                2e-10,2e-10,2e-10  % peak
                 -100, -100, -100];% peak_dB
    check_result(mha,'mha.RmS.',expected);
    check_result(mha,'mha.acmon.RmS_',expected);
end

function test_rmslevel_silence_stft(unit)
    mha = mha_start;
    unittest_teardown(@mha_set,mha,'cmd','quit');

    CH = 5; % Unusual channel count, to compare later
    % Minimalistic setup to test rmslevel with STFT processing
    mha_set(mha,'instance',['test_rmslevel_silence_stft_' unit]);
    mha_set(mha,'nchannels_in', CH);
    mha_set(mha,'iolib','MHAIOParser');
    mha_set(mha,'mhalib','mhachain');
    mha_set(mha,'mha.algos',{'wave2spec','rmslevel','acmon','spec2wave'});
    mha_set(mha,'mha.rmslevel.unit', unit);

    % Prepare. Then, before the first processing block, all monitor
    % variables should contain NaNs, peak AC variables should not exist.
    mha_set(mha,'cmd','start');
    check_result(mha,'mha.rmslevel.',NaN(4,CH));
    % 42 is magic for missing variable in get_result.
    check_result(mha,'mha.acmon.rmslevel_',[NaN(2,CH);42*ones(2,CH)]);
    
    % Finally, after processing 1 block of complete silence, all
    % results should be at minimum possible value.
    mha_set(mha,'io.input',zeros(CH,mha_get(mha,'fragsize')));
    expected_mon = [2e-10  % level
                     -100  % level_dB
                      NaN  % peak missing / not filled
                      NaN];% peak_dB missing / not filled
    expected_mon = repmat(expected_mon,1,CH);
    expected_ac = expected_mon;
    expected_ac(isnan(expected_ac)) = 42; % Magic for "AC variable absent"
    check_result(mha,'mha.rmslevel.',expected_mon);
    check_result(mha,'mha.acmon.rmslevel_',expected_ac);
end

function result = get_result(mha, path)
    % get level, level_db, peak, peak_db from mha at path
    result.level    = mha_get(mha,[path 'level']);
    result.level_db = mha_get(mha,[path 'level_db']);
    try % peak variables may not be available
        result.peak     = mha_get(mha,[path 'peak']);
        result.peak_db  = mha_get(mha,[path 'peak_db']);
    catch
        % in this case, place a magic value in them
        result.peak = ones(size(result.level)) * 42;
        result.peak_db = result.peak;
    end
end

function comparison(actual,expected)
    % Adaptive comparison: We cannot use assert_almost when we expect NaNs
    if any(isnan(expected(:)))
        assert_all(isequaln(actual,expected));
    else
        assert_almost(actual,expected,1e-5);
    end
end

function check_result(mha, path, expected)
    result = get_result(mha,path);
    comparison(expected(1,:), result.level);
    comparison(expected(2,:), result.level_db);
    comparison(expected(3,:), result.peak);
    comparison(expected(4,:), result.peak_db);
end
