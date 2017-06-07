function modellhg_get_levels(mha)
  if nargin < 1
    mha = [];
  end
  mha = mha_ensure_mhahandle( mha );
  if ~isempty(findobj('Tag','modellhg_mhagui_bandlevelmeter'))
    close(findobj('Tag','modellhg_mhagui_bandlevelmeter'))
  end
  libmhagui();
  fh = mhagui.figure('MHA input levels',...
			 'modellhg_mhagui_bandlevelmeter',...
			 [480 510]);
  freq = mha_get(mha,'mha.splcalib.plug.multibandcompressor.cf');
  nfreq = length(freq);
  l = get_levels( mha, nfreq );
  axLevel = axes('Tag','meteraxes_level','Unit','pixels',...
		 'Position',[60 40 400 400]);
  hold off;
  hLI = plot(freq,l(1,:),'b-o','linewidth',2);
  hold on;
  hRI = plot(freq,l(2,:),'r-o','linewidth',2);
  grid on
  set(axLevel,'YLim',[0 120]);
  ylabel('DC input level / dB');
  vHTextLevelL = [];
  vHTextLevelR = [];
  
  lgmm = round(log10([min(freq) max(freq)]));
  lgmm = [lgmm(1):(lgmm(2)-lgmm(1))/8:lgmm(2)];
  flab = 10.^(lgmm);
  flab = round(flab./(10.^(round(lgmm)-1))).*(10.^(round(lgmm)-1));
  set([axLevel],...
      'XScale','log','XLim',[min(freq)/1.2 max(freq)*1.2],...
      'XTick', 1000*2.^[-4:4]);
  xlabel('Frequency / Hz');
  pause(0.8);
  while( ishandle(fh) )
    l = get_levels( mha, nfreq );
    set(hLI,'YData',l(1,:));
    set(hRI,'YData',l(2,:));
    drawnow;
    pause(0.8);
  end
  
function l = get_levels( mha, nfreq )
  algo = mha_get(mha,'mha.splcalib.plug.multibandcompressor.plug.altplugs.select');
  
  %mha_get(mha,['mha.splcalib.plug.multibandcompressor.plug.altplugs.',algo])
  l = mha_get(mha,['mha.splcalib.plug.multibandcompressor.plug.altplugs.',algo,'.level_in']);
  nch = length(l)/nfreq;
  l = reshape(l,[nfreq nch])';

