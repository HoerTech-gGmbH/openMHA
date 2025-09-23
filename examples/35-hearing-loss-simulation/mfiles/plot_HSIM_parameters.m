% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2017 2018 2019 2020 2021 HörTech gGmbH
% Copyright © 2023 2024 2025 Hörzentrum Oldenburg gGmbH
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


function plot_HSIM_parameters()
    % Function for computing and plotting the parameters of an openMHA
    % hearing impairment simulator
    %
    % This function depends on the gammatone toolbox by Hohmann and Herzke
    % (2007), which must be downloaded and then added to the Matlab/Octave
    % search path.
    %
    % NB: When using Octave, figures may have to be closed *manually*
    % (e.g., by issuing "close all" in the Command Window) before
    % re-running this function for the figures to be rendered correctly on
    % subsequent runs.
    %
    % Usage:
    % plot_HSIM_parameters()

    clear;
    close all hidden;
    clc;

    % ---------------------------------------------------------------------

    % Turn warnings for obsolete functions off in Octave:
    if exist('OCTAVE_VERSION', 'builtin')
        warning('off', 'Octave:legacy-function');
    end

    % Set default figure position explicitly to ensure the same output in
    % Matlab and Octave:
    defaultFigPos = get(0, 'DefaultFigurePosition');
    set(0, 'DefaultFigurePosition', [680 558 560 420]);

    % Add paths to necessary openMHA and gammatone filterbank functions:
    addpath(genpath('mfiles'));
    addpath(genpath('gammatone'));


    %% Variables:

    % Desired broadband input signal level / dB SPL:
    desiredInputSignalLevel = 65.0;


    % Audiogram type (symmetric hearing loss) according to
    % Bisgaard et al. (2010); overrides any specifications in the struct 
    % HTL unless sAudiogramType is set to 'Custom':
    sAudiogramType = 'N1';  % {'NH', 'N1', 'N2', 'N4', 'N4', 'N5', 'N6', 'N7', 'S1', 'S2', 'S3', 'Custom'}

    % Hearing threshold levels (only used if sAudiogramType is set to 
    % 'Custom' and values for at least one side and the corresponding
    % frequencies are specified):
    HTL.f = [125 250 500 750 1000 1500 2000 3000 4000 6000 8000];  % frequencies / Hz
    HTL.l = [];  % HTL (left side) / dB HL
    HTL.r = [];  % HTL (right side) / dB HL

    % Most comfortable loudness levels (only used if values for at least
    % one side and the corresponding frequencies are specified):
    MCL.f = [125 250 500 750 1000 1500 2000 3000 4000 6000 8000];  % frequencies / Hz
    MCL.l = [];  % MCL (left side) / dB HL
    MCL.r = [];  % MCL (right side) / dB HL

    % Uncomfortable loudness levels (only used if values for at least one
    % side and the corresponding frequencies are specified):
    UCL.f = [125 250 500 750 1000 1500 2000 3000 4000 6000 8000];  % frequencies / Hz
    UCL.l = [];  % UCL (left side) / dB HL
    UCL.r = [];  % UCL (right side) / dB HL


    % Variables for selecting a particular frequency band and side to be
    % plotted:
    band = 32;  % index of frequency band for which to determine dynamic range expansion and spectral smearing (from 1 to 32)
    side = 'l';  % side (ear) for which to determine dynamic range expansion and spectral smearing ('l' = left, 'r' = right)


    % Variables for the auditory filterbank:
    srate = 48000;  % sampling frequency / Hz
    lowerCutoffFrequency = 70.0;  % lowest possible gammatone filterbank center frequency / Hz
    specifiedCenterFrequency = 1000.0;  % gammatone filterbank "base frequency" / Hz
    upperCutoffFrequency = 8500.0;  % highest possible gammatone filterbank center frequency / Hz
    filtersPerERBaud = 1.0;  % number of gammatone filters per ERB
    gammaFilterOrder = 4;  % gammatone filterbank filter order
    bandwidthFactor = 1.0;  % gammatone filterbank bandwidth widening factor
    displayRange_magnitude = [-40 0];  % display range for magnitude / dB
    displayRange_frequency_filterbank = [];  % display range for frequency / Hz (automatically determined if empty)


    % Variables for dynamic range expansion:
    gtstep = 1.0;  % band-specific step size between input levels / dB
    gtmin = 0.0;  % minimum input level for dynamic range expansion / dB SPL
    gtmax = 140.0;  % maximum input level for dynamic range expansion / dB SPL
    log_interp = true;  % use logarithmic interpolation (dc plugin)


    % Variables for spectral smearing:
    duration = 1.0;  % signal duration / s
    p0 = 20.0 * 10^-6;  % reference sound pressure / Pa
    NFFT = [];  % FFT length / samples
    displayRange_time = [];  % display range for time / s (automatically determined if empty)
    displayRange_frequency_smearing = [];  % display range for frequency / Hz (automatically determined if empty)

    % Smearing factors (only used if sSmearingType is set to 'Custom'
    % and values for at least one side and the corresponding frequencies
    % are specified):
    sSmearingType = 'Automatic';  % {'Automatic', 'Custom'}
    SF.f = [125 250 500 750 1000 1500 2000 3000 4000 6000 8000];  % frequencies / Hz
    SF.l = [];  % smearing factors (left side) / dB HL
    SF.r = [];  % smearing factors (right side) / dB HL


    % Variables for plotting:
    fontSize = 10;  % font size
    lineWidth = 0.75;  % line width
    markerSize = 4;  % marker size


    %% Gammatone filterbank:

    % Compute gammatone filterbank analyzer struct:
    analyzer = Gfb_Analyzer_new(srate, lowerCutoffFrequency, specifiedCenterFrequency, upperCutoffFrequency, filtersPerERBaud, gammaFilterOrder, bandwidthFactor);

    % Compute gammatone filterbank center frequencies / Hz:
    vCenterFrequencies = analyzer.center_frequencies_hz;
    if band > length(vCenterFrequencies)
        warndlg(sprintf('The chosen frequency band index (currently set to %u) must be in the range [1, %u].', band, length(vCenterFrequencies)), 'Index out of bounds');
        return;
    end

    % Plot gammatone filterbank:
    figure;
    set(gcf, 'Color', 'w');
    vImpulse = [1, zeros(1, srate/2-1)];
    mImpulseResponse = Gfb_Analyzer_process(analyzer, vImpulse);
    mFrequencyResponse = fft(real(mImpulseResponse)');
    vFrequency = linspace(0, srate, size(mFrequencyResponse, 1))';
    C = get(gca, 'ColorOrder');
    mMagnitudeResponse = 20*log10((abs(mFrequencyResponse)));
    p = plot(vFrequency, mMagnitudeResponse, 'Color', C(1, :), 'LineWidth', lineWidth);
    set(p(band), 'Color', C(2, :));
    set(p(band), 'LineWidth', 2*lineWidth);
    grid on;
    if isempty(displayRange_frequency_filterbank)
        mMagnitudeResponse(size(mMagnitudeResponse, 1)/2+2:end, :) = NaN;
        mGainsInDisplayRange = mMagnitudeResponse >= displayRange_magnitude(1);
        rowIndices = (1:size(mMagnitudeResponse, 1))';
        rowIndices_expanded = repmat(rowIndices, 1, size(mGainsInDisplayRange, 2));
        highestGainInDisplayRange = max(rowIndices_expanded .* mGainsInDisplayRange);
        highestFrequenyInDisplayRange = min(vFrequency(max(highestGainInDisplayRange)), srate/2);
        xlim([0, ceil(highestFrequenyInDisplayRange/1000)*1000]);
    else
        xlim(displayRange_frequency_filterbank);
    end
    ylim(displayRange_magnitude);
    xlabel('Frequency / Hz');
    ylabel('Magnitude / dB');
    title({'Gammatone Filterbank (Without Hearing Impairment Simulation)', ''});
    hold on;
    p_dummy = plot(NaN, 'Color', C(2, :), 'LineWidth', 2*lineWidth);
    hold off;
    legend(p_dummy, ' Specified Frequency Band');
    set(gca, 'FontSize', fontSize, ...
        'LineWidth', lineWidth);
    pos = get(gcf, 'Position');
    pos(1) = pos(1) - pos(3)/2;
    pos(3) = 2 * pos(3);
    set(gcf, 'Position', pos);
    drawnow;


    %% Normal-hearing threshold levels:

    % Third-octave frequencies / Hz and normal-hearing threshold
    % levels / dB SPL (ISO 226:2023):
    THIRD_OCTAVE_FREQUENCIES = [20 25 31.5 40 50 63 80 100 125 160 200 250 315 400 500 630 800 1000 1250 1600 2000 2500 3150 4000 5000 6300 8000 10000 12500];
    NORMAL_HEARING_THRESHOLD_LEVELS = [78.1 68.7 59.5 51.1 44.0 37.5 31.5 26.5 22.1 17.9 14.4 11.4 8.6 6.2 4.4 3.0 2.2 2.4 3.5 1.7 -1.3 -4.2 -6.0 -5.4 -1.5 6.0 12.6 13.9 12.3];

    % Compute normal-hearing threshold isophone:
    vHearingThresholdLevelFrequencies = THIRD_OCTAVE_FREQUENCIES(1):THIRD_OCTAVE_FREQUENCIES(end);
    vHearingThresholdLevelFrequencies_extrapolated = THIRD_OCTAVE_FREQUENCIES(end):20000;
    vHearingTresholdLevels_interpolated = NaN(size(vHearingThresholdLevelFrequencies));
    vHearingTresholdLevels_extrapolated = NaN(size(vHearingThresholdLevelFrequencies_extrapolated));
    for k = 1:length(vHearingThresholdLevelFrequencies)
        vHearingTresholdLevels_interpolated(k) = compute_HTL(vHearingThresholdLevelFrequencies(k));
    end
    for k = 1:length(vHearingThresholdLevelFrequencies_extrapolated)
        vHearingTresholdLevels_extrapolated(k) = compute_HTL(vHearingThresholdLevelFrequencies_extrapolated(k));
    end

    % Plot normal-hearing threshold isophone:
    figure;
    set(gcf, 'Color', 'w');
    semilogx(THIRD_OCTAVE_FREQUENCIES, NORMAL_HEARING_THRESHOLD_LEVELS, 'o', 'MarkerFaceColor', C(1, :), 'MarkerSize', markerSize);
    hold on;
    semilogx(vHearingThresholdLevelFrequencies, vHearingTresholdLevels_interpolated, 'Color', C(1, :), 'LineWidth', lineWidth);
    semilogx(vHearingThresholdLevelFrequencies_extrapolated, vHearingTresholdLevels_extrapolated, '--', 'Color', C(1, :), 'LineWidth', lineWidth);
    hold off;
    grid on;
    xlim([20 20000]);
    ylim([-10 130]);
    xticks = [31.5 63 125 250 500 1000 2000 4000 8000 16000];
    xticklabels = arrayfun(@num2str, xticks, 'UniformOutput', false);
    set(gca, ...
        'XMinorGrid', 'off', ...
        'XMinorTick', 'off', ...
        'XTick', xticks, ...
        'XTickLabel', xticklabels, ...
        'XTickLabelRotation', 0);
    xlabel('Frequency / Hz');
    ylabel('Level / dB SPL');
    title({'Normal-Hearing Threshold Levels (ISO 226:2023)', ''});
    legend({' HTL', ' HTL (Interpolated)', ' HTL (Extrapolated)'});
    set(gca, 'FontSize', fontSize, ...
        'LineWidth', lineWidth);
    drawnow;


    %% Auditory dynamic range:

    % Impaired hearing threshold levels, most comfortable loudness levels, and
    % uncomfortable loudness levels / dB HL (Pascoe, 1988):
    HEARING_THRESHOLD_LEVELS = 0:5:120;
    MOST_COMFORTABLE_LOUDNESS_LEVELS = [60.25 63.02 63.75 65.55 64.93 68.85 71.64 71.77 75.16 78.50 82.30 83.66 87.27 92.37 95.53 98.85 102.55 104.38 109.50 117.50 117.34 124.06 126.43 129.17 132.50];
    UNCOMFORTABLE_LOUDNESS_LEVELS = [97.25 98.97 99.46 98.08 96.58 101.26 101.76 101.05 102.63 105.37 107.45 107.69 109.97 113.75 114.84 116.76 119.82 120.00 123.67 129.55 126.56 133.13 134.29 136.67 140.00];

    % Compute MCL and UCL as functions of HTL:
    mostComfortableLoudnessLevels_fitted = NaN(size(HEARING_THRESHOLD_LEVELS));
    uncomfortableLoudnessLevels_fitted = NaN(size(HEARING_THRESHOLD_LEVELS));
    for k = 1:length(HEARING_THRESHOLD_LEVELS)
        [mostComfortableLoudnessLevels_fitted(k), uncomfortableLoudnessLevels_fitted(k)] = compute_MCL_and_UCL(HEARING_THRESHOLD_LEVELS(k));
    end

    % Plot MCL and UCL vectors as functions of HTL:
    figure;
    set(gcf, 'Color', 'w');
    p1 = plot(HEARING_THRESHOLD_LEVELS, HEARING_THRESHOLD_LEVELS, 'o', 'MarkerFaceColor', C(1, :), 'MarkerSize', markerSize);
    hold on;
    p2 = plot([HEARING_THRESHOLD_LEVELS(1) HEARING_THRESHOLD_LEVELS(end)], [HEARING_THRESHOLD_LEVELS(1) HEARING_THRESHOLD_LEVELS(end)], 'Color', C(1, :), 'LineWidth', lineWidth);
    p3 = plot(HEARING_THRESHOLD_LEVELS, MOST_COMFORTABLE_LOUDNESS_LEVELS, 'o', 'Color', C(5, :), 'MarkerFaceColor', C(5, :), 'MarkerSize', markerSize);
    p4 = plot(HEARING_THRESHOLD_LEVELS, mostComfortableLoudnessLevels_fitted, 'Color', C(5, :), 'LineWidth', lineWidth);
    p5 = plot(HEARING_THRESHOLD_LEVELS, UNCOMFORTABLE_LOUDNESS_LEVELS, 'o', 'Color', C(2, :), 'MarkerFaceColor', C(2, :), 'MarkerSize', markerSize);
    p6 = plot(HEARING_THRESHOLD_LEVELS, uncomfortableLoudnessLevels_fitted, 'Color', C(2, :), 'LineWidth', lineWidth);
    hold off;
    grid on;
    xlim([0 120]);
    ylim([0 140]);
    xlabel('Hearing Threshold Level / dB HL');
    ylabel('Loudness Level / dB HL');
    title({'Auditory Dynamic Range (Pascoe, 1988)', ''});
    legend([p5 p6 p3 p4 p1 p2], {'UCL', 'UCL (Fitted)', 'MCL', 'MCL (Fitted)', 'HTL', 'HTL (Fitted)'}, 'Location', 'southeast');
    set(gca, 'FontSize', fontSize, ...
        'LineWidth', lineWidth);
    drawnow;

    % If hearing threshold levels are only provided for one side, use the same
    % values for the other side; if no hearing threshold levels are provided,
    % use one of the standard audiogram types proposed by
    % Bisgaard et al. (2010):
    if strcmpi(sAudiogramType, 'Custom')
        if isempty(HTL.f)
            warndlg('Please specify the audiogram frequencies for the HTL.', 'HTL frequencies missing');
        end
        if isempty(HTL.l) && isempty(HTL.r)
            warndlg('Please specify an audiogram manually, or select a standard audiogram.', 'No audiogram specified');
            return;
        elseif isempty(HTL.l) && ~isempty(HTL.r)
            HTL.l = HTL.r;
        elseif ~isempty(HTL.l) && isempty(HTL.r)
            HTL.r = HTL.l;
        end
    else
        [HTL.f, HTL.l] = compute_standard_audiogram(sAudiogramType);
        HTL.r = HTL.l;
    end

    % If most comfortable loudness levels are not provided, estimate the
    % values:
    if isempty(MCL.l) && isempty(MCL.r)
        MCL.f = HTL.f;
        for k = 1:length(HTL.l)
            [MCL.l(k)] = compute_MCL_and_UCL(HTL.l(k));
        end
        for k = 1:length(HTL.r)
            [MCL.r(k)] = compute_MCL_and_UCL(HTL.r(k));
        end
    elseif isempty(MCL.l) && ~isempty(MCL.r)
        if isempty(MCL.f)
            warndlg('Please specify the audiogram frequencies for the MCL.', 'MCL frequencies missing');
        end
        MCL.l = MCL.r;
    elseif ~isempty(MCL.l) && isempty(MCL.r)
        if isempty(MCL.f)
            warndlg('Please specify the audiogram frequencies for the MCL.', 'MCL frequencies missing');
        end
        MCL.r = MCL.l;
    end

    % If uncomfortable loudness levels are not provided, estimate the values:
    if isempty(UCL.l) && isempty(UCL.r)
        UCL.f = HTL.f;
        for k = 1:length(HTL.l)
            [~, UCL.l(k)] = compute_MCL_and_UCL(HTL.l(k));
        end
        for k = 1:length(HTL.r)
            [~, UCL.r(k)] = compute_MCL_and_UCL(HTL.r(k));
        end
    elseif isempty(UCL.l) && ~isempty(UCL.r)
        if isempty(UCL.f)
            warndlg('Please specify the audiogram frequencies for the UCL.', 'UCL frequencies missing');
        end
        UCL.l = UCL.r;
    elseif ~isempty(UCL.l) && isempty(UCL.r)
        if isempty(UCL.f)
            warndlg('Please specify the audiogram frequencies for the UCL.', 'UCL frequencies missing');
        end
        UCL.r = UCL.l;
    end


    %% Audiogram:

    % Plot the audiogram:
    figure;
    set(gcf, 'Color', 'w');
    subplot(1, 2, 1);
    semilogx([125 8000], [0 0], 'k-', 'LineWidth', lineWidth);
    hold on;
    semilogx([125 8000], [mostComfortableLoudnessLevels_fitted(1) mostComfortableLoudnessLevels_fitted(1)], 'k-', 'LineWidth', lineWidth);
    semilogx([125 8000], [uncomfortableLoudnessLevels_fitted(1) uncomfortableLoudnessLevels_fitted(1)], 'k-', 'LineWidth', lineWidth);
    semilogx(HTL.f, HTL.r, '-o', 'Color', C(2, :), 'LineWidth', lineWidth, 'MarkerFaceColor', 'w', 'MarkerSize', 1.75*markerSize);
    semilogx(MCL.f, MCL.r, '-', 'Color', C(2, :), 'LineWidth', lineWidth);
    semilogx(UCL.f, UCL.r, '-', 'Color', C(2, :), 'LineWidth', lineWidth);
    hold off;
    text(MCL.f, MCL.r, 'M', 'BackgroundColor', 'w', 'Color', C(2, :), 'FontSize', fontSize, 'HorizontalAlignment', 'center', 'Margin', eps, 'VerticalAlignment', 'middle');
    text(UCL.f, UCL.r, 'U', 'BackgroundColor', 'w', 'Color', C(2, :), 'FontSize', fontSize, 'HorizontalAlignment', 'center', 'Margin', eps, 'VerticalAlignment', 'middle');
    grid on;
    axis square;
    xlim([125 8000]);
    ylim([-10 140]);
    xticks = [125 250 500 1000 2000 4000 8000];
    xticklabels = arrayfun(@num2str, xticks, 'UniformOutput', false);
    set(gca, ...
        'XMinorGrid', 'off', ...
        'XMinorTick', 'off', ...
        'XTick', xticks, ...
        'XTickLabel', xticklabels, ...
        'YDir', 'reverse', ...
        'YTick', -10:10:140);
    xlabel('Frequency / Hz');
    ylabel('Hearing Level / dB HL');
    title('Right Ear');
    set(gca, 'FontSize', fontSize, ...
        'LineWidth', lineWidth);
    subplot(1, 2, 2);
    semilogx([125 8000], [0 0], 'k-', 'LineWidth', lineWidth);
    hold on;
    semilogx([125 8000], [mostComfortableLoudnessLevels_fitted(1) mostComfortableLoudnessLevels_fitted(1)], 'k-', 'LineWidth', lineWidth);
    semilogx([125 8000], [uncomfortableLoudnessLevels_fitted(1) uncomfortableLoudnessLevels_fitted(1)], 'k-', 'LineWidth', lineWidth);
    semilogx(HTL.f, HTL.l, '-o', 'Color', C(1, :), 'LineWidth', lineWidth, 'MarkerFaceColor', 'w', 'MarkerSize', 1.75*markerSize);
    semilogx(MCL.f, MCL.l, '-', 'Color', C(1, :), 'LineWidth', lineWidth);
    semilogx(UCL.f, UCL.l, '-', 'Color', C(1, :), 'LineWidth', lineWidth);
    hold off;
    text(MCL.f, MCL.l, 'M', 'BackgroundColor', 'w', 'Color', C(1, :), 'FontSize', fontSize, 'HorizontalAlignment', 'center', 'Margin', eps, 'VerticalAlignment', 'middle');
    text(UCL.f, UCL.l, 'U', 'BackgroundColor', 'w', 'Color', C(1, :), 'FontSize', fontSize, 'HorizontalAlignment', 'center', 'Margin', eps, 'VerticalAlignment', 'middle');
    grid on;
    axis square;
    xlim([125 8000]);
    ylim([-10 140]);
    text(8000*1.1, 0, 'HTL_{NH}', 'FontSize', fontSize, 'HorizontalAlignment', 'left', 'VerticalAlignment', 'middle');
    text(8000*1.1, mostComfortableLoudnessLevels_fitted(1), 'MCL_{NH}', 'FontSize', fontSize, 'HorizontalAlignment', 'left', 'VerticalAlignment', 'middle');
    text(8000*1.1, uncomfortableLoudnessLevels_fitted(1), 'UCL_{NH}', 'FontSize', fontSize, 'HorizontalAlignment', 'left', 'VerticalAlignment', 'middle');
    set(gca, ...
        'XMinorGrid', 'off', ...
        'XMinorTick', 'off', ...
        'XTick', xticks, ...
        'XTickLabel', xticklabels, ...
        'YDir', 'reverse', ...
        'YTick', -10:10:140);
    xlabel('Frequency / Hz');
    ylabel('Hearing Level / dB HL');
    title('Left Ear');
    if strcmpi(sAudiogramType, 'Custom')
        p = get(gcf, 'Position');
        ax = findall(gcf, 'Type', 'axes');
        for k = 1:numel(ax)
            pos = get(ax(k), 'Position');
            pos(4) = pos(4) - fontSize*2/p(4);
            set(ax(k), 'Position', pos);
        end
        axes('Position', [0 0 1 1], 'Visible', 'off');
        text(0.5, 0.995, 'Audiogram', 'FontSize', fontSize*1.3, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'top');
    else
        sAudiogramType = regexprep(sAudiogramType, '([A-Z])(\d)', '{\\it$1}_$2');
        p = get(gcf, 'Position');
        ax = findall(gcf, 'Type', 'axes');
        for k = 1:numel(ax)
            pos = get(ax(k), 'Position');
            pos(4) = pos(4) - fontSize*2/p(4);
            set(ax(k), 'Position', pos);
        end
        axes('Position', [0 0 1 1], 'Visible', 'off');
        text(0.5, 0.995, ['Audiogram: Type ' sAudiogramType ' (Bisgaard et al., 2010; Pascoe, 1988)'], 'FontSize', fontSize*1.3, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'top');
    end
    set(gca, 'FontSize', fontSize, ...
        'LineWidth', lineWidth);
    pos = get(gcf, 'Position');
    pos(1) = pos(1) - pos(3)/4;
    pos(3) = 1.5 * pos(3);
    set(gcf, 'Position', pos);
    drawnow;


    %% Dynamic range expansion (input-output characteristic):

    % Normal-hearing auditory dynamic range / dB SPL:
    normalHearingThresholdLevel_dBSPL = compute_HTL(vCenterFrequencies(band));
    normalHearingThresholdLevel_dBHL = 0.0;
    normalMostComfortableLoudnessLevel_dBHL = mostComfortableLoudnessLevels_fitted(1);
    normalUncomfortableLoudnessLevel_dBHL = uncomfortableLoudnessLevels_fitted(1);
    HTL_NH = normalHearingThresholdLevel_dBSPL + normalHearingThresholdLevel_dBHL;
    MCL_NH = normalHearingThresholdLevel_dBSPL + normalMostComfortableLoudnessLevel_dBHL;
    UCL_NH = normalHearingThresholdLevel_dBSPL + normalUncomfortableLoudnessLevel_dBHL;

    % Impaired auditory dynamic range / dB SPL (for specified audiogram and
    % frequency band):
    fc = vCenterFrequencies(band);
    impairedHearingThresholdLevel_dBHL = interp1(log(HTL.f), HTL.(side), log(fc), 'linear', 'extrap');
    impairedMostComfortableLoudnessLevel_dBHL = interp1(log(MCL.f), MCL.(side), log(fc), 'linear', 'extrap');
    impairedUncomfortableLoudnessLevel_dBHL = interp1(log(UCL.f), UCL.(side), log(fc), 'linear', 'extrap');
    HTL_HI = normalHearingThresholdLevel_dBSPL + impairedHearingThresholdLevel_dBHL;
    MCL_HI = normalHearingThresholdLevel_dBSPL + impairedMostComfortableLoudnessLevel_dBHL;
    UCL_HI = normalHearingThresholdLevel_dBSPL + impairedUncomfortableLoudnessLevel_dBHL;

    % Slopes of the two linear pieces of the input-output characteristic:
    slope1 = (MCL_NH - HTL_NH)/(MCL_HI - HTL_HI);
    slope2 = (UCL_NH - MCL_NH)/(UCL_HI - MCL_HI);

    % Intercepts of the two linear pieces of the input-output characteristic:
    intercept1 = HTL_NH - slope1 * HTL_HI;
    intercept2 = MCL_NH - slope2 * MCL_HI;

    % Input (x-data) of the two linear pieces of the input-output
    % characteristic:
    gtinput1 = HTL_HI:gtstep:MCL_HI;
    gtinput2 = MCL_HI:gtstep:UCL_HI;

    % Gaintable data of the two linear pieces of the input-output
    % characteristic:
    gtdata1 = NaN(1, length(gtinput1));
    for k = 1:length(gtinput1)
        gtdata1(k) = slope1 * gtinput1(k) + intercept1;
    end
    gtdata2 = NaN(1, length(gtinput2));
    for k = 1:length(gtinput2)
        gtdata2(k) = slope2 * gtinput2(k) + intercept2;
    end

    % Input (x-data) of the whole input-output characteristic:
    gtinput = [gtinput1(1)-gtstep gtinput1 gtinput2(2:end) gtinput2(end)+gtstep];
    gtinput_extrap = gtmin:gtstep:gtmax;

    % Gaintable data of the whole input-output characteristic:
    gtdata = [gtdata1(1)-100 gtdata1 gtdata2(2:end) gtdata2(end)+0];
    gtdata_extrap = interp1(gtinput, gtdata, gtinput_extrap, 'linear', 'extrap') - gtinput_extrap;
    gtdata = gtdata_extrap;

    displayRange_level = [gtmin gtmax];  % range of levels / dB SPL to be plotted

    % Plot the input-output characteristic:
    figure;
    set(gcf, 'Color', 'w');
    subplot(1, 1, 1);
    plot(displayRange_level, displayRange_level, 'k--', 'LineWidth', lineWidth);
    hold on;
    dc_plot_io(gtmin, gtstep, gtdata, gtmin:gtstep:gtmax, log_interp);
    plot([HTL_HI HTL_HI], [HTL_NH displayRange_level(end)], '--', 'Color', C(2, :), 'LineWidth', lineWidth);
    plot([MCL_HI MCL_HI], [MCL_NH displayRange_level(end)], '--', 'Color', C(2, :), 'LineWidth', lineWidth);
    plot([UCL_HI UCL_HI], [UCL_NH displayRange_level(end)], '--', 'Color', C(2, :), 'LineWidth', lineWidth);
    plot([HTL_HI displayRange_level(end)], [HTL_NH HTL_NH], '--', 'Color', C(2, :), 'LineWidth', lineWidth);
    plot([MCL_HI displayRange_level(end)], [MCL_NH MCL_NH], '--', 'Color', C(2, :), 'LineWidth', lineWidth);
    plot([UCL_HI displayRange_level(end)], [UCL_NH UCL_NH], '--', 'Color', C(2, :), 'LineWidth', lineWidth);
    hold off;
    text(HTL_HI, displayRange_level(end)*1.005, 'HTL_{HI}', 'Color', C(2, :), 'FontSize', fontSize, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'bottom');
    text(MCL_HI, displayRange_level(end)*1.005, 'MCL_{HI}', 'Color', C(2, :), 'FontSize', fontSize, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'bottom');
    text(UCL_HI, displayRange_level(end)*1.005, 'UCL_{HI}', 'Color', C(2, :), 'FontSize', fontSize, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'bottom');
    text(displayRange_level(end)*1.01, HTL_NH, ' HTL_{NH}', 'Color', C(2, :), 'FontSize', fontSize, 'HorizontalAlignment', 'left', 'VerticalAlignment', 'middle');
    text(displayRange_level(end)*1.01, MCL_NH, ' MCL_{NH}', 'Color', C(2, :), 'FontSize', fontSize, 'HorizontalAlignment', 'left', 'VerticalAlignment', 'middle');
    text(displayRange_level(end)*1.01, UCL_NH, ' UCL_{NH}', 'Color', C(2, :), 'FontSize', fontSize, 'HorizontalAlignment', 'left', 'VerticalAlignment', 'middle');
    ax = gca;
    ch = get(ax, 'Children');
    set(ch(end-1), 'Color', C(1, :));
    set(ch(end-1), 'LineWidth', 2*lineWidth);
    set(ax, 'XTick', displayRange_level(1):20:displayRange_level(end));
    set(ax, 'YTick', displayRange_level(1):20:displayRange_level(end));
    grid on;
    axis square;
    xlim(displayRange_level);
    ylim(displayRange_level);
    xlabel('Input Level / dB SPL');
    ylabel('Output Level / dB SPL');
    title(' ');
    p = get(gcf, 'Position');
    ax = findall(gcf, 'Type', 'axes');
    for k = 1:numel(ax)
        pos = get(ax(k), 'Position');
        pos(4) = pos(4) - fontSize*2/p(4);
        set(ax(k), 'Position', pos);
    end
    axes('Position', [0 0 1 1], 'Visible', 'off');
    if strcmpi(side, 'l')
        sSide = 'Left';
    elseif strcmpi(side, 'r')
        sSide = 'Right';
    end
    text(0.5, 0.995, ['Dynamic Range Expansion (' sSide ' Ear, Center Frequency: ' num2str(round(fc)) ' Hz)'], 'FontSize', fontSize*1.3, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'top');
    set(gca, 'FontSize', fontSize, ...
        'LineWidth', lineWidth);
    drawnow;


    %% Relative equivalent rectangular bandwidth as a function of hearing threshold level (in dB SPL):

    % Threshold levels / dB SPL and relative equivalent rectangular bandwidths
    % (as proportions of center frequency) at 1000 Hz
    % (Glasberg & Moore, 1986):
    HEARING_THRESHOLD_LEVELS_1000HZ_DBSPL = [-1 3 6 19 13 10 49 61 41 49 37 52 52 15 39 51 48 25 31 56 58 50 55];
    EQUIVALENT_RECTANGULAR_BANDWIDTHS_1000HZ = [0.15 0.14 0.15 0.14 0.17 0.17 0.37 0.60 0.31 0.46 0.18 0.40 0.55 0.17 0.37 0.26 0.21 0.16 0.16 0.33 0.38 0.25 0.40];

    % Threshold levels / dB SPL and relative equivalent rectangular bandwidths
    % (as proportions of center frequency) at 100 Hz (Peters & Moore, 1992):
    HEARING_THRESHOLD_LEVELS_100HZ_DBSPL = [52 31 40 67 48 57 43 60 64 54 47 69];
    EQUIVALENT_RECTANGULAR_BANDWIDTHS_100HZ = [0.31 0.35 0.32 1.16 0.47 0.48 0.45 0.57 0.47 0.62 0.25 1.09];

    % Threshold levels / dB SPL and relative equivalent rectangular bandwidths
    % (as proportions of center frequency) at 200 Hz (Peters & Moore, 1992):
    HEARING_THRESHOLD_LEVELS_200HZ_DBSPL = [54 24 28 48 58 48 47 51 48 37 35 67];
    EQUIVALENT_RECTANGULAR_BANDWIDTHS_200HZ = [0.48 0.23 0.22 0.38 0.39 1.63 0.83 0.23 0.36 0.26 0.51 1.51];

    % Threshold levels / dB SPL and relative equivalent rectangular bandwidths
    % (as proportions of center frequency) at 400 Hz (Peters & Moore, 1992):
    HEARING_THRESHOLD_LEVELS_400HZ_DBSPL = [53 24 30 51 49 45 48 54 46 35 43 63];
    EQUIVALENT_RECTANGULAR_BANDWIDTHS_400HZ = [0.54 0.31 0.18 0.48 0.30 0.66 0.41 0.27 0.29 0.22 0.31 1.02];

    % Threshold levels / dB SPL and relative equivalent rectangular bandwidths
    % (as proportions of center frequency) at 500 Hz
    % (Glasberg & Moore, 1986):
    HEARING_THRESHOLD_LEVELS_500HZ_DBSPL = [14 12 26 11 13 73 25];
    EQUIVALENT_RECTANGULAR_BANDWIDTHS_500HZ = [0.21 0.20 0.17 0.18 0.19 0.50 0.28];

    % Threshold levels / dB SPL and relative equivalent rectangular bandwidths
    % (as proportions of center frequency) at 800 Hz (Peters & Moore, 1992):
    HEARING_THRESHOLD_LEVELS_800HZ_DBSPL = [62 27 36 46 58 36 48 62 40 33 51 46];
    EQUIVALENT_RECTANGULAR_BANDWIDTHS_800HZ = [0.59 0.31 0.23 0.28 0.37 0.53 0.24 0.30 0.20 0.19 0.30 0.33];

    % Threshold levels / dB SPL and relative equivalent rectangular bandwidths
    % (as proportions of center frequency) at 2000 Hz (Glasberg & Moore, 1986):
    HEARING_THRESHOLD_LEVELS_2000HZ_DBSPL = [5 9 14 14 29 52 54 46 33 54];
    EQUIVALENT_RECTANGULAR_BANDWIDTHS_2000HZ = [0.17 0.15 0.21 0.18 0.32 0.38 0.53 0.33 0.30 0.70];

    % Maximum smearing factor corresponding to the passive cochlea
    % (Moore & Glasberg, 2004):
    MAXIMUM_SMEARING_FACTOR = 10^(0.01 * 57.6);

    displayRange_hearingThresholdLevel_dBSPL = [-10 80];  % Range of hearing threshold levels / dB SPL to be plotted

    % Compute vector of hearing threshold levels / dB SPL:
    vHearingThresholdLevels_fitted_dBSPL = displayRange_hearingThresholdLevel_dBSPL(1):displayRange_hearingThresholdLevel_dBSPL(end);

    % Plot relative equivalent rectangular bandwidth (i.e., ERB normalized by
    % center frequency) as a function of hearing threshold level / dB SPL:
    figure;
    set(gcf, 'Color', 'w');
    p1 = plot(HEARING_THRESHOLD_LEVELS_1000HZ_DBSPL, EQUIVALENT_RECTANGULAR_BANDWIDTHS_1000HZ, 'o', 'Color', C(1, :), 'MarkerFaceColor', C(1, :), 'MarkerSize', markerSize);
    hold on;
    p_dummy = plot(NaN, 'o', 'Color', 'none');
    p2 = plot(HEARING_THRESHOLD_LEVELS_100HZ_DBSPL, EQUIVALENT_RECTANGULAR_BANDWIDTHS_100HZ, '+', 'Color', C(2, :), 'MarkerFaceColor', C(2, :), 'MarkerSize', markerSize);
    p3 = plot(HEARING_THRESHOLD_LEVELS_200HZ_DBSPL, EQUIVALENT_RECTANGULAR_BANDWIDTHS_200HZ, '+', 'Color', C(3, :), 'MarkerFaceColor', C(3, :), 'MarkerSize', markerSize);
    p4 = plot(HEARING_THRESHOLD_LEVELS_400HZ_DBSPL, EQUIVALENT_RECTANGULAR_BANDWIDTHS_400HZ, '+', 'Color', C(4, :), 'MarkerFaceColor', C(4, :), 'MarkerSize', markerSize);
    p5 = plot(HEARING_THRESHOLD_LEVELS_500HZ_DBSPL, EQUIVALENT_RECTANGULAR_BANDWIDTHS_500HZ, '+', 'Color', C(5, :), 'MarkerFaceColor', C(5, :), 'MarkerSize', markerSize);
    p6 = plot(HEARING_THRESHOLD_LEVELS_800HZ_DBSPL, EQUIVALENT_RECTANGULAR_BANDWIDTHS_800HZ, '+', 'Color', C(6, :), 'MarkerFaceColor', C(6, :), 'MarkerSize', markerSize);
    p7 = plot(HEARING_THRESHOLD_LEVELS_2000HZ_DBSPL, EQUIVALENT_RECTANGULAR_BANDWIDTHS_2000HZ, '+', 'Color', C(7, :), 'MarkerFaceColor', C(7, :), 'MarkerSize', markerSize);
    hold off;
    grid on;
    xlim([vHearingThresholdLevels_fitted_dBSPL(1) vHearingThresholdLevels_fitted_dBSPL(end)]);
    ylim([0.0 1.8]);
    xlabel('Hearing Threshold Level / dB SPL');
    ylabel('ERB / {\itf}_c');
    title({'Relative ERB (Glasberg & Moore, 1986; Peters & Moore, 1992)', ''});
    legend([p1, p_dummy, p2, p3, p4, p5, p6, p7], {'1000 Hz', '', '100 Hz', '200 Hz', '400 Hz', '500 Hz', '800 Hz', '2000 Hz'}, 'Location', 'northwest');
    set(gca, 'FontSize', fontSize, ...
        'LineWidth', lineWidth);
    drawnow;


    %% Smearing factor as a function of hearing threshold level (in dB HL):

    displayRange_hearingThresholdLevel_dBHL = [-10 70];  % Range of hearing threshold levels / dB HL to be plotted

    % Compute vector of hearing threshold levels / dB HL:
    normal_hearing_threshold_level_at_1000Hz_dBSPL = compute_HTL(1000);
    vHearingThresholdLevels_dBHL = HEARING_THRESHOLD_LEVELS_1000HZ_DBSPL - normal_hearing_threshold_level_at_1000Hz_dBSPL;
    vHearingThresholdLevels_fitted_dBHL = displayRange_hearingThresholdLevel_dBHL(1):displayRange_hearingThresholdLevel_dBHL(end);

    % Compute vector of smearing factors (fitted function):
    vSmearingFactor_fitted = NaN(size(vHearingThresholdLevels_fitted_dBHL));
    for k = 1:length(vHearingThresholdLevels_fitted_dBHL)
        [vSmearingFactor_fitted(k), NH_ERB_fitted] = compute_smearing_factor(vHearingThresholdLevels_fitted_dBHL(k));
    end

    % Compute vector of smearing factors (ERB data points at 1000 Hz,
    % normalized by the normal-hearing ERB from the fitted function):
    vSmearingFactor = EQUIVALENT_RECTANGULAR_BANDWIDTHS_1000HZ/compute_smearing_factor(0.0);
    vSmearingFactor = vSmearingFactor/NH_ERB_fitted;

    % Plot smearing factor (i.e., ERB normalized by normal-hearing ERB) as a
    % function of hearing threshold level / dB HL:
    figure;
    set(gcf, 'Color', 'w');
    plot([vHearingThresholdLevels_fitted_dBHL(1) vHearingThresholdLevels_fitted_dBHL(end)], [1.0 1.0], '--', 'Color', C(2, :), 'LineWidth', lineWidth);
    hold on;
    plot([vHearingThresholdLevels_fitted_dBHL(1) vHearingThresholdLevels_fitted_dBHL(end)], [MAXIMUM_SMEARING_FACTOR MAXIMUM_SMEARING_FACTOR], '--', 'Color', C(2, :), 'LineWidth', lineWidth);
    p1 = plot(vHearingThresholdLevels_dBHL, vSmearingFactor, 'o', 'MarkerEdgeColor', C(1, :), 'MarkerFaceColor', C(1, :), 'MarkerSize', markerSize);
    p2 = plot(vHearingThresholdLevels_fitted_dBHL, vSmearingFactor_fitted, 'Color', C(1, :), 'LineWidth', lineWidth);
    hold off;
    text(displayRange_hearingThresholdLevel_dBHL(end), 1.0, ' SF_{NH}', 'Color', C(2, :));
    text(displayRange_hearingThresholdLevel_dBHL(end), MAXIMUM_SMEARING_FACTOR, ' SF_{PC}', 'Color', C(2, :));
    grid on;
    xlim([vHearingThresholdLevels_fitted_dBHL(1) vHearingThresholdLevels_fitted_dBHL(end)]);
    ylim([0 5]);
    xlabel('Hearing Threshold Level / dB HL');
    ylabel('ERB / ERB_{NH}');
    title({'Smearing Factor (Glasberg & Moore, 1986; Moore & Glasberg, 2004)', ''});
    legend([p1, p2], {' 1000 Hz', ' 1000 Hz (Fitted)'}, 'Location', 'northwest');
    set(gca, 'FontSize', fontSize, ...
        'LineWidth', lineWidth);
    drawnow;


    %% Spectral smearing:

    % Specify input signal:
    vSignal_in = sin(2 * pi * fc * linspace(0, duration, srate*duration))';
    vSignal_in = vSignal_in./sqrt(mean(vSignal_in.^2));
    vSignal_in = vSignal_in * p0 * 10^(desiredInputSignalLevel/20);

    % Compute time vector / s:
    vTime = ((0:length(vSignal_in)-1)/srate)';

    % Compute smearing width / Hz (if smearing factors are only provided
    % for one side, use the same values for the other side; if no smearing
    % factors are provided, compute them automatically):
    thresholdLevel = interp1(HTL.f, HTL.(side), vCenterFrequencies(band), 'linear', 'extrap');
    if strcmpi(sSmearingType, 'Custom')
        if isempty(SF.f)
            warndlg('Please specify the frequencies for the smearing factor.', 'SF frequencies missing');
            return;
        end
        if isempty(SF.l) && isempty(SF.r)
            warndlg('Please specify smearing factors manually, or select automatic computation.', 'No smearing factors specified');
            return;
        elseif isempty(SF.l) && ~isempty(SF.r)
            SF.l = SF.r;
        elseif ~isempty(SF.l) && isempty(SF.r)
            SF.r = SF.l;
        end
        if any(SF.l < 1) || any(SF.r < 1)
            warndlg('All smearing factors must be ≥ 1.', 'Invalid SF');
            return;
        end
        smearingFactor = max(interp1(SF.f, SF.(side), vCenterFrequencies(band), 'linear', 'extrap'), 1);
    elseif strcmpi(sSmearingType, 'Automatic')
        smearingFactor = compute_smearing_factor(thresholdLevel);
    end
    smearingWidth_Hz = compute_ERB(fc) * (smearingFactor - 1);

    % Compute multiplicative noise signal:
    if strcmpi(sAudiogramType, 'NH')
        vNoise = ones(length(vTime), 1);
    else
        vNoise = randn(length(vTime), 1);
        vNoise = vNoise/sqrt(mean(vNoise.^2));
        [coeff, norm_phase] = compute_lpfilt_parameters(gammaFilterOrder, smearingWidth_Hz, srate);
        filter = Gfb_Filter_new(coeff, gammaFilterOrder);
        filter.normalization_factor = norm_phase;
        vNoise = Gfb_Filter_process(filter, vNoise);
    end

    % Compute output signal:
    vSignal_out = vSignal_in .* vNoise;

    % Compute spectra of input signal and multiplicative noise:
    [vSignalSpectrum_in, vFreq] = computeSpectrum(vSignal_in, srate, NFFT);
    vNoiseSpectrum = computeSpectrum(vNoise, srate, NFFT);

    % Compute magnitude response of low-pass filter used for computing the
    % multiplicative noise, scaled by the root-mean-square amplitude of the
    % noise:
    impulseLength = length(vSignal_in);
    vImpulse = [1, zeros(1, impulseLength-1)];
    if strcmpi(sAudiogramType, 'NH')
        mImpulseResponse = vImpulse;
    else
        mImpulseResponse = Gfb_Filter_process(filter, vImpulse);
    end
    vFilterMagnitudeResponse = computeSpectrum(mImpulseResponse, srate, NFFT);
    vFilterMagnitudeResponse = vFilterMagnitudeResponse./sqrt(mean(vFilterMagnitudeResponse.^2)) * sqrt(mean(vNoiseSpectrum.^2));

    % Compute spectrum of output signal:
    vSignalSpectrum_out = computeSpectrum(vSignal_out, srate, NFFT);

    % Compute optimal range for displaying the time-domain signals / s
    if isempty(displayRange_time)
        displayRange_time = [0 10^-(round(log10(fc))-2)];
    end

    % Compute jitter of input and output signals:
    jitter_in = computeJitter(vSignal_in, srate, fc);
    jitter_out = computeJitter(vSignal_out, srate, fc);

    % Plot the smearing parameters (in the time and frequency domains):
    figure;
    pos = get(gcf, 'Position');
    pos(1) = pos(1) - pos(3)/2;
    pos(2) = pos(2) - 0.5 * pos(4);
    pos(3) = 2 * pos(3);
    pos(4) = 1.5 * pos(4);
    set(gcf, 'Position', pos);
    set(gcf, 'Color', 'w');
    ax1 = subplot(3, 2, 1);
    plot([vTime(1) vTime(end)], [0 0], 'k', 'LineWidth', lineWidth);
    hold on;
    plot(vTime, vSignal_in, 'Color', C(1, :), 'LineWidth', lineWidth);
    hold off;
    ax3 = subplot(3, 2, 3);
    plot([vTime(1) vTime(end)], [0 0], 'k', 'LineWidth', lineWidth);
    hold on;
    plot(vTime, vNoise, 'Color', C(1, :), 'LineWidth', lineWidth);
    hold off;
    ax5 = subplot(3, 2, 5);
    plot([vTime(1) vTime(end)], [0 0], 'k', 'LineWidth', lineWidth);
    hold on;
    plot(vTime, vSignal_out, 'Color', C(1, :));
    hold off;
    t1 = get(ax1, 'Title');
    t3 = get(ax3, 'Title');
    t5 = get(ax5, 'Title');
    set(t1, 'String', 'Input Signal (Time Domain)');
    set(t3, 'String', 'Noise Factor (Time Domain)');
    set(t5, 'String', 'Output Signal (Time Domain)');
    set(ax1, 'XGrid', 'on');
    set(ax3, 'XGrid', 'on');
    set(ax5, 'XGrid', 'on');
    set(ax1, 'YGrid', 'on');
    set(ax3, 'YGrid', 'on');
    set(ax5, 'YGrid', 'on');
    xl5 = get(ax5, 'XLabel');
    set(xl5, 'String', 'Time / s');
    yl1 = get(ax1, 'YLabel');
    yl3 = get(ax3, 'YLabel');
    yl5 = get(ax5, 'YLabel');
    set(yl1, 'String', 'Amplitude / Pa');
    set(yl3, 'String', 'Amplitude');
    set(yl5, 'String', 'Amplitude / Pa');
    set(ax1, 'XLim', displayRange_time);
    set(ax3, 'XLim', displayRange_time);
    set(ax5, 'XLim', displayRange_time);
    maxYLim_signal = max(max(abs([get(ax1, 'YLim') get(ax5, 'YLim')])));
    maxYLim_noise = max(max(abs(get(ax3, 'YLim'))));
    set(ax1, 'YLim', [-maxYLim_signal maxYLim_signal]);
    set(ax3, 'YLim', [-maxYLim_noise maxYLim_noise]);
    set(ax5, 'YLim', [-maxYLim_signal maxYLim_signal]);
    ylpos1 = get(yl1, 'Position');
    ylpos3 = get(yl3, 'Position');
    ylpos5 = get(yl5, 'Position');
    minYLabelXPos_time = min([ylpos1(1), ylpos3(1), ylpos5(1)]);
    ylpos1(1) = minYLabelXPos_time;
    ylpos3(1) = minYLabelXPos_time;
    ylpos5(1) = minYLabelXPos_time;
    set(yl1, 'Position', ylpos1);
    set(yl3, 'Position', ylpos3);
    set(yl5, 'Position', ylpos5);
    xlim1 = get(ax1, 'XLim');
    ylim1 = get(ax1, 'YLim');
    xlim5 = get(ax5, 'XLim');
    ylim5 = get(ax5, 'YLim');
    text(ax1, xlim1(1)+0.025*diff(xlim1), ylim1(end)-0.025*diff(ylim1), ...
        ['Jitter: ' sprintf('%.1f', jitter_in*100) ' %'], ...
        'FontSize', fontSize, 'VerticalAlignment', 'top');
    text(ax5, xlim5(1)+0.025*diff(xlim5), ylim5(end)-0.025*diff(ylim5), ...
        ['Jitter: ' sprintf('%.1f', jitter_out*100) ' %'], ...
        'FontSize', fontSize, 'VerticalAlignment', 'top');
    if isempty(displayRange_frequency_smearing)
        orderOfMagnitudefactor = 1;
        multipleFactor = 2;
        orderOfMagnitude = floor(log10(fc));
        base = orderOfMagnitudefactor * 10^orderOfMagnitude;
        multiple = multipleFactor * ceil(fc/(multipleFactor * base) + eps(fc));
        maxDisplayRange_frequency = multiple * base;
        displayRange_frequency_smearing = [0 maxDisplayRange_frequency];
    end
    ax2 = subplot(3, 2, 2);
    p1 = plot([fc fc], [0 1], 'k--', 'LineWidth', lineWidth);
    hold on;
    plot(vFreq, 20*log10(vSignalSpectrum_in/p0), 'Color', C(1, :), 'LineWidth', lineWidth);
    hold off;
    if fc <= displayRange_frequency_smearing(2)/2
        sLocation = 'northeast';
    else
        sLocation = 'northwest';
    end
    legend(p1, ' Center Frequency', 'Location', sLocation);
    ax4 = subplot(3, 2, 4);
    xdata = [0 smearingWidth_Hz/2 smearingWidth_Hz/2 0];
    ydata = [0 0 1 1];
    zdata = zeros(size(xdata));
    rgb   = C(2, :);
    p2 = patch('XData', xdata, ...
        'YData', ydata, ...
        'ZData', zdata, ...
        'FaceColor', rgb, ...
        'EdgeColor', 'none', ...
        'FaceAlpha', 0.25);
    hold on;
    if strcmpi(sAudiogramType, 'NH')
        stem(vFreq, 20*log10(vNoiseSpectrum), 'LineWidth', 2*lineWidth, 'MarkerFaceColor', C(1, :), 'MarkerSize', markerSize);
    else
        plot(vFreq, 20*log10(vNoiseSpectrum), 'LineWidth', lineWidth);
    end
    p3 = plot(vFreq, 20*log10(vFilterMagnitudeResponse), 'LineWidth', 2*lineWidth);
    legend([p3 p2], {' Filter Gain (Scaled)', ' ½ Filter Bandwidth'}, 'Location', 'northeast');
    hold off;
    box on;
    ax6 = subplot(3, 2, 6);
    xdata = [fc-smearingWidth_Hz/2 fc+smearingWidth_Hz/2 fc+smearingWidth_Hz/2 fc-smearingWidth_Hz/2];
    ydata = [0 0 1 1];
    zdata = zeros(size(xdata));
    rgb   = [192 192 192]/255;
    p4 = patch('XData', xdata, ...
        'YData', ydata, ...
        'ZData', zdata, ...
        'FaceColor', rgb, ...
        'EdgeColor', 'none', ...
        'FaceAlpha', 0.5);
    hold on;
    p5 = plot([fc fc], [0 1], 'k--', 'LineWidth', lineWidth);
    plot(vFreq, 20*log10(vSignalSpectrum_out/p0), 'Color', C(1, :), 'LineWidth', lineWidth);
    legend([p5 p4], {' Center Frequency', ' Smearing Width'}, 'Location', sLocation);
    hold off;
    box on;
    t2 = get(ax2, 'Title');
    t4 = get(ax4, 'Title');
    t6 = get(ax6, 'Title');
    set(t2, 'String', 'Input Signal (Frequency Domain)');
    set(t4, 'String', 'Noise Factor (Frequency Domain)');
    set(t6, 'String', 'Output Signal (Frequency Domain)');
    set(ax2, 'XGrid', 'on');
    set(ax4, 'XGrid', 'on');
    set(ax6, 'XGrid', 'on');
    set(ax2, 'YGrid', 'on');
    set(ax4, 'YGrid', 'on');
    set(ax5, 'YGrid', 'on');
    xl6 = get(ax6, 'XLabel');
    set(xl6, 'String', 'Frequency / Hz');
    yl2 = get(ax2, 'YLabel');
    yl4 = get(ax4, 'YLabel');
    yl6 = get(ax6, 'YLabel');
    set(yl2, 'String', 'Magnitude / dB SPL');
    set(yl4, 'String', 'Magnitude / dB');
    set(yl6, 'String', 'Magnitude / dB SPL');
    set(ax2, 'XLim', displayRange_frequency_smearing);
    set(ax4, 'XLim', displayRange_frequency_smearing);
    set(ax6, 'XLim', displayRange_frequency_smearing);
    minYLim_signal = min(min([get(ax2, 'YLim') get(ax6, 'YLim')]));
    maxYLim_signal = max(max([get(ax2, 'YLim') get(ax6, 'YLim')]));
    minYLim_noise = min(min(get(ax4, 'YLim')));
    maxYLim_noise = max(max(get(ax4, 'YLim')));
    set(ax2, 'YLim', [minYLim_signal maxYLim_signal]);
    set(ax4, 'YLim', [minYLim_noise maxYLim_noise]);
    set(ax6, 'YLim', [minYLim_signal maxYLim_signal]);
    ylpos2 = get(yl2, 'Position');
    ylpos4 = get(yl4, 'Position');
    ylpos6 = get(yl6, 'Position');
    minYLabelXPos_frequency = min([ylpos2(1), ylpos4(1), ylpos6(1)]);
    ylpos2(1) = minYLabelXPos_frequency;
    ylpos4(1) = minYLabelXPos_frequency;
    ylpos6(1) = minYLabelXPos_frequency;
    set(yl2, 'Position', ylpos2);
    set(yl4, 'Position', ylpos4);
    set(yl6, 'Position', ylpos6);
    set(p1, 'YData', [minYLim_signal maxYLim_signal]);
    set(p2, 'YData', [minYLim_noise minYLim_noise maxYLim_noise maxYLim_noise]);
    set(p4, 'YData', [minYLim_signal minYLim_signal maxYLim_signal maxYLim_signal]);
    set(p5, 'YData', [minYLim_signal maxYLim_signal]);
    if ~exist('OCTAVE_VERSION', 'builtin')
        p = get(gcf, 'Position');
        ax = findall(gcf, 'Type', 'axes');
        for k = 1:numel(ax)
            pos = get(ax(k), 'Position');
            pos(4) = pos(4) - fontSize*2/p(4);
            set(ax(k), 'Position', pos);
        end
    end
    axes('Position', [0 0 1 1], 'Visible', 'off');
    text(0.5, 0.995, ['Spectral Smearing (' sSide ' Ear, Center Frequency: ' num2str(round(fc)) ' Hz, Hearing Threshold Level: ' num2str(round(thresholdLevel)) ' dB HL, Smearing Factor: ' num2str(round_dec(smearingFactor, 1)) ')'], 'FontSize', fontSize*1.3, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'top');
    set(ax1, 'FontSize', fontSize);
    set(ax2, 'FontSize', fontSize);
    set(ax3, 'FontSize', fontSize);
    set(ax4, 'FontSize', fontSize);
    set(ax5, 'FontSize', fontSize);
    set(ax6, 'FontSize', fontSize);
    set(ax1, 'LineWidth', lineWidth);
    set(ax2, 'LineWidth', lineWidth);
    set(ax3, 'LineWidth', lineWidth);
    set(ax4, 'LineWidth', lineWidth);
    set(ax5, 'LineWidth', lineWidth);
    set(ax6, 'LineWidth', lineWidth);
    drawnow;

    % Sort the figures:
    h = findall(0, 'Type', 'Figure');
    for k = length(h):-1:1
        figure(k);
    end


    % Turn warnings for obsolete functions back on in Octave:
    if exist('OCTAVE_VERSION', 'builtin')
        warning('on', 'Octave:legacy-function');
    end

    % Reset default figure position:
    set(0, 'DefaultFigurePosition', defaultFigPos);
end


%% Functions:

function HTL = compute_HTL(center_frequency_hz)
    % Function for computing the normal hearing threshold level for a given
    % auditory filter center frequency
    %
    % Usage:
    % HTL = compute_HTL(center_frequency_hz)
    %
    % Input parameter:
    % center_frequency_hz  center frequency / Hz
    %
    % Output parameter:
    % ERB                  hearing threshold level / dB SPL
    %
    % ---------------------------------------------------------------------

    % Third-octave frequencies / Hz and normal-hearing threshold
    % levels / dB SPL (ISO 226:2023):
    THIRD_OCTAVE_FREQUENCIES = [20 25 31.5 40 50 63 80 100 125 160 200 250 315 400 500 630 800 1000 1250 1600 2000 2500 3150 4000 5000 6300 8000 10000 12500];
    NORMAL_HEARING_THRESHOLD_LEVELS = [78.1 68.7 59.5 51.1 44.0 37.5 31.5 26.5 22.1 17.9 14.4 11.4 8.6 6.2 4.4 3.0 2.2 2.4 3.5 1.7 -1.3 -4.2 -6.0 -5.4 -1.5 6.0 12.6 13.9 12.3];

    % Compute the hearing threshold level / dB SPL
    HTL = interp1(THIRD_OCTAVE_FREQUENCIES, NORMAL_HEARING_THRESHOLD_LEVELS, center_frequency_hz, 'spline', 'extrap');
end

function [MCL, UCL] = compute_MCL_and_UCL(hearingThresholdLevel_dBHL)
    % Function for computing the most comfortable uncomfortable loudness
    % levels for a given (frequency-specific) hearing threshold level
    %
    % Usage:
    % [MCL, UCL] = compute_MCL_and_UCL(hearingThresholdLevel_dBHL)
    %
    % Input parameter:
    % hearingThresholdLevel_dBHL  hearing threshold level / dB HL
    %
    % Output parameters:
    % MCL                         most comfortable loudness level / dB HL
    % UCL                         uncomfortable loudness level / dB HL
    %
    % ---------------------------------------------------------------------

    n = 3;  % degree of polynomial fit

    % Impaired hearing threshold levels, most comfortable loudness levels,
    % and uncomfortable loudness levels / dB HL (Pascoe, 1988):
    HEARING_THRESHOLD_LEVELS = 0:5:120;
    MOST_COMFORTABLE_LOUDNESS_LEVELS = [60.25 63.02 63.75 65.55 64.93 68.85 71.64 71.77 75.16 78.50 82.30 83.66 87.27 92.37 95.53 98.85 102.55 104.38 109.50 117.50 117.34 124.06 126.43 129.17 132.50];
    UNCOMFORTABLE_LOUDNESS_LEVELS = [97.25 98.97 99.46 98.08 96.58 101.26 101.76 101.05 102.63 105.37 107.45 107.69 109.97 113.75 114.84 116.76 119.82 120.00 123.67 129.55 126.56 133.13 134.29 136.67 140.00];

    % Compute the MCL:
    p_MCL = polyfit(HEARING_THRESHOLD_LEVELS, MOST_COMFORTABLE_LOUDNESS_LEVELS, n);
    mostComfortableLoudnessLevels_fitted = polyval(p_MCL, HEARING_THRESHOLD_LEVELS);
    MCL = interp1(HEARING_THRESHOLD_LEVELS, mostComfortableLoudnessLevels_fitted, hearingThresholdLevel_dBHL, 'spline');

    % Compute the UCL:
    p_UCL = polyfit(HEARING_THRESHOLD_LEVELS, UNCOMFORTABLE_LOUDNESS_LEVELS, n);
    uncomfortableLoudnessLevels_fitted = polyval(p_UCL, HEARING_THRESHOLD_LEVELS);
    UCL = interp1(HEARING_THRESHOLD_LEVELS, uncomfortableLoudnessLevels_fitted, hearingThresholdLevel_dBHL, 'spline');
end

function [vFreq, vAudiogram] = compute_standard_audiogram(sAudiogramType)
    % Function for computing a standard audiogram (air-conduction hearing
    % thresholds) for a given audiogram type according to
    % Bisgaard et al. (2010)
    %
    % Usage:
    % [vFreq, vAudiogram] = compute_standard_audiogram(sAudiogramType)
    %
    % Input parameter:
    % sAudiogramType  standard audiogram type: any of
    %                {'NH', 'N1', 'N2', 'N3', 'N4', 'N5', 'N6', 'N7', 'S1', 'S2', 'S3'}
    %
    % Output parameters:
    % vFreq          standard audiogram frequencies / Hz
    % vAudiogram     standard audiogram / dB HL
    %
    % ---------------------------------------------------------------------

    % Standard audiogram frequencies and types (Bisgaard et al., 2010):
    STANDARD_AUDIOGRAM_FREQUENCIES = [250 500 750 1000 1500 2000 3000 4000 6000];  % audiogram frequencies / Hz
    STANDARD_AUDIOGRAM_TYPE_NH = [0 0 0 0 0 0 0 0 0];  % normal-hearing / dB HL
    STANDARD_AUDIOGRAM_TYPE_N1 = [10 10 10 10 10 15 20 30 40];  % flat and moderately sloping, very mild / dB HL
    STANDARD_AUDIOGRAM_TYPE_N2 = [20 20 22.5 25 30 35 40 45 50];  % flat and moderately sloping, mild / dB HL
    STANDARD_AUDIOGRAM_TYPE_N3 = [35 35 35 40 45 50 55 60 65];  % flat and moderately sloping, moderate / dB HL
    STANDARD_AUDIOGRAM_TYPE_N4 = [55 55 55 55 60 65 70 75 80];  % flat and moderately sloping, moderate/severe / dB HL
    STANDARD_AUDIOGRAM_TYPE_N5 = [65 70 72.5 75 80 80 80 80 80];  % flat and moderately sloping, severe / dB HL
    STANDARD_AUDIOGRAM_TYPE_N6 = [75 80 82.5 85 90 90 95 100 100];  % flat and moderately sloping, severe / dB HL
    STANDARD_AUDIOGRAM_TYPE_N7 = [90 95 100 105 105 105 105 105 105];  % flat and moderately sloping, profound / dB HL
    STANDARD_AUDIOGRAM_TYPE_S1 = [10 10 10 10 10 15 30 55 70];  % steep sloping, very mild / dB HL
    STANDARD_AUDIOGRAM_TYPE_S2 = [20 20 22.5 25 35 55 75 95 95];  % steep sloping, mild / dB HL
    STANDARD_AUDIOGRAM_TYPE_S3 = [30 35 47.5 60 70 75 80 80 85];  % steep sloping, moderate/severe / dB HL

    % Set the frequency and audiogram (HTL) vectors:
    vFreq = STANDARD_AUDIOGRAM_FREQUENCIES;
    switch sAudiogramType
        case {'NH', 'nh'}
            vAudiogram = STANDARD_AUDIOGRAM_TYPE_NH;
        case {'N1', 'n1'}
            vAudiogram = STANDARD_AUDIOGRAM_TYPE_N1;
        case {'N2', 'n2'}
            vAudiogram = STANDARD_AUDIOGRAM_TYPE_N2;
        case {'N3', 'n3'}
            vAudiogram = STANDARD_AUDIOGRAM_TYPE_N3;
        case {'N4', 'n4'}
            vAudiogram = STANDARD_AUDIOGRAM_TYPE_N4;
        case {'N5', 'n5'}
            vAudiogram = STANDARD_AUDIOGRAM_TYPE_N5;
        case {'N6', 'n6'}
            vAudiogram = STANDARD_AUDIOGRAM_TYPE_N6;
        case {'N7', 'n7'}
            vAudiogram = STANDARD_AUDIOGRAM_TYPE_N7;
        case {'S1', 's1'}
            vAudiogram = STANDARD_AUDIOGRAM_TYPE_S1;
        case {'S2', 's2'}
            vAudiogram = STANDARD_AUDIOGRAM_TYPE_S2;
        case {'S3', 's3'}
            vAudiogram = STANDARD_AUDIOGRAM_TYPE_S3;
    end
end

function [smearingFactor, NH_ERB_fitted] = compute_smearing_factor(hearingThresholdLevel_dBHL)
    % Function for computing the smearing factor (auditory filter bandwidth
    % broadening factor) for a given (frequency-specific) hearing threshold
    % level
    %
    % Usage:
    % [smearingFactor, NH_ERB] = compute_smearing_factor(hearingThresholdLevel_dBHL)
    %
    % Input parameter:
    % hearingThresholdLevel_dBHL  hearing threshold level / dB HL
    %
    % Output parameters:
    % smearingFactor              smearing factor
    % NH_ERB                      fitted normal-hearing equivalent
    %                             rectangular bandwidth (relative to the
    %                             respective center frequency)
    %
    % ---------------------------------------------------------------------

    n = 3;  % degree of polynomial fit

    % Threshold levels / dB SPL and relative equivalent rectangular
    % bandwidths (as proportions of center frequency) at 1000 Hz
    % (Glasberg & Moore, 1986):
    HEARING_THRESHOLD_LEVELS_1000HZ_DBSPL = [-1 3 6 19 13 10 49 61 41 49 37 52 52 15 39 51 48 25 31 56 58 50 55];
    EQUIVALENT_RECTANGULAR_BANDWIDTHS_1000HZ = [0.15 0.14 0.15 0.14 0.17 0.17 0.37 0.60 0.31 0.46 0.18 0.40 0.55 0.17 0.37 0.26 0.21 0.16 0.16 0.33 0.38 0.25 0.40];

    % Maximum smearing factor corresponding to the passive cochlea
    % (Moore & Glasberg, 2004):
    MAXIMUM_SMEARING_FACTOR = 10^(0.01 * 57.6);

    % Convert hearing threshold levels from dB SPL to dB HL:
    vThresholdLevels_dBHL = HEARING_THRESHOLD_LEVELS_1000HZ_DBSPL - compute_HTL(1000);

    % Fit third-degree polynomial function to the data:
    p = polyfit(vThresholdLevels_dBHL, EQUIVALENT_RECTANGULAR_BANDWIDTHS_1000HZ, n);
    ERB_fitted = polyval(p, hearingThresholdLevel_dBHL);

    % Normalize the fitted equivalent rectangular bandwidth to the
    % normal-hearing value:
    NH_ERB_fitted = polyval(p, 0.0);
    ERB_fitted = ERB_fitted/NH_ERB_fitted;

    % Set the lower bound of the function to the normal-hearing value and
    % the upper bound to the value corresponding to a passive cochlea:
    if hearingThresholdLevel_dBHL < 0.0
        smearingFactor = 1.0;
    elseif ERB_fitted > MAXIMUM_SMEARING_FACTOR
        smearingFactor = MAXIMUM_SMEARING_FACTOR;
    else
        smearingFactor = ERB_fitted;
    end
end

function ERB = compute_ERB(center_frequency_hz)
    % Function for computing the normal-hearing equivalent rectangular
    % bandwidth of an auditory filter with a given center frequency
    %
    % Usage:
    % ERB = compute_ERB(center_frequency_hz)
    %
    % Input parameter:
    % center_frequency_hz  center frequency / Hz
    %
    % Output parameter:
    % ERB                  equivalent rectangular bandwidth / Hz
    %
    % ---------------------------------------------------------------------

    % Constants for calculating the equivalent rectangular bandwidth
    % (Equation 13 in Hohmann, 2002, derived from Equation 3 in
    % Glasberg & Moore, 1990, with L = 24.7 and Q = 1000/(24.7 * 4.37)):
    L = 24.7;
    Q = 9.265;

    % Compute the equivalent rectangular bandwidth / Hz:
    ERB = L + center_frequency_hz/Q;
end

function [coeff, norm_phase] = compute_lpfilt_parameters(gammaOrder, smearingWidth, samplingFrequency)
    % Function for computing the filter coefficient as well as the combined
    % normalization and phase correction factor for a gammatone lowpass
    % filter
    %
    % Usage:
    % [coeff, norm_phase] = compute_lpfilt_parameters(gamma_order, smearing_width, sampling_frequency_hz)
    %
    % Input parameters:
    % gamma_order            gammatone filter order
    % smearing_width         smearing width / Hz
    % sampling_frequency_hz  sampling frequency / Hz
    %
    % Output parameters:
    % coeff                  filter coefficient
    % norm_phase             combined normalization and phase correction
    %                        factor
    %
    % ---------------------------------------------------------------------

    % Compute the filter coefficient:
    a_gamma = pi * factorial(2 * gammaOrder - 2) * 2^-(2 * gammaOrder - 2)/factorial(gammaOrder - 1)^2;
    b = smearingWidth/a_gamma;
    coeff = exp(-2 * pi * b/samplingFrequency);

    % Compute the combined normalization and phase correction factor:
    phi = 0:1e-5:2*pi;
    norm_phase = 1/sqrt(sum(abs(1./(1 - coeff * exp(1i * phi)).^4).^2)/length(phi));
end

function [vSpectrum, vFreq] = computeSpectrum(vSignal, srate, NFFT)
    % Function for calculating the one-sided magnitude spectrum of a given
    % input signal
    %
    % Usage:
    % [vSpectrum, vFreq] = computeSpectrum(vSignal, srate, NFFT)
    % [vSpectrum, vFreq] = computeSpectrum(vSignal, srate)
    %
    % Input parameters:
    % vSignal    input signal
    % srate      sampling frequency / Hz
    % NFFT       FFT length / samples (optional; must be even; default:
    %            length of the input signal / samples)
    %
    % Output parameters:
    % vSpectrum  one-sided magnitude spectrum
    % vFreq      frequency vector
    %
    % ---------------------------------------------------------------------

    % Set the FFT length to its default value if it is not specified,
    % rounding it down to the nearest even number:
    if nargin == 2 || isempty(NFFT)
        NFFT = 2 * floor(length(vSignal)/2);
    end

    % Check whether the FFT length is even:
    if mod(NFFT, 2) ~= 0
       error('FFT length must be even.');
    end

    % Make sure that the input signal is a column vector:
    vSignal = vSignal(:);

    % Compute the complex spectrum of the input signal:
    vSpectrum = fft(vSignal, NFFT);

    % Compute the magnitude spectrum of the input signal:
    vSpectrum = abs(vSpectrum);

    % Normalize the magnitude spectrum by the FFT length to ensure that the
    % spectrum has thesame energy as the input signal:
    vSpectrum = vSpectrum/sqrt(NFFT);

    % Discard the negative-frequency bins:
    vSpectrum = vSpectrum(1:NFFT/2+1);

    % Create the frequency vector:
    vFreq = linspace(0, srate/2, length(vSpectrum))';
end

function jitter = computeJitter(vSignal, srate, f0)
    % Function for computing the jitter of a periodic signal, defined as
    % the average absolute difference between consecutive periods, divided
    % by the average period
    %
    % Usage:
    % jitter = computeJitter(vSignal)
    %
    % Input parameter:
    % vSignal  input signal
    %
    % Output parameter:
    % jitter   jitter of the input signal
    %
    % ---------------------------------------------------------------------

    % Calculate the slope of the input signal by forming the first
    % derivative, and set the resulting values to a constant positive or
    % negative value:
    vSlope = diff(vSignal);
    vSlope(vSlope>=0) = 0.5;
    vSlope(vSlope<0) = -0.5;

    % The local maxima of the input signal are at the points where the
    % slope changes sign abruptly:
    vLocalMaxima = find(diff(vSlope)==1);

    % Calculate the period duration (in number of samples) between two
    % adjacent local maxima:
    vPeriods = diff(vLocalMaxima);

    % Calculate the (theoretical) reference period duration (in number of
    % samples):
    referencePeriod = srate/f0;

    % Calculate jitter as the average absolute difference between the
    % period durations and the reference period duration, divided by the
    % reference period duration:
    jitter = mean(abs(vPeriods - referencePeriod))/referencePeriod;
end

function y = round_dec(x, n)
    % Function for rounding a number to the specified number of decimals
    % (needed for compatibility with Octave)
    %
    % Usage:
    % y = round_dec(x, n)
    %
    % Input parameters:
    % x  input number (scalar or vector)
    % n  number of decimals (must be a positive integer)
    %
    % Output parameter:
    % y  output number (rounded)
    %
    % ---------------------------------------------------------------------

    % Ensure that n is a positive integer:
    if (n <= 0) || (mod(n, 1) ~= 0)
        error('n must be a positive integer.');
    end

    factor = 10.^n;
    y = round(x .* factor)./factor;
end


%% References:

% Bisgaard, N., Vlaming, M. S. M. G., & Dahlquist, M. (2010). Standard
%   audiograms for the IEC 60118-15 measurement procedure. Trends in
%   Amplification, 14(2), 113-120. https://doi.org/10.1177/1084713810379609
%
% Glasberg, B. R., & Moore, B. C. (1986). Auditory filter shapes in
%   subjects with unilateral and bilateral cochlear impairments. The
%   Journal of the Acoustical Society of America, 79(4), 1020-1033.
%   https://doi.org/10.1121/1.393374
%
% Glasberg, B. R., & Moore, B. C. (1990). Derivation of auditory filter
%   shapes from notched-noise data. Hearing research, 47(1-2), 103-138.
%   https://doi.org/10.1016/0378-5955(90)90170-t
%
% Hohmann, V. (2002). Frequency analysis and synthesis using a Gammatone
%   filterbank. Acta Acustica united with Acustica, 88(3), 433-442.
%
% Hohmann, V., & Herzke, T. (2007). Software for "Frequency analysis and
%   synthesis using a Gammatone filterbank" (Version 1.1). Zenodo.
%   https://doi.org/10.5281/zenodo.2643400
%
% International Organization for Standardization (ISO). (2023). Acoustics -
%   Normal equal-loudness-level contours (ISO 226:2023).
%   https://www.iso.org/standard/83117.html
%
% Moore, B. C., & Glasberg, B. R. (2004). A revised model of loudness
%   perception applied to cochlear hearing loss. Hearing research,
%   188(1-2), 70-88. https://doi.org/10.1016/S0378-5955(03)00347-2
%
% Pascoe, D. P. (1988). Clinical measurements of the auditory dynamic range
%   and their relation to formulas for hearing aid gain. In J. Hartvig
%   Jensen (Ed.), Hearing aid fitting: Theoretical and practical views.
%   Proceedings of the 13th Danavox Symposium (pp. 129-152). Danavox
%   Jubilee Foundation.
%
% Peters, R. W., & Moore, B. C. (1992). Auditory filter shapes at low
%   center frequencies in young and elderly hearing-impaired subjects. The
%   Journal of the Acoustical Society of America, 91(1), 256-266.
%   https://doi.org/10.1121/1.402769
