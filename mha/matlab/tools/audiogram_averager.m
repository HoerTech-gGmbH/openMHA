function sAud = audiogram_averager( client_id, sAud )
  libaudprof();
  libmhagui();
  sCfg = struct;
  sCfg.client = client_id;
  sCfg.audid = sAud.id;
  sCfg.avgaud = [];
  sCfg.avgaudfinal = [];
  sCfg.clients = audprof.db_load();
  client_idx = strmatch(client_id, ...
			sCfg.clients(1,:),'exact');
  if isempty(client_idx)
    client_idx = 1;
  end
  sCfg.auds = cell(2,0);
  p = get(0,'ScreenSize');
  s = [800,600];
  p = round([p(3:4)/2-s/2,s]);
  fh = mhagui.figure('Audiogram averager','audiogram_averager',[800,600]);
  sCfg.fh = fh;
  add_label([20,560,160,20],'Client:');
  uicontrol(fh,'Style','Listbox',...
	    'Position',[20 180 160 380],...
	    'Callback',@cb_update_data,...
	    'String',audprof.db_prettyclientlist(sCfg.clients),...
	    'value',client_idx,...
	    'tag','audavg:client_list');
  add_label([190,560,220,20],'Audiogram:');
  uicontrol(fh,'Style','listbox',...
	    'Position',[190 360 220 200],...
	    'tag','audavg:aud_list',...
	    'Callback',@cb_plot_selaud);
  add_label([470,560,310,20],'Averaged audiograms:');
  uicontrol(fh,'Style','listbox',...
	    'Position',[470 360 310 200],...
	    'String',{},...
	    'tag','audavg:avg_selection',...
	    'Callback',@cb_plot_avgaud);
  uicontrol(fh,'Style','pushbutton',...
	    'Position',[420,530,40,30],...
	    'String','>>',...
	    'callback',@cb_add_aud_to_list);
  uicontrol(fh,'Style','pushbutton',...
	    'Position',[420,490,40,30],...
	    'String','<<',...
	    'callback',@cb_rm_aud_from_list);
  add_label([20,130,390,20],'New audiogram ID:');
  uicontrol(fh,'Style','edit','String',sprintf('average %s',datestr(now)),...
	    'Position',[20,100,390,30],'tag','audavg:audidedit');
  axes('Units','pixel','Position',[490,110,290,240],...
       'NextPlot','add',...
       'YDir','reverse','XScale','log','Xlim',[100,16000],...
       'YLim',[-10,110],...
       'XTick',1000*2.^[-5:5],'XTickLabel',2.^[-5:5],...
       'tag','audavg:audaxes','XGrid','on','YGrid','on',...
       'Box','on');
  axes('Units','pixel','Position',[210,201,200,148],...
       'FontSize',7,...
       'NextPlot','add',...
       'YDir','reverse','XScale','log','Xlim',[100,16000],...
       'YLim',[-10,110],...
       'XTick',1000*2.^[-5:5],'XTickLabel',2.^[-5:5],...
       'tag','audavg:selaudaxes','XGrid','on','YGrid','on',...
       'Box','on');
  set(findobj(fh,'style','listbox'),'BackgroundColor',ones(1,3));
  set(findobj(fh,'style','edit'),'BackgroundColor',ones(1,3),'HorizontalAlignment','left');
  set(fh,'UserData',sCfg);
  update_data(fh);
  sCfg = mhagui.waitfor(fh);
  sAud = [];
  if ~isempty(sCfg)
    sAud = sCfg.avgaud;
  end
  
function cb_update_data( varargin )
  update_data(gcbf);
  
function update_data( fh )
  try
    sCfg = get(fh,'UserData');
    libaudprof();
    h_client_list = findobj(sCfg.fh,'tag','audavg:client_list');
    h_aud_list = findobj(sCfg.fh,'tag','audavg:aud_list');
    h_sel_list = findobj(sCfg.fh,'tag','audavg:avg_selection');
    h_ed_id = findobj(sCfg.fh,'tag','audavg:audidedit');
    sClient = sCfg.clients{1, ...
		    get(h_client_list,'Value')};
    cAuds = audprof.audprof_getall(sClient);
    ncAuds = cell(2,0);
    for k=1:size(cAuds,2)
      if isempty(strmatch(sprintf('%s: %s',sClient,cAuds{1,k}),...
			  sCfg.auds(1,:),'exact'))
	ncAuds(:,end+1) = cAuds(:,k);
      end
    end
    cAuds = ncAuds;
    set(h_aud_list,'String',cAuds(1,:),'Value',1);
    sCfg.client_auds = cAuds;
    set(h_sel_list,'String',sCfg.auds(1,:));
    sCfg.avgaud = audprof.audprof_average(sCfg.auds(2,:));
    sCfg.avgaud.id = get(h_ed_id,'String');
    sCfg.avgaud.client_id = sCfg.client;
    set(fh,'UserData',sCfg);
    plot_avgaud(fh);
    plot_selaud(fh);
  catch
    disp_err_rethrow();
  end
  
function cb_plot_avgaud(varargin)
  plot_avgaud( gcbf );
  
function plot_avgaud( fh )
  try
    libmhagui();
    libaudprof();
    sCfg = get(fh,'UserData');
    ax = findobj(sCfg.fh,'tag','audavg:audaxes');
    mhagui.audprof_plot(sCfg.avgaud,ax);
    hold on;
    %delete(get(ax,'Children'));
    Nauds = size(sCfg.auds,2);
    col = struct;
    col.l = [0.4,0.4,0.8];
    col.r = [0.8,0.4,0.4];
    h_sel_list = findobj(sCfg.fh,'tag','audavg:avg_selection');  
    kSel = get(h_sel_list,'Value');
    if Nauds
      for k=1:Nauds
	aud_k = sCfg.auds{2,k};
	vp = [];
	for side='lr'
	  for ctype={'htl_ac','htl_bc','ucl'}
	    th = audprof.threshold_get(aud_k,side,ctype{:});
	    if ~isempty(th.data)
	      vp(end+1) = plot(ax,[th.data.f],[th.data.hl],'k.-', ...
			       'Color',col.(side));
	    end
	  end
	end
	if (k==kSel) && ~isempty(vp)
	  set(vp,'MarkerSize',10,'linestyle','-','linewidth',2);
	end
      end
    end
  catch
    disp_err_rethrow();
  end
  
  
function cb_plot_selaud(varargin)
  plot_selaud( gcbf );
  
function plot_selaud(fh)
  libaudprof();
  sCfg = get(fh,'UserData');
  ax = findobj(sCfg.fh,...
	       'tag','audavg:selaudaxes');
  delete(get(ax,'Children'));
  h_client_list = findobj(sCfg.fh,'tag','audavg:client_list');
  h_aud_list = findobj(sCfg.fh,'tag','audavg:aud_list');
  sClient = sCfg.clients{1, ...
		    get(h_client_list,'Value')};
  csAudIDs = get(h_aud_list,'String');
  if ~isempty(csAudIDs)
    sAudID = csAudIDs{get(h_aud_list,'Value')};
    aud = audprof.audprof_get(sClient,sAudID);
    libmhagui();
    mhagui.audprof_plot(aud,ax);
  end

function cb_add_aud_to_list( varargin )
  libconfigdb();
  sCfg = get(gcbf,'UserData');
  h_client_list = findobj(sCfg.fh,'tag','audavg:client_list');
  h_aud_list = findobj(sCfg.fh,'tag','audavg:aud_list');
  sClient = sCfg.clients{1,get(h_client_list, ...
						  'Value')};
  audidx = get(h_aud_list,'Value');
  sAudID = sCfg.client_auds{1,audidx};
  sAudID = sprintf('%s: %s',sClient,sAudID);
  sAud = sCfg.client_auds{2,audidx};
  sAud.id = sAudID;
  sCfg.auds = ...
      configdb.smap_set( sCfg.auds, sAudID, sAud);
  set(gcbf,'UserData',sCfg);
  update_data(gcbf);

function cb_rm_aud_from_list( varargin )
  sCfg = get(gcbf,'UserData');
  h_aud_sel = findobj(sCfg.fh,'tag','audavg:avg_selection');
  nAuds = size(sCfg.auds,2);
  idx0 = get(h_aud_sel,'Value');
  idx = setdiff(1:nAuds,idx0);
  sCfg.auds = sCfg.auds(:,idx);
  if idx0 > length(idx)
    set(h_aud_sel,'Value',max(1,length(idx)));
  end
  update_data(gcbf);
  
function add_label(pos,s)
  uicontrol('style','text','Position',pos,'String',s,...
	    'HorizontalAlignment','left',...
	    'FontWeight','bold',...
	    'BackgroundColor',get(gcf,'Color'));
  
  