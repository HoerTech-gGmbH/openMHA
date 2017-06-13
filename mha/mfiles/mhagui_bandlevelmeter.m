function mhagui_bandlevelmeter( mha )
  if nargin < 1
    mha = [];
  end
  mha = mha_ensure_mhahandle( mha );
  if ~isempty(findobj('Tag','mhagui_bandlevelmeter'))
    figure(findobj('Tag','mhagui_bandlevelmeter'))
    return;
  end
  sCfg = mha_get_basic_cfg_network( mha );
  cLTASS = [];
  if isfield( sCfg.base, 'fftfilterbank' )
    sFB = mha_get(mha,sCfg.base.fftfilterbank);
    if isfield( sFB, 'cLTASS' )
      cLTASS = sFB.cLTASS;
    end
  end
  if isfield( sCfg.base, 'multibandcompressor' )
    sFB = mha_get(mha,sCfg.base.multibandcompressor);
    if isfield( sFB, 'cLTASS' )
      cLTASS = sFB.cLTASS;
    end
  end
  if isfield(sCfg.base,'dc_simple')
    wnd_name = 'MHA Level and gain meter';
    wnd_h = 610;
    wnd_ofs = 320;
  else
    wnd_name = 'MHA Level meter';
    wnd_h = 290;
    wnd_ofs = 0;
  end
  libmhagui();
  fh = mhagui.figure(wnd_name,'mhagui_bandlevelmeter',[480 wnd_h]);
  tc = mha_get(sCfg.mha,[sCfg.base.transducers,'.calib_in.tau_level']);
  mha_set(sCfg.mha,[sCfg.base.transducers,'.calib_out.tau_level'],tc);
  tc = round(1000*tc);
  uicontrol('Style','Frame','Position',[20 wnd_ofs+10 440 40]);
  uicontrol('Style','Text','Position',[30 wnd_ofs+20 140 20],...
	    'String','Broadband RMS tc:');
  uicontrol('Style','RadioButton','String','125 ms',...
	    'Position',[220 wnd_ofs+20 70 20],'Callback',@set_tc,...
	    'Tag','mhagui_levelmeter_tc125',...
	    'Value',(tc==125),...
	    'UserData',sCfg);
  uicontrol('Style','RadioButton','String','2 s',...
	    'Position',[300 wnd_ofs+20 70 20],'Callback',@set_tc,...
	    'Tag','mhagui_levelmeter_tc2k',...
	    'Value',(tc==2000),...
	    'UserData',sCfg);
  uicontrol('Style','RadioButton','String','10 s',...
	    'Position',[380 wnd_ofs+20 70 20],'Callback',@set_tc,...
	    'Tag','mhagui_levelmeter_tc10k',...
	    'Value',(tc==10000),...
	    'UserData',sCfg);
  if isfield(sCfg.base,'dc_simple')
    ui_shownumbers = ...
	uicontrol('Style','CheckBox','String','Show numbers',...
		  'Value',0,'Position',[10 10 260 20]);
    if ~isempty( cLTASS )
      ui_ltass = ...
	  uicontrol('Style','CheckBox','String','Display equivalent LTASS level',...
		    'Value',0,'Position',[10 30 260 20]);
    end
    freq = mha_get(sCfg.mha,[sCfg.base.dc_simple,'.cf']);
    freq_is_cf = 1;
    if isempty(freq)
      freq_is_cf = 0;
      Lin = mha_get(sCfg.mha,[sCfg.base.dc_simple,'.level']);
      freq = 1:(length(Lin)/2);
    end
  else
    freq = [];
  end
  nfreq = length(freq);
  axBB = axes('Tag','meteraxes_broadband','Unit','pixels',...
	      'Position',[120 wnd_ofs+100 330 180]);
  [l,lbb_in, lbb_out] = get_levels( sCfg, nfreq );
  nch.in = length(lbb_in);
  nch.out = length(lbb_out);
  nch.all = nch.in+nch.out;
  if isfield(sCfg.base, 'MHAIOJack')
    csPortsIn = mha_get(sCfg.mha,[sCfg.base.MHAIOJack,'.names_in']);
    csPortsOut = mha_get(sCfg.mha,[sCfg.base.MHAIOJack,'.names_out']);
  else
    csPortsIn = {};
    csPortsOut = {};
    for k=1:nch.in
      csPortsIn{k} = sprintf('in_%d',k);
    end
    for k=1:nch.out
      csPortsOut{k} = sprintf('out_%d',k);
    end
  end
  lw = 180/nch.all*0.75;
  hBB_in = [];
  hBB_out = [];
  hBBt_in = [];
  hBBt_out = [];
  hold off;
  col = ['b','r','g'];
  for k=1:nch.in
    kSide = 3;
    if ~isempty(strfind(csPortsIn{k},'left'))
      kSide = 1;
    end
    if ~isempty(strfind(csPortsIn{k},'right'))
      kSide = 2;
    end
    hBB_in(k) = plot([-100 lbb_in(k)],[k k],[col(kSide),'-'],'linewidth',lw);
    hold on;
    hBBt_in(k) = text(135,k,sprintf('%1.1f',lbb_in(k)),...
		      'HorizontalAlignment','right',...
		      'FontWeight','bold');
  end
  for k=1:nch.out
    kSide = 3;
    if ~isempty(strfind(csPortsOut{k},'left'))
      kSide = 1;
    end
    if ~isempty(strfind(csPortsOut{k},'right'))
      kSide = 2;
    end
    hBB_out(k) = plot([-100 lbb_out(k)],[k k]+length(lbb_in),[col(kSide),'-'],'linewidth',lw);
    hold on;
    hBBt_out(k) = text(135,k+length(lbb_in),sprintf('%1.1f',lbb_out(k)),...
		      'HorizontalAlignment','right',...
		      'FontWeight','bold');
  end
  plot([-20 140],[0.5 0.5]+length(lbb_in),'k-');
  %plot([-20 140],[4.5 4.5],'k-');
  %plot([-20 140],[7.5 7.5],'k-');
  hold off;
  set(axBB,'XLim',[-20 140],'YLim',[0 length(lbb_in)+length(lbb_out)]+0.5,...
	   'YTick',1:(nch.in+nch.out),'YDir','reverse',...
	   'XGrid','on',...
	   'YTickLabel',...
	   [csPortsIn,csPortsOut]);
  xlabel('broad band RMS level / dB SPL');
  if isfield(sCfg.base,'dc_simple')
    axLevel = axes('Tag','meteraxes_level','Unit','pixels',...
		   'Position',[60 220 400 100]);
    hold off;
    hLI = plot(freq,l(1,:),'b-o','linewidth',2);
    hold on;
    hRI = plot(freq,l(2,:),'r-o','linewidth',2);
    grid on
    set(axLevel,'YLim',[0 120]);
    ylabel('DC input level / dB');
    vHTextLevelL = [];
    vHTextLevelR = [];
    for f=freq
      vHTextLevelL(end+1) = text(f,108,'',...
				 'FontWeight','bold',...
				 'Color',[0 0 1],...
				 'HorizontalAlignment','center');
      vHTextLevelR(end+1) = text(f,88,'',...
				 'FontWeight','bold',...
				 'Color',[1 0 0],...
				 'HorizontalAlignment','center');
    end
    axGain = axes('Tag','meteraxes_gain','Unit','pixels',...
		  'Position',[60 90 400 100]);
    hold off;
    hLG = plot(freq,l(3,:)-l(1,:),'b-o','linewidth',2);
    hold on;
    hRG = plot(freq,l(4,:)-l(2,:),'r-o','linewidth',2);
    grid on
    set(axGain,'YLim',[-20 60]);
    if ~freq_is_cf
      idx = 1:length(freq);
      if length(freq)>10
	dx = round(length(freq)/10);
	idx = 1:dx:length(freq);
      end
      set([axLevel;axGain],...
	  'XScale','linear','XLim',[min(freq)-0.5 max(freq)+0.5],...
	  'XTick', freq(idx), 'XTickLabel',freq(idx));
      xlabel('Band number');
    else
      lgmm = round(log10([min(freq) max(freq)]));
      lgmm = [lgmm(1):(lgmm(2)-lgmm(1))/8:lgmm(2)];
      flab = 10.^(lgmm);
      flab = round(flab./(10.^(round(lgmm)-1))).*(10.^(round(lgmm)-1));
      set([axLevel;axGain],...
	  'XScale','log','XLim',[min(freq)/1.2 max(freq)*1.2],...
	  'XTick', flab, 'XTickLabel',flab);
      xlabel('Frequency / Hz');
    end
    ylabel('Gain / dB');
  end
  pause(0.8);
  while( ishandle(fh) )
    [l,lbb_in,lbb_out] = get_levels( sCfg, nfreq );
    if isfield(sCfg.base,'dc_simple')
      b_ltass = 0;
      b_showtext = get(ui_shownumbers,'Value');
      if ~isempty( cLTASS )
	b_ltass = get(ui_ltass,'Value');
      end
      cl = l([1 2],:);
      if b_ltass
	cl(1,:) = cl(1,:)-cLTASS;
	cl(2,:) = cl(2,:)-cLTASS;
      end
      set(hLI,'YData',cl(1,:));
      set(hRI,'YData',cl(2,:));
      set(hLG,'YData',l(3,:)-l(1,:));
      set(hRG,'YData',l(4,:)-l(2,:));
      for k=1:length(vHTextLevelL)
	if b_showtext
	  set(vHTextLevelL(k),'String',sprintf('%1.0f',cl(1,k)));
	  set(vHTextLevelR(k),'String',sprintf('%1.0f',cl(2,k)));
	else
	  set(vHTextLevelL(k),'String','');
	  set(vHTextLevelR(k),'String','');
	end
      end
    end
    for k=1:length(hBB_in)
      set(hBB_in(k),'XData',[-100 lbb_in(k)]);
      set(hBBt_in(k),'String',sprintf('%1.1f',lbb_in(k)));
    end    
    for k=1:length(hBB_out)
      set(hBB_out(k),'XData',[-100 lbb_out(k)]);
      set(hBBt_out(k),'String',sprintf('%1.1f',lbb_out(k)));
    end    
    drawnow;
    pause(0.8);
  end
  
function [l,lbb_in,lbb_out] = get_levels( sCfg, nfreq )
  if isfield(sCfg.base,'dc_simple')
    Lin = mha_get(sCfg.mha,[sCfg.base.dc_simple,'.level']);
    Gain = mha_get(sCfg.mha,[sCfg.base.dc_simple,'.gain']);
    Lout = Lin+Gain;
    nch = length(Lout)/nfreq;
    Lin = reshape(Lin,[nfreq nch]);
    Lout = reshape(Lout,[nfreq nch]);
    l = [Lin';Lout'];
  else
    l = [];
  end
  lbb_in = mha_get(sCfg.mha,[sCfg.base.transducers,'.calib_in.rmslevel']);
  lbb_out = mha_get(sCfg.mha,[sCfg.base.transducers,'.calib_out.rmslevel']);

function err = set_tc( varargin )
  tag = get(gcbo,'tag');
  cfg = get(gcbo,'UserData');
  switch tag
   case 'mhagui_levelmeter_tc10k'
    tc = 10000;
   case 'mhagui_levelmeter_tc2k'
    tc = 2000;
   case 'mhagui_levelmeter_tc125'
    tc = 125;
   otherwise
    tc = 1;
  end
  set(findobj('tag','mhagui_levelmeter_tc10k'),'Value',(tc==10000));
  set(findobj('tag','mhagui_levelmeter_tc2k'),'Value',(tc==2000));
  set(findobj('tag','mhagui_levelmeter_tc125'),'Value',(tc==125));
  mha_set(cfg.mha,[cfg.base.transducers,'.calib_in.tau_level'],0.001*tc);
  mha_set(cfg.mha,[cfg.base.transducers,'.calib_out.tau_level'],0.001*tc);
