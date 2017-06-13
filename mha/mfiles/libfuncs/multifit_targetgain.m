function sTarget = multifit_targetgain( sFit, vLevel )
  sTarget = struct;
  [vLTASS, vF] = private_LTASS_combined();
  sTarget.levels = vLevel;
  sTarget.f = vF;
  sTarget.inlevel = repmat(vLTASS,[length(vLevel),1])+ ...
      repmat(vLevel(:),[1,length(vLTASS)]);
  sGt = multifit_apply_noisegate( sFit.gaintable );
  fGt = sGt.frequencies(:)';
  fGt = [min(fGt(1)/2,min(vF)),fGt,max(2*fGt(end),max(vF))];
  lGt = sGt.levels(:)';
  lGt = [lGt(1)-100,lGt,lGt(end)+100];
  for ch='lr'
    Gt = sGt.(ch);
    Gt = [Gt(:,1),Gt,Gt(:,end)];
    xGt = zeros(2,size(Gt,2));
    for k=1:length(fGt)
      xGt(1,k) = interp1(lGt(2:end-1),Gt(:,k),lGt(1),'linear','extrap');
      xGt(2,k) = interp1(lGt(2:end-1),Gt(:,k),lGt(end),'linear','extrap');
    end
    Gt = [xGt(1,:);Gt;xGt(2,:)];
    sTarget.(ch).outlevel = zeros(size(sTarget.inlevel));
    for k=1:size(sTarget.inlevel,1)
      sTarget.(ch).outlevel(k,:) = ...
	  interp2(fGt,lGt,Gt,vF,sTarget.inlevel(k,:),'linear')+sTarget.inlevel(k,:);
    end
  end

function [vLTASS_combined, vF] = private_LTASS_combined
  vF = [63, 80, 100, 125, 160, 200, 250, 315, 400, 500, 630, 800, ...
	1000, 1250, 1600, 2000, 2500, 3150, 4000, 5000, 6300, 8000, ...
	10000, 12500, 16000];
  vLTASS_combined = [38.6, 43.5, 54.4, 57.7, 56.8, 60.2, 60.3, 59.0, ...
		     62.1, 62.1, 60.5, 56.8, 53.7, 53.0, 52.0, 48.7, ...
		     48.1, 46.8, 45.6, 44.5, 44.3, 43.7, 43.4, 41.3, 40.7]-70;
  