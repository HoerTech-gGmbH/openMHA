function sResponse = measure_ha_response( mha )
  sResponse = struct;
  if nargin < 1
    mha = [];
  end
  mha = mha_ensure_mhahandle(mha);
  global mha_basic_cfg;
  mha_get_basic_cfg_network(mha);
  % number of averages:
  navg = 8;
  % test signal level:
  level = 65;
  % maximal difference to maximum:
  deltaG = 20;
  % slope in dB/octave:
  Fslope = 12;
  mha_set(mha, [mha_basic_cfg.base.gain,'.min'],-100);
  mha_set(mha, [mha_basic_cfg.base.gain,'.max'],10);
  mha_set(mha, [mha_basic_cfg.base.gain,'.gains'],[-100,-100,0]);
  uiwait(msgbox('Please remove the hearing aids from the KEMAR'));
  IRS = struct;
  [IRS.direct,tmp] = ...
      getresponse( mha, mha_basic_cfg.base.response_averager, navg, level );
  mha_set(mha, [mha_basic_cfg.base.gain,'.gains'],[0,0,0]);
  uiwait(msgbox('Please insert the hearing aids into the KEMAR'));
  [IRS.leakage,IRS.processed] = ...
      getresponse( mha, mha_basic_cfg.base.response_averager, navg, level );
  H = struct;
  H.insertion = ...
      realfft(IRS.processed(:,3:4))./realfft(IRS.direct(:,3:4));
  H.feedback = ...
      realfft(IRS.processed(:,1:2))./realfft(IRS.leakage(:,1:2));
  H.leakage = ...
      realfft(IRS.leakage(:,3:4))./realfft(IRS.direct(:,3:4));
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
    H.correction(idx,k) = ...
	10.^(0.05*Fslope*log2(max(eps,(idx-1)/(Fcrit_low(k)-1))))*H.correction(Fcrit_low(k),k);
    idx = [Fcrit_high(k):size(H.correction,1)];
    H.correction(idx,k) = ...
	10.^(0.05*Fslope*log2((Fcrit_high(k)-1)./(idx-1)))*H.correction(Fcrit_high(k),k);
    vDiff = diff(H.correction(:,k));
    fSample = find(diff(sign(vDiff)));
    fSample = [fSample;sqrt(fSample(1:end-1).*fSample(2:end))];
    fSample = unique(round(max(125,[0;fSample])));
    sTmp = struct;
    sTmp.f = (fSample-1)';
    sTmp.g = (20*log10(H.correction(fSample,k)))';
    sTmp.id = sNames{k};
    if ~isempty(sNames{k})
      sCorr{end+1} = sTmp;
    end
  end
  sResponse.H = H;
  sResponse.IRS = IRS;
  sResponse.corr = sCorr;
  
function [irs1,irs2,snr] = getresponse( mha, addr, n, level )
  fs = mha_get(mha,[addr,'.mhaconfig_in.srate']);
  len = 2^15/fs;
  mha_set(mha,[addr,'.schroederlen'],round(len*fs));
  mha_set(mha,[addr,'.level'],level);
  mha_set(mha,[addr,'.cycles'],n);
  pause((n+1)*len+0.5);
  %mha_get(mha,[addr,'.response_ready'])
  mha_set(mha,[addr,'.update'],'commit');
  snr = mha_get(mha,[addr,'.response_snr']);
  rg = [0,2048];
  mha_set(mha,[addr,'.cutirs_range'],rg+0);
  irs1 = mha_get(mha,[addr,'.cutirs_irs'])';
  mha_set(mha,[addr,'.cutirs_range'],rg+round(fs/2));
  irs2 = mha_get(mha,[addr,'.cutirs_irs'])';
  irs1 = smoothirs(irs1);
  irs2 = smoothirs(irs2);
  snr_threshold = 10;
  if any(snr<snr_threshold)
    sSNR = sprintf('%1.1f ',snr);
    sSNR(end) = '';
    sIDX = sprintf('%d ',find(snr<snr_threshold));
    sIDX(end) = '';
    sMsg = sprintf('Low SNR in channels %s! (SNR = [%s] dB)\n%s',...
		   sIDX,sSNR,...
		   'Please check your connections or reduce the noise level!');
    warning(sMsg);
  end
  
function irs = smoothirs( irs )
  len1 = 180;
  len2 = 60;
  wnd1 = hanning(2*len1);
  wnd1 = wnd1(len1+1:end);
  wnd2 = hanning(2*len2);
  wnd2 = wnd2(1:len2);
  wnd = [wnd1;zeros(size(irs,1)-length(wnd1)-length(wnd2),1);wnd2];
  wnd = repmat(wnd,[1,size(irs,2)]);
  [tmp,idx] = max(abs(irs));
  wnd = circshift(wnd,idx);
  irs = irs .* wnd;
  irs = zeropad(irs,44100);
  

