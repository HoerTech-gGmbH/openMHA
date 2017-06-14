function sClient = audprof_client_new( client_id )
% return a new (empty) client
  if nargin < 1
    client_id = '';
  end
  sClient = struct;
  sClient.audprofs = cell([2,0]);
  sClient.firstname = '';
  sClient.lastname = '';
  sClient.birthday = '1900-01-01';
  sClient.id = client_id;
