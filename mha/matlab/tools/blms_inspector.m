function blms_inspector( mha )
  if nargin < 1
    mha = [];
  end
  mha = mha_ensure_mhahandle( mha );
  global mha_basic_cfg;
  mha_get_basic_cfg_network( mha );
  %mhagui_generic(mha,mha_basic_cfg.base.blms);
  %mhagui_generic(mha,mha_basic_cfg.base.fbc_wave);
  pos = get(0,'ScreenSize');
  sz = [800,700];
  fh = figure('Name','BLMS feedback control panel',...
	      'NumberTitle','off','MenuBar','none',...
	      'tag','blms_control_panel',...
	      'Position',[round(pos(3:4)/2-sz/2),sz]);
  
  uicontrol('style','pushbutton','String','RESET',...
	    'Position',[20,590,200,80],'Callback',@blms_reset,...
	    'FontSize',14);
  uicontrol('style','text','String','delay correction',...
	    'Position',[20,540,200,30],...
	    'HorizontalAlignment','left',...
	    'FontSize',14);
  uicontrol('style','edit','tag','blms_edit_delay',...
	    'Position',[75,500,90,40],'BackgroundColor',ones(1,3),...
	    'Callback',@blms_edit_delay,'UserData',0,...
	    'String',num2str(mha_get(mha,[mha_basic_cfg.base.fbc_wave,'.shift_irs'])));
  uicontrol('style','pushbutton','String','-',...
	    'Position',[20,500,40,40],'Callback',@blms_edit_delay,...
	    'UserData',-10);
  uicontrol('style','pushbutton','String','+',...
	    'Position',[180,500,40,40],'Callback',@blms_edit_delay,...
	    'UserData',10);
  uicontrol('style','pushbutton','String','Get/Set maxgain',...
	    'Position',[20,400,200,80],'Callback',@blms_call_getmaxgain,...
	    'UserData',mha);
  uicontrol('style','pushbutton','String','Level meter',...
	    'Position',[20,300,200,80],'Callback',@blms_call_meter,...
	    'UserData',mha);
  %k = 0;
  %for fn={'fbc_wave','blms','noise','sine','mastergain','altplugs','altconfig'}
  %  sfn = fn{:};
  %  if isfield(mha_basic_cfg.base,sfn)
  %    uicontrol('style','pushbutton','String',['''',sfn,''' panel'],...
  %		'Position',[20,440-k,200,40],'Callback',@blms_guigen,...
  %		'UserData',mha_basic_cfg.base.(sfn),...
  %		'FontSize',14);
  %    k = k+60;
  %  end
  %end
  uicontrol('style','pushbutton','String','Close',...
	    'Position',[20,20,200,40],'Callback','close(gcf);',...
	    'FontSize',14);
  
  sData = blms_get_data;
  %
  % plot feedback path
  %
  ax1 = subplot(3,1,1);
  vH_spec = plot(sData.freq,sData.dbgain);
  ylim([-50 10]);
  xlim([250 sData.srate/2]);
  set(ax1,'XScale','log','XTick',1000*2.^[-2:4]);
  xlabel('frequency / Hz');
  ylabel('feedback path gain / dB');
  grid on;
  
  ax2 = subplot(3,1,2);
  vH_gd = plot(sData.freq,sData.groupdelay);
  ylim([0 10]);
  xlim([250 sData.srate/2]);
  set(ax2,'XScale','log','XTick',1000*2.^[-2:4]);
  xlabel('frequency / Hz');
  ylabel('group delay / ms');
  grid on;
  
  %
  % plot irs
  %
  ax3 = subplot(3,1,3);
  vH_irs = plot(sData.irs);
  xlim([1,sData.fftlen]);
  %ylim([-1 1]);
  xlabel('filter taps');
  grid on;
  
  set(ax1,'OuterPosition',[0.3,2/3,0.7,1/3]);
  set(ax2,'OuterPosition',[0.3,1/3,0.7,1/3]);
  set(ax3,'OuterPosition',[0.3,0,0.7,1/3]);
  drawnow;
  while ishandle(fh)
    sData = blms_get_data;
    for k=1:length(vH_spec)
      set(vH_spec(k),'XData',sData.freq,'YData',sData.dbgain(:,k));
      set(vH_gd(k),'XData',sData.freq,'YData',sData.groupdelay(:,k));
      set(vH_irs(k),'YData',sData.irs(:,k));
    end
    pause(0.4);
  end
  
function sData = blms_get_data
  global mha_basic_cfg;
  mha = mha_basic_cfg.mha;
  sData = struct;
  sData.srate = mha_get(mha,[mha_basic_cfg.base.blms,'.mhaconfig_out.srate']);
  sData.fftlen = mha_get(mha,[mha_basic_cfg.base.blms,'.fftlen']);
  sData.spec = mha_get(mha,[mha_basic_cfg.base.blms,'.filterw'])';
  sData.dbgain = 20*log10(abs(sData.spec));
  sData.freq = ([1:floor(sData.fftlen/2+1)]'-1)*sData.srate/ ...
      sData.fftlen;
  sData.irs = mha_get(mha,[mha_basic_cfg.base.blms,'.filterirs'])';
  sData.irs = sData.irs / size(sData.freq,1);
  spec = specsmoother(sData.spec);
  sData.groupdelay = ...
      angle(spec(2:end,:)./spec(1:end-1,:));
  idx = find(sData.groupdelay<0);
  %sData.groupdelay(idx) = sData.groupdelay(idx) + 2*pi;
  sData.groupdelay = ...
      (1000/(2*pi))*sData.groupdelay ./ ...
      repmat(diff(sData.freq),[1,size(sData.spec,2)]);
  sData.groupdelay = [sData.groupdelay(1,:);sData.groupdelay];

function blms_reset(varargin)
  set(gcbo,'Enable','off');
  drawnow;
  global mha_basic_cfg;
  mha = mha_basic_cfg.mha;
  base = mha_basic_cfg.base;
  sCfgNoise = mha_get(mha,base.noise,'writable');
  bBypass = mha_get(mha,[base.fbc_wave,'.bypass']);
  mha_set(mha,[base.noise,'.lev'],65);
  mha_set(mha,[base.noise,'.mode'],'replace');
  mha_set(mha,[base.fbc_wave,'.bypass'],1);
  mha_set(mha,'sleep',1);
  mha_set(mha,[base.blms,'.reset'],1);
  mha_set(mha,'sleep',3);
  mha_set(mha,[base.fbc_wave,'.bypass'],bBypass);
  mha_set(mha,base.noise,sCfgNoise);
  set(gcbo,'Enable','on');

function sOut = specsmoother( sIn )
  sOut = sIn;
  for k=1:size(sIn,2)
    irs = conjrealifft((sIn(:,k)));
    [tmp,idxmax] = max(max(abs(irs),[],2));
    wnd = hann(round(0.5*size(irs,1)));
    wnd = [wnd;zeros(size(irs,1)-size(wnd,1),1)];
    wnd = circshift(wnd,-round(0.25*size(irs,1)));
    wnd = circshift(wnd,idxmax);
    irs = irs.*wnd;
    sOut(:,k) = conj(realfft(irs));
  end

function blms_edit_delay( varargin )
  global mha_basic_cfg;
  mha = mha_basic_cfg.mha;
  base = mha_basic_cfg.base;
  delta = get(gcbo,'UserData');
  edh = findobj('tag','blms_edit_delay');
  val = str2num(get(edh(1),'String'));
  val = max(0,val+delta);
  set(edh(1),'String',num2str(val));
  mha_set(mha,[base.fbc_wave,'.shift_irs'],val);
  
function blms_guigen( varargin )
  global mha_basic_cfg;
  mhagui_generic(mha_basic_cfg.mha,get(gcbo,'UserData'));
  
function blms_call_getmaxgain(varargin)
  modellhg_get_maxgain(get(gcbo,'UserData'));

function blms_call_meter(varargin)
  modellhg_get_levels(get(gcbo,'UserData'));
