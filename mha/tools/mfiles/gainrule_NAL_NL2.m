function sGt = gainrule_NAL_NL2(sAud,sFitmodel)
    % sGt = gainrule_NAL_NL2(sAud, sFitmodel)
    % NAL NL2 dynamic compression gainrule
    % Compute gains for compression according to NAL NL2
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

    date_of_birth = date_of_birth_from_client_id(sAud.client_id);

    % Which ear(s) do we fit? 'l', 'r', or 'lr'?
    bilateral = double(true);
    if length(sFitmodel.side) < 2
        bilateral = double(false);
    end
    
    target_frequencies = sFitmodel.frequencies;
    sGt.r = zeros(length(sFitmodel.levels), length(target_frequencies));
    sGt.l = sGt.r;
    [LTASS_70_narrowband_levels,LTASS_portions,LTASS_freq] = ...
        LTASS_speech_level_in_frequency_bands(sFitmodel.edge_frequencies, 70);
    assert(isequal([125 160 200 250 315 400 500 630 800 1000 1250 ...
                   1600 2000 2500 3150 4000 5000 6300 8000], LTASS_freq(4:22)))
    LTASS_portions = LTASS_portions(4:22,:);

    % Iterate over all ears that need fitting
    for side = sFitmodel.side
        otherside = 'l';
        if (side == 'l')
            otherside = 'r';
        end

        ac = extract_thresholds(side, 'htl_ac');
        bc = extract_thresholds(side, 'htl_bc', ac);
        acOther = extract_thresholds(otherside, 'htl_ac', ac);

        % max 18 channels
        channels = size(target_frequencies,2);
        nlevels = length(sFitmodel.levels);
        if channels > 18, error('Max. 18 bands in NAL!'), end

        for band = 1:channels
            % Compute LTASS broadband levels to achieve desired narrowband
            % levels in this band
            l_delta = LTASS_70_narrowband_levels(band) - 70;
            bblev = sFitmodel.levels - l_delta;

            % NAL NL2 is implemented in a 32 bit windows DLL and cannot be
            % loaded into 64 bit processes. Call a helper command line app.
            cmd_ac = ['--ac=' sprintf('%f,', ac)];
            cmd_bc = ['--bc=' sprintf('%f,', bc)];
            cmd_level = ['--level=' sprintf('%.1f,', bblev)];
            cmd_date_of_birth = sprintf('--date_of_birth=%d', date_of_birth);
            cmd_gender = '--gender=0';
            cmd_tonal = '--tonal=0';
            cmd_experience = '--experience=0';
            cmd_adult = '--adultChild=0';
            cmd_speed = '--compSpeed=1';
            cmd_bilateral = sprintf('--bilateral=%d', bilateral);
            cmd_mic = '--mic=0';
            cmd_direction = '--direction=0';
            cmd_limiting = '--limiting=2';
            cmd_channels = sprintf('--channels=%d', 18); % Compute gains in LTASS bands
            cmd_wbct = '--wbct=52';
            cmd_bandwidth = '--bandwidth=0';
            cmd_hatype = '--hatype=3';
            cmd_tubing = '--tubing=4';
            cmd_venting = '--venting=0';
            cmd_userecdh = '--userecd=0';
            cmd_acother = ['--ac_other=' sprintf('%f,', acOther)];
            cmd = sprintf('nalnl2wrapper.exe %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s', ...
                          cmd_ac, cmd_bc, cmd_level, cmd_date_of_birth, ...
                          cmd_gender, cmd_tonal, cmd_experience, cmd_adult, ...
                          cmd_speed, cmd_bilateral, cmd_mic, cmd_direction, ...
                          cmd_limiting, cmd_channels, cmd_wbct, ...
                          cmd_bandwidth, cmd_hatype, cmd_tubing, ...
                          cmd_venting, cmd_userecdh, cmd_acother);
                          %, cmd_crossover);
            if (~ispc() && ~ismac()) % linux
                cmd=['wine /usr/share/nalnl2wrapper/' cmd];
            end

            [status, output] = system(cmd);
            insertion_gains = sscanf(output, '%f', [19,inf])';
            if (~isequal([length(sFitmodel.levels),19], size(insertion_gains)))
                abort_nal_nl2_missing()
            end

            best_ltass_index = floor(mean(find(LTASS_portions(:,band))));
            if ~isnan(best_ltass_index)
                sGt.(side)(:,band) = insertion_gains(:,best_ltass_index);
            end
        end
    end
    sGt; % Set breakpoint here to inspect gain table


    function dateOfBirth = date_of_birth_from_client_id(client_id)
        % Try to deduce date of birth of this client from the client id
        clock_now = clock;
        year_now = clock_now(1);
        year_birth = str2double(client_id(3:4)) + 2000;
        month_birth = str2double(client_id(5:6));
        day_birth = str2double(client_id(7:8));
        age_years = mod(year_now - year_birth, 100);
        if (age_years <= 0)
            year_birth = year_birth - 100;
        end
        dateOfBirth = year_birth * 10000 + month_birth * 100 + day_birth;
    end

    function thresholds = extract_thresholds(side, name, default)
        % Values at 500Hz, 1000Hz, 2000Hz, 4000Hz need to be entered, all the
        % others are optional. If the optional values are not entered the values
        % should be made equal to 999.
        aud_frequencies_NAL = [250 500 1000 1500 2000 3000 4000 6000 8000];
        required_aud_indices = [2 3 5 7]; % 500, 1000, 2000, 4000
        thresholds = ones(1,9) * 999; % Value 999 means threshold not specified
        if (isfield(sAud, side) && isfield(sAud.(side),name))
            for htl = sAud.(side).(name).data(:)'
                index = find(aud_frequencies_NAL == htl.f);
                if (index) 
                    thresholds(index(1)) = htl.hl;
                end
            end
        end
        missing_required_f = find(thresholds(required_aud_indices) == 999);
        if (missing_required_f)
            if nargin == 3
                thresholds = default;
            else
                error('Threshold %s is missing for %dHz on side %c', name, ...
                      aud_frequencies_NAL(required_aud_indices(missing_required_f(1))), ...
                      side);
            end
        end
    end

    function abort_nal_nl2_missing()
      % Function called when invoking the nalnl2wrapper.exe did not produce
      % the expected output. Fail with suitable error message.
      detail = '';
      if ispc()
        if exist('c:\Program Files\nalnl2wrapper\bin\nalnl2wrapper.exe','file')
          if exist('c:\Program Files\nalnl2wrapper\bin\NAL-NL2.dll','file')
            detail=['Could not execute c:\Program Files\nalnl2wrapper\bin\n' ...
                   'alnl2wrapper.exe. Make sure you have required permissions'];
          else
            detail='Could not find c:\Program Files\nalnl2wrapper\bin\NAL-NL2.dll';
          end
        else
          detail = 'The NAL NL2 command line wrapper is not installed.';
        end
      else
        if exist('/usr/share/nalnl2wrapper/nalnl2wrapper.exe','file')
          if exist('/usr/share/nalnl2wrapper/NAL-NL2.dll')
            detail=['Could not execute /usr/share/nalnl2wrapper/nalnl2wrapp' ...
                   'er.exe with wine. Make sure you have required permissions'];
          else
            detail='Could not find /usr/share/nalnl2wrapper/NAL-NL2.dll';
          end
        else
          detail = 'The NAL NL2 command line wrapper is not installed.';
        end
      end
      error('Could not invoke NAL NL2 wrapper to compute insertion gains\n%s\n%s', ...
            detail, ...
            ['Please see file README.md on the GitHub repository ', ...
             'https://github.com/HoerTech-gGmbH/openMHA for information about', ...
             ' how to obtain the NAL NL2 wrapper.']);
    end
    
end
