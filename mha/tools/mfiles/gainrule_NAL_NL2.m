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
    % sFitmodel.side     - Which ears to fit: 'l', 'r', or 'lr'
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
    
    % Center frequencies of the MHA compressor's filter bank
    target_frequencies = sFitmodel.frequencies;

    % Result matrices for insertion gains in dB, initialize to 0dB.
    sGt.r = zeros(length(sFitmodel.levels), length(target_frequencies));
    sGt.l = sGt.r;

    % MHA compressors use narrow-band input level to determine the insertion
    % gain to apply, but NAL-NL2 formula uses broadband-equivalent LTASS levels
    % instead. This function call computes the required adjustments for each
    % dynamic compressor band.  It also returns the mapping of LTASS
    % third-octave bands to dynamic-compressor bands, which we need later
    % to select a suitable third-octave band to read the insertion gains from
    % (NAL-NL2 computes the insertion gains for LTASS third-octave bands).
    [LTASS_70_narrowband_levels,LTASS_portions,LTASS_freq] = ...
        LTASS_speech_level_in_frequency_bands(sFitmodel.edge_frequencies, 70);
    assert(isequal([125 160 200 250 315 400 500 630 800 1000 1250 ...
                   1600 2000 2500 3150 4000 5000 6300 8000], LTASS_freq(4:22)))

    % NAL-NL2 computes gains only for a subset of the LTASS third-octave bands.
    % Discard from our LTASS-to-MHA mapping the bands not considered by NAL.
    LTASS_portions = LTASS_portions(4:22,:);

    % Iterate over all ears that need fitting. NAL-NL2 computes insertion gains
    % for one ear at a time, but can be told whether the hearing-impaired
    % subject is fitted with one or two hearing aids and also what the
    % hearing loss on the other ear is.
    for side = sFitmodel.side
        % NAL-NL2 can take air conduction, bone conduction, and the hearing loss
        % on the other ear into account when computing insertion gains
        otherside = 'l';
        if (side == 'l')
            otherside = 'r';
        end
        ac = extract_thresholds(side, 'htl_ac');
        bc = extract_thresholds(side, 'htl_bc', ac);
        acOther = extract_thresholds(otherside, 'htl_ac', ac);

        % The number of input levels that the MHA fitting GUI wants insertion
        % gains computed for.  We can compute gains for multiple input levels
        % with a single invocation of our NAL-NL2 wrapper commant-line tool.
        nlevels = length(sFitmodel.levels);

        % NAL-NL2 cannot fit more than max 18 dynamic compressor bands
        channels = size(target_frequencies,2);
        if channels > 18
            error('Max. 18 bands in NAL!')
        end

        % We iterate over all frequency bands because we need to give the
        % NAL-NL2 library broadband-equivalent input levels for each of the
        % narrow-band MHA input levels.  The broadband corrections differ
        % between dynamic compressor frequency bands.
        for band = 1:channels
            % Compute LTASS broadband levels to achieve desired narrowband
            % levels in this band.  Fill l_delta with corrections for the
            % third-octave LTASS bands.
            l_delta = LTASS_70_narrowband_levels(band) - 70;

            % Convert the MHA narrow-band input levels to LTASS broadband
            bblev = sFitmodel.levels - l_delta;

            % NAL NL2 is implemented in a 32 bit windows DLL and cannot be
            % loaded into Windows 64 bit Matlab / Octave processes (or Linux
            % processes). Call a helper command line app as a workaround.
            % (This workaround also works on Linux with the WINE wrapper.)
            % The following lines define the individual command-line
            % parameters to transfer hearing aid and hearing-impairment
            % settings to NAL.  Change the parameters here as desired for
            % your own hearing-aid research experiment.  
            %
            % For most parameters, a numeric value needs to be given that has
            % a special meaning to the NAL-NL2 DLL.  The same numeric value
            % will have different meanings for different parameters.  Please
            % refer to the NAL-NL2 developer documentation to find the numeric
            % values which NAL-NL2 uses for these settings.
            % 
            % Many parameters have no effect when computing insertion gains,
            % but have to be given nonetheless.  This is documented below
            % to the best of our knowledge.  Please double-check against the
            % NAL-NL2 developer documentation if you agree with this
            % assessment.  They still can be set to user-defined values using
            % the command line wrapper tool and are forwarded to the NAL-NL2
            % DLL.

            % Air Conduction thresholds: comma-separated vector of HL values.
            cmd_ac = ['--ac=' sprintf('%f,', ac)];
            % Bone Conduction thresholds: same.
            cmd_bc = ['--bc=' sprintf('%f,', bc)];
            % LTASS broadband input levels: comma-separated vector of SPL
            cmd_level = ['--level=' sprintf('%.1f,', bblev)];
            % Date of birth encoded as single integer as requested by NAL-NL2
            % library documentation.  This will not be used unless the
            % --adultChild parameter below is set to the numeric value
            % which demands to calculate the age from date of birth.
            cmd_date_of_birth = sprintf('--date_of_birth=%d', date_of_birth);
            % Set gender to the numeric value signifying unknown gender,
            % because the MHA FittingGUI audiogram database does not store
            % gender information.  Set to desired NAL-NL2 numeric constant
            % here if you need gender-specific fitting.
            cmd_gender = '--gender=0';
            % Set language to non-tonal as default.  Change here if you want
            % to fit for a tonal language.
            cmd_tonal = '--tonal=0';
            % Set hearing aid experience level to the NAL-NL2 numeric value for
            % "experienced".
            cmd_experience = '--experience=0';
            % Set hearing-impaired subject's age to the NAL-NL2 numeric value for
            % "adult".
            cmd_adult = '--adultChild=0';
            % Set compression speed to the NAL-NL2 numeric value for fast
            % compression.
            cmd_speed = '--compSpeed=1';
            % Match the bilateral parameter with the number of ears to fit.
            cmd_bilateral = sprintf('--bilateral=%d', bilateral);
            % Set microphone location effect to the NAL-NL2 numeric value for
            % "undisturbed field", which is the best match for the MHA
            % microphone calibration instructions even if the microphone is
            % head-worn.
            cmd_mic = '--mic=0';
            % Set direction of sound to the NAL-NL2 numeric value for 0°
            % (sound from front).
            cmd_direction = '--direction=0';
            % Set limiting to <multichannel limiting> which causes the
            % generated insertion gains to exhibit compression limiting for
            % high output levels.
            cmd_limiting = '--limiting=2';
            % We always set the number of channels that the NAL-NL2 library
            % knows about to 18 because only then we can reliably map the
            % generated insertion gains to our compression bands.
            % Compute gains in LTASS bands
            cmd_channels = sprintf('--channels=%d', 18);
            % Set wide band compression threshold.
            cmd_wbct = '--wbct=52';
            % Set "bandwidth of noise" to  the NAL-NL2 numeric value for
            % "broadband".
            cmd_bandwidth = '--bandwidth=0';
            % Set aid type to BTE. Has no effect when computing insertion gains
            cmd_hatype = '--hatype=3';
            % Select "tubing".  Has no effect when computing insertion gains.
            cmd_tubing = '--tubing=4';
            % Select "venting".  Has no effect when computing insertion gains.
            cmd_venting = '--venting=0';
            % Select "RECD": measurement. Has no effect on insertion gains.
            cmd_userecdh = '--userecd=0';
            % Hearing loss of opposite ear: comma-separated vector of HL values
            cmd_acother = ['--ac_other=' sprintf('%f,', acOther)];

            % Combine all parameters into a single command.
            cmd = sprintf('nalnl2wrapper.exe %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s', ...
                          cmd_ac, cmd_bc, cmd_level, cmd_date_of_birth, ...
                          cmd_gender, cmd_tonal, cmd_experience, cmd_adult, ...
                          cmd_speed, cmd_bilateral, cmd_mic, cmd_direction, ...
                          cmd_limiting, cmd_channels, cmd_wbct, ...
                          cmd_bandwidth, cmd_hatype, cmd_tubing, ...
                          cmd_venting, cmd_userecdh, cmd_acother);
            
            % Add the windows emulator to command line if OS is Linux.
            if (~ispc() && ~ismac()) % linux
                if isoctave()
                    cmd=['wine /usr/share/nalnl2wrapper/' cmd];
                else % Matlab
                    % Matlab puts its C++ library directories into LD_LIBRARY_PATH
                    % which can cause failures when running commands through system().
                    % Clear LD_LIBRARY_PATH when running from Matlab.
                    cmd=['LD_LIBRARY_PATH='''' wine /usr/share/nalnl2wrapper/' cmd];
                end
            end

            % Execute the NAL-NL2 wrapper, catch the response in output
            [status, output] = system(cmd);

            % On some systems, the output of the NAL-NL2 wrapper has been observed
            % to contain terminal escape sequences, bracketed by ASCII 27 (ESC).
            % Remove these escape sequences from the output before parsing.
            idx_garbage = find(output==27);
            if( (mod(numel(idx_garbage),2) == 0) && (numel(idx_garbage)>0) )
              % Remove the escape sequences from the output. Work from last pair
              % to first pair to avoid changing the indices of the pairs still
              % to be removed.
              for k=numel(idx_garbage):-2:1
                idx = idx_garbage((k-1)):(idx_garbage(k)+1)
                output(idx) = '';
              end
            end

            % Parse insertion gains computed by NAL-NL2.
            insertion_gains = sscanf(output, '%f', [19,inf])';

            % Sanity check.
            if (~isequal([length(sFitmodel.levels),19], size(insertion_gains)))
                abort_nal_nl2_missing()
            end

            % Select best third-octave band for the current compression band
            best_ltass_index = floor(mean(find(LTASS_portions(:,band))));
            if ~isnan(best_ltass_index)
                % Fill the gain table column for this compression band with
                % the computed insertion gains.
                sGt.(side)(:,band) = insertion_gains(:,best_ltass_index);
            end
        end
    end
    sGt; % Set breakpoint here to inspect the computed NAL-NL2 insertion gains.


    function dateOfBirth = date_of_birth_from_client_id(client_id)
        % Try to deduce date of birth of this client from the client id.
        % Because the year of birth is stored with only two digits, this
        % will not work correctly for people older than 100 years, and
        % will also not work correctly for people born after year 2100.
        % Researchers working with people older than 100 years or born
        % after 2100 need to modify this function before using it.
        %
        % Unless you change the "adultChild" parameter above, the result of
        % this function call is ignored, and the fit is always performed
        % for <adult>.
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

    % Extract hearing thresholds from sAud parameter of mfile main function as
    % needed by NAL-NL2.
    function thresholds = extract_thresholds(side, name, default)
        % Values at 500Hz, 1000Hz, 2000Hz, 4000Hz need to be entered, all the
        % others are optional. If the optional values are not entered the values
        % should be made equal to 999.
        aud_frequencies_NAL = [250 500 1000 1500 2000 3000 4000 6000 8000];
        required_aud_indices = [2 3 5 7]; % 500, 1000, 2000, 4000

        % Initialize thresholds to unknown.
        thresholds = ones(1,9) * 999; % Value 999 means threshold not specified

        % Overwrite values for known thresholds.
        if (isfield(sAud, side) && isfield(sAud.(side),name))
            for htl = sAud.(side).(name).data(:)'
                index = find(aud_frequencies_NAL == htl.f);
                if (index) 
                    thresholds(index(1)) = htl.hl;
                end
            end
        end

        % Check that no required frequencies for NAL-NL2 are missing.
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
