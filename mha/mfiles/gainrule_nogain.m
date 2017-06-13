function sGt = gainrule_nogain( sAud, sCfg )
% GAINRULE_NOGAIN - Prescribe zero dB insertion gain independently
% from audiogram
  ;
  nFreq = length(sCfg.frequencies);
  nLev = length(sCfg.levels);
  sGt = struct;
  for channel='lr'
    sGt.(channel) = zeros(nLev,nFreq);
    sBasIO = struct('gmax_n',40,...
                    'gmax_i',40,...
                    'l_passive',85,...
                    'c_slope',1/4);
    sGt.basilar_io.(channel) = sBasIO;
    sGt.noisegate.(channel).level = -40*ones(size(sCfg.frequencies));
    sGt.noisegate.(channel).slope = ones(size(sCfg.frequencies));
  end
