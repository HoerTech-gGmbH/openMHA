function sGt = gainrule_CR2_NALRP( sAud, sCfg )
  sAud = audprof2aud( sAud );
  warning('this module contains hand-coded parameters! Do not use!');
  sCfg
  nLev = length(sCfg.levels);
  g_L = NalRP(sAud.l.htl,sAud.frequencies,sCfg.frequencies)';
  g_R = NalRP(sAud.r.htl,sAud.frequencies,sCfg.frequencies)';
  sGt = struct;
  sGt.l = repmat(g_L,[nLev 1]);
  sGt.r = repmat(g_R,[nLev 1]);
  sGt.compression = struct;
  % centre frequencies:
  cf = [123.380196 163.471375 208.124634 257.8591 313.252991 374.950287 ...
	443.668335 520.205933 605.452942 700.400452 806.152283 ...
	923.937927 1055.12671 1201.24377 1363.98792 1545.25122 ...
	1747.14075 1972.00366 2222.45483 2501.40503 2812.0979 ...
	3158.14551 3543.57031 3972.85425 4450.9873 4983.52832 ...
	5576.66797 6237.30371 6973.11475 7792.65625];
  % LTASS correction for narrow band levels:
  cLTASS = [-11.5922632 -11.057415 -9.61773682 -9.86552143 -10.6104679 ...
	    -9.44122791 -8.83611298 -9.29962158 -10.4765053 -12.3506994 ...
	    -14.6303072 -16.6554012 -18.1358128 -18.7297878 -19.3246155 ...
	    -19.9757271 -21.308075 -22.9907074 -23.776722 -24.2319355 ...
	    -24.8387814 -25.505928 -26.1589794 -26.756052 -27.3213978 ...
	    -27.8530693 -28.0781555 -28.2206993 -28.4660606 -28.7403412];
  % microphone noise floor in narrow band levels:
  L_noise = [5.19276 6.15587 7.1062 7.9596 8.28572 8.58516 8.96383 ...
	     9.20054 10.4047 11.0281 11.3825 11.833 11.8783 12.6873 ...
	     13.2227 13.589 14.2093 14.614 15.074 15.3235 15.7659 ...
	     16.4543 16.8507 17.5191 18.1405 18.4148 18.992 19.4387 ...
	     19.8577 20.2793];
  
  % kneepoint is at least at noise floor, better at 40dB LTASS:
  l_kneepoint = max(L_noise,cLTASS+40);
  c_slope = 1/2;
  l_target = 65;
  for channel='lr'
    % get the NAL-RP gain, to be applied at 65dB LTASS:
    g_NALRP = NalRP(sAud.(channel).htl,sAud.frequencies,cf);
    %HTL = max(0,freq_interp(sAud.frequencies,sAud.(channel).htl,cf));
    % gmax is 50% hearing loss:
    gmax = (g_NALRP(:) - (c_slope(:)-1).*(cLTASS(:)+65-l_kneepoint(:)))';
    % limit gainloss to [0,40]:
    %gmax = max(0,min(40,gmax));
    % configure compressor parameter:
    l_par = struct('gain',gmax,...
		   'l_kneepoint',l_kneepoint,...
		   'c_slope',c_slope);
    sGt.compression.(channel) = l_par;
  end
  
  
function g = NalRP(htl,fhtl,f)
  f=f(:);%must be column vector
  htl = htl(:);
  fhtl = fhtl(:);

  fPrescr=[250 500 1000 1500 2000 3000 4000 6000]';
  ifP500=2;ifP1000=3;ifP2000=5;%indices into fPrescr
  gainZero=[-17 -8 1 1 -1 -2 -2 -2]';%NAL speech-equalizing REIG for HTL=0
  gainFactor=0.31;%times hearing loss, at each freq
  gainFactorM=0.15;%times 3-freq average hearing loss
  gainFactorP=0.20;%times 3-freq average part exceeding 60 dB HL
  ;
  %gain increases as 46% of HTL below 60, and 66% above 60 dB HL
  hSevere2000=[95 100 105 110 115 120]';%severe loss at 2000 Hz
  gSevere=[4 3 0 -1 -2 -2 -2 -2 ;...%Byrne et al(1991), Fig 9.
	   6 4 0 -2 -3 -3 -3 -3 ;...
	   8 5 0 -3 -5 -5 -5 -5;...
	   11 7 0 -3 -6 -6 -6 -6;...
	   13 8 0 -4 -8 -8 -8 -8;...
	   15 9 0 -5 -9 -9 -9 -9];
  %corrections for high loss at 2000 Hz

  %extrapolate
  fhtl=[1;fhtl;20000];
  %extrapolate
  htl=[htl(1);htl;htl(end)];

  hLoss=interp1(log(fhtl),htl,log(fPrescr));
  hLoss3FA=mean(hLoss([ifP500,ifP1000,ifP2000]));%500, 1000, 2000 Hz
  gPrescr=gainFactor*hLoss+gainFactorM*hLoss3FA+gainZero;%NAL-R original

  if hLoss3FA>60%NAL-RP correction for severe loss
    gPrescr=gPrescr+gainFactorP*(hLoss3FA-60);
  end;
  if hLoss(ifP2000)>=95%add extra NAL-RP correction
    gPrescr=gPrescr+(interp1(hSevere2000,gSevere,hLoss(ifP2000)))';
  end;
  gPrescr=max(0,gPrescr);%no negative REIG

  fPrescr=[realmin;50;fPrescr;10000;100000];%extend for extrapolation
  gPrescr=[0;0;gPrescr;0;0];%extend for extrapolation
  g=interp1(log(fPrescr),gPrescr,log(f));%single column vector
