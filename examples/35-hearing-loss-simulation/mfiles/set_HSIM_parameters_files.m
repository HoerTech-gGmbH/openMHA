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


function set_HSIM_parameters_files()
    % Function for computing, setting (changing), and optionally plotting
    % the parameters of a currently running openMHA hearing impairment
    % simulator (offline processing of a stereo input sound file)
    %
    % This function assumes that there is a running openMHA process with
    % the correct configuration (as, e.g., achieved by invoking
    % "?read:cfg/HSIM_files.cfg" in interactive mode).
    %
    % Note that the folder "mfiles" from the openMHA installation must be
    % on the Matlab/Octave search path. Furthermore, this function depends
    % on the "gammatone" toolbox by Hohmann and Herzke (2007), which must
    % be downloaded and then added to the Matlab/Octave search path.
    %
    % NB: When using Octave, figures may have to be closed *manually*
    % (e.g., by issuing "close all" in the Command Window) before
    % re-running this function for the figures to be rendered correctly on
    % subsequent runs.
    %
    % Usage:
    % set_HSIM_parameters_files()

    clear;
    close all hidden;
    clc;

    % -------------------------------------------------------------------------

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

    % Handle to the currently running openMHA instance:
    handle.host = 'localhost';
    handle.port = 33337;


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


    % Variables for turning dynamic range expansion (i.e., attenuation) and
    % spectral smearing (i.e., distortion) on and off independently:
    simulate_expansion = true;
    simulate_smearing = true;


    % Variables for gammatone filter bank:
    srate = 48000;  % sampling frequency / Hz
    lowerCutoffFrequency = 70.0;  % lowest possible gammatone filterbank center frequency / Hz
    specifiedCenterFrequency = 1000.0;  % gammatone filterbank "base frequency" / Hz
    upperCutoffFrequency = 8500.0;  % highest possible gammatone filterbank center frequency / Hz
    filtersPerERBaud = 1.0;  % number of gammatone filters per ERB
    gammaFilterOrder = 4;  % gammatone filterbank filter order
    bandwidthFactor = 1.0;  % gammatone filterbank bandwidth widening factor
    groupDelay = 0.008;  % gammatone filterbank group delay / s
    displayRange_magnitude = [-40 0];  % display range for magnitude / dB


    % Variable for reducing the numeric precision of the values sent to the
    % openMHA to guarantee conformity with C++ single-precision
    % floating-point format ("float", or "mha_real_t" in openMHA):
    precision = 6;


    % Variables for dynamic range compression:
    attackTime = 0.02;  % attack time constant / s
    releaseTime = 0.1;  % release time constant / s


    % Smearing factors (only used if sSmearingType is set to 'Custom'
    % and values for at least one side and the corresponding frequencies
    % are specified):
    sSmearingType = 'Automatic';  % {'Automatic', 'Custom'}
    SF.f = [125 250 500 750 1000 1500 2000 3000 4000 6000 8000];  % frequencies / Hz
    SF.l = [];  % smearing factors (left side) / dB HL
    SF.r = [];  % smearing factors (right side) / dB HL


    % Variables for plotting:
    plot_parameters = false;  % turn plotting on and off (e.g., for debugging)
    fontSize = 10;  % font size
    lineWidth = 0.75;  % line width
    markerSize = 4;  % marker size


    %% Compute parameters:

    % Compute the gain (in dB) necessary to set the input signal to the desired
    % broadband level:
    filename = mha_get(handle, 'io.in');
    vSignal = audioread(['../release/' filename]);
    gain = compute_input_signal_gain(vSignal, desiredInputSignalLevel);


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


    % Compute gammatone filterbank analyzer and synthesizer structs:
    analyzer = Gfb_Analyzer_new(srate, lowerCutoffFrequency, specifiedCenterFrequency, upperCutoffFrequency, filtersPerERBaud, gammaFilterOrder, bandwidthFactor);
    synthesizer = Gfb_Synthesizer_new(analyzer, groupDelay);

    % Compute gammatone filterbank center frequencies / Hz:
    vCenterFrequencies = analyzer.center_frequencies_hz;

    % Compute gammatone filterbank filter coefficients:
    gtfb_coeff = NaN(1, length(vCenterFrequencies));
    for k = 1:length(vCenterFrequencies)
        gtfb_coeff(k) = analyzer.filters(k).coefficient;
        gtfb_coeff(k) = round_sig(gtfb_coeff(k), precision);
    end

    % Compute gammatone filterbank combined normalization and phase correction
    % factors:
    gtfb_norm_phase = NaN(1, length(vCenterFrequencies));
    for k = 1:length(vCenterFrequencies)
        gtfb_norm_phase(k) = analyzer.filters(k).normalization_factor * synthesizer.delay.phase_factors(k) * sqrt(2);  % sqrt(2) accounts for discarding the imaginary parts
        gtfb_norm_phase(k) = round_sig(gtfb_norm_phase(k), precision);
    end

    % Compute complex (i.e., real and imaginary) gammatone filterbank
    % band-specific delays / samples:
    nComplexNumberParts = 2;
    nChannels = 2;
    gtfb_delay = NaN(1, length(vCenterFrequencies)*nComplexNumberParts*nChannels);
    for k = 1:length(vCenterFrequencies)
        gtfb_delay(2*k-1) = synthesizer.delay.delays_samples(k);
        gtfb_delay(2*k) = synthesizer.delay.delays_samples(k);
        gtfb_delay(2*length(vCenterFrequencies)+1:end) = gtfb_delay(1:2*length(vCenterFrequencies));
    end

    % Compute gammatone filterbank band-specific recombination gains / dB:
    gtfb_gains = NaN(1, length(vCenterFrequencies)*nChannels);
    for k = 1:length(vCenterFrequencies)
        gtfb_gains(k) = 20 * log10(synthesizer.mixer.gains(k));
        gtfb_gains(length(vCenterFrequencies)+k) = 20 * log10(synthesizer.mixer.gains(k));
    end
    gtfb_gains = round_sig(gtfb_gains, precision);


    % Compute audiogram parameters:
    idx = 0;
    vHTL_NH = NaN(1, length(vCenterFrequencies)*nChannels);
    vMCL_NH = NaN(1, length(vCenterFrequencies)*nChannels);
    vUCL_NH = NaN(1, length(vCenterFrequencies)*nChannels);
    vHTL_HI = NaN(1, length(vCenterFrequencies)*nChannels);
    vMCL_HI = NaN(1, length(vCenterFrequencies)*nChannels);
    vUCL_HI = NaN(1, length(vCenterFrequencies)*nChannels);
    for side = 'lr'
        for band = 1:length(vCenterFrequencies)
            idx = idx + 1;

            % Normal_hearing auditory dynamic range / dB SPL:
            normalHearingThresholdLevel_dBSPL = compute_HTL(vCenterFrequencies(band));
            normalHearingThresholdLevel_dBHL = 0.0;
            [normalMostComfortableLoudnessLevel_dBHL, normalUncomfortableLoudnessLevel_dBHL] = compute_MCL_and_UCL(normalHearingThresholdLevel_dBHL);
            vHTL_NH(idx) = normalHearingThresholdLevel_dBSPL + normalHearingThresholdLevel_dBHL;
            vMCL_NH(idx) = normalHearingThresholdLevel_dBSPL + normalMostComfortableLoudnessLevel_dBHL;
            vUCL_NH(idx) = normalHearingThresholdLevel_dBSPL + normalUncomfortableLoudnessLevel_dBHL;

            % Impaired auditory dynamic range / dB SPL (for specified audiogram
            % and frequency band):
            impairedHearingThresholdLevel_dBHL = interp1(log(HTL.f), HTL.(side), log(vCenterFrequencies(band)), 'linear', 'extrap');
            impairedMostComfortableLoudnessLevel_dBHL = interp1(log(MCL.f), MCL.(side), log(vCenterFrequencies(band)), 'linear', 'extrap');
            impairedUncomfortableLoudnessLevel_dBHL = interp1(log(UCL.f), UCL.(side), log(vCenterFrequencies(band)), 'linear', 'extrap');
            vHTL_HI(idx) = normalHearingThresholdLevel_dBSPL + impairedHearingThresholdLevel_dBHL;
            vMCL_HI(idx) = normalHearingThresholdLevel_dBSPL + impairedMostComfortableLoudnessLevel_dBHL;
            vUCL_HI(idx) = normalHearingThresholdLevel_dBSPL + impairedUncomfortableLoudnessLevel_dBHL;
        end
    end
    minimumLevel = min(vHTL_NH);
    maximumLevel = max(vUCL_HI);

    % Compute gains for dynamic range expansion / dB:
    gtmin = floor(minimumLevel);  % minimum input level for dynamic range expansion / dB SPL
    gtstep = 1.0;  % band-specific step size between input levels / dB
    gtmax = ceil(maximumLevel);  % maximum input level for dynamic range expansion / dB SPL
    if simulate_expansion == true
        idx = 0;
        gtdata = NaN(length(vCenterFrequencies)*nChannels, length(gtmin:gtstep:gtmax));
        for side = 'lr'
            for band = 1:length(vCenterFrequencies)
                idx = idx + 1;

                % Slopes of the two linear pieces of the input-output characteristic:
                slope1 = (vMCL_NH(idx) - vHTL_NH(idx))/(vMCL_HI(idx) - vHTL_HI(idx));
                slope2 = (vUCL_NH(idx) - vMCL_NH(idx))/(vUCL_HI(idx) - vMCL_HI(idx));

                % Intercepts of the two linear pieces of the input-output characteristic:
                intercept1 = vHTL_NH(idx) - slope1 * vHTL_HI(idx);
                intercept2 = vMCL_NH(idx) - slope2 * vMCL_HI(idx);

                % Input (x-data) of the two linear pieces of the input-output
                % characteristic:
                gtinput1 = vHTL_HI(idx):gtstep:vMCL_HI(idx);
                gtinput2 = vMCL_HI(idx):gtstep:vUCL_HI(idx);

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

                % Gaintable data of the whole input-output characteristic for
                % one band:
                gtdata_band = [gtdata1(1)-100 gtdata1 gtdata2(2:end) gtdata2(end)+0];
                gtdata_band_extrap = interp1(gtinput, gtdata_band, gtinput_extrap, 'linear', 'extrap') - gtinput_extrap;

                % Gaintable data of the whole input-output characteristic for
                % all bands:
                gtdata(idx, :) = gtdata_band_extrap;
            end
        end
        gtdata = round_sig(gtdata, precision);
    elseif simulate_expansion == false
        gtdata = zeros(length(vCenterFrequencies)*nChannels, length(gtmin:gtstep:gtmax));
    end


    % Compute gammatone filter coefficients as well as combined normalization
    % and phase correction factors for smectral smearing:
    cFields_lpfilt_coeff = cell(1, length(vCenterFrequencies)*nChannels);
    cValues_lpfilt_coeff = cell(1, length(vCenterFrequencies)*nChannels);
    cFields_lpfilt_norm_phase = cell(1, length(vCenterFrequencies)*nChannels);
    cValues_lpfilt_norm_phase = cell(1, length(vCenterFrequencies)*nChannels);
    cFields_smeared_out = cell(1, length(vCenterFrequencies)*nChannels);
    cValues_smeared_out = cell(1, length(vCenterFrequencies)*nChannels);
    if simulate_smearing == true
        idx = 0;
        smearing_factor = NaN(1, length(vCenterFrequencies)*nChannels);
        smearing_width = NaN(1, length(vCenterFrequencies)*nChannels);
        for side = 'lr'
            for band = 1:length(vCenterFrequencies)
                idx = idx + 1;

                % Compute smearing width / Hz (if smearing factors are only
                % provided for one side, use the same values for the other
                % side; if no smearing factors are provided, compute them
                % automatically):
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
                    smearing_factor(idx) = max(interp1(SF.f, SF.(side), vCenterFrequencies(band), 'linear', 'extrap'), 1);
                elseif strcmpi(sSmearingType, 'Automatic')
                    smearing_factor(idx) = compute_smearing_factor(thresholdLevel);
                end
                smearing_width(idx) = compute_ERB(vCenterFrequencies(band)) * (smearing_factor(idx) - 1);

                [lpfilt_coeff, lpfilt_norm_phase] = compute_lpfilt_parameters(gammaFilterOrder, smearing_width(idx), srate);
                lpfilt_coeff = round_sig(lpfilt_coeff, precision);
                lpfilt_norm_phase = round_sig(lpfilt_norm_phase, precision);

                cFields_lpfilt_coeff{idx} = ['mha.c.split_smearing.s.' upper(side) num2str(sprintf('%04d', round(vCenterFrequencies(band)))) '.lpfilt.coeff'];
                cValues_lpfilt_coeff{idx} = lpfilt_coeff;

                cFields_lpfilt_norm_phase{idx} = ['mha.c.split_smearing.s.' upper(side) num2str(sprintf('%04d', round(vCenterFrequencies(band)))) '.lpfilt.norm_phase'];
                cValues_lpfilt_norm_phase{idx} = lpfilt_norm_phase;

                cFields_smeared_out{idx} = ['mha.c.split_smearing.s.' upper(side) num2str(sprintf('%04d', round(vCenterFrequencies(band)))) '.smeared.out'];
                if smearing_width(idx) == 0.0
                    cValues_smeared_out{idx} = '[signal:0]';
                else
                    cValues_smeared_out{idx} = '[ac_mul:0]';
                end
            end
        end
    elseif simulate_smearing == false
        idx = 0;
        smearing_factor = NaN(1, length(vCenterFrequencies)*nChannels);
        smearing_width = NaN(1, length(vCenterFrequencies)*nChannels);
        for side = 'lr'
            for band = 1:length(vCenterFrequencies)
                idx = idx + 1;

                smearing_factor(idx) = 1.0;
                smearing_width(idx) = smearing_factor(idx) - 1;

                [lpfilt_coeff, lpfilt_norm_phase] = compute_lpfilt_parameters(gammaFilterOrder, smearing_width(idx), srate);
                lpfilt_coeff = round_sig(lpfilt_coeff, precision);
                lpfilt_norm_phase = round_sig(lpfilt_norm_phase, precision);

                cFields_lpfilt_coeff{idx} = ['mha.c.split_smearing.s.' upper(side) num2str(sprintf('%04d', round(vCenterFrequencies(band)))) '.lpfilt.coeff'];
                cValues_lpfilt_coeff{idx} = lpfilt_coeff;

                cFields_lpfilt_norm_phase{idx} = ['mha.c.split_smearing.s.' upper(side) num2str(sprintf('%04d', round(vCenterFrequencies(band)))) '.lpfilt.norm_phase'];
                cValues_lpfilt_norm_phase{idx} = lpfilt_norm_phase;

                cFields_smeared_out{idx} = ['mha.c.split_smearing.s.' upper(side) num2str(sprintf('%04d', round(vCenterFrequencies(band)))) '.smeared.out'];
                cValues_smeared_out{idx} = '[signal:0]';
            end
        end
    end


    %% Set parameters:

    % Set broadband input signal gain / dB:
    mha_set(handle, 'mha.c.gain.min', -abs(gain));
    mha_set(handle, 'mha.c.gain.max', abs(gain));
    mha_set(handle, 'mha.c.gain.gains', gain);

    % Set gammatone filterbank filter coefficients:
    mha_set(handle, 'mha.c.split_gtfb.c.gtfb_analyzer.coeff', gtfb_coeff);

    % Set gammatone filterbank combined normalization and phase correction
    % factors:
    mha_set(handle, 'mha.c.split_gtfb.c.gtfb_analyzer.norm_phase', gtfb_norm_phase);

    % Set complex (i.e., real and imaginary) gammatone filterbank band-specific
    % delays / samples:
    mha_set(handle, 'mha.c.split_gtfb.c.delay.delay', gtfb_delay);

    % Set gammatone filterbank band-specific recombination gains / dB:
    mha_set(handle, 'mha.c.split_dc.c.gain.gains', gtfb_gains);

    % Set gaintable settings (in dB SPL and dB, respectively) and time
    % constants (in s) for dynamic range expansion:
    mha_set(handle, 'mha.c.split_dc.c.dc.gtmin', gtmin);
    mha_set(handle, 'mha.c.split_dc.c.dc.gtstep', gtstep);
    mha_set(handle, 'mha.c.split_dc.c.dc.gtdata', gtdata);
    mha_set(handle, 'mha.c.split_dc.c.dc.tau_attack', attackTime);
    mha_set(handle, 'mha.c.split_dc.c.dc.tau_decay', releaseTime);

    % Set gammatone filter coefficients as well as combined normalization and
    % phase correction factors for smectral smearing:
    idx = 0;
    for side = 'lr'
        for band = 1:length(vCenterFrequencies)
            idx = idx + 1;

            mha_set(handle, cFields_lpfilt_coeff{idx}, cValues_lpfilt_coeff{idx});
            mha_set(handle, cFields_lpfilt_norm_phase{idx}, cValues_lpfilt_norm_phase{idx});
            mha_set(handle, cFields_smeared_out{idx}, cValues_smeared_out{idx});
        end
    end


    %% Plot parameters:

    if plot_parameters == true

        % Plot gammatone filterbank:
        figure;
        set(gcf, 'Color', 'w');
        vImpulse = [1, zeros(1, srate/2-1)];
        mImpulseResponse = Gfb_Analyzer_process(analyzer, vImpulse);
        mFrequencyResponse = fft(real(mImpulseResponse)');
        vFrequency = linspace(0, srate, size(mFrequencyResponse, 1))';
        C = get(gca, 'ColorOrder');
        mMagnitudeResponse = 20*log10((abs(mFrequencyResponse)));
        plot(vFrequency, mMagnitudeResponse, 'Color', C(1, :), 'LineWidth', lineWidth);
        grid on;
        mMagnitudeResponse(size(mMagnitudeResponse, 1)/2+2:end, :) = NaN;
        mGainsInDisplayRange = mMagnitudeResponse >= displayRange_magnitude(1);
        highestGainInDisplayRange = max((1:size(mMagnitudeResponse, 1))' .* mGainsInDisplayRange);
        highestFrequenyInDisplayRange = min(vFrequency(max(highestGainInDisplayRange)), srate/2);
        xlim([0, highestFrequenyInDisplayRange]);
        ylim(displayRange_magnitude);
        xlabel('Frequency / Hz');
        ylabel('Magnitude / dB');
        title({'Gammatone Filterbank (Without Hearing Impairment Simulation)', ''});
        set(gca, 'FontSize', fontSize, ...
            'LineWidth', lineWidth);
        pos = get(gcf, 'Position');
        pos(1) = pos(1) - pos(3)/2;
        pos(3) = 2 * pos(3);
        set(gcf, 'Position', pos);


        % Plot audiogram parameters:
        figure;
        pos = get(gcf, 'Position');
        pos(1) = pos(1) - pos(3)/4;
        pos(2) = pos(2) - pos(4);
        pos(3) = 1.5 * pos(3);
        pos(4) = 2 * pos(4);
        set(gcf, 'Position', pos);
        set(gcf, 'Color', 'w');
        ax1 = subplot(2, 2, 1);
        semilogx(vCenterFrequencies, vHTL_NH(length(vCenterFrequencies)+1:end)-vHTL_NH(length(vCenterFrequencies)+1:end), 'k-', 'LineWidth', lineWidth);
        hold on;
        semilogx(vCenterFrequencies, vMCL_NH(length(vCenterFrequencies)+1:end)-vHTL_NH(length(vCenterFrequencies)+1:end), 'k-', 'LineWidth', lineWidth);
        semilogx(vCenterFrequencies, vUCL_NH(length(vCenterFrequencies)+1:end)-vHTL_NH(length(vCenterFrequencies)+1:end), 'k-', 'LineWidth', lineWidth);
        semilogx(vCenterFrequencies, vHTL_HI(length(vCenterFrequencies)+1:end)-vHTL_NH(length(vCenterFrequencies)+1:end), '-o', 'Color', C(2, :), 'LineWidth', lineWidth, 'MarkerFaceColor', 'w', 'MarkerSize', 1.75*markerSize);
        semilogx(vCenterFrequencies, vMCL_HI(length(vCenterFrequencies)+1:end)-vHTL_NH(length(vCenterFrequencies)+1:end), '-', 'Color', C(2, :), 'LineWidth', lineWidth);
        semilogx(vCenterFrequencies, vUCL_HI(length(vCenterFrequencies)+1:end)-vHTL_NH(length(vCenterFrequencies)+1:end), '-', 'Color', C(2, :), 'LineWidth', lineWidth);
        hold off;
        text(vCenterFrequencies, vMCL_HI(length(vCenterFrequencies)+1:end)-vHTL_NH(length(vCenterFrequencies)+1:end), 'M', 'BackgroundColor', 'w', 'Color', C(2, :), 'FontSize', fontSize, 'HorizontalAlignment', 'center', 'Margin', eps, 'VerticalAlignment', 'middle');
        text(vCenterFrequencies, vUCL_HI(length(vCenterFrequencies)+1:end)-vHTL_NH(length(vCenterFrequencies)+1:end), 'U', 'BackgroundColor', 'w', 'Color', C(2, :), 'FontSize', fontSize, 'HorizontalAlignment', 'center', 'Margin', eps, 'VerticalAlignment', 'middle');
        grid on;
        axis square;
        xl = [-Inf Inf];
        yl = [-10 Inf];
        xlim(xl);
        ylim(yl);
        vXTicks = round(1000 * 2.^(-6:4));
        vXTicks = vXTicks(vXTicks>=min(vCenterFrequencies) & vXTicks<=max(vCenterFrequencies));
        xticklabels = arrayfun(@num2str, vXTicks, 'UniformOutput', false);
        vYTicks = -10:10:floor(max(vUCL_HI)/10)*10;
        set(ax1, ...
            'XMinorGrid', 'off', ...
            'XMinorTick', 'off', ...
            'XTick', vXTicks, ...
            'XTickLabel', xticklabels, ...
            'YDir', 'reverse', ...
            'YTick', vYTicks);
        ylabel('Hearing Level / dB HL');
        title('Right Ear');
        set(ax1, 'FontSize', fontSize, ...
            'LineWidth', lineWidth);
        ax2 = subplot(2, 2, 2);
        semilogx(vCenterFrequencies, vHTL_NH(1:length(vCenterFrequencies))-vHTL_NH(1:length(vCenterFrequencies)), 'k-', 'LineWidth', lineWidth);
        hold on;
        semilogx(vCenterFrequencies, vMCL_NH(1:length(vCenterFrequencies))-vHTL_NH(1:length(vCenterFrequencies)), 'k-', 'LineWidth', lineWidth);
        semilogx(vCenterFrequencies, vUCL_NH(1:length(vCenterFrequencies))-vHTL_NH(1:length(vCenterFrequencies)), 'k-', 'LineWidth', lineWidth);
        semilogx(vCenterFrequencies, vHTL_HI(1:length(vCenterFrequencies))-vHTL_NH(1:length(vCenterFrequencies)), '-o', 'Color', C(1, :), 'LineWidth', lineWidth, 'MarkerFaceColor', 'w', 'MarkerSize', 1.75*markerSize);
        semilogx(vCenterFrequencies, vMCL_HI(1:length(vCenterFrequencies))-vHTL_NH(1:length(vCenterFrequencies)), '-', 'Color', C(1, :), 'LineWidth', lineWidth);
        semilogx(vCenterFrequencies, vUCL_HI(1:length(vCenterFrequencies))-vHTL_NH(1:length(vCenterFrequencies)), '-', 'Color', C(1, :), 'LineWidth', lineWidth);
        hold off;
        text(vCenterFrequencies, vMCL_HI(1:length(vCenterFrequencies))-vHTL_NH(1:length(vCenterFrequencies)), 'M', 'BackgroundColor', 'w', 'Color', C(1, :), 'FontSize', fontSize, 'HorizontalAlignment', 'center', 'Margin', eps, 'VerticalAlignment', 'middle');
        text(vCenterFrequencies, vUCL_HI(1:length(vCenterFrequencies))-vHTL_NH(1:length(vCenterFrequencies)), 'U', 'BackgroundColor', 'w', 'Color', C(1, :), 'FontSize', fontSize, 'HorizontalAlignment', 'center', 'Margin', eps, 'VerticalAlignment', 'middle');
        grid on;
        axis square;
        xl = [-Inf Inf];
        yl = [-10 Inf];
        xlim(xl);
        ylim(yl);
        text(vCenterFrequencies(end), vHTL_NH(length(vCenterFrequencies))-vHTL_NH(length(vCenterFrequencies)), ' HTL_{NH}', 'FontSize', fontSize, 'HorizontalAlignment', 'left', 'VerticalAlignment', 'middle');
        text(vCenterFrequencies(end), vMCL_NH(length(vCenterFrequencies))-vHTL_NH(length(vCenterFrequencies)), ' MCL_{NH}', 'FontSize', fontSize, 'HorizontalAlignment', 'left', 'VerticalAlignment', 'middle');
        text(vCenterFrequencies(end), vUCL_NH(length(vCenterFrequencies))-vHTL_NH(length(vCenterFrequencies)), ' UCL_{NH}', 'FontSize', fontSize, 'HorizontalAlignment', 'left', 'VerticalAlignment', 'middle');
        set(ax2, ...
            'XMinorGrid', 'off', ...
            'XMinorTick', 'off', ...
            'XTick', vXTicks, ...
            'XTickLabel', xticklabels, ...
            'YDir', 'reverse', ...
            'YTick', vYTicks);
        title('Left Ear');
        set(ax2, 'FontSize', fontSize, ...
            'LineWidth', lineWidth);
        ax3 = subplot(2, 2, 3);
        semilogx(vCenterFrequencies, vHTL_NH(length(vCenterFrequencies)+1:end), 'k-', 'LineWidth', lineWidth);
        hold on;
        semilogx(vCenterFrequencies, vMCL_NH(length(vCenterFrequencies)+1:end), 'k-', 'LineWidth', lineWidth);
        semilogx(vCenterFrequencies, vUCL_NH(length(vCenterFrequencies)+1:end), 'k-', 'LineWidth', lineWidth);
        semilogx(vCenterFrequencies, vHTL_HI(length(vCenterFrequencies)+1:end), '-o', 'Color', C(2, :), 'LineWidth', lineWidth, 'MarkerFaceColor', 'w', 'MarkerSize', 1.75*markerSize);
        semilogx(vCenterFrequencies, vMCL_HI(length(vCenterFrequencies)+1:end), '-', 'Color', C(2, :), 'LineWidth', lineWidth);
        semilogx(vCenterFrequencies, vUCL_HI(length(vCenterFrequencies)+1:end), '-', 'Color', C(2, :), 'LineWidth', lineWidth);
        hold off;
        text(vCenterFrequencies, vMCL_HI(length(vCenterFrequencies)+1:end), 'M', 'BackgroundColor', 'w', 'Color', C(2, :), 'FontSize', fontSize, 'HorizontalAlignment', 'center', 'Margin', eps, 'VerticalAlignment', 'middle');
        text(vCenterFrequencies, vUCL_HI(length(vCenterFrequencies)+1:end), 'U', 'BackgroundColor', 'w', 'Color', C(2, :), 'FontSize', fontSize, 'HorizontalAlignment', 'center', 'Margin', eps, 'VerticalAlignment', 'middle');
        grid on;
        axis square;
        xl = [-Inf Inf];
        yl = [-10 Inf];
        xlim(xl);
        ylim(yl);
        set(ax3, ...
            'XMinorGrid', 'off', ...
            'XMinorTick', 'off', ...
            'XTick', vXTicks, ...
            'XTickLabel', xticklabels, ...
            'YDir', 'reverse', ...
            'YTick', vYTicks);
        xlabel('Center Frequency / Hz');
        ylabel('Hearing Level / dB SPL');
        set(ax3, 'FontSize', fontSize, ...
            'LineWidth', lineWidth);
        ax4 = subplot(2, 2, 4);
        semilogx(vCenterFrequencies, vHTL_NH(1:length(vCenterFrequencies)), 'k-', 'LineWidth', lineWidth);
        hold on;
        semilogx(vCenterFrequencies, vMCL_NH(1:length(vCenterFrequencies)), 'k-', 'LineWidth', lineWidth);
        semilogx(vCenterFrequencies, vUCL_NH(1:length(vCenterFrequencies)), 'k-', 'LineWidth', lineWidth);
        semilogx(vCenterFrequencies, vHTL_HI(1:length(vCenterFrequencies)), '-o', 'Color', C(1, :), 'LineWidth', lineWidth, 'MarkerFaceColor', 'w', 'MarkerSize', 1.75*markerSize);
        semilogx(vCenterFrequencies, vMCL_HI(1:length(vCenterFrequencies)), '-', 'Color', C(1, :), 'LineWidth', lineWidth);
        semilogx(vCenterFrequencies, vUCL_HI(1:length(vCenterFrequencies)), '-', 'Color', C(1, :), 'LineWidth', lineWidth);
        hold off;
        text(vCenterFrequencies, vMCL_HI(1:length(vCenterFrequencies)), 'M', 'BackgroundColor', 'w', 'Color', C(1, :), 'FontSize', fontSize, 'HorizontalAlignment', 'center', 'Margin', eps, 'VerticalAlignment', 'middle');
        text(vCenterFrequencies, vUCL_HI(1:length(vCenterFrequencies)), 'U', 'BackgroundColor', 'w', 'Color', C(1, :), 'FontSize', fontSize, 'HorizontalAlignment', 'center', 'Margin', eps, 'VerticalAlignment', 'middle');
        grid on;
        axis square;
        xl = [-Inf Inf];
        yl = [-10 Inf];
        xlim(xl);
        ylim(yl);
        text(vCenterFrequencies(end), vHTL_NH(length(vCenterFrequencies)), ' HTL_{NH}', 'FontSize', fontSize, 'HorizontalAlignment', 'left', 'VerticalAlignment', 'middle');
        text(vCenterFrequencies(end), vMCL_NH(length(vCenterFrequencies)), ' MCL_{NH}', 'FontSize', fontSize, 'HorizontalAlignment', 'left', 'VerticalAlignment', 'middle');
        text(vCenterFrequencies(end), vUCL_NH(length(vCenterFrequencies)), ' UCL_{NH}', 'FontSize', fontSize, 'HorizontalAlignment', 'left', 'VerticalAlignment', 'middle');
        set(ax4, ...
            'XMinorGrid', 'off', ...
            'XMinorTick', 'off', ...
            'XTick', vXTicks, ...
            'XTickLabel', xticklabels, ...
            'YDir', 'reverse', ...
            'YTick', vYTicks);
        xlabel('Center Frequency / Hz');
        set(ax4, 'FontSize', fontSize, ...
            'LineWidth', lineWidth);
        p = get(gcf, 'Position');
        ax = findall(gcf, 'Type', 'axes');
        for k = 1:numel(ax)
            pos = get(ax(k), 'Position');
            pos(4) = pos(4) - fontSize*2/p(4);
            set(ax(k), 'Position', pos);
        end
        axes('Position', [0 0 1 1], 'Visible', 'off');
        text(0.5, 0.995, 'Audiogram', 'FontSize', fontSize*1.3, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'top');
        yl1 = get(ax1, 'YLabel');
        yl2 = get(ax2, 'YLabel');
        yl3 = get(ax3, 'YLabel');
        yl4 = get(ax4, 'YLabel');
        ylpos1 = get(yl1, 'Position');
        ylpos2 = get(yl2, 'Position');
        ylpos3 = get(yl3, 'Position');
        ylpos4 = get(yl4, 'Position');
        minYLabelXPos_1_3 = min([ylpos1(1) ylpos3(1)]);
        minYLabelXPos_2_4 = min([ylpos2(1) ylpos4(1)]);
        ylpos1(1) = minYLabelXPos_1_3;
        ylpos2(1) = minYLabelXPos_2_4;
        ylpos3(1) = minYLabelXPos_1_3;
        ylpos4(1) = minYLabelXPos_2_4;
        set(yl1, 'Position', ylpos1);
        set(yl2, 'Position', ylpos2);
        set(yl3, 'Position', ylpos3);
        set(yl4, 'Position', ylpos4);


        % Plot dynamic range expansion parameters:
        for side = 'lr'
            figure;
            set(gcf, 'Color', 'w');
            if strcmp(side, 'l')
                bands = 1:length(vCenterFrequencies);
            elseif strcmp(side, 'r')
                bands = length(vCenterFrequencies)+1:2*length(vCenterFrequencies);
            end
            for band = bands
                subplot(4, 8, mod(band-1, 32)+1);
                plot([gtmin gtmax], [gtmin gtmax], 'k--', 'LineWidth', lineWidth);
                hold on;
                dc_plot_io(gtmin, gtstep, gtdata(band, :), gtmin:gtstep:gtmax, true);
                plot([vHTL_HI(band) vHTL_HI(band)], [vHTL_NH(band) gtmax], '--', 'Color', C(2, :), 'LineWidth', lineWidth);
                plot([vMCL_HI(band) vMCL_HI(band)], [vMCL_NH(band) gtmax], '--', 'Color', C(2, :), 'LineWidth', lineWidth);
                plot([vUCL_HI(band) vUCL_HI(band)], [vUCL_NH(band) gtmax], '--', 'Color', C(2, :), 'LineWidth', lineWidth);
                plot([vHTL_HI(band) gtmax], [vHTL_NH(band) vHTL_NH(band)], '--', 'Color', C(2, :), 'LineWidth', lineWidth);
                plot([vMCL_HI(band) gtmax], [vMCL_NH(band) vMCL_NH(band)], '--', 'Color', C(2, :), 'LineWidth', lineWidth);
                plot([vUCL_HI(band) gtmax], [vUCL_NH(band) vUCL_NH(band)], '--', 'Color', C(2, :), 'LineWidth', lineWidth);
                hold off;
                if (band <= 8) || ((band > 4 * 8) && (band <= 5 * 8))
                    text(vHTL_HI(band), gtmax*1.005, ' HTL_{HI}', 'Color', C(2, :), 'FontSize', fontSize, 'HorizontalAlignment', 'left', 'Rotation', 45, 'VerticalAlignment', 'baseline');
                    text(vMCL_HI(band), gtmax*1.005, ' MCL_{HI}', 'Color', C(2, :), 'FontSize', fontSize, 'HorizontalAlignment', 'left', 'Rotation', 45, 'VerticalAlignment', 'baseline');
                    text(vUCL_HI(band), gtmax*1.005, ' UCL_{HI}', 'Color', C(2, :), 'FontSize', fontSize, 'HorizontalAlignment', 'left', 'Rotation', 45, 'VerticalAlignment', 'baseline');
                end
                if mod(band, 8) == 0
                    text(gtmax*1.01, vHTL_NH(band), ' HTL_{NH}', 'Color', C(2, :), 'FontSize', fontSize, 'HorizontalAlignment', 'left', 'VerticalAlignment', 'middle');
                    text(gtmax*1.01, vMCL_NH(band), ' MCL_{NH}', 'Color', C(2, :), 'FontSize', fontSize, 'HorizontalAlignment', 'left', 'VerticalAlignment', 'middle');
                    text(gtmax*1.01, vUCL_NH(band), ' UCL_{NH}', 'Color', C(2, :), 'FontSize', fontSize, 'HorizontalAlignment', 'left', 'VerticalAlignment', 'middle');
                end
                ax = gca;
                ch = get(ax, 'Children');
                set(ch(end-1), 'Color', C(1, :));
                set(ch(end-1), 'LineWidth', 2*lineWidth);
                xt =  ceil(gtmin/25)*25:25:gtmax;
                xt(xt == 0) = 0;
                yt = xt;
                set(gca, 'XTick', xt, 'YTick', yt);
                tickLabels = ceil(gtmin/50)*50:50:gtmax;
                xtl = cell(size(xt));
                for k = 1:length(xt)
                    if ismember(xt(k), tickLabels)
                        xtl{k} = num2str(xt(k));
                    else
                        xtl{k} = '';
                    end
                end
                ytl = xtl;
                set(gca, 'XTickLabel', xtl, 'XTickLabelRotation', 0);
                set(gca, 'YTickLabel', ytl);
                if (band <= 3 * 8) || ((band > 4 * 8) && (band <= 7 * 8))
                    xlab = get(ax, 'XLabel');
                    set(xlab, 'String', '');
                else
                    xlabel('Input Level / dB SPL');
                end
                if mod((band-1), 8) ~= 0
                    ylab = get(ax, 'YLabel');
                    set(ylab, 'String', '');
                else
                    ylabel('Output Level / dB SPL');
                end
                grid on;
                axis square;
                xlim([minimumLevel maximumLevel]);
                ylim([minimumLevel maximumLevel]);
                center_frequency = vCenterFrequencies(mod(band-1, length(vCenterFrequencies))+1);
                xl = xlim;
                yl = ylim;
                text(xl(1)+0.05*diff(xl), yl(end)-0.025*diff(yl), ...
                    [num2str(round(center_frequency)) ' Hz'], ...
                    'FontSize', fontSize, 'FontWeight', 'bold', 'VerticalAlignment', 'top');
                set(gca, 'FontSize', fontSize, ...
                    'LineWidth', lineWidth);
            end
            axes('Position', [0 0 1 1], 'Visible', 'off');
            if strcmp(side, 'l')
                text(0.5, 0.995, 'Dynamic Range Expansion (Left Ear)', 'FontSize', fontSize*1.3, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'top');
            elseif strcmp(side, 'r')
                text(0.5, 0.995, 'Dynamic Range Expansion (Right Ear)', 'FontSize', fontSize*1.3, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'top');
            end
            pos = get(gcf, 'Position');
            pos(1) = pos(1) - pos(3);
            pos(2) = pos(2) - pos(4);
            pos(3) = 3 * pos(3);
            pos(4) = 2 * pos(4);
            set(gcf, 'Position', pos);
        end


        % Plot spectral smearing parameters:
        figure;
        pos = get(gcf, 'Position');
        pos(1) = pos(1) - pos(3)/2;
        pos(2) = pos(2) - pos(4);
        pos(3) = 2 * pos(3);
        pos(4) = 2 * pos(4);
        set(gcf, 'Position', pos);
        set(gcf, 'Color', 'w');
        bands = length(vCenterFrequencies)+1:2*length(vCenterFrequencies);
        ax1 = subplot(2, 2, 1);
        plot(vCenterFrequencies, smearing_factor(bands), '-o', 'LineWidth', lineWidth, 'MarkerFaceColor', C(1, :), 'MarkerSize', markerSize);
        grid on;
        xlim([0 Inf]);
        yl1 = ylim;
        ylabel('Smearing Factor');
        xticklabels('');
        title('Right Ear');
        set(ax1, 'FontSize', fontSize, ...
            'LineWidth', lineWidth);
        ax3 = subplot(2, 2, 3);
        plot(vCenterFrequencies, smearing_width(bands), '-o', 'LineWidth', lineWidth, 'MarkerFaceColor', C(1, :), 'MarkerSize', markerSize);
        grid on;
        xlim([0 Inf]);
        yl3 = ylim;
        xlabel('Center Frequency / Hz');
        ylabel('Smearing Width / Hz');
        set(ax3, 'FontSize', fontSize, ...
            'LineWidth', lineWidth);
        bands = 1:length(vCenterFrequencies);
        ax2 = subplot(2, 2, 2);
        plot(vCenterFrequencies, smearing_factor(bands), '-o', 'LineWidth', lineWidth, 'MarkerFaceColor', C(1, :), 'MarkerSize', markerSize);
        grid on;
        xlim([0 Inf]);
        yl2 = ylim;
        yl_3_4_max = max(yl1(end), yl2(end));
        set(ax1, 'YLim', [0 yl_3_4_max]);
        set(ax2, 'YLim', [0 yl_3_4_max]);
        xticklabels('');
        title('Left Ear');
        set(ax2, 'FontSize', fontSize, ...
            'LineWidth', lineWidth);
        ax4 = subplot(2, 2, 4);
        plot(vCenterFrequencies, smearing_width(bands), '-o', 'LineWidth', lineWidth, 'MarkerFaceColor', C(1, :), 'MarkerSize', markerSize);
        grid on;
        xlim([0 Inf]);
        yl4 = ylim;
        yl_3_4_max = max(yl3(end), yl4(end));
        set(ax3, 'YLim', [0 yl_3_4_max]);
        set(ax4, 'YLim', [0 yl_3_4_max]);
        xlabel('Center Frequency / Hz');
        set(ax4, 'FontSize', fontSize, ...
            'LineWidth', lineWidth);
        p = get(gcf, 'Position');
        ax = findall(gcf, 'Type', 'axes');
        for k = 1:numel(ax)
            pos = get(ax(k), 'Position');
            pos(4) = pos(4) - fontSize*2/p(4);
            set(ax(k), 'Position', pos);
        end
        axes('Position', [0 0 1 1], 'Visible', 'off');
        text(0.5, 0.995, 'Spectral Smearing', 'FontSize', fontSize*1.3, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'top');
        yl1 = get(ax1, 'YLabel');
        yl2 = get(ax2, 'YLabel');
        yl3 = get(ax3, 'YLabel');
        yl4 = get(ax4, 'YLabel');
        ylpos1 = get(yl1, 'Position');
        ylpos2 = get(yl2, 'Position');
        ylpos3 = get(yl3, 'Position');
        ylpos4 = get(yl4, 'Position');
        minYLabelXPos_1_3 = min([ylpos1(1) ylpos3(1)]);
        minYLabelXPos_2_4 = min([ylpos2(1) ylpos4(1)]);
        ylpos1(1) = minYLabelXPos_1_3;
        ylpos2(1) = minYLabelXPos_2_4;
        ylpos3(1) = minYLabelXPos_1_3;
        ylpos4(1) = minYLabelXPos_2_4;
        set(yl1, 'Position', ylpos1);
        set(yl2, 'Position', ylpos2);
        set(yl3, 'Position', ylpos3);
        set(yl4, 'Position', ylpos4);


        % Sort the figures:
        h = findall(0, 'Type', 'Figure');
        for k = length(h):-1:1
            figure(k);
        end
    end


    % Turn warnings for obsolete functions back on in Octave:
    if exist('OCTAVE_VERSION', 'builtin')
        warning('on', 'Octave:legacy-function');
    end

    % Reset default figure position:
    set(0, 'DefaultFigurePosition', defaultFigPos);
end


%% Functions:

function gain = compute_input_signal_gain(vSignal, desiredSignalLevel)
    % Function for computing the gain that is necessary to set the input
    % signal to the desired broadband level (in dB SPL), based on the
    % current broadband level of the first channel of the input signal
    %
    % Usage:
    % gain = compute_input_signal_gain(vSignal, desiredInputSignalLevel)
    %
    % Input parameters:
    % vSignal             input signal
    % desiredSignalLevel  desired broadband signal level / dB SPL
    %
    % Output parameter:
    % gain                gain / dB
    %
    % ---------------------------------------------------------------------

    p0 = 20.0 * 10^-6;  % reference sound pressure / Pa

    % Determine the broadband level of the input signal:
    signalRMS = sqrt(mean(vSignal(:, 1).^2));
    signalLevel = 20 * log10(signalRMS/p0);

    % Calculate the gain to be applied to the input signal:
    gain = desiredSignalLevel - signalLevel;
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
    STANDARD_AUDIOGRAM_TYPE_N6 = [75 80 82.5 85 90 90 95 100 100];  % flat and moderately sloping, severe / dB HLvHearingThresholdLevel
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

function y = round_sig(x, n)
    % Function for rounding a number to the specified number of significant
    % digits (needed for compatibility with Octave)
    %
    % Usage:
    % y = round_sig(x, n)
    %
    % Input parameters:
    % x  input number (scalar or vector)
    % n  number of significant digits (must be a positive integer)
    %
    % Output parameter:
    % y  output number (rounded)
    %
    % ---------------------------------------------------------------------

    % Ensure that n is a positive integer:
    if (n <= 0) || (mod(n, 1) ~= 0)
        error('n must be a positive integer.');
    end

    % Initialization:
    y = zeros(size(x));

    % Round the real part:
    re = real(x);
    idx = re ~= 0;
    factor = 10.^(n - floor(log10(abs(re(idx)))) - 1);
    y(idx) = round(re(idx) .* factor)./factor;

    % Round the imaginary part:
    im = imag(x);
    idx = im ~= 0;
    factor = 10.^(n - floor(log10(abs(im(idx)))) - 1);
    y(idx) = y(idx) + 1i*(round(im(idx) .* factor)./factor);
end

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

    % Compute the hearing threshold level / dB SPL:
    HTL = interp1(THIRD_OCTAVE_FREQUENCIES, NORMAL_HEARING_THRESHOLD_LEVELS, center_frequency_hz, 'spline');
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

function [smearingFactor, NH_ERB] = compute_smearing_factor(hearingThresholdLevel_dBHL)
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
    ERB = polyval(p, hearingThresholdLevel_dBHL);

    % Normalize the fitted equivalent rectangular bandwidth to the
    % normal-hearing value:
    NH_ERB = polyval(p, 0.0);
    ERB = ERB/NH_ERB;

    % Set the lower bound of the function to the normal-hearing value and
    % the upper bound to the value corresponding to a passive cochlea:
    if hearingThresholdLevel_dBHL < 0.0
        smearingFactor = 1.0;
    elseif ERB > MAXIMUM_SMEARING_FACTOR
        smearingFactor = MAXIMUM_SMEARING_FACTOR;
    else
        smearingFactor = ERB;
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

function [coeff, norm_phase] = compute_lpfilt_parameters(gamma_order, smearing_width, sampling_frequency_hz)
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
    a_gamma = pi * factorial(2 * gamma_order - 2) * 2^-(2 * gamma_order - 2)/factorial(gamma_order - 1)^2;
    b = smearing_width/a_gamma;
    coeff = exp(-2 * pi * b/sampling_frequency_hz);

    % Compute the combined normalization and phase correction factor:
    phi = 0:1e-5:2*pi;
    norm_phase = 1/sqrt(sum(abs(1./(1 - coeff * exp(1i * phi)).^4).^2)/length(phi));
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
