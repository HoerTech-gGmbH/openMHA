function [timestamps1, timestamps2] = ...
  measure_timestamps(sampling_rate, buffer_size, channels, ioconfig, duration, load_factor)
% Records two series of timestamps during mha processing.
% A mha is started, loads plugins timestamp (twice), cpuload, acsave.
% io configuration is determined by ioconfig.
%
% Parameters:
% sampling_rate  MHA sampling rate in Hz. For some audio backends, has to match
%                the fixed sampling rate of the audio backend.
% buffer_size    MHA fragsize. For some audio backends, has to match the
%                fixed buffer size of the audio backend.
% channels       MHA nchannels_in. For some audio backends ...(see above).
% ioconfig       structure containing complete MHA io configuration. Has to
%                include iolib assignment and all necessary configuration
%                for that iolib.
% duration       Duration of timestamp recording in seconds.
% load_factor    factor to set in plugin cpuload. Higher factors increase CPU
%                load.

  mha = mha_start
  mha_set(mha, 'srate', sampling_rate);
  mha_set(mha, 'fragsize', buffer_size);
  mha_set(mha, 'nchannels_in', channels);
  mha_set(mha, 'mhalib', 'mhachain');
  mha_set(mha, 'mha.algos', '[timestamp:timestamps1 cpuload timestamp:timestamps2 acsave]');
  mha_set(mha, 'mha.cpuload.factor', load_factor);
  mha_set(mha, 'mha.cpuload.use_sine', 'yes');
  mha_set(mha, '', ioconfig);
  mha_set(mha, 'cmd','prepare');
  mha_set(mha, 'mha.acsave.reclen', duration);
  mha_set(mha, 'mha.acsave.fileformat', 'mat4');
  mha_set(mha, 'mha.acsave.name','measure_mha_latency.mat');
  mha_set(mha, 'cmd','start');
  pause(duration + 0.25);
  mha_set(mha, 'cmd','stop');
  mha_set(mha, 'mha.acsave.flush','yes');
  mha_set(mha, 'cmd','quit');
  load('measure_mha_latency.mat');
