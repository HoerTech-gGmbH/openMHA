function s = mha_getinfo( handle, base )
% MHA_GETINFO - show information about a MHA configuration entry
%
% Usage:
%
% s = mha_getinfo( handle, base )
%
% handle : MHA handle (containing hostname and port number)
% base : configuration entry name
%
% s : structure with fields containing information. Which field is
%     available depends on the type of MHA configuration
%     entry. Possible fields are 'val', 'help', 'type', 'perm',
%     'entries', and 'range'
%
% (c) 2005 Universitaet Oldenburg
% Author: Giso Grimm
  ;
  s.cmds = mha_mha2matlab('vector<string>',...
			  mhactl_wrapper(handle,[base '?cmds']));
  mycmds = {'val','help','type','perm','entries','range','id'};
  availcmds = {};
  cmds = {};
  for cmd=mycmds
    if any(strmatch(['?' cmd{:}],s.cmds))
      availcmds = {availcmds{:} [base '?' cmd{:}]};
      cmds = {cmds{:} cmd{:}};
    end
  end
  resp = mhactl_wrapper(  handle, availcmds );
  for k=1:length(resp)
    s = setfield(s, cmds{k}, resp{k});
  end
