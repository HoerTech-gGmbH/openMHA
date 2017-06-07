function mpoconfig( mha )
  if nargin < 1
    mha = [];
  end
  mha = mha_ensure_mhahandle( mha );
  global mha_basic_cfg;
  libmhagui();
  mha_get_basic_cfg_network( mha );
  idx = strmatch('dc_afterburn',mha_basic_cfg.all_id_plugs(:,2));
  close(findobj('tag','mpoconfig'));
  fh = mhagui.figure('MPO configuration','mpoconfig',[600,320+100*size(idx,1)]);
  ax = axes('Unit','Pixel','OuterPosition',[0,0,600,320],...
	    'XLim',[50,20000],'XScale','log','XTick',round(1000*2.^[-4:4]),...
	    'NextPlot','add');
  grid on;
  x=80;
  for dmpo=[-10,-5,-1,1,5,10]
    sl = sprintf('%+d dB',dmpo);
    if dmpo > 0
      vcol = [0.7+0.02*dmpo 0.7-0.03*dmpo 0.7-0.03*dmpo];
    else
      vcol = [0.7+0.03*dmpo 0.7-0.02*dmpo 0.7+0.03*dmpo];
    end
    for algo = [1:size(idx,1)]
      uicontrol('Style','PushButton','String',sl,...
                'Position',[x,320+(algo-1)*100,60,60],'UserData',[dmpo,size(idx,1)-algo+1],...
                'Callback',@update_mpo,'BackgroundColor',vcol);
      if (x == 80)
          uicontrol('Style','Text','String',mha_basic_cfg.all_id_plugs{idx(size(idx,1)-algo+1),1},...
              'Position',[5,320+60+(algo-1)*100,590,20]);
      end
    end
    x = x + 80;
  end
  global mpoconfig_data;
  mpoconfig_data = struct;
  mpoconfig_data.ph = plot(0,0*idx);
  mpoconfig_data.th = [];
  for k=1:numel(mpoconfig_data.ph)
    mpoconfig_data.th(k) = text(60,0,'','FontSize',22,...
				'Color',get(mpoconfig_data.ph(k),'Color'),...
				'FontWeight','bold',...
				'VerticalAlignment','bottom');
  end
  mpo_inc(0);
%  maxgainconfig(mha);
  
function e = update_mpo( varargin )
  e = 0;
  userdata = get(gcbo,'UserData');
  mpo_inc(userdata(1),userdata(2));
  
  
function mpo_inc( dmpo, algo )
  global mha_basic_cfg;
  global mpoconfig_data;
  idx = strmatch('dc_afterburn',mha_basic_cfg.all_id_plugs(:,2));
  if nargin < 2
      algo = [1:size(idx)];
  end
  csAddr =mha_basic_cfg.all_id_plugs(idx(algo),1);
  for k=1:numel(algo)
    mpo = mha_get(mha_basic_cfg.mha,...
		      [csAddr{k},'.mpo']);
    vf = mha_get(mha_basic_cfg.mha,...
		 [csAddr{k},'.f']);
    mpo = mpo + dmpo;
    mha_set(mha_basic_cfg.mha,[csAddr{k},'.mpo'],mpo);
    mha_set(mha_basic_cfg.mha,[csAddr{k},'.commit'],'commit');
    
    set(mpoconfig_data.ph(algo(k)),'XData',vf,'YData',mpo);
    med_mpo = median(mpo);
    set(mpoconfig_data.th(algo(k)),'Position',[100,med_mpo,0],...
		      'String',sprintf('%1.1f dB',med_mpo));
  end
		      
function maxgainconfig( mha )
  if nargin < 1
    mha = [];
  end
  mha = mha_ensure_mhahandle( mha );
  global mha_basic_cfg;
  mha_get_basic_cfg_network( mha );
  close(findobj('tag','maxgainconfig'));
  libmhagui();
  fh = mhagui.figure('Maxgain configuration','maxgainconfig',[600,400]);
  ax = axes('Unit','Pixel','OuterPosition',[0,0,600,320],...
	    'XLim',[50,20000],'XScale','log','XTick',round(1000*2.^[-4:4]),...
	    'NextPlot','add');
  grid on;
  x=80;
  for dmaxgain=[-10,-5,-1,1,5,10]
    if dmaxgain < 0
      sl = sprintf('%d dB',dmaxgain);
    else
      sl = sprintf('+%d dB',dmaxgain);
    end
    if dmaxgain > 0
      vcol = [0.7+0.02*dmaxgain 0.7-0.03*dmaxgain 0.7-0.03*dmaxgain];
    else
      vcol = [0.7+0.03*dmaxgain 0.7-0.02*dmaxgain 0.7+0.03*dmaxgain];
    end
    uicontrol('Style','PushButton','String',sl,...
	      'Position',[x,320,60,60],'UserData',dmaxgain,...
	      'Callback',@update_maxgain,'BackgroundColor',vcol);
    x = x + 80;
  end
  global maxgainconfig_data;
  maxgainconfig_data = struct;
  idx = strmatch('dc_afterburn',mha_basic_cfg.all_id_plugs(:,2));
  maxgainconfig_data.ph = plot(0,0*idx);
  maxgainconfig_data.th = [];
  for k=1:numel(maxgainconfig_data.ph)
    maxgainconfig_data.th(k) = text(60,0,'','FontSize',22,...
				'Color',get(maxgainconfig_data.ph(k),'Color'),...
				'FontWeight','bold',...
				'VerticalAlignment','bottom');
  end
  maxgain_inc(0);
  
function e = update_maxgain( varargin )
  e = 0;
  maxgain_inc(get(gcbo,'UserData'));
  
  
function maxgain_inc( dmaxgain )
  global mha_basic_cfg;
  global maxgainconfig_data;
  idx = strmatch('dc_afterburn',mha_basic_cfg.all_id_plugs(:,2));
  csAddr =mha_basic_cfg.all_id_plugs(idx,1);
  for k=1:numel(csAddr)
    maxgain = mha_get(mha_basic_cfg.mha,...
		      [csAddr{k},'.maxgain']);
    vf = mha_get(mha_basic_cfg.mha,...
		 [csAddr{k},'.f']);
    maxgain = maxgain + dmaxgain;
    mha_set(mha_basic_cfg.mha,[csAddr{k},'.maxgain'],maxgain);
    mha_set(mha_basic_cfg.mha,[csAddr{k},'.commit'],'commit');
    set(maxgainconfig_data.ph(k),'XData',vf,'YData',maxgain);
    med_maxgain = median(maxgain);
    set(maxgainconfig_data.th(k),'Position',[100,med_maxgain,0],...
		      'String',sprintf('%1.1f dB',med_maxgain));
  end
		      
