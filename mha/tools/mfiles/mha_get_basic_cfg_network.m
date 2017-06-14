function s = mha_get_basic_cfg_network( mha )
% MHA_GET_BASIC_CFG_NETWORK - Gather some basic information on a
% MHA process via the network
%
% Usage:
% s = mha_get_basic_cfg_network( mha );
%
% mha : MHA handle (optional; default: localhost/33337)
% s   : structure containing information fields
  
  if nargin < 1
    mha = struct('host','localhost','port',33337);
  end
  global mha_basic_cfg;
  if isstruct(mha_basic_cfg) && ...
	isfield(mha_basic_cfg,'mha') && ...
	isequal(mha_basic_cfg.mha,mha) && ...
	strcmp(mha_basic_cfg.instance,mha_get(mha,'instance'))
    s = mha_basic_cfg;
    return
  end
  s = struct;
  s.mha = mha;
  [s.base,s.all_id_plugs] = mha_findid(mha);
  for cIO={'in','out'}
    sIO = cIO{:};
    s.nch.(sIO) = mha_get(s.mha,['nchannels_',sIO]);
    s.names.(sIO) = {};
    for k=1:s.nch.(sIO)
      s.names.(sIO){k} = sprintf([sIO,'_%d'],k);
    end
    if isfield(s.base,'MHAIOJack')
      jnames = mha_get(s.mha,[s.base.MHAIOJack,'.names_',sIO]);
      if length(jnames) == length(s.names.(sIO))
	s.names.(sIO) = jnames;
      end
    end
    s.side.(sIO) = zeros(1,s.nch.(sIO));
    csSide = {'left','right'};
    for k=1:s.nch.(sIO)
      for kSide=1:length(csSide)
	if ~isempty(strfind(s.names.(sIO){k},csSide{kSide}))
	  s.side.(sIO)(k) = kSide;
	end
      end
    end
  end
  if isfield(s.base,'transducers')
    nch = struct;
    nch.in = ...
	length(mha_get(s.mha,[s.base.transducers, ...
		    '.calib_in.rmslevel']));
    nch.out = ...
	length(mha_get(s.mha,[s.base.transducers, ...
		    '.calib_out.rmslevel']));
    if ~isequal(nch,s.nch)
      s.base = rmfield(s.base,'transducers');
      warning('Plugin transducers unusable (mismatching channel dimensions)');
    end
  end
  [s.cfgname,s.ininame,s.instance] = mha_create_configname( mha );
  mha_basic_cfg = s;
  
function [cfgname,ininame,instance] = mha_create_configname( mha )
% MHA_CREATE_CONFIGNAME - create a configuration file name from
% hostname, port and instance name
%
% Usage:
% [cfgname,ininame,instance] = mha_create_configname( mha )
%
% Author: Giso Grimm
% Date: 7/2007
  ;
  instance = mha_get(mha,'instance');
  cfgname = ...
      filter_string(sprintf('mhacfg_%s_%d_%s',mha.host,mha.port,instance));
  cfgname = [cfgname,'.mat'];
  ininame = ...
      filter_string(sprintf('%s_ini',instance));
  ininame = [ininame,'.mat'];

function s = filter_string( s )
  s = lower(s);
  s(find(s<'0')) = '_';
  s(find((s>'9').*(s<'_'))) = '_';
  s(find((s>'_').*(s<'a'))) = '_';
  s(find(s>'z')) = '_';
