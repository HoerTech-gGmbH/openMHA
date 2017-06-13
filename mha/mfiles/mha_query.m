function s = mha_query( handle, base, query )
% MHA_QUERY - query information about a MHA configuration entry
%
% Usage:
%
% s = mha_query( handle, base, query )
%
% handle : MHA handle (containing hostname and port number)
% base : configuration entry name
% query : query to be sent to MHA
%
% s : MHA response to query
%
% (c) 2006 Universitaet Oldenburg
% Author: Giso Grimm
  ;
  s = mhactl_wrapper( handle, [base '?' query] );
