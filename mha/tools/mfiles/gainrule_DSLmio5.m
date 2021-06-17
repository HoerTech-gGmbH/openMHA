function [sGt] = gainrule_DSLmio5(sAud,sFitmodel)
    % function sGt = gainrule_DSLmio5(sAud,sFitmodel)
    %
    % DSL gainrule for MHA dynamic compressors. This function requires that a
    % global variable RECD exists as a struct with fields f and dB, where both
    % fields are vectors of same length:
    %    RECD.f: frequencies in Hz of RECD correction
    %    RECD.dB: RECD corrections in dB
    %
    % Input Parameters: (all frequencies given in Hz)
    % sAud - Auditory profile information: A struct with the following fields:
    % sAud.client_id - 8 character ID of the hearing impaired person, containing
    %                  initials and date of birth.
    % sAud.id        - ID of the audiogram of this hearing impaired person. By
    %                  default it contains the date of the audiogram measurement,
    %                  but this can be changed by the person entering the data.
    % sAud.l         - audiogram data for left ear
    % sAud.r         - audiogram data for right ear
    %
    % The audiogram data is itself a struct and may contain the following fields,
    % here exemplified only for the left ear:
    % sAud.l.htl_ac  - Hearing thresholds for air conduction
    % sAud.l.htl_bc  - Hearing thresholds for bone conduction
    % sAud.l.ucl     - Uncomfortable levels
    %
    % Each of these three fields is only present if the corresponding measurments
    % have been entered in the audiogram entry dialog by the user.
    % The measurement data can be extracted from the following fields, here only
    % exemplified for the air conduction thresholds:
    %
    % sAud.l.htl_ac.data - a vector of structs containing the air conduction
    %                      thresholds for the left ear.
    %                      Each struct element of the vector has the fields f, hl:
    % sAud.l.htl_ac.data(n).f  - nth measured frequency for the left ear.
    % sAud.l.htl_ac.data(n).hl - corresponding air conduction threshold in dB HL.
    %
    %
    %
    % sFitmodel - information about the dynamic compressor to fit:
    % sFitmodel.frequencies - Center frequencies of the dynamic compression bands.
    % sFitmodel.edge_frequencies - A vector containing in this order:
    %                    - The lower cutoff frequency of the lowest frequency band
    %                    - Crossover frequencies between adjacent bands
    %                    - The upper cutoff frequency of the highest frequency band
    % sFitmodel.levels - The input band levels in dB SPL for which to compute
    %                    insertion gains.
    % sFitmodel.channels - The number of ears to fit (1 or 2).
    % sFitmodel.sides    - Which ears to fit: 'l', 'r', or 'lr'
    %
    % The vector sFitmodel.edge_frequencies always contains one more element than
    % the vector sFitmodel.frequencies.
    %
    % Note that the audiogram frequencies may differ from the dynamic compressor
    % frequencies, and may also differ across ears and threshold types. Interpolation
    % or extrapolation may be required as a consequence in order to compute the gains
    % for all requested compression bands.
    %
    % Outpout parameter:
    % sGt - A struct containing the dynamic compression gaintables in the following
    %       fields:
    % sGt.l - A (LxB) real-valued matrix containing applicable insertion gains in
    %         dB for the left ear.
    %         L is the number of input levels in sFitmodel.levels.
    %         B is the number of center frequencies in sFitmodel.frequencies.
    % sGt.r - The same for the right ear

    % load dsl.oct file which interfaces the DLSmio 5 DLL

    if ispc() || ismac()
      abort_dsl_missing();
    end

    try
      dsl;
    catch
      abort_dsl_missing();
    end

    % Tell DSL library the directory where the dslmio.dat file can be found
    if (dsl_set_path('/usr/lib'))
      abort_dsl_missing();
    end

    % Compute the age of this client in months.
    % ((from the difference between the client id (birthdate) and the current date))
    clock_now = clock;
    year_now = clock_now(1);
    month_now = clock_now(2);
    day_now = clock_now(3);
    year_birth = str2double(sAud.client_id(3:4));
    month_birth = str2double(sAud.client_id(5:6));
    day_birth = str2double(sAud.client_id(7:8));
    age_years = mod(year_now - year_birth, 100);
    age_months = month_now - month_birth + age_years * 12
    if (day_birth > day_now)
        age_months = age_months - 1;
    end

    % Which ear(s) do we fit? 'l', 'r', or 'lr'?
    numears = dsl.DSL_BINAURAL;
    if length(sFitmodel.side) < 2
        numears = dsl.DSL_MONAURAL;
    end
    % Iterate over all ears that need fitting
    for side = sFitmodel.side

        % Create a datacontainer with the hearing threshold levels if available
        if (isfield(sAud, side) && isfield(sAud.(side),'htl_ac'))
            acs = dsl.dsl_datacontainer_new(dsl.dsl_HL_unit);
            for htl = sAud.(side).htl_ac.data(:)'
                dsl.dsl_datacontainer_insert_level(acs, htl.f, htl.hl);
            end
        else
            acs = dsl.dsl_datacontainer_null();
        end

        % Create a datacontainer with the bone conduction thresholds if available
        if (isfield(sAud, side) && isfield(sAud.(side), 'htl_bc'))
            bcs = dsl.dsl_datacontainer_new(dsl.dsl_HL_unit);
            for htl = sAud.(side).htl_bc.data(:)'
                dsl.dsl_datacontainer_insert_level(bcs, htl.f, htl.hl);
            end
        else
            bcs = dsl.dsl_datacontainer_null();
        end

        % Create a datacontainer with the uncomfortable levels if available
        if (isfield(sAud, side) && isfield(sAud.(side), 'ucl'))
            ucs = dsl.dsl_datacontainer_new(dsl.dsl_HL_unit);
            for ucl = sAud.(side).ucl.data(:)'
                dsl.dsl_datacontainer_insert_level(ucs, ucl.f, ucl.hl);
            end
        else
            ucs = dsl.dsl_datacontainer_null;
        end

        % Typically, audiograms are measured with supra-aural headphones.
        % The closest match available in DSL are TDH headphones.
        assessment = dsl.dsl_define_assessment(age_months, dsl.DSL_TDH, acs, ucs, bcs);

        % delete no longer needed input data
        dsl.dsl_datacontainer_delete(acs);
        dsl.dsl_datacontainer_delete(bcs);
        dsl.dsl_datacontainer_delete(ucs);

        % Tell DSL where our compression bands are.
        dsl_channel_array = dsl.dsl_channel_array_new();
        for i = 2:(length(sFitmodel.edge_frequencies)-1)
          dsl.dsl_channel_array_add_crossover_frequency(dsl_channel_array, round(sFitmodel.edge_frequencies(i)));
        end

        % Initialize the gaintable for this side with all 0dB gains
        sGt.(side) = repmat(0, length(sFitmodel.levels), length(sFitmodel.frequencies));

        all_input_signals = [];
        all_insertion_gains = [];

        % loop over all input levels
        for level_index = 1:length(sFitmodel.levels)
          level = sFitmodel.levels(level_index);
          % For input levels below 50 dB, DSL will not prescribe insertion gains.
          % Compute noise gate instead.
          level = max(level,50.01);

          % loop over all compression frequency bands
          for freq_index = 1:length(sFitmodel.frequencies)
            freq_mha = sFitmodel.frequencies(freq_index);

            % Find the two DSL frequencies closest to the mha center freq
            lowerbound_dsl_idx = 0;
            upperbound_dsl_idx = dsl.dsl_number_of_all_valid_frequencies()-1;
            while (upperbound_dsl_idx - lowerbound_dsl_idx) > 1
              if dsl.dsl_get_valid_frequency(lowerbound_dsl_idx+1) <= freq_mha
                lowerbound_dsl_idx = lowerbound_dsl_idx + 1;
              end
              if dsl.dsl_get_valid_frequency(upperbound_dsl_idx-1) >= freq_mha
                upperbound_dsl_idx = upperbound_dsl_idx - 1;
              end
            end

            % Compute insertion gains for both of these frequencies in order to interpolate
            idxs_dsl = [lowerbound_dsl_idx, upperbound_dsl_idx];
            gains_dsl = [0,0];
            for i = 1:2
              idx_dsl = idxs_dsl(i);
              freq_dsl = dsl.dsl_get_valid_frequency(idx_dsl);

              % MHA measures narrow-band levels in each band, NOT the equivalent broadband LTASS.
              % Therefore use PURETONE to generate the gains.
              dsl_input_signal = dsl.dsl_input_signal_new(dsl.dsl_PURETONE_signal());
              spectrum = dsl.dsl_container_new();
              dsl.dsl_container_insert_level(spectrum, freq_dsl, level);
              dsl.dsl_input_signal_user_define_spectra(dsl_input_signal,spectrum);
              dsl.dsl_container_delete(spectrum);

              % Compute the gain for this signal (MHA needs REIG!)
              dslmio = dsl.dsl_generate_targets(assessment, dsl_input_signal, dsl.DSL_NOISE, dsl.DSL_Adults, ...
                                 numears, dsl.DSL_WDRC, dsl_channel_array, dsl.dsl_container_null, dsl.dsl_container_null);
              targets = dsl.dsl_report_targets(assessment, dslmio, dsl.DSL_REIG, dsl.DSL_BTE, dsl_venting_correction_null);
              [~,gains_dsl(i)] = dsl.dsl_datacontainer_get_level_at_frequency(targets, freq_dsl);

              % Cleanup
              dsl.dsl_dslmio_delete(dslmio);
              dsl.dsl_datacontainer_delete(targets);
              dsl.dsl_input_signal_delete(dsl_input_signal);
            end

            % store mean gain as result for this level and band
            sGt.(side)(level_index,freq_index) = mean(gains_dsl);

            % Apply noise gate for input levels below 50 dB
            if level > sFitmodel.levels(level_index)
              sGt.(side)(level_index,freq_index) = mean(gains_dsl) - (sFitmodel.levels(level_index)-level);
            end
          end
        end

        % DSL itself, but also the noise gate, will compute negative insertion gains in some conditions. Replace those with 0dB.
        sGt.(side) = max(sGt.(side), 0);
    end
end

function abort_dsl_missing()
% Function called when invoking the dsl5 wrapper did not produce
% expected output. Fail with suitable error message.
  detail = '';
  if ispc() || ismac()
    detail = 'The DSL5 wrapper at the moment only supports Linux.';
  else
    detail = 'The dslmio file could not be found in directory /usr/lib/. Please place it there and repeat.'
  end
  if ~isoctave()
    detail = sprintf('%s\n%s', detail, 'The DSL5 wrapper is not compatible with Matlab. It requires Octave');
  end
  error('Could not invoke DSLmio5 wrapper to compute insertion gains\n%s\n%s', ...
        detail, ...
        ['Please see file README.md on the GitHub repository ', ...
         'https://github.com/HoerTech-gGmbH/openMHA for information about', ...
         ' how to obtain the DSLmio5 wrapper.']);
end
