function sGt = gainrule_plack2004_loudfit_65( sAud, sCfg )
% GAINRULE_PLACK2004_40_65 - Gain prescription rule after Plack (2004)
%
% Outer and middle ear filter based on ISO 226:2003 equal-loudness
% contour for 80 phon.
%
% Amplification of 65 dB LTASS is matched to loudfit rule by linear
% shift of the gain.
%
% Author: Giso Grimm
% Date: May 2009

  sGt = gainrule_plack2004( sAud, sCfg );
  sGain = target_fun( sGt, sCfg, 65 );
  sGtCorr = gainrule_loudfit( sAud, sCfg );
  sGainCorr = target_fun(sGtCorr, sCfg, 65 );
  for channel='lr'
    sGain.(channel) = sGain.(channel) - sGainCorr.(channel);
    sGain.(channel) = repmat(sGain.(channel),[length(sCfg.levels),1]);
    sGt.(channel) = min(33,sGt.(channel) - sGain.(channel));
    sGt.noisegate.(channel).level = -40*ones(size(sCfg.frequencies));
    sGt.noisegate.(channel).slope = ones(size(sCfg.frequencies));
  end
  
function sGain = target_fun( sGt, sCfg, vLev )
  sGain = struct;
  vLT = LTASS_combined(sCfg.frequencies);
  for ch='lr'
    Gt = sGt.(ch);
    vGain = zeros(length(vLev),length(sCfg.frequencies));
    for k=1:length(vLev)
      for kf=1:length(sCfg.frequencies)
	vGain(k,kf) = interp1(sCfg.levels,Gt(:,kf),vLev(k)-vLT(kf),'linear','extrap');
      end
    end
    sGain.(ch) = vGain;
  end
  
function L = LTASS_combined( vF )
  vFin = [63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300, 8000, 10000, 12500, 16000];
  vLTASS_combined = 70-[38.6, 43.5, 54.4, 57.7, 56.8, 60.2, 60.3, 59.0, ...
		    62.1, 62.1, 60.5, 56.8, 53.7, 53.0, 52.0, 48.7, 48.1, 46.8, 45.6, 44.5, 44.3, 43.7, 43.4, 41.3, 40.7];
  L = interp1(log(vFin),vLTASS_combined,log(vF),'linear','extrap');
