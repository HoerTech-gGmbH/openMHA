function [func, t] = java_tcp(host, port)
% Create a TCP connection using Java classes
%
% [SEND_DATA, T] = JAVA_TCP(HOST, PORT) will open a connection to the specified
% HOST on port PORT.  It returns a function handle SEND_DATA, with which you
% can send data to the server listening there, as well as the (already
% connected) underlying Java SocketChannel object T, whose methods may be
% invoked as necessary.
%
% For documentation on the Java objects used here see for example
% http://docs.oracle.com/javase/7/docs/api/java/nio/channels/SocketChannel.html
% and http://docs.oracle.com/javase/7/docs/api/java/nio/ByteBuffer.html.
%
% Input parameters
% ----------------
%
%      host:   The host to connect to.
%      port:   The port to connect to.
%
% Output parameters
% -----------------
%
%      send_data:   A handle to a function that allows you to send data to the
%                   server listening on HOST:PORT.  It receives a single
%                   argument, which must be a string.
%      t:           The Java SocketChannel object that handles the connection, already
%                   connected.

p = inputParser();
if exist('OCTAVE_VERSION', 'builtin')
    p = p.addRequired('host', @ischar);
    p = p.addRequired('port', @isnumeric);
    p = p.parse(host, port);
else
    p.addRequired('host', @ischar);
    p.addRequired('port', @isnumeric);
    p.parse(host, port);
end

% Create the Java SocketChannel object and open a connection to the specified
% host and port.
t = javaMethod('open','java.nio.channels.SocketChannel');
t.configureBlocking(true);
t.connect(javaObject('java.net.InetSocketAddress', p.Results.host, p.Results.port));

func = @(data_str) send(t, data_str);

end

function send(t, data_str)
    p = inputParser();
    if exist('OCTAVE_VERSION', 'builtin')
        p = p.addRequired('data_str', @ischar);
        p = p.parse(data_str);
    else
        p.addRequired('data_str', @ischar);
        p.parse(data_str);
    end

    % wrap the data in a ByteBuffer object
    java_str = javaObject('java.lang.String', data_str);
    buf = javaMethod('wrap', 'java.nio.ByteBuffer', java_str.getBytes());

    % transfer the data
    t.write(buf);
end
