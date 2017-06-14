function h = mha_start( port, pre_binary, post_binary )
% MHA_START - start a MHA process
%
% Syntax:
%
% h = mha_start( port, pre_binary, post_binary )
%
% port : MHA configuration port; use [] or 0 for automatic port
% number
%
  h.host = 'localhost';
  next_task = 'open serversocket';
  if nargin < 1
    port = [];
  end
  if nargin < 2
    %pre_binary = {'/usr/local/bin/valgrind','--tool=memcheck','--log-file=mha_start_valgrind'};
    pre_binary = {};
  end
  if nargin < 3
    %post_binary = {'--log=/proc/self/fd/2'};
    post_binary = {};
  end
  global mhalogfile;
  if ~isempty(mhalogfile)
    post_binary = [post_binary, {['--log=' mhalogfile]}];
  end

  if isempty(port)
     port = 0;
  end
  thispath = fileparts(mfilename('fullpath'));
  global MHA_INSTALL_DIR;
  if ~isempty(MHA_INSTALL_DIR)
    binary = [MHA_INSTALL_DIR,'/mha'];
  else
    MHA_INSTALL_DIR = getenv('MHA_INSTALL_DIR');
    if ~isempty(MHA_INSTALL_DIR)
      binary = [MHA_INSTALL_DIR,'/mha'];
    else
      binary = 'mha';
    end
  end
  h.timeout = 50;
  
  serverport = 0;
  backlog = 1;
  address = javaMethod('getByName','java.net.InetAddress','127.0.0.1');
  acceptor = javaObject('java.net.ServerSocket',serverport, backlog, address);
  acceptor.setSoTimeout(3000);

  my_port = acceptor.getLocalPort();

  command = ...
    {binary, '--port', num2str(port), '--announce', num2str(my_port)};
  command = {pre_binary{:} command{:} post_binary{:}};
  javaruntime = javaMethod('getRuntime', 'java.lang.Runtime');
  octave_version = ver('Octave');
  if( ~isempty(octave_version) && ...
      strcmp(octave_version.Version,'4.0.0') )
    mhaProcess = javaruntime.exec(strjoin(command,' '));
  else
    mhaProcess = javaruntime.exec(command);
  end
  connection = acceptor.accept();
  reader = javaObject('java.io.BufferedReader', ... 
                      javaObject('java.io.InputStreamReader', ...
                                 connection.getInputStream()));
  data = reader.readLine();
  h.pid = sscanf(char(data), 'pid = %d');
  data = reader.readLine();
  h.port = sscanf(char(data), 'port = %d');
  reader.close();
  connection.close();
  acceptor.close();
  
