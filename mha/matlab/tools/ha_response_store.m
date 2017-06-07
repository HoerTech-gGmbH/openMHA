function ha_response_store( sResponse, fname )
% STORE_HA_RESPONSE - store a Hearing Aid correction in mhacontrol database
%
% Usage:
% store_ha_response( sResponse [, fname ] )
%
% sResponse : structure containing a member "corr", array of receiver
%             correction with members "id" (identifier), "f" (sample
%             frequencies in Hz) and "g" (corresponding gains)
% fname     : file name of database file (file save dialog if
%             omitted)
%
% Author: Giso Grimm
% Date: 7/2009
  %
  if nargin < 2
    [fname,pname] = ...
	uigetfile({'mhacfg_*.mat','MHA control configuration'},...
		  'Save as');
    if ~(isequal(fname,0) || isequal(pname,0))
      fname = fullfile(pname, fname);
    else
      fname = '';
    end
  end
  cfdb = libconfigdb();
  if ~isempty(fname);
    rdb = cfdb.readfile(fname,'response_db',cell(2,0))
    for k=1:length(sResponse.corr)
      sCorr = sResponse.corr{k};
      disp(sprintf('Adding %s...',sCorr.id));
      rdb = cfdb.smap_set(rdb,sCorr.id,sCorr);
    end
    cfdb.writefile(fname,'response_db',rdb);
  else
    rdb = cell(2,0);
    for k=1:length(sResponse.corr)
      sCorr = sResponse.corr{k};
      disp(struct2s( sCorr ))
    end
  end

function s = struct2s( str )
  s = 'struct(';
  for fn=fieldnames(str)'
    sVal = num2str(str.(fn{:}));
    s = sprintf('%s''%s'',[%s],',s,fn{:},sVal);
  end
  s(end) = [];
  s = [s,')'];