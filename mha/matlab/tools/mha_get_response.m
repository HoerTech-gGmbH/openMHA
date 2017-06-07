function [irs,fs,sResp] = mha_get_response( mha, id, inout, b_plot )
  if (nargin < 1) || isempty(mha)
    mha = struct('host','localhost','port',33337);
  end
  if nargin < 2
    id = [];
  end
  if nargin < 3
    inout = 'out';
  end
  if nargin < 4
    b_plot = 0;
  end
  if isempty(id)
    id = 'flat';
  end
  global mha_basic_cfg;
  mha_get_basic_cfg_network( mha );
  if ~isfield(mha_basic_cfg.base,'transducers')
    error('No plugin ''transducers'' found, please check your MHA configuration');
  end
  cRDB = mha_get_response_db(mha);
  cfdb = libconfigdb();
  [sResp,idx] = cfdb.smap_get( cRDB, id );
  if isempty(idx)
    error(sprintf('Response ''%s'' not found.',id));
  end
  fs = ...
      mha_get(mha,...
	      [mha_basic_cfg.base.transducers,...
	       '.calib_',inout,'.config.srate']);
  fragsize = ...
      mha_get(mha,...
	      [mha_basic_cfg.base.transducers,...
	       '.calib_',inout,'.config.fragsize']);
  if std(sResp.g) == 0
    irs = 10.^(0.05*mean(sResp.g));
  else
    vFreqHz = 0:round(fs/2);
    vGaindB = zeros(size(vFreqHz));
    fc = 100;
    idx_log = fc+1:length(vFreqHz);
    idx_lin = 1:fc;
    vfr = sResp.f;
    vgr = sResp.g;
    for k=1:5
      vfr = [vfr(1)/2,vfr];
      vgr = [vgr(1)-(vgr(2)-vgr(1)),vgr];
      vfr(end+1) = vfr(end)*2;
      vgr(end+1) = vgr(end);
    end
    intp_mode = 'spline';
    f_lin = [fc sResp.f(1)];
    g_lin = [interp1(log(vfr),vgr,log(fc),intp_mode,'extrap'),...
	     sResp.g(1)];
    vGaindB(idx_log) = ...
	interp1(log(vfr),vgr,log(vFreqHz(idx_log)),...
		intp_mode,'extrap');
    vGaindB(idx_lin) = ...
	interp1(f_lin,g_lin,vFreqHz(idx_lin),...
		intp_mode,'extrap');
    vGain = 10.^(0.05*vGaindB');
    irs = smoothspec( vGain, fragsize+1 );
  end
  if b_plot
    ax = findobj('tag','response_ax');
    if isempty(ax)
      ax = axes('tag','response_ax');
    end
    axes(ax);
    h = plot(sResp.f,sResp.g,'r--o',...
	     vFreqHz,20*log10(vGain),'b-',...
	     vFreqHz,20*log10(abs(realfft(zeropad(irs,fs)))),'k-');
    set(h(3),'linewidth',2);
    set(gca,'XLim',[50 fs/2],'XScale','log');
    title(id);
    grid on
  end
