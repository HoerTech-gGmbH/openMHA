function mha = mha_setup( mha_description, mha_startup_cmds, hostname, port )
% mha = mha_setup( mha_description, mha_startup_cmds, hostname, port )
%
% starts a new mha process and returns a
% communication handle struct to it.
% contents of handle:
% tcp.cmd.port   tcp server port for parser commands
% tcp.cmd.host   ip host for parser commands port
% success_prompt parser prompt when successful
% failure_prompt parser prompt when cmd failed
% nchannels_in   number of input channels
% nchannels_out  number of output channels
% wndshift       chunk size in frames
% 
% - if the tcp io library is used (default):
% tcp.sound.port tcp server port for sound data
% tcp.sound.host host for sound data port
% tcp.sound.handle connection handle for sound data
% 

  % logmsg('Entering mha_setup');

  % Fill in some default values if needed.
  if nargin < 2
    mha_startup_cmds = {'cmd=start'};
  end
  if nargin < 3
    hostname = '';
  end
  if nargin < 4
    port = [];
  end

  mha_description = complete_mha_description(mha_description);
  % logmsg('INSIDE mha_setup');

  [mha,mha_description] = start_mha_process(mha_description, hostname, port);

  % logmsg('Process started in mha_setup');

  mha = configure_mha(mha, mha_description);

  start_mha(mha, mha_startup_cmds);
  mha = open_sound_connection(mha, mha_description);
  
  % logmsg('Leaving mha_setup');

function dsc = complete_mha_description(mha_description)
  thispath = fileparts(mfilename('fullpath'));

  dsc = mha_description;
  unit_peaklevel = 20*log10(1/2e-5); % Amplitude 1 == 1 Pa
  
  setenv('MHA_LIBRARY_PATH',my_getenv('MHA_INSTALL_DIR'));
  if ~isfield(dsc, 'srate')
    dsc.srate =  44100;
  end
  if ~isfield(dsc, 'nchannels_in')
    dsc.nchannels_in = 2;
  end
  if ~isfield(dsc, 'mhalib')
    dsc.mhalib = 'concurrentchains';
    dsc.mha.fftname = 'wave2spec';
    dsc.mha.ifftname = 'spec2wave';
  end
  if ~isfield(dsc, 'iolib')
    dsc.iolib = 'MHAIOTCP';
  end
  if ~isfield(dsc,'io')
    dsc.io = struct;
  end

function [mha,dsc] = start_mha_process(dsc, hostname, port)
% create new mha (actually, start process only if hostname is empty)
  mha.success_prompt = sprintf('(MHA:success)\n'); % ?NETPROMPT#\n');
  mha.failure_prompt = sprintf('(MHA:failure)\n'); % !NETPROMPT#\n');
  mha.nchannels_in = dsc.nchannels_in;
  %mha.nchannels_out = dsc.nchannels_out;

  if isempty(hostname)
    mha_child_process = mha_start;
    if ispc  % extra pause for slow windows networking
      pause(3)
    end
    mha.tcp.cmd = mha_child_process; %mhactl(mha_child_process, 'open');
  else
    mha.tcp.cmd.host = hostname;
    mha.tcp.cmd.port = port;
  end
  
  if strfind(dsc.iolib, 'MHAIOTCP')
    mha.tcp.sound.host = mha.tcp.cmd.host;
    if ~isfield(dsc.io, 'port')
      dsc.io.port = 0;
    end
    mha.tcp.sound.port = dsc.io.port;
  end

function mha = configure_mha(mha, dsc)
  try
    % send initial configuration
    % logmsg('configure_mha: sending initial configuration ...');
    mha_set(mha,'',dsc);
    % logmsg('configure_mha: finished sending initial configuration');
    mha.wndshift = mha_get(mha, 'fragsize');
  catch
    err = lasterror;
    %disp(err);
    % logmsg(sprintf('configure_mha: error during configuration: %s', err.message));
    mha_teardown(mha, 'tcpcmd', 'process');
    rethrow(err);
  end
  
function start_mha(mha, mha_startup_cmds)
  % start mha
  try
    [responses] = mhactl_wrapper(mha.tcp.cmd, mha_startup_cmds);
  catch
    err = lasterror;
    try 
        %disp(err);
        mha_teardown(mha, 'tcpcmd', 'process');
    catch
        ;
    end
    rethrow(err);
  end
  
function mha = open_sound_connection(mha, dsc)
  try
    % establish tcp sound connect if needed
    if strfind(dsc.iolib, 'MHAIOTCP')
      if (mha.tcp.sound.port == 0)
        % Port was chosen by mha
        mha.tcp.sound.port = mha_get(mha,'io.port');
      end
      if ispc   % extra pause for slow windows networking
        pause(3);
      end
      mha.tcp.sound.handle = ...
	  mha_tcpsound(mha.tcp.sound.host, mha.tcp.sound.port);
    end
  catch
    err = lasterror;
    disp(err);
    mha_teardown(mha, 'tcpcmd', 'process');
    rethrow(err);
  end

  mha.this_field_was_installed_by_mha_setup = 1;

