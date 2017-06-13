function [answer, info] = mha_get(handle, field, perm )
% Retrieve value of some parser variable from a MHA server.
%
% [answer, info] = mha_get(mha_handle, field [, perm ] )
%
% Input arguments:
% - handle     : Handle structure of the MHA server, with the
%                fields 'host' and 'port'.
% - field      : Variable name of MHA element to be returned.
%
% - perm       : Get only variables with permission perm (default: '')
%
% Return value:
% - answer     : Contents of 'field' in a Matlab-representation.
% - info       : Help comment of the variable 'field'.
%
% A running MHA server is required.
%
% (c) 2005 Universitaet Oldenburg, Germany
%          Author: Tobias Herzke, Giso Grimm
%
  % logmsg('ENTERING MHA_GET');
  if nargin < 3
    perm = [];
  end
  if nargin < 2
    field = '';
  end

  s.type = mha_query( handle, field, 'type');
  
  if isequal(s.type,'parser')
    answer = struct;
    s.entries = mha_query( handle, field, 'entries');
    entries = mha_mha2matlab( 'vector<string>', s.entries );
    for entr=entries
      nfield = [field '.' entr{:}];
      perm_ok = 1;
      if ~isempty( perm )
	cmds = mha_mha2matlab('vector<string>',mhactl_wrapper( handle, [nfield '?cmds']));
	if strmatch( '?perm', cmds )
	  mhaperm = mhactl_wrapper( handle, [nfield '?perm']);
	  perm_ok = strmatch(mhaperm,perm);
	end
      end
      if perm_ok
	answer = setfield(answer, entr{:}, mha_get(handle,nfield,perm));
      end
    end
  else
    s.val = mha_query( handle, field, 'val');
    answer = mha_mha2matlab( s.type, s.val );
  end
  % logmsg('LEAVING MHA_GET');
