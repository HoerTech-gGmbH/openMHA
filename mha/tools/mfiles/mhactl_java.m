function [r,state] = mhactl_java(handle, eval, query)

% Array of existing connections that may be reused for the current query.
% A connection is identified by the triple host, portname, timeout
persistent connections;

if isequal(handle, 'retire_connections')
  retire_connections(connections(2,:));
  r=[];state=[];return
end

if ~isequal(eval, 'eval')
  error('second parameter to mhactl_java has to be string ''eval''.')
end

if isempty(connections)
  connections = {};
  if exist('OCTAVE_VERSION','builtin') > 0
				% Octave does not exit properly if one of the
				% SwitchOffDelay timeout threads
				% is still running. So exit all threads when
				% octave exits.
    atexit('mhactl_java_retire_connections');
  end
end

% add field default values if fields not present
if ~isfield(handle,'timeout')
  global mhactl_timeout;
  if isempty(mhactl_timeout)
    mhactl_timeout = 50;
  end
  handle.timeout = mhactl_timeout;
end
if ~isfield(handle,'host')
  handle.host = 'localhost';
end
if ~isfield(handle,'port')
  handle.port = 33337;
end

% search for existing connection object
connection = [];
for c = 1:size(connections,2)
  if structequal(handle, connections{1,c},{'host','port','timeout'})
    connection = connections{2,c};
  end
end
if isempty(connection) % No matching existing connection, create new
    connection = javaObject('de.hoertech.mha.control.Connection',...
			    handle.host, handle.port, 1.2);
  connections = [connections, {handle; connection}];
  connection.setTimeout(handle.timeout * 1000);
end

% Communicate
r = {};
state = [];
for q = 1:length(query)
  q = query{q};
  response = [];
  try
    response = connection.parse(q);
    sResponse = char(response.toString());
    r = [r, {sResponse}];
    state = [state, ~response.getSuccess()];
  catch
    e = lasterror();
    r = [r, {e.message}];
    state = [state, 1];
  end
end

% Check struct equality, but only in the given fields
function eq = structequal(s1, s2, fields)
  eq = true;
  for f = fields;
      f = f{1};
      if isfield(s1,f) ~= isfield(s2,f)
          eq = false; return;
      end
      if isfield(s1,f)
          if ~isequal(s1.(f), s2.(f))
              eq = false; return;
          end
      end
  end

function retire_connections(connections)
  for c = connections
    c{1}.retire();
  end
