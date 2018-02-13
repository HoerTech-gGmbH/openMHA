function [send_data, t] = connect_to_webapp(varargin)
% Connect to the visualisation web-app
%
% [SEND_DATA, T] = CONNECT_TO_WEBAPP will open a TCP/IP connection to
% 'localhost' on port 9990.  It returns a function handle SEND_DATA, with
% which you can send data to the server listening there, as well as the
% underlying connection object T, whose type depends on the MATLAB
% installation (see USE_NATIVE_TCP below).
%
% CONNECT_TO_WEBAPP(HOST) will open a connection to HOST.
%
% CONNECT_TO_WEBAPP(HOST, PORT) will open a connection to HOST:PORT.
%
% CONNECT_TO_WEBAPP(..., USE_NATIVE_TCP) will prefer the "native" TCP/IP
% implementation for connecting to the web-app if USE_NATIVE_TCP is true (the
% default).  In MATLAB this is the tcpip class from the Instrument Control
% Toolbox, in which case the type of T is "tcpip".  In Octave this is the
% TCP/IP implementation from the instrument-control package, in which case the
% type of T is "octave_tcp".  If USE_NATIVE_TCP is false, or no "native" TCP/IP
% implementation could be found, it will use the java_tcp() function that is
% part of this project, and the type of T is "sun.nio.ch.SocketChannelImpl".
% The reason why this option exists is that the "native" implementation might
% be lacking in comparison to the Java implementation, thus potentially
% warranting an explicit override.  For example, the tcp() function from the
% Octave instrument-control package (version 0.2.1) does not appear to support
% IPv6 or name resolution.
%
% See also TCPIP, JAVA_TCP.
%
% Input parameters
% ----------------
%
%      host:            The host to connect to (optional).
%      port:            The port to connect to (optional).
%      use_native_tcp:  Specifies how to create TCP/IP connection (see
%                       description above; optional).
%
% Output parameters
% -----------------
%
%      send_data:   A handle to a function that allows you to send data to the
%                   server listening on HOST:PORT.  It receives a single
%                   argument, which must be a vector.
%      t:           The object that handles the (already opened) connection.

is_octave = logical(exist('OCTAVE_VERSION', 'builtin'));

p = inputParser();
if is_octave
    p = p.addOptional('host', '127.0.0.1', @ischar);
    p = p.addOptional('port', 9990, @isnumeric);
    p = p.addOptional('use_native_tcp', true, @islogical);
    p = p.parse(varargin{:});
else
    p.addOptional('host', 'localhost', @ischar);
    p.addOptional('port', 9990, @isnumeric);
    p.addOptional('use_native_tcp', true, @islogical);
    p.parse(varargin{:});
end

use_native_tcp = p.Results.use_native_tcp;

if use_native_tcp && ~is_octave && exist('tcpip', 'class')
    t = tcpip(p.Results.host, p.Results.port);
    fopen(t);
    send_fun = @(data_str) fwrite(t, data_str);
elseif use_native_tcp && is_octave && exist('tcp', 'file')
    t = tcp(p.Results.host, p.Results.port);
    send_fun = @(data_str) tcp_write(t, data_str);
else
    [send_fun, t] = java_tcp(p.Results.host, p.Results.port);
end

send_data = @(data) fun(send_fun, data);

end

function fun(send_fun, data)
    p = inputParser();
    if exist('OCTAVE_VERSION', 'builtin')
        p = p.addRequired('data', @isvector);
        p = p.parse(data);
    else
        p.addRequired('data', @isvector);
        p.parse(data);
    end

    send_fun(sprintf('%s\n', num2str(p.Results.data(:).')));
end

