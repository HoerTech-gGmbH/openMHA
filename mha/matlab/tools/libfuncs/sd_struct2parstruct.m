function s = sd_struct2parstruct( sIn )
% struct2parstruct - convert field names of structure into param
% control structure
%
% Usage:
% s = sd_struct2parstruct( sIn )
% 
% Fieldnames of sIn are converted into content of 's.fields',
% contents of sIn are converted into content of 's.values'.
%
% Example:
% sIn.par1 = {'apple','orange','lemmon'}
% sIn.par2 = [1:3]
% s = sd.struct2parstruct( sIn )
%
  s = struct;
  s.fields = fieldnames(sIn)';
  s.values = {};
  for k=1:numel(s.fields)
    s.values{k} = sIn.(s.fields{k});
  end