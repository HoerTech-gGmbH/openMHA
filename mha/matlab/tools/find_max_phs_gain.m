function ret = find_max_phs_gain( varargin )
  if mod(length(varargin),2)
    error('Even number of input arguments expected');
  end
  sPar.freq = [250,500,1000,1500,2000,3000,4000,6000];
  sPar.gain = [53,57,54,41,59,49,31,32];
  sPar.audfreq = [250,500,1000,1500,2000,3000,4000,6000];
  sPar.gainrule = @gainrule_NALRP;
  sPar.levels = [50 80];
  for k=1:2:length(varargin)
    sIdent = varargin{k};
    sVal = varargin{k+1};
    if ~ischar(sIdent)
      error('Identifier must be of type string');
    end
    sPar.(sIdent) = sVal;
  end
  vL = 120*ones(size(sPar.audfreq));
  vHL = fminsearch( @(x) errfun(x,sPar), vL, optimset('MaxFunEvals',10000) );
  [err,vGain,vIdx,vHL] = errfun( vHL, sPar );
  sGainrule = func2str(sPar.gainrule);
  if strncmp( sGainrule,'gainrule_',9)
    sGainrule(1:9) = [];
  end
  disp(sprintf('Maximal hearing loss based on gainrule "%s":', ...
	       sGainrule));
  sHL = sprintf('%1.1f ',vHL);
  sHL(end) = [];
  disp(sprintf('  HL = [%s]',sHL));
  sHL = sprintf('%g ',sPar.audfreq);
  sHL(end) = [];
  disp(sprintf('  audfreq = [%s]\n',sHL));
  sGains = sprintf('%1.1f ',vGain);
  sGains(end) = [];
  disp(sprintf('Prescribed gains:\n  G = [%s]\n',sGains));
  sGains = sprintf('%1.1f ',sPar.gain);
  sGains(end) = [];
  disp(sprintf('Nominal maximum gains:\n  G = [%s]\n',sGains));
  sGains = sprintf('%1.1f ',sPar.gain-vGain);
  sGains(end) = [];
  disp(sprintf('Difference to nominal maximum gains:\n  Dg = [%s]\n',sGains));
  if nargout > 0
    ret = vHL;
  end
  
function [err,vGain,vIdx,vL] = errfun( vL, sPar )
  vF = [250,500,1000,1500,2000,3000,4000,6000];
  vG = [53,57,54,41,59,49,31,32];
  %vL = max(0,vL);
  %vL = min(120,vL);
  
  sCfg.frequencies = sPar.freq;
  sCfg.levels = sPar.levels;
  sAud.l.htl = vL;
  sAud.r.htl = vL;
  sAud.frequencies = sPar.audfreq;
  sGain = sPar.gainrule(sAud,sCfg);
  [vGain,vIdx] = max(sGain.l);
  err = sum((vGain-vG).^2);
  if any(vL < 0)
    err = err*2;
  end