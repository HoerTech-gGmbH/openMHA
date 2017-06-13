function mha = mha_ensure_mhahandle( mha, port )
% MHA_ENSURE_MHAHANDLE - convert struct or string into valid MHA
% handle with default values.
%
% Usage:
% mha = mha_ensure_mhahandle( mha )
% mha = mha_ensure_mhahandle( host [, port ] )
%
% (c) 2007-2008 HoerTech gGmbH
  if nargin < 1
    mha = [];
  end
  if isempty(mha)
    mha = struct;
  end
  if ischar(mha)
    if nargin >= 2
      mha = struct('host',mha,'port',port);
    else
      mha = struct('host',mha);
    end
  end
  def_mha = struct;
  def_mha.host = 'localhost';
  def_mha.port = 33337;
  mha = merge_structs(mha,def_mha);