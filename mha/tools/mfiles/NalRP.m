function g = NalRP(htl,fhtl,f)

%
%specifies hearing-aid REIG (dB)
%according to the NAL-RP prescription,
%for a single AudExcModel object.
%***** Note: Conduction loss not (yet) accounted for *****
%
%References:
%Byrne & Dillon (1986). Ear Hearing 7, 257-265. (NAL-R)
%Byrne & Parkinson, Newall (1990). Ear Hearing 11, 40-49.
%Byrne % Parkinson, Newall (1991). In The Vanderbilt Hearing aid resport
%II, York Press, 295-300. (NAL-RP)
%
%Input:
%aM=    the AudExcModel object
%f=     column vector with frequency samples for given psd.
%psd=   2-dim or 3-dim matrix with power spectrum density values
%       psd(i,j,k)= power density at freq. f(i), time frame j, and channel k.
%       (Not used, as NAL-RP prescribes linear gain.)
%
%Result:
%g=     prescribed REIG (in dB)
%       Prescribed gain is interpolated in log-frequency scale.
%       size(g)== size(psd)
%
%Arne Leijon 2006-06-07 tested
% modified 2007-04-11 Giso Grimm (adapted to HearCom PHS)
%
% This file is part of the HörTech Open Master Hearing Aid (openMHA)
% Copyright © 2007 2009 2011 2013 2015 2017 2018 2020 HörTech gGmbH
%
% openMHA is free software: you can redistribute it and/or modify
% it under the terms of the GNU Affero General Public License as published by
% the Free Software Foundation, version 3 of the License.
%
% openMHA is distributed in the hope that it will be useful,
% but WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
% GNU Affero General Public License, version 3 for more details.
%
% You should have received a copy of the GNU Affero General Public License,
% version 3 along with openMHA.  If not, see <http://www.gnu.org/licenses/>.

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
