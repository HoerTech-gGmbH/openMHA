function [cProf,sJack] = mha_profiling_get( mhah )
% MHA_PROFILING_GET - get profiling information from a MHA.
%
% Usage:
% cProf = mha_profiling_get( mhah );
%
% mhah : MHA handle
% cProf: cell array with configuration path, profiling structure in
%        each row
% 
% Author: Giso Grimm
% Date: 4/2009
  ;
  cProf = mha_listid( mhah );
  idx = strmatch('chain_profiler',cProf(:,2)', 'exact' );
  idx_jack = strmatch('MHAIOJack',cProf(:,2)', 'exact' );
  if ~isempty( idx_jack )
    sJack = mha_get(mhah,[cProf{idx_jack,1},'.state']);
  else
    sJack = struct;
  end
  cProf = cProf(idx,:);
  for k=1:size(cProf,1)
    cProf{k,2} = mha_get(mhah,cProf{k,1},'monitor');
  end
