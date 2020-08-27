function [r,state] = mhactl_java(handle, eval, query)
% [r,state] = mhactl_java(handle, eval, query)
% Use java class to communicate with MHA over TCP.
% Can manage several connections to different MHA instances in parallel.
% handle: struct with fields host and port and optionally timeout in seconds.
% eval:   String 'eval'. Historic reasons.
% query:  Cell array of strings to send to MHA over TCP.
% r:      Cell array of strings with responses.
% state:  Vector of result codes: 0 for success, 1 for failure.
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2011 2013 2014 2017 2020 HörTech gGmbH

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

% Array of existing connections that may be reused for the current query.
% A connection is identified by the triple host, portname, timeout
persistent connections;
persistent last_mhactl_java_invocation;

if isempty(last_mhactl_java_invocation)
  last_mhactl_java_invocation = now();
end

if isequal(handle, 'retire_connections')
  retire_connections(connections(2,:));
  r=[];state=[];connections={};return
end

if (now() - last_mhactl_java_invocation) * 24 * 3600 > 1.2
  retire_connections(connections(2,:));
  connections={};
end

last_mhactl_java_invocation = now();
  
if ~isequal(eval, 'eval')
  error('second parameter to mhactl_java has to be string ''eval''.')
end

if isempty(connections)
  connections = {};
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
  connection = javaObject('de.hoertech.mha.control.Connection');
  connection.setTimeout(handle.timeout * 1000);
  connection.setAddress(handle.host, handle.port);
  connections = [connections, {handle; connection}];
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
    c{1}.connect(false);
  end
