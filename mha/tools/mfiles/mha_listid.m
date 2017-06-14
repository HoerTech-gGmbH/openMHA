function csBase = mha_listid( mhah )
% MHA_LISTID - return list of plugin IDs
%
% Usage:
% csBase = mha_listid( mhah )
%
% mha: MHA handle
% csBase : cell array of size N x 2,
% first column contains MHA configuration entries, second column
% the corresponding plugin ID
%
% Author: Giso Grimm
% Date: 7/2007
  ;
  s = mha_query( mhah, '', 'listid' );
  csBase = cell([0 2]);
  while length(s)
    [str,s] = strtok(s,sprintf('\n'));
    if length(str)==0
      str = s;
      s = '';
    end
    idx = strfind(str,' = ');
    if isempty(idx)
      error('malformed query response');
    end
    base = str(1:idx-1);
    id = str(idx+3:end);
    csBase(end+1,:) = {base,id};
  end
