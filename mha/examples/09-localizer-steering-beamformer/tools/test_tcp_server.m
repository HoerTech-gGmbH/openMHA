function test_tcp_server(varargin)
% Test the visualisation web-app
%
% TEST_TCP_SERVER(DELAY, MODEL_LENGTH) sends test data with MODEL_LENGTH
% elements to the server (i.e., visualisation_web/tcp_server.py) every DELAY
% seconds.
%
% TEST_TCP_SERVER(..., WEIGHT) will filter the data with a simple low-pass
% filter with with coefficients a1 = 1-WEIGHT and b0 = WEIGHT.  This parameter
% is only used when 'type' is 'rand' (see below).
%
% TEST_TCP_SERVER(..., HOST) will open a connection to HOST.
%
% TEST_TCP_SERVER(..., PORT) will open a connection on the port PORT.
%
% TEST_TCP_SERVER(..., 'TYPE', TYPE) will create data of a particular form.
% When TYPE is 'rand', the data sent to the server is uniformly distributed
% random and low-pass filtered (see option WEIGHT above).  When TYPE is
% 'hann', a Hann window is circularly shifted every update, which is
% particularly suitable for testing the polar plot.
%
% See also CONNECT_TO_WEBAPP.
%
% Input parameters
% ----------------
%
%      delay:           The interval between data transfers.
%      model_length:    The length of the data to send.
%      weight:          A parameter to a low-pass filter (see the description
%                       above; optional).
%      host:            The host to connect to (optional).
%      port:            The port to connect to (optional).
%      type:            The type of data to send ('rand' or 'hann'; optional).
%                       The default is 'rand'.

p = inputParser();
if exist('OCTAVE_VERSION', 'builtin')
    p = p.addRequired('delay', @(x) isscalar(x) & (x > 0));
    p = p.addRequired('model_length', @(x) isscalar(x) & (x > 0));
    p = p.addOptional('weight', 0.1, @(x) isscalar(x) & (x > 0));
    p = p.addOptional('host', '127.0.0.1', @ischar);
    p = p.addOptional('port', 9990, @isnumeric);
    p = p.addParamValue('type', 'rand', @ischar);
    p = p.parse(varargin{:});
else
    p.addRequired('delay', @(x) isscalar(x) & (x > 0));
    p.addRequired('model_length', @(x) isscalar(x) & (x > 0));
    p.addOptional('weight', 0.1, @(x) isscalar(x) & (x > 0));
    p.addOptional('host', 'localhost', @ischar);
    p.addOptional('port', 9990, @isnumeric);
    p.addParamValue('type', 'rand', @ischar);
    p.parse(varargin{:});
end

params = p.Results;

addpath('visualisation_web/');
[send_data, ~] = connect_to_webapp(params.host, params.port);

if strcmp(params.type, 'rand')
    data = struct('new_data', [], ...
                  'old_data', zeros(1, params.model_length));
    data_fun = @(data) rand_fun(data, send_data, params);
elseif strcmp(params.type, 'hann')
    win_len = round(params.model_length/4);
    data = zeros(1, params.model_length);
    data(1:win_len) = hann(win_len);
    data_fun = @(data) hann_fun(data, send_data);
else
    error(['Invalid type "' params.type '"']);
end

while 1
    data = data_fun(data);

    pause(params.delay);
end

end

function data = rand_fun(data, send_data, params)
    new_data = rand(1, params.model_length);
    data.new_data = new_data*params.weight + data.old_data*(1 - params.weight);
    data.old_data = data.new_data;
    send_data(data.new_data);
end

function data = hann_fun(data, send_data)
    send_data(data);
    data = circshift(data, [0, 1]);
end

