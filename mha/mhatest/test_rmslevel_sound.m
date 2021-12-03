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

function test_rmslevel_sound

    % Test output computed by RMSlevel for complete silence input in 3 conditions:
    % 1) waveform processing, 2) stft processing with SPL 3) stft with HL
    test_rmslevel_sound_wave();
    test_rmslevel_sound_stft('spl');
    test_rmslevel_sound_stft('hl');
end

function test_rmslevel_sound_wave
    mha = mha_start;
    unittest_teardown(@mha_set,mha,'cmd','quit');

    CH = 3; % Unusual channel count
    S = 70; % unusual fragsize, resulting in block rate 630Hz
    % Minimalistic setup to test rmslevel monitor and AC variables
    mha_set(mha,'instance','test_rmslevel_sound_wave');
    mha_set(mha,'nchannels_in', CH);
    mha_set(mha,'fragsize',S);
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
    
    % Finally, after processing 1 block of sine wave 630Hz, all
    % results should be at expected values.
    sine = repmat(sin([1:S]/S*2*pi),CH,1);
    mha_set(mha,'io.input',sine);
    expected_level = sqrt(0.5);
    expected_peak = 1;
    expected_peak_dB = 20*log10(1/2e-5);
    expected_level_dB = expected_peak_dB + 20*log10(expected_level);
    expected = repmat([expected_level
                       expected_level_dB
                       expected_peak
                       expected_peak_dB], 1, CH);
    check_result(mha,'mha.RmS.',expected);
    check_result(mha,'mha.acmon.RmS_',expected);
end

function test_rmslevel_sound_stft(unit)
    mha = mha_start;
    unittest_teardown(@mha_set,mha,'cmd','quit');

    CH = 5; % Unusual channel count, to compare later
    S = 70; % unusual fragsize, resulting in block rate 630Hz
    % Minimalistic setup to test rmslevel with STFT processing
    mha_set(mha,'instance',['test_rmslevel_sound_stft_' unit]);
    mha_set(mha,'nchannels_in', CH);
    mha_set(mha,'fragsize',S);
    mha_set(mha,'iolib','MHAIOParser');
    mha_set(mha,'mhalib','mhachain');
    mha_set(mha,'mha.algos',{'wave2spec','rmslevel','acmon','spec2wave'});
    mha_set(mha,'mha.wave2spec.wndlen', 2*S);
    mha_set(mha,'mha.wave2spec.wndtype', 'rect')
    mha_set(mha,'mha.rmslevel.unit', unit);

    % Prepare. Then, before the first processing block, all monitor
    % variables should contain NaNs, peak AC variables should not exist.
    mha_set(mha,'cmd','start');
    check_result(mha,'mha.rmslevel.',NaN(4,CH));
    % 42 is magic for missing variable in get_result.
    check_result(mha,'mha.acmon.rmslevel_',[NaN(2,CH);42*ones(2,CH)]);
    
    sine = repmat(sin([1:S]/S*2*pi),CH,1);
    mha_set(mha,'io.input',sine);
    mha_set(mha,'io.input',sine);
    % Finally, after processing 1 full analysis window of sinusoid, all
    % results should be at minimum possible value.
    expected_mon = [         sqrt(0.5)        % level
                    20*log10(sqrt(0.5)/2e-5)  % level_dB
                      NaN                     % peak missing / not filled
                      NaN                   ];% peak_dB missing / not filled
    if isequal('hl',unit)
      expected_mon(2) = expected_mon(2) - 3.568; % HL correction for 630Hz
    end
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
        assert_almost(actual,expected,2e-3);
    end
end

function check_result(mha, path, expected)
    result = get_result(mha,path);
    comparison(expected(1,:), result.level);
    comparison(expected(2,:), result.level_db);
    comparison(expected(3,:), result.peak);
    comparison(expected(4,:), result.peak_db);
end
