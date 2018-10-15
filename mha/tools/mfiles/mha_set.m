function mha_set( handle, field, value )
% answer = mha_set(handle, field, value)
%
% set MHA variables
%
% handle     : Handle of MHA server, e.g. for mhactl interface a
%              struct containing the fields 'host' and 'port'.
%
% field      : Name of MHA variable or parser.
%
% value      : New value of the MHA variable. If value is a struct,
%              then field has to be the name of a MHA parser.

  assignments = struct2mhacfg( value, [], field );
  mhactl_wrapper(handle,assignments);
