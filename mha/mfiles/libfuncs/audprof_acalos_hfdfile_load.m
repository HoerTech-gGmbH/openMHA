function sAudProf = audprof_acalos_hfdfile_load( sAudProf, fname )
% load a HFD file (OMA exported loudness data)
% sAudProf : Auditory profile
% fname    : File name to load
  hfd = gt_Subject_new_from_hfd( fname, false );
  for side='lr'
    acalos = [];
    for k=1:numel(hfd.(side))
      acalos = ...
	  audprof_acalos_entry_add( acalos, ...
				    hfd.(side)(k).frequency, ...
				    hfd.(side)(k).mlow, ...
				    hfd.(side)(k).mhigh, ...
				    hfd.(side)(k).lcut, ...
				    hfd.(side)(k).measured_data );
    end
    sAudProf.(side).acalos = acalos;
  end
  
function subject = gt_Subject_add_fit(subject, fit, side)
% subject = gt_Subject_add_fit(subject, fit, side)
%
% Add a Fit to this subject on the given side

  fits = gt_Subject_get_fits(subject, side);
  fits = gt_Fits_add_fit(fits, fit);
  subject = gt_Subject_set_fits(subject, fits, side);  



function str = gt_Subject_as_kls_string(subject)
% str = gt_Subject_as_kls_string(subject)
%
% convert Subject data into kls formatted string (losing measured_data).

sides = {'Links', 'Rechts'};
str = '';
for sideindex = 1:2
    str = sprintf('%s"%s" "%s" "%s" %s LinBez2x3 4', ...
        str, subject.client_id, datestr(now), subject.client_id, ...
        sides{sideindex});
    fits = gt_Subject_get_fits(subject, sides{sideindex});
    for fit = fits
        str = sprintf('%s\t%d %.2f %.4g %.4g 0', ...
            str, fit.frequency, fit.lcut, fit.mlow, fit.mhigh);
    end
    str = sprintf('%s\n', str);
end


function frequencies = gt_Subject_frequencies(subject, side);
% frequencies = gt_Subject_frequencies(subject, side);
%
% returns the frequencies of the fits for the given side
  fits = gt_Subject_get_fits(subject, side);
  frequencies = [fits.frequency];


function fits = gt_Subject_get_fits(subject, side)
% fits = gt_Subject_get_fits(subject, side)
%
% Returns all fits of the subject for the given side.
%
% PARAMETERS:
%     subject:
%         A Subject struct created by one of the various gt_Subject_new*
%         functions.
%     side:
%         A string describing the side of the ear. Only the first character
%         in this string matters. It should be 'l' or 'r'.
% OUTPUT:
%     fits:
%         An array of Fit structures describing the loudness function. (See
%         gt_Fit_new for a description of the Fit structures).
s = lower(side(1));
if isfield(subject, s)
    fits = subject.(s);
else
    fits = gt_Fit_new;
end





function target_subject = gt_Subject_interpolate(source_subject, target_frequencies)
% target_subject = gt_Subject_interpolate(source_subject, target_frequencies)
% 
% Returns a gt_Subject structure with Fits for the given target_frequencies.
% The fit parameters are interpolated from the ones given in source_kls.
%
% PARAMETERS:
%     target_frequencies:
%         A row vector containing the target frequencies in Hz
%     source_subject:
%         A gt_Subject structure representing a categorial loudness
%         measurement
%
% OUTPUT:
%     target_subject:
%         A gt_Subjetct structure with Fits for the desired target_frequencies.
%
% (C) 2003 Universität Oldenburg

target_subject = source_subject;
for cellname = fieldnames(target_subject)'
    name = cellname{1};
    if isequal(1, length(name))
        target_subject.(name) = ...
            gt_Fit_interpolate(target_subject.(name), target_frequencies);
    end
end
target_subject.original = source_subject;



function subject = gt_Subject_new_from_audiogram(audiogram, ...
                                                 target_frequencies, nh)
% subject = gt_Subject_new_from_audiogram(audiogram)
%
% Create subject object from audiogram data
%
% OPTIONS:
%   audiogram
%     A struct with fields:
%       audiogram.frequencies -- Audiogram frequencies / Hz
%       audiogram.l.htl       -- Hearing threshold levels left / dB HL
%       audiogram.l.ucl       -- Uncomfortable levels left / dB HL
%       audiogram.r.htl       -- Hearing threshold levels right / dB HL
%       audiogram.r.ucl       -- Uncomfortable levels right / dB HL
%       audiogram.name        -- client_id
%     All (sub-)fields have to contain voectors of the same length
%   target_frequencies [optional]
%     Set to audiogram.frequencies or [] or regret it!
%   nh [optional]
%     mean normal hearing listener loudness function parameters

if (nargin < 2)
    target_frequencies = [];
end
if isempty(target_frequencies)
  target_frequencies = audiogram.frequencies;
end
if nargin < 3
  nh = gt_Subject_nh;
end

subject.client_id = audiogram.name;
num_frequencies = length(target_frequencies);

for side = 'lr'
    htls = interp1(...
        log(audiogram.frequencies), ...
        audiogram.(side).htl, ...
        log(target_frequencies));
    ucls = interp1(...
        log(audiogram.frequencies), ...
        audiogram.(side).ucl, ...
        log(target_frequencies));
    for freq_index = 1:num_frequencies
        fit = gt_Fit_new_from_aud(...
            htls(freq_index), ...
            ucls(freq_index), ...
            target_frequencies(freq_index), ...
            gt_Subject_get_fits(nh,side));
        subject = gt_Subject_add_fit(subject, fit, side);
    end
end


function subject = gt_Subject_new_from_db
subject = gt_Subject_new_from_string(kls_data_from_db);

function data = kls_data_from_db
  if (~exist('mwsh'))
    data = '';
    error('gt:Subject:new_from_db', ...
        'Mex file ("mwsh") for database access not found');
  end
  %winshell laden
  status = mwsh('load', 'wsh');
  %pruefen ob status==0, sonst Fehler
  if (~isequal(status, 0))
    error('Cannot load winshell for database access');
  end

  %Messung wählen
  status = mwsh(['call kls.e select']);
  %pruefen ob status==0, sonst Fehler
  if (~isequal(status, 0))
    error('Error while selecting measurement from database');
  end

  %String aus winshell holen
  data = mwsh('/tmp/Result', 'get');

  %winshell entladen
  status = mwsh('unload', 'wsh');





function subject = gt_Subject_new_from_db_with_data(refit)
% subject = gt_Subject_new_from_db_with_data(refit)
% 
% extract loudness data from mess-ol db
% If refit is true, then recompute the fit parameters using Toms new
% fitting prescription + passcoe option.

  if (~exist('mwsh'))
    error('gt:Subject:new_from_db', ...
        'Mex file ("mwsh") for database access not found');
  end
  
  %winshell laden
  status = mwsh('load', 'wsh');
  %pruefen ob status==0, sonst Fehler
  if (~isequal(status, 0))
    error('gt:Subject:new_from_db_with_data', ...
        'Cannot load winshell for database access');
  end

  %Messung wählen
  status = mwsh(['call kls.e select']);
  %pruefen ob status==0, sonst Fehler
  if (~isequal(status, 0))
    error('Error while selecting measurement from database');
  end

  % Daten aus winshell holen
  
  subject.client_id = strtok(mwsh('/tmp/Result', 'get'));
  
  %lese Anzahl subheader in KLS-Result
  numhd = sscanf(mwsh('/tmp/NumHd', 'get'), '%d', 1);

  for header_index = [0:(numhd-1)] 
      subheader = sprintf('/tmp/ResultHd/%d/', header_index);
      side = mwsh([subheader, 'Seite'], 'get');
      lcut = get_mwsh(subheader, 'Lcut');
      mlow = get_mwsh(subheader, 'Mlow');
      mhigh = get_mwsh(subheader, 'Mhigh');
      data = get_mwsh(subheader, 'DatenAbfolge');
      if isempty(data)
        data = get_mwsh(subheader, 'Daten');
      end
      frequency = get_mwsh(subheader, 'Frequenz');
      if (~isempty(lcut) & ~isempty(mlow) & ~isempty(mhigh) & ...
              ~isempty(data) & ~isempty(frequency))
        if refit
          fit = gt_Fit_new(data, frequency, 'tom2005');
        else
          fit = gt_Fit_new([lcut, mlow, mhigh], frequency, data);
        end
        subject = gt_Subject_add_fit(subject, fit, side);  
      end
  end
  %winshell entladen
  status = mwsh('unload', 'wsh');

function float_result = get_mwsh(subheader, fieldname)
  string_result = mwsh([subheader, fieldname], 'get');
  float_result = sscanf(string_result, '%f', [1,inf]);



function subject = gt_Subject_new_from_hfd(filename, refit)
% subject = gt_Subject_new_from_hfd(hfd_filename, refit)
%
% generate subject data structure containing loudness functions from the
% result file (containing the shs header generated by mess-ol from
% categorical loudness scaling.
% If refit is true, then recompute the fit parameters using Toms new
% fitting prescription + passcoe option.

if nargin < 2
  refit = 0;
end

fd = fopen(filename);
if (fd < 0)
    error(['Unable to open file ', filename, ' for reading']);
end

shs_data = fscanf(fd, '%c', [1,inf]);
fclose(fd);
subject = gt_Subject_new_from_shs(shs_data, refit);


function subject = gt_Subject_new_from_kls_file(filename)
fd = fopen(filename);
if (fd < 0)
    error(['Unable to open file ', filename, ' for reading']);
end

kls_data = fscanf(fd, '%c', [1,inf]);
fclose(fd);
subject = gt_Subject_new_from_string(kls_data);



function kls = gt_Subject_new_from_shs(shs_string, refit)
% subject = gt_Subject_new_from_shs(shs_string, refit)
%
% generate subject data structure containing loudness functions from the
% shs header string as generated by mess-ol from categorical loudness
% scaling.
% If refit is true, then recompute the fit parameters using Toms new
% fitting prescription + passcoe option.

  if nargin < 2
    refit = 0
  end
  
  %------------------------------------------------------------
  % split SHS data into lines 
  %------------------------------------------------------------
  lines_ = split_data_into_lines(shs_string);
  
  kls.client_id = '';

  %------------------------------------------------------------
  % scan each line
  %------------------------------------------------------------
  data = [];
  mlow = [];
  mhigh = [];
  lcut = [];
  frequency = [];
  seite = '';
  
  line_no = 0;
  while ( line_no < length(lines_) )
    line_no = line_no + 1;
    line_ = lines_{line_no};
    tokens = parse_shs_line(line_);
    if isequal(tokens{1}, 'ClientId:')
      kls.client_id = tokens{2};
    elseif isequal(tokens{1}, 'Daten:')
      if isempty(data) % Bevorzuge DatenAbfolge
        data = [tokens{2:end}];
      end
    elseif isequal(tokens{1}, 'DatenAbfolge:')
      data = [tokens{2:end}];
    elseif isequal(tokens{1}, 'Lcut:')
      lcut = (tokens{2});
    elseif isequal(tokens{1}, 'Mlow:')
      mlow = (tokens{2});
    elseif isequal(tokens{1}, 'Mhigh:')
      mhigh = (tokens{2});
    elseif isequal(tokens{1}, 'Frequenz:')
      frequency = (tokens{2});
    elseif isequal(tokens{1}, 'Seite:')
      seite = tokens{2};
    elseif isequal(tokens{1}, '}')
      if ~isempty(seite) & ~isempty(frequency) & ~isempty(lcut) & ...
           ~isempty(mlow) & ~isempty(mhigh)
        if refit
          fit = gt_Fit_new_from_data(data, frequency, 'tom2005')
        else
          params = [lcut, mlow, mhigh];
          fit = gt_Fit_new(params, frequency, data);
        end
        kls = gt_Subject_add_fit(kls, fit, seite);  
      end
      data = [];
      mlow = [];
      mhigh = [];
      lcut = [];
      frequency = [];
      seite = '';
    end
  end
  if (isempty(kls.client_id))
      error('insufficient shs data (client id).');
  end

% ------------------------------------------------------------
% Tools for reading KLS data from string
% ------------------------------------------------------------
  
function tokens = parse_shs_line(string)
  tokens = {};
  count = 0;
  rest = string;
  while (~isempty(rest))
    count = count + 1;
    [token, rest] = parse_1_shs_token(rest);
    if (isempty(token))
      break;
    end
    tokens{count} = token;
  end
  
function [token, rest] = parse_1_shs_token(string)
  rest = skip_space(string);
  token = [];
  if (isempty(rest))
    return
  end
  if isequal(rest(1), '"')
    delimiter = '"';
  else
    %            space, carriage return, newline , tab
    delimiter = [' '  , char(13)       , char(10), char(9)];
  end
  [token, rest] = strtok(rest, delimiter);
  rest = rest(2:length(rest));
  if (~isempty(token))
    if (~isnan(str2double(token)))
      token = str2double(token);
    end
  end
  
function rest = skip_space(string)
  rest = string;
  if (~isempty(rest))
    if (isspace(rest(1)))
      rest = skip_space(rest(2:length(rest)));
    end
  end

function kls = gt_Subject_new_from_string(kls_string)
  %------------------------------------------------------------
  % split KLS data into lines 
  %------------------------------------------------------------
  lines_ = split_data_into_lines(kls_string);
  
  kls.client_id = '';

  %------------------------------------------------------------
  % scan each line
  %------------------------------------------------------------
  line_no = 0;
  while ( line_no < length(lines_) )
    line_no = line_no + 1;
    line_ = lines_{line_no};
    tokens = parse_kls_line(line_);
    if (isempty(tokens)), break; end
    if (length(tokens) < 6)
        error(sprintf('invalid kls data (too few tokens in line %d)', ...
                      line_no));
    end

    ThisClientId = tokens{1};
    sDateTime = tokens{2};
    sRemark = tokens{3};
    ThisSide = tokens{4};
    ThisFitName = tokens{5};
    ThisNumber = tokens{6}; % number of fit parameters
    if ~isequal(ThisNumber,4)
        error(sprintf('%s (number of fit params on line %d not 4).', ...  
                      'invalid kls data', line_no));
    end

    % check client id on this line:
    if line_no == 1 & isempty(kls.client_id),
      kls.client_id = ThisClientId;
    else
      if kls.client_id ~= ThisClientId
        error('inconsistent kls data (client id).');
      end
    end

    % read all frequencies/params:
    ThisValues = cell2mat(tokens(7:length(tokens)));
    SetSize = ThisNumber+1;
    NumSets = floor(length(ThisValues)/SetSize);
    %
    for I = 1:NumSets,
      frequency = ThisValues((I-1)*SetSize+1);
      params = ThisValues((I-1)*SetSize+2:(I-1)*SetSize+1+ThisNumber);
      fit = gt_Fit_new(params, frequency);
      kls = gt_Subject_add_fit(kls, fit, ThisSide);  
    end
  end
  if (isempty(kls.client_id))
      error('insufficient kls data (client id).');
  end
% ------------------------------------------------------------
% Tools for reading KLS data from string
% ------------------------------------------------------------
  
function tokens = parse_kls_line(string)
  tokens = {};
  count = 0;
  rest = string;
  while (~isempty(rest))
    count = count + 1;
    [token, rest] = parse_1_kls_token(rest);
    if (isempty(token))
      break;
    end
    tokens{count} = token;
  end
  
function [token, rest] = parse_1_kls_token(string)
  rest = skip_white_space(string);
  token = [];
  if (isempty(rest))
    return
  end
  if isequal(rest(1), '"')
    delimiter = '"';
  else
    %            space, carriage return, newline , tab
    delimiter = [' '  , char(13)       , char(10), char(9)];
  end
  [token, rest] = strtok(rest, delimiter);
  rest = rest(2:length(rest));
  if (~isempty(token))
    if (~isnan(str2double(token)))
      token = str2double(token);
    end
  end
  
function rest = skip_white_space(string)
  rest = string;
  if (~isempty(rest))
    if (isspace(rest(1)))
      rest = skip_white_space(rest(2:length(rest)));
    end
  end



function subject = gt_Subject_nh
% loads the loudness functions of the average normal hearing listener into
% a gt_Subject structure
subject = gt_Subject_new_from_kls_file('nh.kls');




function gt_Subject_plot(subject, side, nh, hl_corr)
% gt_Subject_plot(subject, side, nh, hl_corr)

if nargin < 3
    nh = gt_Subject_nh;
end
if nargin < 4
    hl_corr.name = 'HL';
    hl_corr.f = gt_Subject_frequencies(subject,side);
    hl_corr.d = zeros(size(hl_corr.f));
else
    if isequal('char',class(hl_corr))
        mode = hl_corr;
        hl_corr.name = ['SPL ',mode,mode];
        hl_corr.f = gt_Subject_frequencies(subject,side);
        hl_corr.d = -hl_correction(hl_corr.f, mode);
    else
        hl_corr.d = interp1(hl_corr.f, hl_corr.d, gt_Subject_frequencies(subject,side));
        hl_corr.f = gt_Subject_frequencies(subject,side);
    end
end

fits = gt_Subject_get_fits(subject, side);
nh_fits = ...
    gt_Subject_get_fits(gt_Subject_interpolate(...
                                nh, ...
                                gt_Subject_frequencies(subject, side)), ...
                        side);
side = lower(side(1));
colors = 'br';
side_index = 1+isequal(side,'r');
color = colors(side_index);
rows = ceil(length(fits) / 2);                    
for fit_index = 1:length(fits)
    fit = fits(fit_index);
    fit.lcut = fit.lcut - hl_corr.d(fit_index);
    nh_fit = nh_fits(fit_index);
    nh_fit.lcut = nh_fit.lcut - hl_corr.d(fit_index);
    subplot(rows,2,fit_index);
    gt_Fit_plot(fit,color);
    hold on;
    gt_Fit_plot(nh_fit,['--' color]);
    axis([0 120 0 50]);
    set(gca,'ytick',[0:5:50],'xtick',[0:10:120])
    grid on
    title(sprintf('%s: %dHz', subject.client_id, fit.frequency))
    xlabel(['dB ' hl_corr.name])
    ylabel('cu')
end

set(gcf, 'PaperPosition', [0.6345    0.6345   19.7150   28.4084]);

function kls = gt_Subject_set_fits(kls, fits, side)
kls.(lower(side(1))) = fits;



function lines_ = split_data_into_lines(data)
% function lines = split_data_into_lines(data)
%
% split the string <data> into <lines> 
  lines_ = {};
  line_delimiter = [char(13), char(10)]; % recognized end-of-line characters
  
  % data given as string
  while length(data)
    [line_, data] = strtok(data, line_delimiter);
    if (~isempty(line_) & ~all(isspace(line_)))
      lines_{length(lines_)+1} = line_;
    end
  end

function fit = gt_Fit_new(fitparams, frequency, measured_data, fitname)
% fit = gt_Fit_new(data, frequency, measured_data, fitname)
%
% function creates a new Fit structure from the given arguments. A fit
% structure can be used to transform sound levels to categorial units
% (gt_Fit_cu) and vice versa (gt_Fit_level).
%
% The structure fields of a Fit structure are:
%     frequency (in Hz), lcut, mlow, mhigh, measured_data
%     
% PARAMETERS: (All are optional)
%     fitparams:
%         An array containing the fit parameters Lcut, Mlow, Mhigh, in
%         this order.
%         If this parameter is missing, an empty (1x0) structure array is
%         returned from this function.
%     frequency:
%         The frequency of the loudness function represented by this Fit.
%     measured_data:
%         The data points from the categorial loudness scaling.
%     fitname:
%         If given, must be 'LinBez2x3'.
%         Other fit models are not implemented for this Fit structure.
% OUTPUT:
%     fit:
%         The new fit structure describing the loudness function.

  if (nargin < 4)
    fitname = 'LinBez2x3';
  end
  if (nargin < 3)
      measured_data = [];
  end
  if (nargin < 2)
      frequency = [];
  end
  end_index = 1;
  if (nargin < 1)
      fitparams = [nan,nan,nan];
      end_index = 0;
  end
  fit.frequency = frequency;
  if ~isequal('LinBez2x3', fitname)
    error('will only work with ''LinBez2x3'' fit')
  end
  
  fit.lcut = fitparams(1);
  fit.mlow = fitparams(2);
  fit.mhigh = fitparams(3);
  fit.measured_data = measured_data;
  fit = fit(1:end_index);



function fit = gt_Fit_new_from_aud(hsl, ucl, frequency, nh_fits)
% fit = gt_Fit_new_from_aud(hsl, ucl, frequency, nh_fits)
%
% function creates a new Fit structure from audiogram data. A fit
% structure can be used to transform sound levels to categorial units
% (gt_Fit_cu) and vice versa (gt_Fit_level).
%
% The structure fields of a Fit structure are:
%     frequency (in Hz), lcut, mlow, mhigh, measured_data
%     
% PARAMETERS:
%     hsl:
%         The hearing threshold in dB HL.
%     ucl:
%         The uncomfortable level in dB HL
%     frequency:
%         The frequency of the audiogram data.
%     nh_fits:
%         Optional. The Loudness function fits for a normal hearing
%         listener.
% OUTPUT:
%     fit:
%         The new fit structure describing the loudness function.

if (nargin < 4)
    nh = gt_Subject_nh;
    nh_fits = nh.l;
end

if (frequency <= 0)
    error('frequency must be > 0');
end

nh_freq = [nh_fits.frequency];
nh_mhigh = [nh_fits.mhigh];

% for interpolation restrict frequency to covered range
interp_frequency = frequency;
if frequency < min(nh_freq)
    interp_frequency = min(nh_freq);
end
if frequency > max(nh_freq)
    interp_frequency = max(nh_freq);
end

mhigh = interp1(log(nh_freq), nh_mhigh, log(interp_frequency), 'linear');

fit = gt_Fit_new(audiogram2tomfit(hsl, ucl, mhigh), frequency);



function fit = gt_Fit_new_from_data(measured_data, frequency, fit_mode)
% fit = gt_Fit_new_from_data(measured_data, frequency, fit_mode)
%
% function creates a new Fit structure from loudness scaling data points.
% A fit structure can be used to transform sound levels to categorical
% units (gt_Fit_cu) and vice versa (gt_Fit_level).
%
% The structure fields of a Fit structure are:
%     frequency (in Hz), lcut, mlow, mhigh, measured_data
%     
% PARAMETERS:
%     measured_data:
%         The data points from the categorial loudness scaling.
%     frequency:
%         The frequency of the audiogram data.
%     fit_mode:
%         Optional. The fitting mode, either 'tom2000' or
%         'tom2005' (with passcoe ucl). Default is traditional 'tom2000'
%     
% OUTPUT:
%     fit:
%         The new fit structure describing the loudness function.

if (nargin < 3)
  fit_mode = 'tom2000';
end
if (frequency <= 0)
    error('frequency must be > 0');
end
if isequal('tom2005', fit_mode)
  [lcut, mlow, mhigh] = tomfit_passcoe(measured_data);
else
  [lcut, mlow, mhigh] = tomfit_orig(measured_data);
end
fit = gt_Fit_new([lcut, mlow, mhigh], frequency, measured_data);



function [Lcut,Mlow,Mhigh] = tomfit_orig(daten)

% Startwerte für Optimierung:
%fitparams =[Lcut,Mlow,Mhigh];
fitparams_start =[75,0.35,0.65];

[fitparams, exval, exitflag, output] = ...
    fminsearch( @(fitparams)costfc2(fitparams,daten),fitparams_start);

Lcut = fitparams(1);
Mlow = fitparams(2);
Mhigh = fitparams(3);

function [x] = costfc2(fitparams,daten)
% (zu minimierende Kostenfunktion)
% gibt Summe der quadratischen Fehler des Fit-Modells zu den Daten aus
% verwendet linear fortgeführte klsku.dll
% fitparams =[Lcut,Mlow,Mhigh]
% daten = [level1, cu1, level2, cu2, ... ]

level = daten(1:2:end);
cu = daten(2:2:end);
cu_fit = klsku(level,'LinBez2x3',[fitparams, 0]);

%% lineare Fortführung von klsku:
x2=klsku(50,'LinBez2x3',[fitparams, 0],1); %get level at CU = 50
x0=klsku(0,'LinBez2x3',[fitparams, 0],1); %get level at CU = 0

cu_fit(level<x0) = fitparams(2)*(level(level<x0) - x0) + 0;
cu_fit(level>x2) = fitparams(3)*(level(level>x2) - x2) + 50;

cu_fit(cu==50 & (cu_fit > cu))   = 50;
cu_fit(cu==0 & (cu_fit < cu))    = 0;

x = sum((cu_fit - cu).^2);





function [Lcut,Mlow,Mhigh] = tomfit_passcoe(daten)
%
% Berechnet [Lcut,Mlow,Mhigh] nach verbessertem Verfahren
%
% Thomas Bisitz, Mai/Juni 2005
%
% 3b: ohne 'if Lcut < UCL'

[Lcut, Mlow, Mhigh] = tomfit_orig(daten);

MaxUCL = 140;
vHFSData = [daten(1:2:end);daten(2:2:end)];

HTL = klsku(0,'LinBez2x3',[Lcut,Mlow,Mhigh],1);
UCL = klsku(50,'LinBez2x3',[Lcut,Mlow,Mhigh],1);

bReFit = 0;
% Falls die UCL höher als MaxUCL geschätzt wird, wird sie auf MaxUCL
% festgelegt, und der Fit wird mit dieser Nebenbedingung nochmal durchgeführt.
if UCL>MaxUCL,
  UCL = MaxUCL;
  bReFit = 1;
end 
% Falls die obere Steigung eindeutig zu niedrig ist (< 0.25 cu/dB) oder
% wenn nicht mindestens 4 Lautheitsurteile vorliegen, die mindestens als
% 'laut'(35cu) beurteilt wurden, wird die UCL nach Pascoe abgeschätzt
% und dann der Fit mit dieser Randbedingung wiederholt
	
if Mhigh<0.25 | sum(vHFSData(2,:)>=35)<4,
  UCL = PascoeUCL(HTL);
  bReFit = 1;
end
    
if bReFit
  fitparams_start =[UCL-20,0.35];
  [fitparams, exval, exitflag, output] = ...
      fminsearch(@(fitparams)costfc2([fitparams, 25/(UCL-fitparams(1))], ...
                                     daten), ...
                 fitparams_start);
  Lcut  = fitparams(1);
  Mlow  = fitparams(2);
  Mhigh = (50-25)/(UCL-Lcut);
end	


function UCL=PascoeUCL(HTL,mode)
%
%	UCL=PascoeUCL(HTL,mode)
%
%	schätzt die Unbehaglichkeitsschwelle UCL aus der Hörschwelle HTL ab
%	nach:
%	Pascoe, D. P., (1988), Clinical measurements of the auditory dynamic range
%       and their relation to formulas for hearing aid gain, In: J.H. Jensen (Editor),
%       Hearing Aid fitting, 13th Danavox Symposium, 129-152.
%
%	mode = 'smoothed': einfache Abschätzung nach Tabelle 4 (pooled data)

if nargin<2
	mode = 'smoothed';
end

if strcmp(mode,'smoothed')
	UCLverHTL=[...
		-100	100;
		40	100;
		120	140];
end
UCL=interp1(UCLverHTL(:,1),UCLverHTL(:,2),HTL);


function fits = gt_Fits_add_fit(fits, fit)
  if any([fits.frequency] == fit.frequency)
      error('gt:Fits:add_fit:BAD_FREQ', ...
            'More than 1 set of fit params for %dHz', ...
            fit.frequency);
  end
  fits = [fits,fit];



  
