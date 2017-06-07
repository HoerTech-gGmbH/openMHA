function data = modellhg_get_maxgain( mha )
  global mha_basic_cfg;
  mha = mha_ensure_mhahandle( mha );
  mha_get_basic_cfg_network( mha );
  mha = mha_basic_cfg.mha;
  base = mha_basic_cfg.base;
  bBypass = mha_get(mha,[base.fbc_wave,'.bypass']);

  sCfgNoise = mha_get(mha,base.noise,'writable');
  mha_set(mha,[base.noise,'.lev'],65);
  mha_set(mha,[base.noise,'.mode'],'replace');
  mha_set(mha,[base.fbc_wave,'.bypass'],1);
  mha_set(mha,'sleep',1);
  mha_set(mha,[base.blms,'.reset'],1);
  mha_set(mha,'sleep',6);
  sData.srate = mha_get(mha,[mha_basic_cfg.base.blms,'.mhaconfig_out.srate']);
  sData.fftlen = mha_get(mha,[mha_basic_cfg.base.blms,'.fftlen']);
  sData.spec = mha_get(mha,[mha_basic_cfg.base.blms,'.filterw'])';
  sData.dbgain = 20*log10(abs(sData.spec));
  sData.freq = ([1:floor(sData.fftlen/2+1)]'-1)*sData.srate/ ...
      sData.fftlen;
  mha_set(mha,[base.fbc_wave,'.bypass'],bBypass);
  mha_set(mha,base.noise,sCfgNoise);
  figure
  axes('OuterPosition',[0,0,1,0.8]);
  h = plot(sData.freq,sData.dbgain,'linewidth',2);
  set(h(1),'Color',[0,0,1]);
  set(h(2),'Color',[1,0,0]);
  ylim([-60 0]);
  xlim([125 sData.srate/2]);
  set(gca,'XScale','log','XTick',1000*2.^[-2:4]);
  xlabel('frequency / Hz');
  ylabel('feedback path gain / dB');
  grid on;
  idx = strmatch('dc_simple',mha_basic_cfg.all_id_plugs(:,2),'exact');
  cf = 1000*2.^[-2:3];
  ef = 1000*2.^[-2.5:3.5];
  ef_int = min(max(1,floor((ef/sData.srate)*sData.fftlen)),size(sData.dbgain,1));
  g = zeros(size(cf,2),size(sData.dbgain,2));
  for k=1:size(g,1)
    for ch=1:size(g,2)
      g(k,ch) = min(-max(sData.dbgain(ef_int(k):ef_int(k+1),ch)+6),60);
    end
    th = ...
	text(cf(k),5,sprintf('%gdB',round(g(k,1))),...
	     'Color',[0,0,1]);
    th(end+1) = ...
	text(cf(k),10,sprintf('%gdB',round(g(k,2))),...
	     'Color',[1,0,0]);
    unit = '';
    if cf(k)>=1000
      unit = 'k';
    end
    th(end+1) = ...
	text(cf(k),15,sprintf('%g%s',(0.001*(cf(k)>=1000)+(cf(k)<1000))*cf(k),unit),...
	     'Color',[0,0,0]);
    set(th,'HorizontalAlignment','center',...
	   'FontWeight','bold','FontSize',14);
  end
  data = round([cf;g']);

  if ~isempty(findobj('tag','finetuning_frame'))
    global finetuning_sFT;
    if ~isequal(finetuning_sFT.f,cf)
      error('mismatching finetuning frequencies');
    end
    finetuning_sFT.l.maxgain = round(g(:,1)');
    finetuning_sFT.r.maxgain = round(g(:,2)');
    libfinetuning();
    finetuning.updategui();
    finetuning.triggerguicallback();
  end
  