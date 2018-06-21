% MHA_START - start a MHA process
%
% Syntax:
%
% h = mha_start( port, pre_binary, post_binary )
%
% port : MHA configuration port; use [] or 0 for automatic port
% number

% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2005 2006 2007 2008 2009 2011 2013 2014 2015 2017 HörTech gGmbH
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

function h = mha_start( port, pre_binary, post_binary )
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
  acceptor.setSoTimeout(10000);

  my_port = acceptor.getLocalPort();

  command = ...
    {binary, '--port', num2str(port), '--announce', num2str(my_port)};
  command = {pre_binary{:} command{:} post_binary{:}};

  javaruntime = javaMethod('getRuntime', 'java.lang.Runtime');

  % in octave on windows, the directories that octave adds to the PATH
  % can contain libraries that conflict with the libraries MHA is linked
  % with. Temporarily remove these directories from PATH to avoid conflicts.  
  if ispc() && isoctave()
    PATH_backup = getenv('PATH');
    remove_octave_directories_from_windows_path()
  end
  
  % octave 4.0.0 did not translate cellstrings correctly to Java string arrays.
  octave_version = ver('Octave');
  if( isoctave() && strcmp(octave_version.Version,'4.0.0') )
    mhaProcess = javaruntime.exec(strjoin(command,' ')); % workaround
  else
    mhaProcess = javaruntime.exec(command);
  end
  
  % restore PATH
  if ispc() && isoctave()
    setenv('PATH', PATH_backup);
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
  
% Local Variables:
% mode: octave
% indent-tabs-mode: nil
% coding: utf-8-unix
% End:
