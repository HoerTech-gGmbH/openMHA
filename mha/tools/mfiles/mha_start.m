% MHA_START - start a new MHA process (without starting signal processing)
% 
% Location of the mha executable to invoke is taken from global variable
% MHA_INSTALL_DIR or environment variable MHA_INSTALL_DIR. If both are
% empty, then mha must be available on the PATH.
%
% [mha_handle, mha_process] = mha_start( port, pre_binary, post_binary )
%
% All input parameters are optional and may be omitted.
% port :       TCP port that the new MHA should accept configuration commands
%              on; use [] or 0 for automatic port selection (default).
% pre_binary:  Cell array with command line parameters to place before "mha".
%              Use only for debugging, e.g. with valgrind. Default is {}.
% post_binary: Cell array with command line parameters to place after "mha"
%              and other parameters used by mha_start (mha_start uses --port
%              and --announce).  Can be used to provide additional options
%              to the MHA and for initial configuration commands (without
%              spaces). Default is {}.
% mha_handle:  A struct that can be used with other mfile functions like
%              mha_get, mha_set to interact with the newly started MHA instance.
% mha_process: Java Process object that was used to start the new MHA instance.
%              Can be used to read from the MHA process'es standard output
%              and error streams within matlab/octave.
%              Note: mha_process would only be needed for debugging and can
%              be ignored by most users.
%              Note: When using octave 5.2.0 with openjdk 11 (e.g. on
%              ubuntu 20.04), mha_process cannot be returned because of an
%              error in the octave java integration. An empty array is returned
%              instead.  Use octave 5.2.0 with openjdk-8 instead when you need
%              the mha_process object.  (Normally, you don't).

% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2005 2006 2007 2008 2009 2011 2013 2014 2015 2017 HörTech gGmbH
% Copyright © 2020 HörTech gGmbH
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

function [mha_handle, mha_process] = mha_start( port, pre_binary, post_binary )
  mha_process = [];
  mha_handle.host = 'localhost';
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
  mha_handle.timeout = 50;
  
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
  
  octave_version = ver('Octave');
  % octave 4.0.0 did not translate cellstrings correctly to Java string arrays.
  if( isoctave() && strcmp(octave_version.Version,'4.0.0') )
    mha_process = javaruntime.exec(strjoin(command,' ')); % workaround
  % octave 5.2.0 with openjdk11 raised a meaningless error after runtime.exec
  elseif (isoctave() && strcmp(octave_version.Version,'5.2.0'))
    try
      mha_process = javaruntime.exec(command);
    catch exception
      if strcmp(exception.message, ...
                ['[java] java.lang.OutOfMemoryError: unable to create ', ...
                 'native thread: possibly out of memory or ', ...
                 'process/resource limits reached'])
        disp(['expected exception for this octave version, ', ...
              'does not affect functionality'])
      else
        rethrow(exception);
      end
    end
  else
    mha_process = javaruntime.exec(command);
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
  mha_handle.pid = sscanf(char(data), 'pid = %d');
  data = reader.readLine();
  mha_handle.port = sscanf(char(data), 'port = %d');
  reader.close();
  connection.close();
  acceptor.close();
  
% Local Variables:
% mode: octave
% indent-tabs-mode: nil
% coding: utf-8-unix
% End:
