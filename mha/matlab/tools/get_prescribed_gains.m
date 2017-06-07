function s = get_prescribed_gains( hostname, port )
% GET_PRESCRIBED_GAINS - Read the currently configured gains from a MHA
%
% Usage:
% s = get_prescribed_gains( [ hostname [, port ] ] )
%
% hostname : Hostname or IP address of MHA host (default: 'localhost')
% port     : Port number of MHA host (default: 33337)
%
% s : structure containing the complete configuration of dynamic
% compression and finetuning plugin (if configured in the MHA). See
% documentation of dynamic compression plugin for details on the
% fields.
%
% If this function is called without an output argument, a summary
% is displayed in the console.
%
% Author: Giso Grimm
% Date: 11/2007
  global mha_basic_cfg;
  if nargin < 1
    hostname = 'localhost';
  end
  if nargin < 2
    port = 33337;
  end
  if isstruct(hostname)
    mha = hostname;
  else
    mha = struct('host',hostname,'port',port);
  end
  %
  mha_get_basic_cfg_network( mha );

  sResult = struct;
  csFields = {'dc_hearcom',...
	      'dc_simple',...
	      'finetuning'};
  for k=1:length(csFields)
    if isfield(mha_basic_cfg.base,csFields{k})
      sResults.(csFields{k}) = mha_get(mha_basic_cfg.mha, ...
				       mha_basic_cfg.base.(csFields{k}));
    end
  end
  if nargout > 0
    s = sResults;
  else
    if isfield(sResults,'dc_hearcom')
      nbands = length(sResults.dc_hearcom.cf);
      nchannels = length(sResults.dc_hearcom.g50)/nbands;
      disp('Client ID:');
      disp(['  "',sResults.dc_hearcom.clientid,'"']);
      disp('Gain rule:');
      disp(['  "',sResults.dc_hearcom.gainrule,'"']);
      disp('Center frequencies / Hz:')
      disp(sprintf('  %g',sResults.dc_hearcom.cf));
      disp('Gains at 50 dB input level / dB:');
      if nchannels == 2
	disp(['  L: ',sprintf('%5.1f ',sResults.dc_hearcom.g50(1:nbands))]);
	disp(['  R: ',sprintf('%5.1f ',sResults.dc_hearcom.g50(nbands+[1:nbands]))]);
      else
	error('not implemented yet');
      end
      disp('Gains at 80 dB input level / dB:');
      if nchannels == 2
	disp(['  L: ',sprintf('%5.1f ',sResults.dc_hearcom.g80(1:nbands))]);
	disp(['  R: ',sprintf('%5.1f ',sResults.dc_hearcom.g80(nbands+[1:nbands]))]);
      else
	error('not implemented yet');
      end
      disp('Current input level / dB:');
      if nchannels == 2
	disp(['  L: ',sprintf('%5.1f ',sResults.dc_hearcom.level(1:nbands))]);
	disp(['  R: ',sprintf('%5.1f ',sResults.dc_hearcom.level(nbands+[1:nbands]))]);
      else
	error('not implemented yet');
      end
      disp('Current gain (without finetuning) / dB:');
      if nchannels == 2
	disp(['  L: ',sprintf('%5.1f ',sResults.dc_hearcom.gain(1:nbands))]);
	disp(['  R: ',sprintf('%5.1f ',sResults.dc_hearcom.gain(nbands+[1:nbands]))]);
      else
	error('not implemented yet');
      end
    end
    if isfield(sResults,'finetuning')
      disp('Finetuning gains / dB:');
      if sResults.finetuning.nchannels == 2
	disp(['  L: ',sprintf('%5.1f ',sResults.finetuning.gains(1,:))]);
	disp(['  R: ',sprintf('%5.1f ',sResults.finetuning.gains(2,:))]);
      else
	error('not implemented yet');
      end
    end
  end
