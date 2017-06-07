function sResponse = ha_response_createcorr( sResponse )
% maximal difference to maximum:
  deltaG = 20;
  % slope in dB/octave:
  Fslope = 12;
  %
  H = sResponse.H;
  [Gmax,Fmax] = max(20*log10(abs(H.insertion(1:4000,:))));
  [Gmax4k,Fmax4k] = max(20*log10(abs(H.insertion(4001:end,:))));
  H.correction = 1./abs(H.insertion);
  %sCorr = struct('f',[],'g',[],'id','');
  sCorr = {};
  sNames = inputdlg( {'channel 1','channel 2'},'Response ID');
  for k=1:2
    idx_low = ...
	find(20*log10(abs(H.insertion(1:Fmax(k),k)))<Gmax(k)-deltaG);
    if isempty(idx_low)
      idx_low = 1;
    end
    Fcrit_low(k) = max(idx_low);
    idx_high = ...
	find(20*log10(abs(H.insertion(Fmax4k(k)+1:end,k)))<Gmax4k(k)-deltaG);
    if isempty(idx_high)
      idx_high = size(H.insertion,1)-Fmax(k);
    end
    Fcrit_high(k) = Fmax4k(k)+min(idx_high);
    %
    idx = [1:Fcrit_low(k)];
    if length(idx) > 1
      H.correction(idx,k) = ...
	  10.^(0.05*Fslope*log2(max(eps,(idx-1)/(Fcrit_low(k)-1))))*H.correction(Fcrit_low(k),k);
    end
    idx = [Fcrit_high(k):size(H.correction,1)];
    if length(idx) > 1
      H.correction(idx,k) = ...
	  10.^(0.05*Fslope*log2((Fcrit_high(k)-1)./(idx-1)))*H.correction(Fcrit_high(k),k);
    end
    vDiff = diff(H.correction(:,k));
    fSample = find(diff(sign(vDiff)));
    fSample = [fSample;...
	       sqrt(fSample(1:end-1).*fSample(2:end));...
	       1000*2.^[-10:1/3:10]'];
    fSample = round(1000*2.^(round(12*log2(0.001*max(1,fSample)))/12));
    fSample = unique(max(125,min(size(H.correction,1),fSample)));
    sTmp = struct;
    sTmp.f = (fSample-1)';
    sTmp.g = (20*log10(H.correction(fSample,k)))';
    sTmp.id = sNames{k};
    if ~isempty(sNames{k})
      sCorr{end+1} = sTmp;
    end
  end
  sResponse.corr = sCorr;
