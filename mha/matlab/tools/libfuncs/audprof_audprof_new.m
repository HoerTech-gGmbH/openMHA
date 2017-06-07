function sAud = audprof_audprof_new( client_id )
  if nargin < 1
    client_id = '';
  end
  sAud = struct;
  sAud.id = '';
  sAud.client_id = client_id;
