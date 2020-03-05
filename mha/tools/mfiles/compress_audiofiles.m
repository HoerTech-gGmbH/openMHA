% Matlab skript performs hearing aid dynamic compression on a set of audio files

% This setup is restricted to work with audiofiles with 44100Hz sampling rate
sampling_rate = 44100;
num_channels = 2;
amplification_headroom = 50;

%% Load files from folder
[soundfiles,audiofiles_directory] = uigetfile('*.wav','Select .wav Files','MultiSelect', 'on');

% If only one file is selected, the variable soundfiles will be of type
% char instead of a cell array -> convert soundfiles to cell array
if ~iscell(soundfiles)
  soundfiles = {soundfiles};
end
if ~isequal(class(audiofiles_directory),'char')
  error('Aborted.');
end

soundfiles_src = fullfile(audiofiles_directory, soundfiles);

%% Check sound files
if numel(soundfiles) == 0
  error(['No sound files in directory ', audiofiles_directory])
end

input_levels_re_fullscale = zeros(numel(soundfiles),num_channels);

for i=1:numel(soundfiles_src)
  soundfile = soundfiles_src{i};
  [y,fs] = audioread(soundfile);
  if sampling_rate ~= fs
    error('Sound file %s has sampling rate %d, expected %d', ...
          soundfile, fs, sampling_rate);
  end
  if size(y,2) ~= num_channels
    error('sound files need %d channels, this file differs: %s', num_channels, soundfile);
  end
  if size(y,1) == 0
    error('sound file is empty: %s', soundfile);
  end
  
  input_levels_re_fullscale(i,:) = 10*log10(mean(y.^2));
end

%% Processing

% Get and check peaklevel
peaklevel = inputdlg('Peaklevel / dB SPL:', 'Type in Peaklevel');
peaklevel = str2double(peaklevel);
if (~isequal(class(peaklevel),'double')) || (numel(peaklevel) ~= 1) || ~isreal(peaklevel)
  error('Expected a single, real number for peaklevel');
end

fprintf(' LEFT RIGHT The sound files have the following input levels / dB SPL:\n');
for index = 1:size(input_levels_re_fullscale,1)
  l = peaklevel + input_levels_re_fullscale(index,:);
  fprintf('%5.1f %5.1f   %s\n', l(1,1), l(1,2), soundfiles{index});
end
fprintf('\n');
fprintf('Output peaklevel will be %.1f dB (for %.1f dB amplification headroom)\n', ...
        peaklevel + amplification_headroom, amplification_headroom);
input('CHECK all levels above and proceed only if they are correct.');

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
  dsc.io.out = 'NUL';
end

% Start the MHA doing all processing
mha = mha_start;
mha_set(mha,'',dsc);
mha_set(mha,'cmd','prepare');
samples_delay = compute_ola_delay(dsc.mha.overlapadd.wnd.len, dsc.fragsize, ...
                                  dsc.mha.overlapadd.fftlen);

mhagui_fitting(mha);
uiwait;
clientid = mha_get(mha, 'mha.overlapadd.mhachain.dc.clientid');



if exist(['./',clientid]) == 7 % 7 means is_directory
  % TODO: We may want to ask here if existing files should be overwritten

  fprintf('A folder for client id %s exists already, will not overwrite\n', ...
         clientid);
  
else
  fprintf('Creating folder %s for storing output files.\n', clientid);
  mkdir(clientid);

  % Process all files sequentially
  for soundfile_index = 1:numel(soundfiles_src)
    infile = soundfiles_src{soundfile_index};
    soundfile = soundfiles{soundfile_index};
    outfile = [clientid,'/',soundfile];
    fprintf('processing file %s for audiogram %s...\n', outfile, clientid);
    pause(0.01); % allow output above to become visible
    [y,fs] = audioread(infile);
    samples_file = size(y,1);
    % process file twice, first time-reversed
    y = [flipud(y);y;zeros(samples_delay,num_channels)];
    audiowrite([clientid,'/input.wav'],y,fs,'BitsPerSample',32);

    % trigger MHA
    mha_set(mha,'cmd','release');
    mha_set(mha,'io.in',[clientid,'/input.wav']);
    mha_set(mha,'io.out',outfile);
    mha_set(mha,'cmd','start');
    mha_set(mha,'cmd','release');

    % Remove time-reversed part and check for clipping
    [y,fs] = audioread(outfile);
    y = y(1+samples_file+samples_delay:end,:);
    if any(abs(y)>1)
      fprintf('output file %s contains samples > 1, aborting!\n', ...
              outfile);
      break;
    end
    audiowrite(outfile,y,fs,'BitsPerSample',32);
  end
end

mha_set(mha,'cmd','quit');
