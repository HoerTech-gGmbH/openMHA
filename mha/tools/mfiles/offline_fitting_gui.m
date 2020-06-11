% This function temporarily starts an openMHA instance and has it process test
% data. Your Octave/Matlab session needs to be set up to call mha_start, mha_set
% etc. Please refer to the getting started guide for examples how to do this:
% http://www.openmha.org/docs/openMHA_starting_guide.pdf
% The function plots the input-output characteristic that it measures in a new
% figure.

% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2019 2020 HörTech gGmbH
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


% Matlab skript performs hearing aid dynamic compression on a set of audio files

% This setup is restricted to work with audiofiles with 44100Hz sampling rate
sampling_rate = 44100;
num_channels = 2;
amplification_headroom = 50;

%% Load files from folder
select_new_files = true;
while select_new_files
  [soundfiles,audiofiles_directory] = uigetfile('*.wav','Select .wav Files','MultiSelect', 'on');

% If only one file is selected, the variable soundfiles will be of type
% char instead of a cell array -> convert soundfiles to cell array
  if ~iscell(soundfiles)
    soundfiles = {soundfiles};
  end
  if ~isequal(class(audiofiles_directory),'char')
    answer_nodir = questdlg('You did not choose any soundfiles, do you want to try again?', ...
                            'Soundfile Error','Try Again','Abort','Try Again');
    switch answer_nodir
      case 'Try Again'
        continue;
      case 'Abort'
        error('Aborted by user.');
    end
  end

  soundfiles_src = fullfile(audiofiles_directory, soundfiles);

  %% Check sound files
  if numel(soundfiles) == 0
    errordlg(sprintf('No sound files in directory %s.', audiofiles_directory), ...
             'Empty Directory');
    error(sprintf('No sound files in directory %s.', audiofiles_directory));
  end

  input_levels_re_fullscale = zeros(numel(soundfiles),num_channels);
  select_new_files = false;
  for i=1:numel(soundfiles_src)
    soundfile = soundfiles_src{i};
    [y,fs] = audioread(soundfile);
    if sampling_rate ~= fs
      errordlg(sprintf('Soundfile %s has sampling rate %d, expected %d', ...
                       soundfile, fs, sampling_rate), ...
               'Soundfile Error');
      select_new_files = true;
      break;
    end
    if size(y,2) ~= num_channels
      errordlg(sprintf('Soundfiles need %d channels, this file differs: %s', ...
                       num_channels, soundfile), ...
               'Soundfile Error');
      select_new_files = true;
      break;
    end
    if size(y,1) == 0
      errordlg(sprintf('Soundfile is empty: %s', soundfile), 'Soundfile Error');
      select_new_files = true;
      break;
    end

    input_levels_re_fullscale(i,:) = 10*log10(mean(y.^2));
  end
end
%% Processing

% Get and check peaklevel
get_new_peaklvl = true;
while get_new_peaklvl
  peaklevel = inputdlg('Peaklevel / dB SPL:', 'Type in the Peaklevel in dB SPL that corresponds to 0 dB FS');
  if isempty(peaklevel)
    error('Aborted by user.');
  end
  peaklevel = str2double(peaklevel);
  if (~isequal(class(peaklevel),'double') || (numel(peaklevel) ~= 1) || ~isreal(peaklevel) || isnan(peaklevel))
    errordlg('Expected a single, real number for peaklevel');
    continue;
  end


  listdlg_msg = {sprintf('You have selected an input peaklevel of %.1f dB SPL.' , peaklevel)};
  listdlg_msg = [listdlg_msg, sprintf('The corresponding output peaklevel will be %.1f dB SPL (for %.1f dB amplification headroom).', ...
                                      peaklevel + amplification_headroom, amplification_headroom)];
  listdlg_msg = [listdlg_msg, sprintf('Please check all input levels below and proceed only if they are correct.')];
  listdlg_msg = [listdlg_msg,'LEFT RIGHT Soundfile Input Levels / dB SPL'];
  list_items = {};
  for index = 1:size(input_levels_re_fullscale,1)
    l = peaklevel + input_levels_re_fullscale(index,:);
    list_items = [list_items, sprintf('%5.1f %5.1f   %s', l(1,1), l(1,2), soundfiles{index})];
  end
  [~,lvl_corr] = listdlg('PromptString',listdlg_msg,'ListString',list_items, ...
                         'Name','Check Input Levels','OKString','Next','CancelString','Enter new peak level', ...
                         'SelectionMode','single','ListSize',[500 300]);
  switch lvl_corr
    case 1
      get_new_peaklvl = false;
    case 0
  end
end

% Get output directory
get_dir = true;
while get_dir
  outdir = uigetdir('.');
  if ~ischar(outdir)
    answer_outdir = questdlg('Do you want to abort the program?','Output Directory Error','Try Again','Abort','Abort');
    switch answer_outdir
      case 'Try Again'
        continue;
      case 'Abort'
        error('Aborted by user.');
    end
  end
  get_dir = false;
end

% Create mha configuration
dsc.nchannels_in = num_channels;
dsc.fragsize = 64;
dsc.srate = sampling_rate;
dsc.mhalib = 'transducers';
dsc.iolib = 'MHAIOFile';
dsc.mha.plugin_name = 'overlapadd';
dsc.mha.calib_in.peaklevel = peaklevel;
dsc.mha.calib_out.peaklevel = peaklevel + amplification_headroom;
dsc.mha.overlapadd.fftlen = 256;
dsc.mha.overlapadd.wnd.len = 128;
dsc.mha.overlapadd.plugin_name = 'mhachain';
dsc.mha.overlapadd.mhachain.algos = {'fftfilterbank', 'dc', 'combinechannels'};
dsc.mha.overlapadd.mhachain.fftfilterbank.f = [177 297 500 841 1414 2378 4000 6727 11314];
dsc.mha.overlapadd.mhachain.fftfilterbank.fscale = 'log';
dsc.mha.overlapadd.mhachain.fftfilterbank.ovltype = 'rect';
dsc.mha.overlapadd.mhachain.dc.gtdata = ...
zeros(numel(dsc.mha.overlapadd.mhachain.fftfilterbank.f)*num_channels, 2);
dsc.mha.overlapadd.mhachain.dc.gtmin = [0];
dsc.mha.overlapadd.mhachain.dc.gtstep = [1];
dsc.mha.overlapadd.mhachain.dc.tau_attack = [0.005];
dsc.mha.overlapadd.mhachain.dc.tau_decay = [0.015];
dsc.mha.overlapadd.mhachain.dc.fb = 'fftfilterbank';
dsc.mha.overlapadd.mhachain.dc.chname = 'fftfilterbank_nchannels';
dsc.mha.overlapadd.mhachain.combinechannels.outchannels = num_channels;
dsc.io.in = soundfiles_src{1};
dsc.io.out = '/dev/null';
if ispc()
  [dummy1, filename] = fileparts(dsc.io.in);
  dsc.io.out = fullfile(outdir, [filename, '.temp.wav']);
end

% Start the MHA doing all processing
mha = mha_start;
mha_set(mha,'',dsc);
mha_set(mha,'cmd','prepare');
samples_delay = compute_ola_delay(dsc.mha.overlapadd.wnd.len, dsc.fragsize, ...
                                  dsc.mha.overlapadd.fftlen);

% Launch fitting GUI
launch_gui = true;
while launch_gui
  mhagui_fitting(mha);
  uiwait;
  clientid = mha_get(mha, 'mha.overlapadd.mhachain.dc.clientid');

  if isempty(clientid)
    answer_id = questdlg('No Client ID found! If you want to try again, please make sure that you created a fit.',
                         'Client ID Error','Try Again','Abort','Try Again');
    switch answer_id
      case 'Try Again'
        continue;
      case 'Abort'
        error('Aborted by user.')
    end
  end
  launch_gui = false;
end

if ~exist([outdir,'/',clientid])
  mkdir([outdir,'/',clientid]);
end

% Check soundfile duration for possible timeout error
all_durations = zeros(numel(soundfiles_src),1);
for idx = 1:numel(soundfiles_src)
  all_durations(idx) = audioinfo(soundfiles_src{idx}).Duration / 60;
end
if any(all_durations > 10)
  new_timeout = str2double(inputdlg(sprintf(['WARNING! Some of your audiofiles are longer than 10 minutes (largest: %.1f min).', ...
                                             char(10), 'Depending on the performance of your computer this could lead ', ...
                                             char(10), 'to a timeout of your openMHA-connection. The current timeout ', ...
                                             char(10), 'is %.1f seconds. You may enter a new timeout in seconds here.'], ...
                                            max(all_durations),mha.timeout), ...
                                    'Possible Timeout Conflict'));
  if ~isempty(new_timeout) && ~isnan(new_timeout) && isreal(new_timeout) && numel(new_timeout) == 1
    mha.timeout = new_timeout;
  end
end


overwrite_all = false;
allow_clipping_all = false;

% Process all files sequentially
w_bar = waitbar(0,'Start processing of your sound files ...');
for soundfile_index = 1:numel(soundfiles_src)

  infile = soundfiles_src{soundfile_index};
  soundfile = soundfiles{soundfile_index};
  outfile = [outdir,'/',clientid,'/',soundfile];
  if ~overwrite_all && exist(outfile)
    answer_outfile = questdlg(sprintf('Soundfile %s already exists, do you want to overwrite it?',outfile),
                              'Output File Conflict', ...
                              'Yes','Yes, for all','No','Yes');
    switch answer_outfile
      case 'Yes'
      case 'Yes, for all'
        overwrite_all = true;
      case 'No'
        continue;
    end
  end
  fprintf('processing file %s for audiogram %s...\n', outfile, clientid);
  pause(0.01); % allow output above to become visible
  [y,fs] = audioread(infile);
  samples_file = size(y,1);
  % file twice, first time-reversed
  y = [flipud(y);y;zeros(samples_delay,num_channels)];
  audiowrite([outdir,'/',clientid,'/input.wav'],y,fs,'BitsPerSample',32);

  % trigger MHA
  mha_set(mha,'cmd','release');
  mha_set(mha,'io.in',[outdir,'/',clientid,'/input.wav']);
  mha_set(mha,'io.out',outfile);
  mha_set(mha,'cmd','start');
  mha_set(mha,'cmd','release');


  % time-reversed part and check for clipping
  [y,fs] = audioread(outfile);
  y = y(1+samples_file+samples_delay:end,:);
  if any(abs(y)>1) && ~allow_clipping_all
    answer_clip = questdlg(sprintf('WARNING! File %s will contain samples > 1. Do you want to write anyway?',outfile),...
                           'Output File Warning',
                           'Yes','Yes, for all','No','Abort','No');
    switch answer_clip
      case 'Yes'
      case 'Yes, for all'
        allow_clipping_all = true;
      case 'No'
        continue;
      case 'Abort'
        break;
    end
  end
  audiowrite(outfile,y,fs,'BitsPerSample',32);
  waitbar(soundfile_index/numel(soundfiles_src),w_bar, ...
          sprintf('%i of %i files processed.',soundfile_index,numel(soundfiles_src)));
end
close(w_bar);
mha_set(mha,'cmd','quit');
